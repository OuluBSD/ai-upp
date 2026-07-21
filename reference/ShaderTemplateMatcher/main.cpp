#include "ShaderTemplateMatcher.h"

using namespace Upp;

static bool RunSelfTest(const String& report_path)
{
	VsmImageBuffer frame = VsmImageBuffer::MakeSolid(16, 8, 0, 1);
	VsmImageBuffer crop_map = VsmImageBuffer::MakeSolid(8, 4, 0, 1);
	for(int y = 0; y < 2; y++)
		for(int x = 0; x < 2; x++) {
			byte value = (byte)(x == y ? 240 : 120);
			crop_map.Set(x, y, value);
			frame.Set(6 + x, 3 + y, value);
		}
	VsmShaderTemplateManifest manifest;
	manifest.crop_map_width = crop_map.width;
	manifest.crop_map_height = crop_map.height;
	VsmShaderTemplate& templ = manifest.templates.Add();
	templ.id = "synthetic-digit";
	templ.label = "synthetic-digit";
	templ.w = templ.foreground_w = 2;
	templ.h = templ.foreground_h = 2;
	String error;
	if(!manifest.Validate(error)) {
		Cout() << "ERROR: manifest: " << error << "\n";
		return false;
	}
	VsmCpuShaderTemplateMatcher matcher;
	VsmShaderEvidence evidence;
	if(!matcher.Match(frame, crop_map, manifest, evidence, error)) {
		Cout() << "ERROR: match: " << error << "\n";
		return false;
	}
	String shader_path = report_path + ".frag";
	if(!SaveFile(shader_path, VsmCpuShaderTemplateMatcher::FragmentShaderSource())) {
		Cout() << "ERROR: cannot write shader source: " << shader_path << "\n";
		return false;
	}
	String evidence_path = report_path + ".vsm";
	if(!evidence.Save(evidence_path)) {
		Cout() << "ERROR: cannot write evidence: " << evidence_path << "\n";
		return false;
	}
	const VsmShaderMatchHit& hit = evidence.best_hits[0];
	String report = Format("{\n  \"backend\":\"cpu-reference\",\n"
	                       "  \"gpu_runtime\":false,\n  \"template_count\":1,\n"
	                       "  \"dimensions\":\"%d`x%d\",\n  \"best_x\":%d,\n"
	                       "  \"best_y\":%d,\n  \"best_score\":%.6f,\n"
	                       "  \"evidence\":\"%s\",\n  \"shader\":\"%s\"\n}\n",
	                       frame.width, frame.height, hit.x, hit.y, hit.score,
	                       evidence_path, shader_path);
	if(!SaveFile(report_path, report)) return false;
	Cout() << "shader-selftest backend=cpu-reference gpu_runtime=false"
	       << " templates=1 dimensions=" << frame.width << "x" << frame.height
	       << " best=" << hit.x << "," << hit.y << " score=" << hit.score << "\n";
	Cout() << "shader-selftest evidence=" << evidence_path << " shader=" << shader_path << "\n";
	return true;
}

static bool RunFiles(const String& frame_path, const String& crop_path,
	                 const String& manifest_path, const String& report_path)
{
	VsmImageBuffer frame, crop_map;
	VsmShaderTemplateManifest manifest;
	if(!frame.Load(frame_path) || !crop_map.Load(crop_path) || !manifest.Load(manifest_path)) {
		Cout() << "ERROR: cannot load frame, crop map, or manifest\n";
		return false;
	}
	VsmShaderRecognitionService service;
	service.manifest.Load(manifest_path);
	service.crop_map.Load(crop_path);
	VsmShaderEvidence evidence;
	Vector<VsmEvidenceTextRun> runs;
	String error;
	if(!service.Process(frame, evidence, runs, error)) {
		Cout() << "ERROR: recognition: " << error << "\n";
		return false;
	}
	String evidence_path = report_path + ".vsm";
	if(!evidence.Save(evidence_path)) return false;
	VsmOccupancyBenchmark benchmark = VsmBenchmarkOccupancy(evidence.image, 220, 4);
	String json = "{\n  \"backend\":\"cpu-reference\",\n  \"ocr\":false,\n";
	json << "  \"runs\": " << StoreAsJson(runs, true) << ",\n";
	json << "  \"occupancy_benchmark\": " << StoreAsJson(benchmark, true) << ",\n";
	json << "  \"evidence\": \"" << evidence_path << "\"\n}\n";
	if(!SaveFile(report_path, json)) return false;
	Cout() << "shader-match backend=cpu-reference ocr=false runs=" << runs.GetCount()
	       << " evidence=" << evidence_path << "\n";
	return true;
}

CONSOLE_APP_MAIN
{
	const Vector<String>& args = CommandLine();
	if(args.GetCount() >= 1 && args[0] == "--fragment-shader") {
		String path = args.GetCount() >= 2 ? args[1] : "tmp/vsm_template_match.frag";
		if(!SaveFile(path, VsmCpuShaderTemplateMatcher::FragmentShaderSource())) {
			Cout() << "ERROR: cannot write " << path << "\n";
			SetExitCode(1);
		}
		else
			Cout() << "shader-source path=" << path << "\n";
		return;
	}
	if(args.IsEmpty() || args[0] == "--selftest") {
		String path = args.GetCount() >= 2 ? args[1] : "tmp/vsm_shader_selftest.json";
		if(!RunSelfTest(path)) SetExitCode(1);
		return;
	}
	if(args[0] == "--match" && args.GetCount() >= 5) {
		if(!RunFiles(args[1], args[2], args[3], args[4])) SetExitCode(1);
		return;
	}
	Cout() << "usage: ShaderTemplateMatcher.exe --selftest [report.json]\n"
	       << "       ShaderTemplateMatcher.exe --fragment-shader [path]\n"
	       << "       ShaderTemplateMatcher.exe --match frame.vsm crop-map.vsm manifest.json report.json\n";
	SetExitCode(2);
}
