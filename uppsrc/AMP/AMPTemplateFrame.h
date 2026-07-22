#ifndef _AMP_AMPTemplateFrame_h_
#define _AMP_AMPTemplateFrame_h_

bool BuildAmpGrayFrame(const Image& image, Vector<int>& frame, String& error);

bool MatchAmpImageCpu(const Image& image, const Vector<int>& atlas,
                      const AmpTemplateAtlasManifest& manifest, int threshold,
                      AmpTemplateMatchResult& result, String& error);

bool MatchAmpImageAmp(const Image& image, const Vector<int>& atlas,
                      const AmpTemplateAtlasManifest& manifest, int threshold,
                      const String& device_path, AmpTemplateMatchResult& result,
                      String& error);

bool MatchAmpImagesCpu(const Image& frame, const Image& atlas,
                       const AmpTemplateAtlasManifest& manifest, int threshold,
                       AmpTemplateMatchResult& result, String& error);

bool MatchAmpImagesAmp(const Image& frame, const Image& atlas,
                       const AmpTemplateAtlasManifest& manifest, int threshold,
                       const String& device_path, AmpTemplateMatchResult& result,
                       String& error);

#endif
