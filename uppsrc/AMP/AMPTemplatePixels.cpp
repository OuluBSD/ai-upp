#include "AMP.h"

NAMESPACE_UPP

bool AmpTemplatePixelBuffer::IsValid() const
{
	return width > 0 && height > 0 &&
	       rgb.GetCount() == width * height &&
 	      gray.GetCount() == width * height &&
 	      otsu.GetCount() == width * height &&
 	      otsu_threshold >= 0 && otsu_threshold <= 255;
}

int PackAmpRgb(const RGBA& pixel)
{
	return (int)pixel.r | ((int)pixel.g << 8) | ((int)pixel.b << 16);
}

byte AmpRgbRed(int pixel)
{
	return (byte)(pixel & 255);
}

byte AmpRgbGreen(int pixel)
{
	return (byte)((pixel >> 8) & 255);
}

byte AmpRgbBlue(int pixel)
{
	return (byte)((pixel >> 16) & 255);
}

byte AmpRgbGray(int pixel)
{
	return (byte)((AmpRgbRed(pixel) * 77 + AmpRgbGreen(pixel) * 150 +
	               AmpRgbBlue(pixel) * 29) >> 8);
}

static int OtsuThresholdHistogram(const int* histogram, int total, int sum)
{
	if(total <= 0)
		return 0;
	int background_count = 0;
	int background_sum = 0;
	double best = -1;
	int threshold = 0;
	for(int candidate = 0; candidate < 256; candidate++) {
		background_count += histogram[candidate];
		background_sum += candidate * histogram[candidate];
		if(background_count == 0)
			continue;
		int foreground_count = total - background_count;
		if(foreground_count == 0)
			break;
		double background_mean = (double)background_sum / background_count;
		double foreground_mean = (double)(sum - background_sum) / foreground_count;
		double between = (double)background_count * foreground_count *
		                 (background_mean - foreground_mean) *
		                 (background_mean - foreground_mean);
		if(between > best) {
			best = between;
			threshold = candidate;
		}
	}
	return threshold;
}

int AmpOtsuThreshold(const Vector<int>& gray)
{
	if(gray.IsEmpty())
		return 0;
	int histogram[256] = {};
	int sum = 0;
	for(int value : gray) {
		value = clamp(value, 0, 255);
		histogram[value]++;
		sum += value;
	}
	return OtsuThresholdHistogram(histogram, gray.GetCount(), sum);
}

int AmpOtsuThresholdCrop(const Vector<int>& gray, int stride, int x, int y,
                         int width, int height)
{
	if(stride <= 0 || width <= 0 || height <= 0 || x < 0 || y < 0 ||
	   x + width > stride)
		return 0;
	int histogram[256] = {};
	int total = 0;
	int sum = 0;
	for(int row = 0; row < height; row++)
		for(int col = 0; col < width; col++) {
			int index = (y + row) * stride + x + col;
			if(index < 0 || index >= gray.GetCount())
				return 0;
			int value = clamp(gray[index], 0, 255);
			histogram[value]++;
			total++;
			sum += value;
		}
	return OtsuThresholdHistogram(histogram, total, sum);
}

int AmpLocalOtsuThreshold(const Vector<int>& gray, int stride, int height,
                          int x, int y, int radius, bool gaussian)
{
	if(stride <= 0 || radius < 0 || x < 0 || y < 0 ||
	   height <= 0 || x >= stride || y >= height || y * stride + x >= gray.GetCount())
		return 0;
	int histogram[256] = {};
	int total = 0;
	int sum = 0;
	int side = radius * 2 + 1;
	for(int yy = -radius; yy <= radius; yy++)
		for(int xx = -radius; xx <= radius; xx++) {
			int sample_x = clamp(x + xx, 0, stride - 1);
			int sample_y = clamp(y + yy, 0, height - 1);
			int index = sample_y * stride + sample_x;
			if(index < 0 || index >= gray.GetCount())
				continue;
			int distance = abs(xx) + abs(yy);
			int weight = gaussian ? max(1, side * side - distance * distance)
			                     : max(1, side - distance);
			int value = clamp(gray[index], 0, 255);
			histogram[value] += weight;
			total += weight;
			sum += value * weight;
		}
	return OtsuThresholdHistogram(histogram, total, sum);
}

bool BuildAmpLocalOtsu(const Vector<int>& gray, int width, int height,
                       int radius, bool gaussian, Vector<int>& output,
                       String& error)
{
	if(width <= 0 || height <= 0 || radius < 0 ||
	   gray.GetCount() != width * height) {
		error = "invalid grayscale frame for local Otsu";
		return false;
	}
	output.SetCount(gray.GetCount());
	for(int y = 0; y < height; y++)
		for(int x = 0; x < width; x++) {
			int threshold = AmpLocalOtsuThreshold(gray, width, height, x, y, radius, gaussian);
			output[y * width + x] = gray[y * width + x] > threshold ? 255 : 0;
		}
	return true;
}

String AmpTemplatePreprocessingName(AmpTemplatePreprocessing mode)
{
	switch(mode) {
	case AMP_TEMPLATE_RGB:  return "rgb";
	case AMP_TEMPLATE_GRAY: return "gray";
	case AMP_TEMPLATE_OTSU: return "otsu";
	}
	return String();
}

bool ParseAmpTemplatePreprocessing(const String& name,
	                               AmpTemplatePreprocessing& mode,
	                               String& error)
{
	String value = ToLower(TrimBoth(name));
	if(value == "rgb" || value == "color") {
		mode = AMP_TEMPLATE_RGB;
		return true;
	}
	if(value == "gray" || value == "grayscale") {
		mode = AMP_TEMPLATE_GRAY;
		return true;
	}
	if(value == "otsu") {
		mode = AMP_TEMPLATE_OTSU;
		return true;
	}
	error = Format("unknown template preprocessing: %s", name);
	return false;
}

bool BuildAmpPixelBuffer(const Image& image, AmpTemplatePixelBuffer& pixels,
	                     String& error)
{
	if(image.IsEmpty()) {
		error = "frame image is empty";
		return false;
	}
	pixels = AmpTemplatePixelBuffer();
	pixels.width = image.GetWidth();
	pixels.height = image.GetHeight();
	pixels.rgb.SetCount(pixels.width * pixels.height);
	pixels.gray.SetCount(pixels.width * pixels.height);
	for(int y = 0; y < pixels.height; y++) {
		const RGBA* row = image[y];
		for(int x = 0; x < pixels.width; x++) {
			int index = y * pixels.width + x;
			pixels.rgb[index] = PackAmpRgb(row[x]);
			pixels.gray[index] = AmpRgbGray(pixels.rgb[index]);
		}
	}
	pixels.otsu_threshold = AmpOtsuThreshold(pixels.gray);
	pixels.otsu.SetCount(pixels.gray.GetCount());
	for(int i = 0; i < pixels.gray.GetCount(); i++)
		pixels.otsu[i] = pixels.gray[i] > pixels.otsu_threshold ? 255 : 0;
	return true;
}

END_UPP_NAMESPACE
