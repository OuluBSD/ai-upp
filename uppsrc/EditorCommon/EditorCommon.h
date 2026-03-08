#ifndef _EditorCommon_EditorCommon_h_
#define _EditorCommon_EditorCommon_h_

#include <Core/Core.h>
#include <plugin/jpg/jpg.h>
#include <plugin/png/png.h>
#include <EditorCommon/Recognition.h>
#include <EditorCommon/GpuPreprocess.h>

namespace Upp {



struct TextHarvestSummary : Moveable<TextHarvestSummary> {
	int buckets = 0;
	int total = 0;
	int unlabeled = 0;
	int labeled = 0;
	int reviewed = 0;
	int rejected = 0;
	int seen = 0;
	int filtered = 0;
	int orb_same_skipped = 0;
	int missing_rect = 0;
	int empty_patch = 0;
	int dup_hash = 0;
	int kept = 0;
	void Jsonize(JsonIO& jio) {
		jio("buckets", buckets)("total", total)("unlabeled", unlabeled)("labeled", labeled)("reviewed", reviewed)("rejected", rejected)
		   ("seen", seen)("filtered", filtered)("orb_same_skipped", orb_same_skipped)("missing_rect", missing_rect)("empty_patch", empty_patch)("dup_hash", dup_hash)("kept", kept);
	}
};



struct ProjectPlatform : public Moveable<ProjectPlatform> {
	String name;
	int max_instances = 1;
	void Jsonize(JsonIO& jio) { jio("name", name)("max_instances", max_instances); }
};

struct ProjectFile {
	String name;
	Vector<ProjectPlatform> platforms;
	String common_code;
	VectorMap<String, String> settings;
	void Jsonize(JsonIO& jio) { jio("name", name)("platforms", platforms)("common_code", common_code)("settings", settings); }
};

struct PlatformFile {
	String name;
	String code;
	Array<GameRule> rules;
	void Jsonize(JsonIO& jio) { jio("name", name)("code", code)("rules", rules); }
};

struct HarvestDatasetStats : Moveable<HarvestDatasetStats> {
	int buckets = 0;
	int total = 0;
	int unlabeled = 0;
	int labeled = 0;
	int reviewed = 0;
	int rejected = 0;
	VectorMap<String, int> status_counts;
	VectorMap<String, int> label_counts;
	void Jsonize(JsonIO& jio) {
		jio("buckets", buckets)("total", total)("unlabeled", unlabeled)("labeled", labeled)("reviewed", reviewed)("rejected", rejected)("status_counts", status_counts)("label_counts", label_counts);
	}
};

struct RuleImageStats : Moveable<RuleImageStats> {
	String dominant_channel = "g";
	double variance_r = 0;
	double variance_g = 0;
	double variance_b = 0;
	void Jsonize(JsonIO& jio) {
		jio("dominant_channel", dominant_channel)("variance_r", variance_r)("variance_g", variance_g)("variance_b", variance_b);
	}
};

struct ScreenGameCliOptions {
	String project_name;
	String remote_addr;
	String platform_name;
	String provider_name;
	String frame_arg;
	String rule_name;
	String pattern_arg;
	String export_preview_path;
	String export_exec_json_path;
	String export_semantic_json_path;
	String export_card_patches_dir;
	String harvest_text_patches_dir;
	String harvest_filter;
	bool harvest_include_orb_same = false;
	bool harvest_implicit_text = false;
	String harvest_summary_dir;
	bool harvest_queue = false;
	bool harvest_readiness = false;
	String harvest_required_labels;
	int harvest_min_per_label = 1;
	String harvest_summary_json_path;
	String harvest_status_filter = "any";
	bool harvest_duplicates = false;
	int harvest_duplicates_limit = 100;
	bool harvest_dedup_reject = false;
	String harvest_dedup_reviewer = "dedup";
	String harvest_edit_match;
	String harvest_set_status;
	String harvest_set_label;
	String harvest_set_reviewer;
	String harvest_apply_labels_path;
	String harvest_export_labels_path;
	bool harvest_char_coverage = false;
	String harvest_required_chars;
	bool list_platforms = false;
	bool list_frames = false;
	bool list_rules = false;
	bool list_props = false;
	bool list_images = false;
	bool run_one = false;
	bool run_all = false;
	bool live_remote_test = false;
	bool synthetic_window_motion_test = false;
	int  live_seconds = 10;
	int  synthetic_fps = 10;
	int  live_warmup_frames = 1;
	int  temporal_local_dx = 10;
	int  temporal_local_dy = 10;
	int  decode_threads = 1;
	int  queue_capacity = 4;
	int  live_rule_cache_limit = 256;
	String drop_policy = "oldest";
	String live_report_json;
	bool override_keep_keyframe = false;
	bool exec_script_debug = false;
	bool validate_frame = false;
	bool trace_script_api = false;
	int expect_window_min = 1;
	int expect_script_objects_min = 0;
	int expect_resolved_min = 0;
	int expect_class_semantic_min = -1;
	int expect_text_semantic_min = -1;
	int expect_policy_semantic_min = -1;
	int expect_text_backend_status_min = -1;
	int expect_class_orb_status_min = -1;
	int expect_class_nn_status_min = -1;
	int expect_class_alt_min = -1;
	int expect_threshold_pass_min = -1;
	int expect_text_rect_total_min = -1;
	int expect_text_rect_shifted_min = -1;
	double expect_min_text_rect_shift_rate = -1.0;
	int expect_ocr_compare_exact_min = -1;
	int expect_ocr_compare_normalized_min = -1;
	int expect_ocr_compare_missing_max = -1;
	int expect_ocr_compare_total_min = -1;
	int expect_ocr_compare_total_max = -1;
	double expect_ocr_compare_normalized_rate_min = -1.0;
	double expect_ocr_compare_missing_rate_max = -1.0;
	int expect_max_text_total_us = -1;
	double expect_max_text_disagreement_rate = -1.0;
	String expect_text_backends;
	bool quit_after_run = false;
	bool semantic_summary = false;
	bool dump_ocr = false;
	bool dump_ocr_shifted_only = false;
	bool dump_ocr_verbose = false;
	bool compare_ocr_to_annotations = false;
	bool compare_ocr_diff_only = false;
	String compare_ocr_filter;
	String export_ocr_compare_json_path;
	String dump_ocr_filter;
	bool orb_find = false;
	bool verbose = false;
	bool debug_orb = false;
	bool fast_exec = false;
	bool profile_exec = false;
	bool gpu_preprocess = false;
	bool gpu_probe = false;
	String gpu_backend = "auto";
	String gpu_route = "auto";
	bool save_frame_state = false;
	bool skip_autoexec = false;
	bool test_orb = false;
	bool test_eglimage = false;
	bool test_v4l2_eglimage = false;
	bool test_d3d11_interop = false;
	bool test_jpg = false;
	bool test_png = false;
	bool test_tesseract_render = false;
	bool test_convnet_cards = false;
	bool test_convnet_disk_pipeline = false;
	bool convnet_generate_dataset_texas = false;
	int training_minutes = 20;
	String dataset_path;
	String model_tag = "v1";
	bool train_cards = false;
	String video_device = "/dev/video0";
	String video_format_policy = "auto";
	int convnet_iters = 2;
	String convnet_dump_dir;
	String convnet_data_dir;
	String convnet_model_dir;
	String convnet_disk_tag = "all";
	String openai_key_file;
	String openai_model = "gpt-4.1-mini";
	bool headless = false;
};

struct FrameRuleRects : Moveable<FrameRuleRects> {
	String rule;
	Vector<Rect> rects;
	void Jsonize(JsonIO& jio) { jio("rule", rule)("rects", rects); }
};

struct FrameRuleData : Moveable<FrameRuleData> {
	String rule;
	Vector<String> images;
	Rect rect;
	bool has_rect = false;
	String ocr_text;
	String openai_example;
	void Jsonize(JsonIO& jio) { jio("rule", rule)("images", images)("rect", rect)("has_rect", has_rect)("ocr_text", ocr_text)("openai_example", openai_example); }
};

struct FrameAnalysisData : Moveable<FrameAnalysisData> {
	Vector<Rect> window_rects;
	VectorMap<String, String> props;
	Vector<FrameRuleRects> rule_rects;
	Vector<FrameRuleData> rule_data;
	bool use_layout = false;
	void Jsonize(JsonIO& jio) { jio("window_rects", window_rects)("props", props)("rule_rects", rule_rects)("rule_data", rule_data)("use_layout", use_layout); }
};

struct RuleImageMetadata : Moveable<RuleImageMetadata> {
	String source_frame;
	String source_rule;
	Rect source_rect;
	String source_provider;
	String source_platform;
	String label;
	double label_confidence = 0.0;
	String review_status = "unlabeled";
	String value_text;
	String dedupe_hash;
	String capture_kind;
	String reviewer;
	String captured_at;
	void Jsonize(JsonIO& jio) {
		jio("source_frame", source_frame)
		   ("source_rule", source_rule)
		   ("source_rect", source_rect)
		   ("source_provider", source_provider)
		   ("source_platform", source_platform)
		   ("label", label)
		   ("label_confidence", label_confidence)
		   ("review_status", review_status)
		   ("value_text", value_text)
		   ("dedupe_hash", dedupe_hash)
		   ("capture_kind", capture_kind)
		   ("reviewer", reviewer)
		   ("captured_at", captured_at);
	}
};

void AddScreenGameArgs(CommandLineArguments& cla);
bool ParseScreenGameCli(CommandLineArguments& cla, ScreenGameCliOptions& opt);
int RunScreenGameHeadless(const ScreenGameCliOptions& opt);
void YUYVToImage(const unsigned char* src, int w, int h, ImageBuffer& ib);
VectorMap<String, String> BuildDynamicPropsFromRects(const Vector<Rect>& rects);
Image RenderPreviewWithRects(const Image& img, const Vector<Rect>& rects);
String SavePreviewWithRectsJpg(const Image& img, const Vector<Rect>& rects, const String& export_path = String());
String GetProjectsRootPath();
String GetProjectDirPath(const String& project_name);
String GetProjectPlatformsDirPath(const String& project_name);
String GetProjectCommonCodePath(const String& project_name);
String GetPlatformCodePath(const String& project_name, const String& platform_name);
String GetProjectThemePath(const String& project_name, const String& platform_name);
String GetProjectSettingString(const String& project_name, const String& setting_key);
String ResolveProjectSettingPath(const String& project_name, const String& setting_key);
String GetPlatformLocalLoopScriptPath(const String& project_name, const String& platform_name);
String ResolveProjectLocalLoopScriptPath(const String& project_name);
String LoadProjectCommonCode(const String& project_name, const String& json_fallback);
String LoadPlatformCode(const String& project_name, const String& platform_name, const String& json_fallback);
bool SaveProjectCommonCode(const String& project_name, const String& code);
bool SavePlatformCode(const String& project_name, const String& platform_name, const String& code);
String GetFrameSidecarPath(const String& frame_path);
bool SaveFrameAnalysisData(const String& frame_path, const FrameAnalysisData& data);
bool LoadFrameAnalysisData(const String& frame_path, FrameAnalysisData& data);
String GetRuleImageSidecarPath(const String& image_path);
bool SaveRuleImageMetadata(const String& image_path, const RuleImageMetadata& data);
bool LoadRuleImageMetadata(const String& image_path, RuleImageMetadata& data);
bool GetReferenceRuleRect(const FrameAnalysisData& ref_data, const Array<GameRule>& rules, const String& rule_name, Rect& out);
bool ResolveRuntimeRuleRect(const FrameAnalysisData& ref_data, const Array<GameRule>& rules,
                            const String& anchor_rule_name, const Rect& anchor_runtime_rect,
                            const String& target_rule_name, Rect& out);

String MsTextFromUs(int64 us);


bool CollectHarvestDatasetStats(const String& root, const String& status_filter, HarvestDatasetStats& st, Vector<String>* queue_lines = nullptr);
bool CollectHarvestDuplicateGroups(const String& root, const String& status_filter, VectorMap<String, Vector<String>>& dup_groups, int& duplicate_items);
bool CollectHarvestCharCoverage(const String& root, const String& status_filter, VectorMap<String, int>& char_counts, int& char_rows);
String ResolveFramePath(const String& project_name, const String& platform_name, const String& provider, const String& frame_arg);
Image LoadImageForPath(const String& path);
bool LoadProjectFileByName(const String& name, ProjectFile& pf, String& path_out);
bool LoadPlatformFileByName(const String& project_name, const String& platform_name, PlatformFile& pf);
String GetProjectDirPath(const String& project_name);
String GetProjectThemePath(const String& project_name, const String& platform_name);
String GetPlatformLocalLoopScriptPath(const String& project_name, const String& platform_name);
String ResolveProjectLocalLoopScriptPath(const String& project_name);
String LoadProjectCommonCode(const String& project_name, const String& json_fallback);
String LoadPlatformCode(const String& project_name, const String& platform_name, const String& json_fallback);
String GetScreenshotsRoot(const String& project_name, const String& platform_name);
String GetScreenshotsDir(const String& project_name, const String& platform_name, const String& provider_name);
const Vector<String>& GetRuleImagesRef(const GameRule& r);
String ResolveRuleImagePath(const String& project_name, const String& image_path);
bool LoadUseLayoutFrameData(const String& project_name, const String& platform_name, const String& provider_name, FrameAnalysisData& out);
void ApplyFrameRuleDataToRules(Array<GameRule>& rules, const FrameAnalysisData& data);
void FillMissingRuleImagesFromDirs(Array<GameRule>& rules, const String& project_name, const String& platform_name);
void ScriptLogStdout(String s);
String GetRuleSamplesDir(const String& project_name, const String& platform_name, const String& rule_name);
void ListPngFiles(const String& dir, Vector<String>& out);

int RunOrbSelfTest();
int RunEGLImageSelfTest();
int RunV4L2EGLImageSelfTest(const String& device_path, const String& format_policy);
int RunJpgSelfTest();
int RunPngSelfTest();
int RunTesseractRenderSelfTest();
int RunConvNetCardsSelfTest(int iters, const String& dump_dir, bool real_mode = true);
int RunConvNetDiskPipelineTest(const String& data_dir, const String& model_dir, int iters, const String& tag);

extern bool g_match_fast_exec;
extern bool g_match_profile_exec;
extern bool g_cli_verbose;
extern bool g_cli_trace_api;

}

#include <EditorCommon/Scripting.h>
#endif
