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
	int otsu_threshold = 0;

	bool IsValid() const;
};

int PackAmpRgb(const RGBA& pixel);
byte AmpRgbRed(int pixel);
byte AmpRgbGreen(int pixel);
byte AmpRgbBlue(int pixel);
byte AmpRgbGray(int pixel);
int AmpOtsuThreshold(const Vector<int>& gray);

String AmpTemplatePreprocessingName(AmpTemplatePreprocessing mode);
bool ParseAmpTemplatePreprocessing(const String& name,
	                               AmpTemplatePreprocessing& mode,
	                               String& error);
bool BuildAmpPixelBuffer(const Image& image, AmpTemplatePixelBuffer& pixels,
	                     String& error);

#endif
