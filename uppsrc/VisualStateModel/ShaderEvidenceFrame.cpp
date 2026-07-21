#include "VisualStateModel.h"

namespace Upp {

static const Rect kVsmLeftTableRect  = RectC(8, 1, 944, 682);
static const Rect kVsmRightTableRect = RectC(968, 1, 944, 682);

static VsmImageBuffer CropVsmBuffer(const VsmImageBuffer& source, const Rect& requested)
{
	Rect bounds(0, 0, source.width, source.height);
	Rect r = requested & bounds;
	if(r.IsEmpty())
		return VsmImageBuffer();
	VsmImageBuffer result;
	result.Create(r.Width(), r.Height(), source.channels);
	for(int y = 0; y < r.Height(); y++)
		for(int x = 0; x < r.Width(); x++)
			for(int c = 0; c < source.channels; c++)
				result.Set(x, y, source.Get(r.left + x, r.top + y, c), c);
	return result;
}

static int RunVsmShaderEvidenceFrame(const String& video_path, const String& manifest_path,
	                                  const String& crop_map_path, int frame_second)
{
	Cout() << "shader-evidence-frame start manifest=" << manifest_path
	       << " crop_map=" << crop_map_path << "\n";
	Cout().Flush();
	VsmShaderRecognitionService service;
	String error;
	Cout() << "shader-evidence-frame stage=load-manifest\n";
	Cout().Flush();
	if(!service.manifest.Load(manifest_path)) {
		Cerr() << "ERROR: shader manifest cannot be loaded: " << manifest_path << "\n";
		return 1;
	}
	Cout() << "shader-evidence-frame stage=load-crop-map\n";
	Cout().Flush();
	if(!service.crop_map.Load(crop_map_path)) {
		Cerr() << "ERROR: shader crop map cannot be loaded: " << crop_map_path << "\n";
		return 1;
	}
	if(service.crop_map.width != service.manifest.crop_map_width ||
	   service.crop_map.height != service.manifest.crop_map_height) {
		Cerr() << "ERROR: shader asset crop-map dimensions mismatch: expected="
		       << service.manifest.crop_map_width << "x" << service.manifest.crop_map_height
		       << " actual=" << service.crop_map.width << "x" << service.crop_map.height << "\n";
		return 1;
	}
	Cout() << "shader-evidence-frame stage=validate-manifest\n";
	Cout().Flush();
	if(!service.manifest.Validate(error)) {
		Cerr() << "ERROR: shader manifest invalid: " << error << "\n";
		return 1;
	}
	service.use_threshold = false;

	VsmVideoFileFrameSource video;
	Cout() << "shader-evidence-frame stage=open-video\n";
	Cout().Flush();
	if(!video.Open(video_path)) {
		Cerr() << "ERROR: cannot open direct MP4 source: " << video.GetLastError() << "\n";
		return 1;
	}
	if(frame_second > 0 && !video.SeekMs((int64)frame_second * 1000)) {
		Cerr() << "ERROR: cannot seek direct MP4 source: " << video.GetLastError() << "\n";
		return 1;
	}
	VsmImageBuffer source;
	int64 timestamp_ms = -1;
	Cout() << "shader-evidence-frame stage=decode-frame\n";
	Cout().Flush();
	if(!video.ReadFrame(source, timestamp_ms)) {
		Cerr() << "ERROR: cannot decode diagnostic frame: " << video.GetLastError() << "\n";
		return 1;
	}
	VsmImageBuffer left = CropVsmBuffer(source, kVsmLeftTableRect);
	VsmImageBuffer right = CropVsmBuffer(source, kVsmRightTableRect);
	if(left.IsEmpty() || right.IsEmpty()) {
		Cerr() << "ERROR: diagnostic windows are outside decoded frame: source="
		       << source.Info() << "\n";
		return 1;
	}
	VsmImageBuffer packed;
	Vector<VsmPackedWindow> windows;
	Cout() << "shader-evidence-frame stage=pack-windows\n";
	Cout().Flush();
	if(!VsmPackTwoWindows(left, right, packed, windows, error)) {
		Cerr() << "ERROR: diagnostic window packing failed: " << error << "\n";
		return 1;
	}
	ASSERT(windows.GetCount() == 2);
	windows[0].id = "L";
	windows[1].id = "R";
	for(VsmPackedWindow& window : windows)
		window.timestamp_ms = timestamp_ms;
	VsmShaderEvidenceAdapter adapter;
	adapter.SetService(service);
	Vector<VsmShaderWindowEvidence> results;
	Cout() << "shader-evidence-frame stage=process-evidence\n";
	Cout().Flush();
	if(!adapter.ProcessPacked(packed, windows, results, error)) {
		Cerr() << "ERROR: shader evidence processing failed: " << error << "\n";
		return 1;
	}

	Cout() << "shader-evidence-frame decoder=libavcodec video=" << video_path
	       << " timestamp_ms=" << timestamp_ms << " source=" << source.Info()
	       << " manifest=" << manifest_path
	       << " fixture_kind=" << service.manifest.fixture_kind
	       << " fixture_id=" << (service.manifest.fixture_id.IsEmpty() ? "none" : service.manifest.fixture_id)
	       << " crop_map=" << crop_map_path << "\n";
	Cout() << "shader-evidence-assets status=accepted templates="
	       << service.manifest.templates.GetCount()
	       << " crop_map=" << service.crop_map.width << "x" << service.crop_map.height
	       << " fixture_kind=" << service.manifest.fixture_kind
	       << " fixture_id=" << (service.manifest.fixture_id.IsEmpty() ? "none" : service.manifest.fixture_id)
	       << "\n";
	for(const VsmShaderWindowEvidence& result : results) {
		Cout() << "shader-evidence window=" << result.id
		       << " timestamp_ms=" << result.timestamp_ms
		       << " evidence=" << result.evidence.image.Info()
		       << " runs=" << result.runs.GetCount()
		       << " error=" << (result.error.IsEmpty() ? "none" : result.error) << "\n";
		for(const VsmEvidenceTextRun& run : result.runs)
			Cout() << "shader-evidence-run window=" << result.id
			       << " text=" << run.text
			       << " bounds=" << run.bounds.left << "," << run.bounds.top
			       << "," << run.bounds.Width() << "," << run.bounds.Height()
			       << " confidence=" << Format("%.4f", run.confidence)
			       << " ambiguous=" << (run.ambiguous ? 1 : 0) << "\n";
	}
	Cout() << "shader-evidence-frame status=pass windows=" << results.GetCount() << "\n";
	return 0;
}

int VsmRunShaderEvidenceFrame(const String& video_path, const String& manifest_path,
	                           const String& crop_map_path, int frame_second)
{
	return RunVsmShaderEvidenceFrame(video_path, manifest_path, crop_map_path, frame_second);
}

int VsmRunShaderEvidenceFrameConfig(const String& config_path)
{
	String content = LoadFile(config_path);
	VsmShaderEvidenceFrameConfig config;
	if(content.IsEmpty() || !LoadFromJson(config, content)) {
		Cerr() << "ERROR: shader evidence descriptor cannot be loaded: " << config_path << "\n";
		return 1;
	}
	if(config.video.IsEmpty() || config.manifest.IsEmpty() || config.crop_map.IsEmpty()) {
		Cerr() << "ERROR: shader evidence descriptor requires video, manifest, and crop_map: "
		       << config_path << "\n";
		return 1;
	}
	if(!FileExists(config.video)) {
		Cerr() << "ERROR: shader evidence asset missing kind=video path=" << config.video << "\n";
		return 1;
	}
	if(!FileExists(config.manifest)) {
		Cerr() << "ERROR: shader evidence asset missing kind=manifest path=" << config.manifest << "\n";
		return 1;
	}
	if(!FileExists(config.crop_map)) {
		Cerr() << "ERROR: shader evidence asset missing kind=crop_map path=" << config.crop_map << "\n";
		return 1;
	}
	if(config.frame_second < 0) {
		Cerr() << "ERROR: shader evidence descriptor frame_second must be nonnegative: "
		       << config_path << "\n";
		return 1;
	}
	VsmShaderTemplateManifest identity;
	if(!identity.Load(config.manifest)) {
		Cerr() << "ERROR: shader evidence manifest identity cannot be loaded: "
		       << config.manifest << "\n";
		return 1;
	}
	if(!config.required_fixture_kind.IsEmpty() &&
	   identity.fixture_kind != config.required_fixture_kind) {
		Cerr() << "ERROR: shader evidence fixture kind mismatch: required="
		       << config.required_fixture_kind << " actual=" << identity.fixture_kind << "\n";
		return 1;
	}
	if(!config.required_fixture_id.IsEmpty() && identity.fixture_id != config.required_fixture_id) {
		Cerr() << "ERROR: shader evidence fixture id mismatch: required="
		       << config.required_fixture_id << " actual="
		       << (identity.fixture_id.IsEmpty() ? "none" : identity.fixture_id) << "\n";
		return 1;
	}
	Cout() << "shader-evidence-frame-config path=" << config_path
	       << " video=" << config.video << " manifest=" << config.manifest
	       << " crop_map=" << config.crop_map << " fixture_kind=" << identity.fixture_kind
	       << " fixture_id=" << (identity.fixture_id.IsEmpty() ? "none" : identity.fixture_id)
	       << " frame_second=" << config.frame_second << "\n";
	Cout().Flush();
	return VsmRunShaderEvidenceFrame(config.video, config.manifest, config.crop_map,
	                                 config.frame_second);
}

}
