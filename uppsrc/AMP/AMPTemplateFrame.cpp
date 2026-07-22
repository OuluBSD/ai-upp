#include "AMP.h"

NAMESPACE_UPP

static byte FrameGray(const RGBA& pixel)
{
	return (byte)((pixel.r * 77 + pixel.g * 150 + pixel.b * 29) >> 8);
}

bool BuildAmpGrayFrame(const Image& image, Vector<int>& frame, String& error)
{
	if(image.IsEmpty()) {
		error = "frame image is empty";
		return false;
	}
	frame.SetCount(image.GetWidth() * image.GetHeight());
	for(int y = 0; y < image.GetHeight(); y++) {
		const RGBA* row = image[y];
		for(int x = 0; x < image.GetWidth(); x++)
			frame[y * image.GetWidth() + x] = FrameGray(row[x]);
	}
	return true;
}

bool MatchAmpImageCpu(const Image& image, const Vector<int>& atlas,
                      const AmpTemplateAtlasManifest& manifest, int threshold,
                      AmpTemplateMatchResult& result, String& error)
{
	Vector<int> frame;
	if(!BuildAmpGrayFrame(image, frame, error))
		return false;
	return MatchAmpTemplatesCpu(frame, image.GetWidth(), image.GetHeight(), atlas,
	                            manifest, threshold, result, error);
}

bool MatchAmpImageAmp(const Image& image, const Vector<int>& atlas,
                      const AmpTemplateAtlasManifest& manifest, int threshold,
                      const String& device_path, AmpTemplateMatchResult& result,
                      String& error)
{
	Vector<int> frame;
	if(!BuildAmpGrayFrame(image, frame, error))
		return false;
	return MatchAmpTemplatesAmp(frame, image.GetWidth(), image.GetHeight(), atlas,
	                            manifest, threshold, device_path, result, error);
}

END_UPP_NAMESPACE
