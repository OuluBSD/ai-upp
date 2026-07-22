#ifndef _AMP_AMPTemplatePixels_h_
#define _AMP_AMPTemplatePixels_h_

enum AmpTemplatePreprocessing {
	AMP_TEMPLATE_RGB,
	AMP_TEMPLATE_GRAY,
	AMP_TEMPLATE_OTSU,
};

struct AmpTemplatePixelBuffer : Moveable<AmpTemplatePixelBuffer> {
	int width = 0;
	int height = 0;
	Vector<int> rgb;
	Vector<int> gray;
	Vector<int> otsu;
	Vector<int> local_otsu;
	int otsu_threshold = 0;

	bool IsValid() const;
};

int PackAmpRgb(const RGBA& pixel);
byte AmpRgbRed(int pixel);
byte AmpRgbGreen(int pixel);
byte AmpRgbBlue(int pixel);
byte AmpRgbGray(int pixel);
int AmpOtsuThreshold(const Vector<int>& gray);
int AmpOtsuThresholdCrop(const Vector<int>& gray, int stride, int x, int y,
                         int width, int height);
int AmpLocalOtsuThreshold(const Vector<int>& gray, int stride, int height,
                          int x, int y, int radius, bool gaussian);
bool BuildAmpLocalOtsu(const Vector<int>& gray, int width, int height,
                       int radius, bool gaussian, Vector<int>& output,
                       String& error);

String AmpTemplatePreprocessingName(AmpTemplatePreprocessing mode);
bool ParseAmpTemplatePreprocessing(const String& name,
	                               AmpTemplatePreprocessing& mode,
	                               String& error);
bool BuildAmpPixelBuffer(const Image& image, AmpTemplatePixelBuffer& pixels,
	                     String& error);

#endif
