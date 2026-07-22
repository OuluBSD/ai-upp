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

int AmpOtsuThreshold(const Vector<int>& gray)
{
	if(gray.IsEmpty())
		return 0;
	int histogram[256] = {};
	int total = gray.GetCount();
	int sum = 0;
	for(int value : gray) {
		value = clamp(value, 0, 255);
		histogram[value]++;
		sum += value;
	}
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
