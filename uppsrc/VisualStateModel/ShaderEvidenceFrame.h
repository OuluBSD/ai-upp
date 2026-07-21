#ifndef _VisualStateModel_ShaderEvidenceFrame_h_
#define _VisualStateModel_ShaderEvidenceFrame_h_

namespace Upp {

struct VsmShaderEvidenceFrameConfig {
	String video;
	String manifest;
	String crop_map;
	String required_fixture_kind;
	String required_fixture_id;
	int frame_second = 0;

	void Jsonize(JsonIO& json) {
		json("video", video)("manifest", manifest)("crop_map", crop_map)
			("required_fixture_kind", required_fixture_kind)
			("required_fixture_id", required_fixture_id)("frame_second", frame_second);
	}
};

int VsmRunShaderEvidenceFrame(const String& video_path, const String& manifest_path,
	                           const String& crop_map_path, int frame_second);
int VsmRunShaderEvidenceFrameConfig(const String& config_path);

}

#endif
