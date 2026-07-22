#include "AMP.h"

NAMESPACE_UPP

bool BuildAmpGrayFrame(const Image& image, Vector<int>& frame, String& error)
{
	AmpTemplatePixelBuffer pixels;
	if(!BuildAmpPixelBuffer(image, pixels, error))
		return false;
	frame = pick(pixels.gray);
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

bool MatchAmpImagesCpu(const Image& frame, const Image& atlas,
                       const AmpTemplateAtlasManifest& manifest, int threshold,
                       AmpTemplateMatchResult& result, String& error)
{
	AmpTemplatePixelBuffer frame_pixels, atlas_pixels;
	if(!BuildAmpPixelBuffer(frame, frame_pixels, error) ||
	   !BuildAmpPixelBuffer(atlas, atlas_pixels, error))
		return false;
	return MatchAmpTemplatePixelsCpu(frame_pixels, atlas_pixels, manifest,
	                                threshold, result, error);
}

bool MatchAmpImagesAmp(const Image& frame, const Image& atlas,
                       const AmpTemplateAtlasManifest& manifest, int threshold,
                       const String& device_path, AmpTemplateMatchResult& result,
                       String& error)
{
	AmpTemplatePixelBuffer frame_pixels, atlas_pixels;
	if(!BuildAmpPixelBuffer(frame, frame_pixels, error) ||
	   !BuildAmpPixelBuffer(atlas, atlas_pixels, error))
		return false;
	return MatchAmpTemplatePixelsAmp(frame_pixels, atlas_pixels, manifest,
	                                threshold, device_path, result, error);
}

END_UPP_NAMESPACE
