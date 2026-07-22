#include "VisualStateModel.h"

namespace Upp {

static int PackVsmRgb(byte r, byte g, byte b)
{
	return (int)r | ((int)g << 8) | ((int)b << 16);
}

static bool BuildAmpPixels(const VsmImageBuffer& input,
                           AmpTemplatePixelBuffer& output, String& error)
{
	if(input.IsEmpty() || (input.channels != 1 && input.channels < 3)) {
		error = "VSM image must be non-empty grayscale or RGB";
		return false;
	}
	output = AmpTemplatePixelBuffer();
	output.width = input.width;
	output.height = input.height;
	int count = input.width * input.height;
	output.rgb.SetCount(count);
	output.gray.SetCount(count);
	for(int y = 0; y < input.height; y++)
		for(int x = 0; x < input.width; x++) {
			int i = y * input.width + x;
			byte r = input.Get(x, y, 0);
			byte g = input.channels == 1 ? r : input.Get(x, y, 1);
			byte b = input.channels == 1 ? r : input.Get(x, y, 2);
			output.rgb[i] = PackVsmRgb(r, g, b);
			output.gray[i] = (r * 77 + g * 150 + b * 29) >> 8;
		}
	output.otsu_threshold = AmpOtsuThreshold(output.gray);
	output.otsu.SetCount(count);
	for(int i = 0; i < count; i++)
		output.otsu[i] = output.gray[i] > output.otsu_threshold ? 255 : 0;
	return true;
}

static bool BuildAmpAtlas(const VsmImageBuffer& source,
                          const VsmShaderTemplateManifest& manifest,
                          AmpTemplatePixelBuffer& output, String& error)
{
	VsmImageBuffer atlas;
	atlas.Create(source.width, source.height, source.channels);
	atlas.pixels <<= source.pixels;
	for(const VsmShaderTemplate& templ : manifest.templates) {
		if(!templ.polarity) continue;
		Rect area = templ.GetForegroundRect();
		if(area.IsEmpty()) area = Rect(0, 0, templ.w, templ.h);
		for(int y = area.top; y < area.bottom; y++)
			for(int x = area.left; x < area.right; x++)
				for(int c = 0; c < atlas.channels; c++)
					atlas.Set(templ.x + x, templ.y + y,
					          255 - atlas.Get(templ.x + x, templ.y + y, c), c);
	}
	return BuildAmpPixels(atlas, output, error);
}

static bool BuildAmpManifest(const VsmShaderTemplateManifest& source,
                             AmpTemplateAtlasManifest& output, String& error)
{
	output = AmpTemplateAtlasManifest();
	output.atlas_name = "vsm-crop-map";
	output.atlas_width = source.crop_map_width;
	output.atlas_height = source.crop_map_height;
	for(const VsmShaderTemplate& templ : source.templates) {
		AmpTemplateAtlasEntry& entry = output.entries.Add();
		entry.id = templ.id;
		entry.kind = templ.label;
		entry.x = templ.x;
		entry.y = templ.y;
		entry.width = templ.w;
		entry.height = templ.h;
		entry.preprocessing = "gray";
	}
	return output.Validate(error);
}

static void ConvertEvidence(const AmpTemplateMatchResult& source,
                            const AmpTemplateAtlasManifest& amp_manifest,
                            const VsmShaderTemplateManifest& manifest,
                            VsmShaderEvidence& output)
{
	output = VsmShaderEvidence();
	output.image.Create(source.evidence_width, source.evidence_height, 3);
	for(int y = 0; y < source.evidence_height; y++)
		for(int x = 0; x < source.evidence_width; x++) {
			int packed = source.evidence_rgb[y * source.evidence_width + x];
			output.image.Set(x, y, (byte)(packed & 255), 0);
			output.image.Set(x, y, (byte)((packed >> 8) & 255), 1);
			output.image.Set(x, y, (byte)((packed >> 16) & 255), 2);
		}
	output.best_hits.SetCount(manifest.templates.GetCount());
	output.best_scores.SetCount(manifest.templates.GetCount(), 0.0);
	output.mean_scores.SetCount(manifest.templates.GetCount(), 0.0);
	for(int i = 0; i < source.entries.GetCount() && i < manifest.templates.GetCount(); i++) {
		const AmpTemplateMatchHit& hit = source.entries[i];
		VsmShaderMatchHit& converted = output.best_hits[i];
		converted.template_index = i;
		converted.x = hit.x;
		converted.y = hit.y;
		int area = max(1, amp_manifest.entries[i].width * amp_manifest.entries[i].height);
		converted.score = 1.0 - min(1.0, (double)hit.score / (area * 255.0));
		output.best_scores[i] = converted.score;
	}
}

bool VsmAmpEvidenceAdapter::Process(const VsmImageBuffer& frame,
                                    const VsmImageBuffer& crop_map,
                                    const VsmShaderTemplateManifest& manifest,
                                    int threshold,
                                    const String& backend,
                                    const String& device_path,
                                    VsmShaderEvidence& evidence,
                                    String& error)
{
	if(crop_map.width != manifest.crop_map_width || crop_map.height != manifest.crop_map_height) {
		error = "AMP adapter crop-map dimensions do not match manifest";
		return false;
	}
	AmpTemplatePixelBuffer amp_frame, amp_atlas;
	AmpTemplateAtlasManifest amp_manifest;
	if(!BuildAmpPixels(frame, amp_frame, error) ||
	   !BuildAmpAtlas(crop_map, manifest, amp_atlas, error) ||
	   !BuildAmpManifest(manifest, amp_manifest, error))
		return false;
	AmpTemplateMatchResult result;
	bool native = backend == "native-amp" || backend == "native-amp-kernel";
	bool ok = native
		? MatchAmpTemplatePixelsAmp(amp_frame, amp_atlas, amp_manifest,
		                            threshold, device_path, result, error)
		: MatchAmpTemplatePixelsCpu(amp_frame, amp_atlas, amp_manifest,
		                            threshold, result, error);
	if(!ok) return false;
	ConvertEvidence(result, amp_manifest, manifest, evidence);
	return true;
}

}
