#include <Core/Core.h>
#include <EditorCommon/EditorCommon.h>
#include <Painter/Painter.h>
#include <plugin/png/png.h>

#include <Painter/Painter.h>
#include <plugin/png/png.h>
#include <Painter/Painter.h>
#include <EditorCommon/EditorCommon.h>
#include <Painter/Painter.h>
#include <plugin/png/png.h>

#include "GpuPreprocess.h"
#include "Capture.h"
#include "Scripting.h"
#include <atomic>
#include <cstdio>
#include <plugin/jpg/jpg.h>
#ifdef PLATFORM_WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
// Phase 12 Stage 4B: V4L2 headers for webcam capture
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <drm/drm_fourcc.h>
// Phase 12: EGL/GL headers for DMA-BUF import test
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>

#ifdef None
#undef None
#endif

#endif

namespace Upp {













static uint64 HashImagePixels(const Image& img) {
	if (img.IsEmpty())
		return 1469598103934665603ull;
	const RGBA* px = ~img;
	int n = img.GetLength();
	uint64 h = 1469598103934665603ull;
	for (int i = 0; i < n; i++) {
		h ^= (uint64)px[i].r; h *= 1099511628211ull;
		h ^= (uint64)px[i].g; h *= 1099511628211ull;
		h ^= (uint64)px[i].b; h *= 1099511628211ull;
		h ^= (uint64)px[i].a; h *= 1099511628211ull;
	}
	return h;
}

static String HashHex(uint64 h) {
	char b[32];
	snprintf(b, sizeof(b), "%016llx", (unsigned long long)h);
	return String(b);
}

static bool ParseHex64Text(const String& s, uint64& out) {
	String t = TrimBoth(s);
	if (t.StartsWith("0x") || t.StartsWith("0X"))
		t = t.Mid(2);
	if (t.IsEmpty())
		return false;
	uint64 v = 0;
	for (int i = 0; i < t.GetCount(); i++) {
		byte c = (byte)t[i];
		int x = -1;
		if (c >= '0' && c <= '9') x = c - '0';
		else if (c >= 'a' && c <= 'f') x = 10 + (c - 'a');
		else if (c >= 'A' && c <= 'F') x = 10 + (c - 'A');
		else return false;
		v = (v << 4) | (uint64)x;
	}
	out = v;
	return true;
}

static String SanitizeNameForPath(const String& s) {
	String out;
	out.Reserve(s.GetCount());
	for (int i = 0; i < s.GetCount(); i++) {
		byte c = (byte)s[i];
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.')
			out.Cat((char)c);
		else
			out.Cat('_');
	}
	return out.IsEmpty() ? String("unnamed") : out;
}

static bool ParseRectString(const String& s, Rect& out) {
	Vector<String> t = Split(TrimBoth(s), ' ');
	Vector<int> nums;
	for (int i = 0; i < t.GetCount(); i++) {
		String tok = TrimBoth(t[i]);
		if (tok.IsEmpty())
			continue;
		nums.Add(StrInt(tok));
	}
	if (nums.GetCount() != 4)
		return false;
	out = RectC(nums[0], nums[1], nums[2], nums[3]);
	return true;
}

static void LoadExistingHarvestHashes(const String& bucket_dir, Index<uint64>& seen_hashes) {
	seen_hashes.Clear();
	if (bucket_dir.IsEmpty() || !DirectoryExists(bucket_dir))
		return;
	FindFile ff(AppendFileName(bucket_dir, "*.png"));
	while (ff) {
		if (ff.IsFile()) {
			String p = AppendFileName(bucket_dir, ff.GetName());
			RuleImageMetadata md;
			uint64 h = 0;
			bool have_hash = false;
			if (LoadRuleImageMetadata(p, md) && ParseHex64Text(md.dedupe_hash, h))
				have_hash = true;
			if (!have_hash) {
				Image img = StreamRaster::LoadFileAny(p);
				if (!img.IsEmpty()) {
					h = HashImagePixels(img);
					have_hash = true;
				}
			}
			if (have_hash)
				seen_hashes.FindAdd(h);
		}
		ff.Next();
	}
}

static String BuildHarvestStamp(int seq) {
	Time now = GetSysTime();
	return Format("%04d%02d%02d_%02d%02d%02d_%03d",
	              now.year, now.month, now.day,
	              now.hour, now.minute, now.second, seq);
}

bool CollectHarvestDatasetStats(const String& root, const String& status_filter, HarvestDatasetStats& st, Vector<String>* queue_lines ) {
	st = HarvestDatasetStats();
	if (root.IsEmpty() || !DirectoryExists(root))
		return false;
	FindFile dir_ff(AppendFileName(root, "*"));
	while (dir_ff) {
		if (dir_ff.IsFolder() && dir_ff.GetName()[0] != '.') {
			st.buckets++;
			String bucket = dir_ff.GetName();
			String bucket_dir = AppendFileName(root, bucket);
			FindFile ff(AppendFileName(bucket_dir, "*.png"));
			while (ff) {
				if (ff.IsFile()) {
					String img_path = AppendFileName(bucket_dir, ff.GetName());
					RuleImageMetadata md;
					if (LoadRuleImageMetadata(img_path, md)) {
						st.total++;
						String status = ToLower(TrimBoth(md.review_status));
						if (status.IsEmpty())
							status = "unlabeled";
						int qs = st.status_counts.Find(status);
						if (qs < 0) st.status_counts.Add(status, 1);
						else st.status_counts[qs]++;
						if (status == "unlabeled") st.unlabeled++;
						else if (status == "labeled") st.labeled++;
						else if (status == "reviewed") st.reviewed++;
						else if (status == "rejected") st.rejected++;
						bool pass = (status_filter == "any" || status_filter.IsEmpty() || status == status_filter);
						if (pass) {
							String label = TrimBoth(md.label);
							if (!label.IsEmpty()) {
								int ql = st.label_counts.Find(label);
								if (ql < 0) st.label_counts.Add(label, 1);
								else st.label_counts[ql]++;
							}
						}
						if (queue_lines && pass && status == "unlabeled")
							queue_lines->Add(bucket + " " + img_path);
					}
				}
				ff.Next();
			}
		}
		dir_ff.Next();
	}
	return true;
}

bool CollectHarvestDuplicateGroups(const String& root, const String& status_filter,
                                          VectorMap<String, Vector<String>>& groups, int& duplicate_items) {
	groups.Clear();
	duplicate_items = 0;
	if (root.IsEmpty() || !DirectoryExists(root))
		return false;
	FindFile dir_ff(AppendFileName(root, "*"));
	while (dir_ff) {
		if (dir_ff.IsFolder() && dir_ff.GetName()[0] != '.') {
			String bucket = dir_ff.GetName();
			String bucket_dir = AppendFileName(root, bucket);
			FindFile ff(AppendFileName(bucket_dir, "*.png"));
			while (ff) {
				if (ff.IsFile()) {
					String img_path = AppendFileName(bucket_dir, ff.GetName());
					RuleImageMetadata md;
					String status = "unlabeled";
					String h;
					if (LoadRuleImageMetadata(img_path, md)) {
						status = ToLower(TrimBoth(md.review_status));
						if (status.IsEmpty())
							status = "unlabeled";
						h = TrimBoth(md.dedupe_hash);
					}
					if (!(status_filter == "any" || status_filter.IsEmpty() || status == status_filter)) {
						ff.Next();
						continue;
					}
					if (h.IsEmpty()) {
						Image img = StreamRaster::LoadFileAny(img_path);
						if (!img.IsEmpty())
							h = HashHex(HashImagePixels(img));
					}
					if (h.IsEmpty()) {
						ff.Next();
						continue;
					}
					String key = ToLower(h);
					int q = groups.Find(key);
					if (q < 0) {
						Vector<String>& arr = groups.Add(key);
						arr.Add(img_path);
					}
					else
						groups[q].Add(img_path);
				}
				ff.Next();
			}
		}
		dir_ff.Next();
	}
	for (int i = groups.GetCount() - 1; i >= 0; --i) {
		if (groups[i].GetCount() <= 1)
			groups.Remove(i);
		else
			duplicate_items += groups[i].GetCount();
	}
	return true;
}

bool CollectHarvestCharCoverage(const String& root, const String& status_filter,
                                       VectorMap<String, int>& char_counts, int& labeled_rows) {
	char_counts.Clear();
	labeled_rows = 0;
	if (root.IsEmpty() || !DirectoryExists(root))
		return false;
	FindFile dir_ff(AppendFileName(root, "*"));
	while (dir_ff) {
		if (dir_ff.IsFolder() && dir_ff.GetName()[0] != '.') {
			String bucket_dir = AppendFileName(root, dir_ff.GetName());
			FindFile ff(AppendFileName(bucket_dir, "*.png"));
			while (ff) {
				if (ff.IsFile()) {
					String img_path = AppendFileName(bucket_dir, ff.GetName());
					RuleImageMetadata md;
					if (!LoadRuleImageMetadata(img_path, md)) {
						ff.Next();
						continue;
					}
					String status = ToLower(TrimBoth(md.review_status));
					if (status.IsEmpty())
						status = "unlabeled";
					if (!(status_filter == "any" || status_filter.IsEmpty() || status == status_filter)) {
						ff.Next();
						continue;
					}
					String label = md.label;
					if (TrimBoth(label).IsEmpty()) {
						ff.Next();
						continue;
					}
					labeled_rows++;
					for (int i = 0; i < label.GetCount(); i++) {
						String ch = String(label.Mid(i, 1));
						int q = char_counts.Find(ch);
						if (q < 0) char_counts.Add(ch, 1);
						else char_counts[q]++;
					}
				}
				ff.Next();
			}
		}
		dir_ff.Next();
	}
	return true;
}

static bool HarvestTextPatches(const String& out_root,
                               const String& platform_name,
                               const String& provider_name,
                               const String& source_frame,
                               const Image& frame_img,
                               const Vector<ScriptRuntimeObject>& script_objects,
                               const String& filter_pattern,
                               bool include_orb_same,
                               bool implicit_text,
                               TextHarvestSummary& out_summary) {
	out_summary = TextHarvestSummary();
	if (out_root.IsEmpty() || frame_img.IsEmpty())
		return false;
	RealizeDirectory(out_root);
	VectorMap<String, Index<uint64>> bucket_hashes;
	int seq = 0;
	for (int i = 0; i < script_objects.GetCount(); i++) {
		const ScriptRuntimeObject& o = script_objects[i];
		int qtext = o.props.Find("text");
		if (qtext < 0)
			continue;
		int qcap = o.props.Find("harvest.capture");
		bool script_marked = qcap >= 0 && (o.props[qcap] == "1" || ToLower(o.props[qcap]) == "true");
		if (!implicit_text && !script_marked)
			continue;
		out_summary.seen++;
		if (!filter_pattern.IsEmpty() && !PatternMatchMulti(filter_pattern, o.name)) {
			out_summary.filtered++;
			continue;
		}
		if (!include_orb_same) {
			int qsame = o.props.Find("text.orb_same");
			if (qsame >= 0 && (o.props[qsame] == "1" || ToLower(o.props[qsame]) == "true")) {
				out_summary.orb_same_skipped++;
				continue;
			}
		}
		Rect rr;
		int qocr = o.props.Find("text.ocr_rect");
		if (qocr >= 0 && ParseRectString(o.props[qocr], rr)) {
		}
		else if (!o.rect.IsEmpty())
			rr = o.rect;
		if (rr.IsEmpty()) {
			out_summary.missing_rect++;
			continue;
		}
		rr.Intersect(frame_img.GetSize());
		if (rr.IsEmpty()) {
			out_summary.missing_rect++;
			continue;
		}
		Image patch = Crop(frame_img, rr);
		if (patch.IsEmpty()) {
			out_summary.empty_patch++;
			continue;
		}
		String bucket_name = SanitizeNameForPath(o.name);
		String bucket_dir = AppendFileName(out_root, bucket_name);
		RealizeDirectory(bucket_dir);
		int bi = bucket_hashes.Find(bucket_name);
		if (bi < 0) {
			Index<uint64>& idx = bucket_hashes.Add(bucket_name);
			LoadExistingHarvestHashes(bucket_dir, idx);
			bi = bucket_hashes.GetCount() - 1;
		}
		uint64 h = HashImagePixels(patch);
		if (bucket_hashes[bi].Find(h) >= 0) {
			out_summary.dup_hash++;
			continue;
		}
		String stamp = BuildHarvestStamp(seq++);
		String file_name = "patch_" + stamp + ".png";
		String patch_path = AppendFileName(bucket_dir, file_name);
		if (!PNGEncoder().SaveFile(patch_path, patch))
			continue;
		RuleImageMetadata md;
		md.source_frame = source_frame;
		md.source_rule = o.name;
		md.source_rect = rr;
		md.source_provider = provider_name;
		md.source_platform = platform_name;
		md.label = String();
		md.label_confidence = 0.0;
		md.review_status = "unlabeled";
		md.value_text = qtext >= 0 ? o.props[qtext] : String();
		md.dedupe_hash = HashHex(h);
		md.capture_kind = "text_patch";
		md.reviewer = String();
		md.captured_at = AsString(GetSysTime());
		SaveRuleImageMetadata(patch_path, md);
		bucket_hashes[bi].FindAdd(h);
		out_summary.kept++;
	}
	return true;
}

String GetProjectsRootPath() { return AppendFileName(GetCurrentDirectory(), "gamescreen"); }
String GetProjectDirPath(const String& project_name) { return AppendFileName(GetProjectsRootPath(), project_name); }
String GetProjectPlatformsDirPath(const String& project_name) { return AppendFileName(GetProjectDirPath(project_name), "platforms"); }
static String GetProjectFilePath(const String& name) { return AppendFileName(GetProjectDirPath(name), "project.json"); }
static String GetPlatformsDir(const String& project_name) { return GetProjectPlatformsDirPath(project_name); }
static String GetPlatformFilePath(const String& project_name, const String& platform_name) { return AppendFileName(GetPlatformsDir(project_name), platform_name + ".json"); }
static String GetPlatformDir(const String& project_name, const String& platform_name) { return AppendFileName(GetPlatformsDir(project_name), platform_name); }
String GetRuleSamplesDir(const String& project_name, const String& platform_name, const String& rule_name) {
	if (project_name.IsEmpty() || platform_name.IsEmpty() || rule_name.IsEmpty())
		return String();
	return AppendFileName(AppendFileName(GetPlatformDir(project_name, platform_name), "rules"), rule_name);
}
static String MakeProjectRelative(const String& project_name, const String& path) {
	String proj = NormalizePath(GetProjectDirPath(project_name));
	String full = NormalizePath(path);
	if (proj.IsEmpty() || full.IsEmpty())
		return path;
	if (full.StartsWith(proj)) {
		int cut = proj.GetCount();
		if (cut < full.GetCount() && (full[cut] == DIR_SEP || full[cut] == '/'))
			cut++;
		return full.Mid(cut);
	}
	return path;
}
String GetProjectCommonCodePath(const String& project_name) { return AppendFileName(GetProjectDirPath(project_name), "common.py"); }
String GetPlatformCodePath(const String& project_name, const String& platform_name) { return AppendFileName(GetPlatformsDir(project_name), platform_name + ".py"); }
String GetProjectThemePath(const String& project_name, const String& platform_name) {
	return AppendFileName(GetPlatformsDir(project_name), platform_name + ".theme.json");
}
String GetProjectSettingString(const String& project_name, const String& setting_key) {
	if (project_name.IsEmpty() || setting_key.IsEmpty())
		return String();
	String project_path = GetProjectFilePath(project_name);
	if (!FileExists(project_path))
		return String();
	ProjectFile pf;
	try {
		LoadFromJson(pf, LoadFile(project_path));
	}
	catch (...) {
		return String();
	}
	int q = pf.settings.Find(setting_key);
	if (q < 0)
		return String();
	return pf.settings[q];
}
String ResolveProjectSettingPath(const String& project_name, const String& setting_key) {
	String raw = TrimBoth(GetProjectSettingString(project_name, setting_key));
	if (raw.IsEmpty())
		return String();
	if (IsFullPath(raw))
		return raw;
	return AppendFileName(GetProjectDirPath(project_name), raw);
}
String GetPlatformLocalLoopScriptPath(const String& project_name, const String& platform_name) {
	if (project_name.IsEmpty() || platform_name.IsEmpty())
		return String();
	return AppendFileName(GetPlatformsDir(project_name), platform_name + ".local-loop.py");
}
String ResolveProjectLocalLoopScriptPath(const String& project_name) {
	return ResolveProjectSettingPath(project_name, "local_loop_script");
}
String LoadProjectCommonCode(const String& project_name, const String& json_fallback) {
	String p = GetProjectCommonCodePath(project_name);
	return FileExists(p) ? LoadFile(p) : json_fallback;
}
String LoadPlatformCode(const String& project_name, const String& platform_name, const String& json_fallback) {
	String p = GetPlatformCodePath(project_name, platform_name);
	return FileExists(p) ? LoadFile(p) : json_fallback;
}
bool SaveProjectCommonCode(const String& project_name, const String& code) {
	if (project_name.IsEmpty()) return false;
	String p = GetProjectCommonCodePath(project_name);
	RealizeDirectory(GetFileDirectory(p));
	return SaveFile(p, code);
}
bool SavePlatformCode(const String& project_name, const String& platform_name, const String& code) {
	if (project_name.IsEmpty() || platform_name.IsEmpty()) return false;
	String p = GetPlatformCodePath(project_name, platform_name);
	RealizeDirectory(GetFileDirectory(p));
	return SaveFile(p, code);
}
String GetFrameSidecarPath(const String& frame_path) {
	if (frame_path.IsEmpty())
		return String();
	return ForceExt(frame_path, ".json");
}

String GetRuleImageSidecarPath(const String& image_path) {
	if (image_path.IsEmpty())
		return String();
	return ForceExt(image_path, ".json");
}

bool SaveRuleImageMetadata(const String& image_path, const RuleImageMetadata& data) {
	String p = GetRuleImageSidecarPath(image_path);
	if (p.IsEmpty())
		return false;
	RealizeDirectory(GetFileDirectory(p));
	return SaveFile(p, StoreAsJson(const_cast<RuleImageMetadata&>(data)));
}

bool LoadRuleImageMetadata(const String& image_path, RuleImageMetadata& data) {
	data = RuleImageMetadata();
	String p = GetRuleImageSidecarPath(image_path);
	if (p.IsEmpty() || !FileExists(p))
		return false;
	try {
		LoadFromJson(data, LoadFile(p));
		return true;
	}
	catch (...) {
		return false;
	}
}

bool SaveFrameAnalysisData(const String& frame_path, const FrameAnalysisData& data) {
	String p = GetFrameSidecarPath(frame_path);
	if (p.IsEmpty())
		return false;
	RealizeDirectory(GetFileDirectory(p));
	return SaveFile(p, StoreAsJson(const_cast<FrameAnalysisData&>(data)));
}
bool LoadFrameAnalysisData(const String& frame_path, FrameAnalysisData& data) {
	data.window_rects.Clear();
	data.props.Clear();
	data.rule_rects.Clear();
	data.rule_data.Clear();
	data.use_layout = false;
	String p = GetFrameSidecarPath(frame_path);
	if (p.IsEmpty() || !FileExists(p))
		return false;
	try {
		LoadFromJson(data, LoadFile(p));
		return true;
	}
	catch (...) {
		return false;
	}
}

bool GetReferenceRuleRect(const FrameAnalysisData& ref_data, const Array<GameRule>& rules, const String& rule_name, Rect& out) {
	for (const FrameRuleData& d : ref_data.rule_data) {
		if (d.rule == rule_name && d.has_rect && !d.rect.IsEmpty()) {
			out = d.rect;
			return true;
		}
	}
	if (rule_name == "window-size" && !ref_data.window_rects.IsEmpty() && !ref_data.window_rects[0].IsEmpty()) {
		out = ref_data.window_rects[0];
		return true;
	}
	for (int i = 0; i < rules.GetCount(); i++) {
		const GameRule& r = rules[i];
		if (r.name == rule_name && !r.rect.IsEmpty()) {
			out = r.rect;
			return true;
		}
	}
	return false;
}

bool ResolveRuntimeRuleRect(const FrameAnalysisData& ref_data, const Array<GameRule>& rules,
                            const String& anchor_rule_name, const Rect& anchor_runtime_rect,
                            const String& target_rule_name, Rect& out) {
	if (anchor_runtime_rect.IsEmpty())
		return false;
	Rect ref_anchor;
	Rect ref_target;
	if (!GetReferenceRuleRect(ref_data, rules, anchor_rule_name, ref_anchor))
		return false;
	if (!GetReferenceRuleRect(ref_data, rules, target_rule_name, ref_target))
		return false;
	Point rel = ref_target.TopLeft() - ref_anchor.TopLeft();
	Size sz = ref_target.GetSize();
	out = RectC(anchor_runtime_rect.left + rel.x, anchor_runtime_rect.top + rel.y, sz.cx, sz.cy);
	return !out.IsEmpty();
}

static bool GetWindowSizeHintRect(const FrameAnalysisData& data, Rect& out) {
	for (const FrameRuleData& rd : data.rule_data) {
		if (rd.rule == "window-size" && rd.has_rect && !rd.rect.IsEmpty()) {
			out = rd.rect;
			return true;
		}
	}
	if (!data.window_rects.IsEmpty() && !data.window_rects[0].IsEmpty()) {
		out = data.window_rects[0];
		return true;
	}
	return false;
}

String GetScreenshotsRoot(const String& project_name, const String& platform_name) {
	if (project_name.IsEmpty() || platform_name.IsEmpty()) return String();
	return AppendFileName(AppendFileName(GetProjectDirPath(project_name), "screenshots"), platform_name);
}
String GetScreenshotsDir(const String& project_name, const String& platform_name, const String& provider_name) {
	if (project_name.IsEmpty() || platform_name.IsEmpty() || provider_name.IsEmpty()) return String();
	return AppendFileName(GetScreenshotsRoot(project_name, platform_name), provider_name);
}

static bool FindUseLayoutWindowHint(const String& project_name, const String& platform_name, const String& provider_name, Rect& out) {
	String dir = GetScreenshotsDir(project_name, platform_name, provider_name);
	if (dir.IsEmpty())
		return false;
	FindFile ff(AppendFileName(dir, "*.*"));
	while (ff) {
		if (ff.IsFile()) {
			String ext = ToLower(GetFileExt(ff.GetName()));
			if (ext == ".jpg" || ext == ".jpeg" || ext == ".png") {
				FrameAnalysisData d;
				String path = AppendFileName(dir, ff.GetName());
				if (LoadFrameAnalysisData(path, d) && d.use_layout) {
					if (GetWindowSizeHintRect(d, out))
						return true;
				}
			}
		}
		ff.Next();
	}
	return false;
}

bool LoadUseLayoutFrameData(const String& project_name, const String& platform_name, const String& provider_name, FrameAnalysisData& out) {
	out = FrameAnalysisData();
	String dir = GetScreenshotsDir(project_name, platform_name, provider_name);
	if (dir.IsEmpty())
		return false;
	FindFile ff(AppendFileName(dir, "*.*"));
	while (ff) {
		if (ff.IsFile()) {
			String ext = ToLower(GetFileExt(ff.GetName()));
			if (ext == ".jpg" || ext == ".jpeg" || ext == ".png") {
				String path = AppendFileName(dir, ff.GetName());
				FrameAnalysisData d;
				if (LoadFrameAnalysisData(path, d) && d.use_layout) {
					out = pick(d);
					return true;
				}
			}
		}
		ff.Next();
	}
	return false;
}

const Vector<String>& GetRuleImagesRef(const GameRule& r) { return r.images.GetCount() ? r.images : r.samples; }

String ResolveRuleImagePath(const String& project_name, const String& image_path) {
	if (image_path.IsEmpty()) return String();
	if (IsFullPath(image_path)) return image_path;
	return AppendFileName(GetProjectDirPath(project_name), image_path);
}

Image LoadImageForPath(const String& path) {
	String ext = ToLower(GetFileExt(path));
	if (ext == ".jpg" || ext == ".jpeg") return JPGRaster().LoadFile(path);
	return StreamRaster::LoadFileAny(path);
}

bool LoadProjectFileByName(const String& name, ProjectFile& pf, String& path_out) {
	path_out = GetProjectFilePath(name);
	if (!FileExists(path_out)) return false;
	try { LoadFromJson(pf, LoadFile(path_out)); }
	catch (...) { return false; }
	if (pf.name.IsEmpty()) pf.name = name;
	return true;
}

bool LoadPlatformFileByName(const String& project_name, const String& platform_name, PlatformFile& pf) {
	String path = GetPlatformFilePath(project_name, platform_name);
	if (!FileExists(path)) return false;
	try { LoadFromJson(pf, LoadFile(path)); }
	catch (...) { return false; }
	if (pf.name.IsEmpty()) pf.name = platform_name;
	return true;
}

String ResolveFramePath(const String& project_name, const String& platform_name, const String& provider, const String& frame_arg) {
	if (frame_arg.IsEmpty()) return String();
	if (IsFullPath(frame_arg)) return frame_arg;
	if (!platform_name.IsEmpty()) {
		if (!provider.IsEmpty()) return AppendFileName(GetScreenshotsDir(project_name, platform_name, provider), frame_arg);
		return AppendFileName(GetScreenshotsRoot(project_name, platform_name), frame_arg);
	}
	return AppendFileName(GetProjectDirPath(project_name), frame_arg);
}

static bool ParseHostPort(const String& addr, String& host, int& port) {
	int c = addr.ReverseFind(':');
	if (c <= 0 || c + 1 >= addr.GetCount())
		return false;
	host = TrimBoth(addr.Left(c));
	String p = TrimBoth(addr.Mid(c + 1));
	if (host.IsEmpty())
		return false;
	port = StrInt(p);
	return port > 0 && port <= 65535;
}

struct OrbCacheStats : Moveable<OrbCacheStats> {
	int64 hits = 0;
	int64 misses = 0;
	int64 evictions = 0;
};

struct RemoteFramePacket : Moveable<RemoteFramePacket> {
	uint32 id = 0;
	String payload;
	int64 fetch_us = 0;
};

struct DecodedFrame : Moveable<DecodedFrame> {
	uint32 id = 0;
	Image img;
	int64 fetch_us = 0;
	int64 decode_us = 0;
};

void YUYVToImage(const unsigned char* src, int w, int h, ImageBuffer& ib) {
	const unsigned char* s = src;
	for (int y = 0; y < h; y++) {
		RGBA* t = ib[y];
		for (int x = 0; x < w / 2; x++) {
			int y0 = s[0]; int u0 = s[1]; int y1 = s[2]; int v0 = s[3];
			s += 4;
			auto YUV2RGB = [](int yy, int u, int v, RGBA& p) {
				int c = yy - 16; int d = u - 128; int e = v - 128;
				p.r = (byte)clamp((298 * c + 409 * e + 128) >> 8, 0, 255);
				p.g = (byte)clamp((298 * c - 100 * d - 208 * e + 128) >> 8, 0, 255);
				p.b = (byte)clamp((298 * c + 516 * d + 128) >> 8, 0, 255);
				p.a = 255;
			};
			YUV2RGB(y0, u0, v0, t[0]);
			YUV2RGB(y1, u0, v0, t[1]);
			t += 2;
		}
	}
}

static bool GrabRemoteFramePacket(TcpSocket& sock, uint32& last_id, RemoteFramePacket& out) {
	out = RemoteFramePacket();
	if (sock.IsEof() || sock.IsError())
		return false;
	TimeStop ts;
	sock.Put(&last_id, 4);
	uint32 resp_id = 0;
	uint32 sz = 0;
	if (!sock.GetAll(&resp_id, 4) || !sock.GetAll(&sz, 4))
		return false;
	if (sz == 0)
		return true;
	String payload = sock.GetAll(sz);
	if (payload.GetCount() != (int)sz)
		return false;
	last_id = resp_id;
	out.id = resp_id;
	out.payload = pick(payload);
	out.fetch_us = ts.Elapsed();
	return true;
}

static bool DecodeRemoteFramePacket(const RemoteFramePacket& pkt, DecodedFrame& out) {
	out = DecodedFrame();
	out.id = pkt.id;
	out.fetch_us = pkt.fetch_us;
	if (pkt.payload.IsEmpty())
		return true;
	TimeStop ts;
	if (pkt.payload.GetCount() >= 16 && memcmp(~pkt.payload, "YUV0", 4) == 0) {
		const byte* p = (const byte*)~pkt.payload;
		uint32 w = *(const uint32*)(p + 4);
		uint32 h = *(const uint32*)(p + 8);
		uint32 sz = *(const uint32*)(p + 12);
		if ((int)sz <= 0 || pkt.payload.GetCount() < 16 + (int)sz || w == 0 || h == 0)
			return false;
		if ((int)sz != (int)(w * h * 2))
			return false;
		ImageBuffer ib((int)w, (int)h);
		YUYVToImage(p + 16, (int)w, (int)h, ib);
		out.img = ib;
	}
	else {
		out.img = JPGRaster().LoadString(pkt.payload);
	}
	out.decode_us = ts.Elapsed();
	return !out.img.IsEmpty();
}

void ApplyFrameRuleDataToRules(Array<GameRule>& rules, const FrameAnalysisData& data) {
	if (data.rule_data.IsEmpty())
		return;
	for (int i = 0; i < rules.GetCount(); i++) {
		GameRule& r = rules[i];
			for (const FrameRuleData& rd : data.rule_data) {
				if (rd.rule != r.name)
					continue;
				if (!rd.images.IsEmpty())
					r.images <<= rd.images;
				if (rd.has_rect)
					r.rect = rd.rect;
				int qp = r.props.Find("openai-example");
				if (!rd.openai_example.IsEmpty())
					r.props.GetAdd("openai-example") = rd.openai_example;
				else if (qp >= 0)
					r.props.Remove(qp);
				break;
			}
		}
	}

void FillMissingRuleImagesFromDirs(Array<GameRule>& rules, const String& project_name, const String& platform_name) {
	for (int i = 0; i < rules.GetCount(); i++) {
		GameRule& r = rules[i];
		if (!GetRuleImagesRef(r).IsEmpty())
			continue;
		String dir = GetRuleSamplesDir(project_name, platform_name, r.name);
		if (!DirectoryExists(dir))
			continue;
		FindFile ff(AppendFileName(dir, "*.png"));
		while (ff) {
			if (ff.IsFile()) {
				String rel = MakeProjectRelative(project_name, AppendFileName(dir, ff.GetName()));
				r.images.Add(rel);
			}
			ff.Next();
		}
	}
}

void ListPngFiles(const String& dir, Vector<String>& out) {
	out.Clear();
	if (!DirectoryExists(dir))
		return;
	FindFile ff(AppendFileName(dir, "*.png"));
	while (ff) {
		if (ff.IsFile())
			out.Add(AppendFileName(dir, ff.GetName()));
		ff.Next();
	}
	Sort(out);
}

static Image ComposeSyntheticFrame(const Image& overlay, int x, int y, Size frame_sz = Size(1920, 1080), Color bg = GrayColor(128)) {
	ImageBuffer ib(frame_sz);
	for (int yy = 0; yy < frame_sz.cy; yy++) {
		RGBA* row = ib[yy];
		for (int xx = 0; xx < frame_sz.cx; xx++) {
			row[xx].r = bg.GetR();
			row[xx].g = bg.GetG();
			row[xx].b = bg.GetB();
			row[xx].a = 255;
		}
	}
	if (!overlay.IsEmpty()) {
		Size os = overlay.GetSize();
		for (int oy = 0; oy < os.cy; oy++) {
			int yy = y + oy;
			if (yy < 0 || yy >= frame_sz.cy)
				continue;
			const RGBA* s = overlay[oy];
			RGBA* d = ib[yy];
			for (int ox = 0; ox < os.cx; ox++) {
				int xx = x + ox;
				if (xx < 0 || xx >= frame_sz.cx)
					continue;
				d[xx] = s[ox];
			}
		}
	}
	return ib;
}

static void MaskRectImage(Image& img, Rect r) {
	if (img.IsEmpty()) return;
	r.Intersect(img.GetSize());
	if (r.IsEmpty()) return;
	ImageBuffer ib(img);
	for (int y = r.top; y < r.bottom; y++) {
		RGBA* row = ib[y];
		for (int x = r.left; x < r.right; x++) row[x] = Black();
	}
	img = ib;
}

static double IoURect(const Rect& a, const Rect& b) {
	Rect inter = a & b;
	if (inter.IsEmpty()) return 0.0;
	double inter_area = (double)inter.GetWidth() * inter.GetHeight();
	double area_a = (double)a.GetWidth() * a.GetHeight();
	double area_b = (double)b.GetWidth() * b.GetHeight();
	double uni = area_a + area_b - inter_area;
	if (uni <= 0.0) return 0.0;
	return inter_area / uni;
}

bool g_match_fast_exec = false;
bool g_match_profile_exec = false;
static void CliPrint(const String& s);

static int PixelDiffRGB(const RGBA& a, const RGBA& b) {
	return abs((int)a.r - (int)b.r) + abs((int)a.g - (int)b.g) + abs((int)a.b - (int)b.b);
}

static int PixelDiffChannel(const RGBA& a, const RGBA& b, int ch) {
	switch (ch) {
	case 0: return abs((int)a.r - (int)b.r);
	case 1: return abs((int)a.g - (int)b.g);
	case 2: return abs((int)a.b - (int)b.b);
	default: return PixelDiffRGB(a, b);
	}
}

static int ChannelFromName(const String& s) {
	String n = ToLower(TrimBoth(s));
	if (n == "r" || n == "red")
		return 0;
	if (n == "g" || n == "green")
		return 1;
	if (n == "b" || n == "blue")
		return 2;
	return -1;
}

static String ChannelNameFromId(int ch) {
	switch (ch) {
	case 0: return "r";
	case 1: return "g";
	case 2: return "b";
	default: return "g";
	}
}

static int ComputeDominantChannel(const Image& img, RuleImageStats& out) {
	if (img.IsEmpty())
		return 1;
	double sum_r = 0, sum_g = 0, sum_b = 0;
	const int w = img.GetWidth();
	const int h = img.GetHeight();
	const int sx = max(1, w / 64);
	const int sy = max(1, h / 64);
	int n = 0;
	for (int y = 0; y < h; y += sy) {
		const RGBA* row = img[y];
		for (int x = 0; x < w; x += sx) {
			sum_r += row[x].r;
			sum_g += row[x].g;
			sum_b += row[x].b;
			n++;
		}
	}
	n = max(1, n);
	double mean_r = sum_r / n;
	double mean_g = sum_g / n;
	double mean_b = sum_b / n;
	double var_r = 0, var_g = 0, var_b = 0;
	for (int y = 0; y < h; y += sy) {
		const RGBA* row = img[y];
		for (int x = 0; x < w; x += sx) {
			double dr = row[x].r - mean_r;
			double dg = row[x].g - mean_g;
			double db = row[x].b - mean_b;
			var_r += dr * dr;
			var_g += dg * dg;
			var_b += db * db;
		}
	}
	var_r /= n;
	var_g /= n;
	var_b /= n;
	out.variance_r = var_r;
	out.variance_g = var_g;
	out.variance_b = var_b;
	int ch = 1;
	double best = var_g;
	if (var_r > best) { best = var_r; ch = 0; }
	if (var_b > best) { ch = 2; }
	out.dominant_channel = ChannelNameFromId(ch);
	return ch;
}

static int GetDominantChannelForImage(const String& image_path, const Image& img, VectorMap<String, int>* channel_cache) {
	if (channel_cache) {
		int q = channel_cache->Find(image_path);
		if (q >= 0)
			return (*channel_cache)[q];
	}
	String json_path = ForceExt(image_path, ".json");
	RuleImageStats stats;
	int ch = -1;
	if (FileExists(json_path)) {
		try {
			LoadFromJson(stats, LoadFile(json_path));
			ch = ChannelFromName(stats.dominant_channel);
		}
		catch (...) {
			ch = -1;
		}
	}
	if (ch < 0) {
		ch = ComputeDominantChannel(img, stats);
		stats.dominant_channel = ChannelNameFromId(ch);
		SaveFile(json_path, StoreAsJson(stats));
	}
	if (channel_cache)
		channel_cache->Add(image_path, ch);
	return ch;
}

static Vector<Point> BuildProbePoints(int w, int h) {
	Vector<Point> pts;
	if (w <= 0 || h <= 0)
		return pts;
	pts.Reserve(128);
	pts.Add(Point(0, 0));
	pts.Add(Point(w - 1, 0));
	pts.Add(Point(0, h - 1));
	pts.Add(Point(w - 1, h - 1));
	pts.Add(Point(w / 2, h / 2));
	pts.Add(Point(w / 2, 0));
	pts.Add(Point(w / 2, h - 1));
	pts.Add(Point(0, h / 2));
	pts.Add(Point(w - 1, h / 2));
	int sx = max(1, w / 14);
	int sy = max(1, h / 14);
	for (int y = 0; y < h && pts.GetCount() < 140; y += sy)
		for (int x = 0; x < w && pts.GetCount() < 140; x += sx)
			pts.Add(Point(x, y));
	return pts;
}

static bool MatchPatternAtProbes(const Image& frame, const Image& pattern, const Vector<Point>& probes, int sx, int sy, int ch, int64 best_err, int64& err, int& samples) {
	err = 0;
	samples = 0;
	if (probes.IsEmpty())
		return false;
	for (const Point& p : probes) {
		err += PixelDiffChannel(frame[sy + p.y][sx + p.x], pattern[p.y][p.x], ch);
		samples++;
		if (err >= best_err)
			return false;
	}
	return true;
}

static bool MatchPatternAtSamples(const Image& frame, const Image& pattern, int sx, int sy, int step, int ch, int64 best_err, int64& err, int& samples) {
	const int pw = pattern.GetWidth();
	const int ph = pattern.GetHeight();
	err = 0;
	samples = 0;
	if (pw <= 0 || ph <= 0)
		return false;
	step = max(step, 1);
	for (int y = 0; y < ph; y += step) {
		for (int x = 0; x < pw; x += step) {
			err += PixelDiffChannel(frame[sy + y][sx + x], pattern[y][x], ch);
			samples++;
			if (err >= best_err)
				return false;
		}
	}
	return true;
}

static bool MatchPatternAt(const Image& frame, const Image& pattern, int sx, int sy, int ch, int64 best_err, int64& err, int& samples) {
	const int pw = pattern.GetWidth();
	const int ph = pattern.GetHeight();
	err = 0;
	samples = 0;
	if (pw <= 0 || ph <= 0)
		return false;
	const int edge_step = 3;
	const int inner_step = 3;
	for (int x = 0; x < pw; x += edge_step) {
		err += PixelDiffChannel(frame[sy][sx + x], pattern[0][x], ch);
		err += PixelDiffChannel(frame[sy + ph - 1][sx + x], pattern[ph - 1][x], ch);
		samples += 2;
		if (err >= best_err)
			return false;
	}
	for (int y = edge_step; y < ph - edge_step; y += edge_step) {
		err += PixelDiffChannel(frame[sy + y][sx], pattern[y][0], ch);
		err += PixelDiffChannel(frame[sy + y][sx + pw - 1], pattern[y][pw - 1], ch);
		samples += 2;
		if (err >= best_err)
			return false;
	}
	for (int y = inner_step; y < ph - inner_step; y += inner_step) {
		for (int x = inner_step; x < pw - inner_step; x += inner_step) {
			err += PixelDiffChannel(frame[sy + y][sx + x], pattern[y][x], ch);
			samples++;
			if (err >= best_err)
				return false;
		}
	}
	return true;
}

static bool FindOrbMatchesForRule(const String& project_name, const GameRule& rule, const Image& img, Vector<Rect>& out, Vector<double>& scores, const Rect* roi = nullptr, VectorMap<String, Image>* image_cache = nullptr, OrbCacheStats* cache_stats = nullptr, const Vector<GpuKp>* gpu_keypoints = nullptr, const Vector<BinDescriptor>* gpu_descriptors = nullptr) {
	out.Clear();
	scores.Clear();
	const Vector<String>& imgs = GetRuleImagesRef(rule);
	if (img.IsEmpty() || imgs.IsEmpty()) return false;
	TimeStop total_ts;
	VectorMap<String, int> channel_cache;
	if (gpu_descriptors && !gpu_descriptors->IsEmpty()) {
		for (const String& filename : imgs) {
			String path = ResolveRuleImagePath(project_name, filename);
			Image pattern;
			if (image_cache) {
				int ci = image_cache->Find(path);
				if (ci < 0) {
					Image loaded = LoadImageForPath(path);
					if (loaded.IsEmpty()) continue;
					ci = image_cache->GetCount();
					image_cache->Add(path, loaded);
				}
				pattern = (*image_cache)[ci];
			} else pattern = LoadImageForPath(path);
			if (pattern.IsEmpty()) continue;
			
			OrbSystem orb;
			orb.SetInput(pattern);
			orb.InitDefault();
			
			// Full GPU path (all levels at once)
			Rect roi_rect = roi ? *roi : Rect(img.GetSize());
			orb.ProcessGpu(*gpu_keypoints, *gpu_descriptors, roi_rect);
			
			if (orb.GetLastGoodMatches() > 4) {
				const Vector<Pointf>& corners = orb.GetLastCorners();
				if (corners.GetCount() == 4) {
					float minx = corners[0].x, maxx = corners[0].x;
					float miny = corners[0].y, maxy = corners[0].y;
					for (int i = 1; i < 4; i++) {
						minx = min(minx, (float)corners[i].x);
						maxx = max(maxx, (float)corners[i].x);
						miny = min(miny, (float)corners[i].y);
						maxy = max(maxy, (float)corners[i].y);
					}
					out.Add(Rect((int)minx, (int)miny, (int)maxx, (int)maxy));
					scores.Add((double)orb.GetLastGoodMatches());
				}
			}
		}
		if (g_match_profile_exec)
			CliPrint("perf orb_rule_gpu " + rule.name + " us=" + AsString(total_ts.Elapsed()) + " matches=" + AsString(out.GetCount()));
		return !out.IsEmpty();
	}
	if (g_match_fast_exec) {
		const Rect frame_rc = img.GetSize();
		Image img_small_025;
		bool img_small_025_ready = false;
		for (const String& filename : imgs) {
			String path = ResolveRuleImagePath(project_name, filename);
			Image pattern;
			if (image_cache) {
				int ci = image_cache->Find(path);
				if (ci < 0) {
					if (cache_stats) cache_stats->misses++;
					Image loaded = LoadImageForPath(path);
					if (loaded.IsEmpty())
						continue;
					ci = image_cache->GetCount();
					image_cache->Add(path, loaded);
				}
				else {
					if (cache_stats) cache_stats->hits++;
				}
				pattern = (*image_cache)[ci];
			}
			else
				pattern = LoadImageForPath(path);
			if (pattern.IsEmpty())
				continue;
			int dominant_ch = GetDominantChannelForImage(path, pattern, &channel_cache);
			int pw = pattern.GetWidth();
			int ph = pattern.GetHeight();
			if (pw <= 0 || ph <= 0 || pw > img.GetWidth() || ph > img.GetHeight())
				continue;
			Rect sr = roi ? *roi : frame_rc;
			sr.Intersect(frame_rc);
			if (sr.IsEmpty())
				continue;
			int minx = max(sr.left, 0);
			int miny = max(sr.top, 0);
			int maxx = min(sr.right - pw, img.GetWidth() - pw);
			int maxy = min(sr.bottom - ph, img.GetHeight() - ph);
			if (minx > maxx || miny > maxy)
				continue;
			bool found = false;
			int64 best_err = INT64_MAX;
			int best_samples = 0;
			Rect best_rect;
			bool exact_seed = false;
			int coarse = (max(pw, ph) >= 400 ? 8 : 4);
			Vector<Point> probes = BuildProbePoints(pw, ph);
			const int search_area = (maxx - minx + 1) * (maxy - miny + 1);
			if (pw * ph >= 180000 && search_area >= 900000) {
				const double sc = 0.25;
				if (!img_small_025_ready) {
					const int sw0 = max(1, (int)floor(img.GetWidth() * sc));
					const int sh0 = max(1, (int)floor(img.GetHeight() * sc));
					img_small_025 = Rescale(img, Size(sw0, sh0));
					img_small_025_ready = true;
				}
				const int sw = img_small_025.GetWidth();
				const int sh = img_small_025.GetHeight();
				const int spw = max(1, (int)floor(pw * sc));
				const int sph = max(1, (int)floor(ph * sc));
				Image pat_small = Rescale(pattern, Size(spw, sph));
				int sminx = max(0, (int)floor(minx * sc));
				int sminy = max(0, (int)floor(miny * sc));
				int smaxx = min(sw - spw, (int)ceil(maxx * sc));
				int smaxy = min(sh - sph, (int)ceil(maxy * sc));
				if (sminx <= smaxx && sminy <= smaxy) {
					bool sfound = false;
					int64 sbest_err = INT64_MAX;
					Rect sbest_rect;
					for (int y = sminy; y <= smaxy; y += 2) {
						for (int x = sminx; x <= smaxx; x += 2) {
							int64 err = 0;
							int samples = 0;
							if (!MatchPatternAtSamples(img_small_025, pat_small, x, y, 2, dominant_ch, sbest_err, err, samples))
								continue;
							if (!sfound || err < sbest_err) {
								sfound = true;
								sbest_err = err;
								sbest_rect = RectC(x, y, spw, sph);
							}
						}
					}
					if (sfound) {
						int cx = (int)round((double)sbest_rect.left / sc);
						int cy = (int)round((double)sbest_rect.top / sc);
						const int radius = 96;
						minx = max(minx, cx - radius);
						miny = max(miny, cy - radius);
						maxx = min(maxx, cx + radius);
						maxy = min(maxy, cy + radius);
						if (minx > maxx || miny > maxy)
							continue;
					}
				}
			}
			// For tight ROI tracking, try center position first.
			if (roi && !found) {
				int cx = minx + (maxx - minx) / 2;
				int cy = miny + (maxy - miny) / 2;
				int64 err = 0;
				int samples = 0;
				if (MatchPatternAt(img, pattern, cx, cy, dominant_ch, INT64_MAX, err, samples) && samples > 0) {
					double norm = (double)err / (double)samples;
					if (norm < 18.0) {
						found = true;
						exact_seed = true;
						best_err = err;
						best_samples = samples;
						best_rect = RectC(cx, cy, pw, ph);
					}
				}
			}
			const int row_count = ((maxy - miny) / coarse) + 1;
			const bool use_parallel = search_area >= 400000 && row_count > 16;
			if (!found && use_parallel) {
					int workers = min(max(1, CoWork::GetPoolSize() + 1), row_count);
					Vector<int64> worker_err;
					Vector<int> worker_samples;
					Vector<Rect> worker_rect;
				worker_err.SetCount(workers);
				worker_samples.SetCount(workers);
				worker_rect.SetCount(workers);
				for (int i = 0; i < workers; i++) {
					worker_err[i] = INT64_MAX;
					worker_samples[i] = 0;
					worker_rect[i] = Rect(0, 0, 0, 0);
					}
					std::atomic<int> next_row(0);
					CoWork cw;
					for (int wi = 0; wi < workers; wi++) {
						cw & [&, wi] {
							int64 local_best = INT64_MAX;
							int local_samples = 0;
						Rect local_rect;
						while (true) {
							int r = next_row.fetch_add(1, std::memory_order_relaxed);
								if (r >= row_count)
									break;
								int y = miny + r * coarse;
								for (int x = minx; x <= maxx; x += coarse) {
									int64 err = 0;
									int samples = 0;
									if (!MatchPatternAtProbes(img, pattern, probes, x, y, dominant_ch, local_best, err, samples))
										continue;
									if (err < local_best) {
										local_best = err;
										local_samples = samples;
										local_rect = RectC(x, y, pw, ph);
									}
								}
							}
						worker_err[wi] = local_best;
						worker_samples[wi] = local_samples;
						worker_rect[wi] = local_rect;
					};
				}
				cw.Finish();
				for (int wi = 0; wi < workers; wi++) {
					if (worker_err[wi] < best_err) {
						found = true;
						best_err = worker_err[wi];
						best_samples = worker_samples[wi];
						best_rect = worker_rect[wi];
					}
				}
			}
			else if (!found) {
				for (int y = miny; y <= maxy; y += coarse) {
					for (int x = minx; x <= maxx; x += coarse) {
						int64 err = 0;
						int samples = 0;
						if (!MatchPatternAtProbes(img, pattern, probes, x, y, dominant_ch, best_err, err, samples))
							continue;
						if (!found || err < best_err) {
							found = true;
							best_err = err;
							best_samples = samples;
							best_rect = RectC(x, y, pw, ph);
						}
					}
				}
			}
			if (found) {
				if (!(exact_seed && roi && maxx == minx && maxy == miny)) {
					int refine = max(coarse * 2, 8);
					int rx0 = max(minx, best_rect.left - refine);
					int ry0 = max(miny, best_rect.top - refine);
					int rx1 = min(maxx, best_rect.left + refine);
					int ry1 = min(maxy, best_rect.top + refine);
					best_err = INT64_MAX;
					best_samples = 0;
					for (int y = ry0; y <= ry1; y++) {
						for (int x = rx0; x <= rx1; x++) {
							int64 err = 0;
							int samples = 0;
							if (!MatchPatternAt(img, pattern, x, y, dominant_ch, best_err, err, samples))
								continue;
							if (err < best_err) {
								best_err = err;
								best_samples = samples;
								best_rect = RectC(x, y, pw, ph);
							}
						}
					}
				}
				out.Add(best_rect);
				double norm = (best_samples > 0) ? ((double)best_err / (double)best_samples) : 1e9;
				scores.Add(1.0 / (1.0 + norm));
			}
		}
		if (g_match_profile_exec)
			CliPrint("perf orb_rule_fast " + rule.name + " us=" + AsString(total_ts.Elapsed()) + " matches=" + AsString(out.GetCount()));
		return !out.IsEmpty();
	}
	Vector<Image> levels;
	Vector<double> level_scales;
	levels.Add(img);
	level_scales.Add(1.0);
	const double extra_scales[2] = {0.75, 0.5};
	for (int i = 0; i < 2; i++) {
		double sc = extra_scales[i];
		int w = max(1, (int)floor(img.GetWidth() * sc));
		int h = max(1, (int)floor(img.GetHeight() * sc));
		if (w == img.GetWidth() && h == img.GetHeight())
			continue;
		levels.Add(Rescale(img, Size(w, h)));
		level_scales.Add(sc);
	}
	const int max_matches_per_sample = 3;
	for (const String& filename : imgs) {
		String path = ResolveRuleImagePath(project_name, filename);
		Image pattern;
		if (image_cache) {
			int ci = image_cache->Find(path);
			if (ci < 0) {
				Image loaded = LoadImageForPath(path);
				if (loaded.IsEmpty())
					continue;
				ci = image_cache->GetCount();
				image_cache->Add(path, loaded);
			}
			pattern = (*image_cache)[ci];
		}
		else
			pattern = LoadImageForPath(path);
		if (pattern.IsEmpty()) continue;
		OrbSystem orb;
		orb.SetInput(pattern);
		orb.InitDefault();
		for (int li = 0; li < levels.GetCount(); li++) {
			Image work = levels[li];
			double sc = level_scales[li];
			double inv = sc > 0.0 ? (1.0 / sc) : 1.0;
			for (int attempt = 0; attempt < max_matches_per_sample; attempt++) {
				orb.SetInput(work);
                                if (roi) {
                                    Rect r = *roi;
                                    if (sc != 1.0) {
                                        r.left = (int)(r.left * sc);
                                        r.top = (int)(r.top * sc);
                                        r.right = (int)(r.right * sc);
                                        r.bottom = (int)(r.bottom * sc);
                                    }
                                    orb.ProcessROI(r);
                                } else {
                                    orb.Process();
                                }
				if (orb.GetLastGoodMatches() <= 4) break;
				const Vector<Pointf>& corners = orb.GetLastCorners();
				if (corners.GetCount() < 4) break;
				double minx = corners[0].x, miny = corners[0].y;
				double maxx = corners[0].x, maxy = corners[0].y;
				for (int i = 1; i < corners.GetCount(); i++) {
					minx = min(minx, (double)corners[i].x);
					miny = min(miny, (double)corners[i].y);
					maxx = max(maxx, (double)corners[i].x);
					maxy = max(maxy, (double)corners[i].y);
				}
				Rect rs((int)floor(minx), (int)floor(miny), (int)ceil(maxx), (int)ceil(maxy));
				if (rs.IsEmpty()) {
					int px = (int)floor(minx);
					int py = (int)floor(miny);
					rs = RectC(px, py, 1, 1);
				}
				Rect r((int)floor(rs.left * inv), (int)floor(rs.top * inv),
				       (int)ceil(rs.right * inv), (int)ceil(rs.bottom * inv));
				if (r.IsEmpty())
					r = RectC((int)floor(rs.left * inv), (int)floor(rs.top * inv), 1, 1);
				bool duplicate = false;
				for (const Rect& prev : out) {
					if (IoURect(prev, r) > 0.6) { duplicate = true; break; }
				}
				if (!duplicate) {
					out.Add(r);
					scores.Add((double)orb.GetLastGoodMatches());
				}
				MaskRectImage(work, rs);
			}
		}
	}
	if (g_match_profile_exec)
		CliPrint("perf orb_rule " + rule.name + " us=" + AsString(total_ts.Elapsed()) + " matches=" + AsString(out.GetCount()));
	return !out.IsEmpty();
}

static bool FindBruteforceMatchForRule(const String& project_name, const GameRule& rule, const Image& frame, const Rect* search, Rect& out, double& score) {
	out = Rect(0, 0, 0, 0);
	score = 0.0;
	if (frame.IsEmpty())
		return false;
	const Vector<String>& imgs = GetRuleImagesRef(rule);
	if (imgs.IsEmpty())
		return false;
	const Rect frame_rc = frame.GetSize();
	bool found = false;
	int64 best_err = INT64_MAX;
	int best_samples = 0;
	Rect best_rect;
	for (const String& filename : imgs) {
		String path = ResolveRuleImagePath(project_name, filename);
		Image pattern = LoadImageForPath(path);
		if (pattern.IsEmpty())
			continue;
		const int pw = pattern.GetWidth();
		const int ph = pattern.GetHeight();
		if (pw <= 0 || ph <= 0 || pw > frame.GetWidth() || ph > frame.GetHeight())
			continue;
		Rect sr = search ? *search : frame_rc;
		sr.Intersect(frame_rc);
		if (sr.IsEmpty())
			continue;
		int minx = max(sr.left, 0);
		int miny = max(sr.top, 0);
		int maxx = min(sr.right - pw, frame.GetWidth() - pw);
		int maxy = min(sr.bottom - ph, frame.GetHeight() - ph);
		if (minx > maxx || miny > maxy)
			continue;
		int coarse = 4;
		for (int y = miny; y <= maxy; y += coarse) {
			for (int x = minx; x <= maxx; x += coarse) {
				int64 err = 0;
				int samples = 0;
				if (!MatchPatternAt(frame, pattern, x, y, -1, best_err, err, samples))
					continue;
				if (samples <= 0)
					continue;
				if (!found || err < best_err) {
					found = true;
					best_err = err;
					best_samples = samples;
					best_rect = RectC(x, y, pw, ph);
				}
			}
		}
		if (found) {
			int rx0 = max(minx, best_rect.left - coarse);
			int ry0 = max(miny, best_rect.top - coarse);
			int rx1 = min(maxx, best_rect.left + coarse);
			int ry1 = min(maxy, best_rect.top + coarse);
			for (int y = ry0; y <= ry1; y++) {
				for (int x = rx0; x <= rx1; x++) {
					int64 err = 0;
					int samples = 0;
					if (!MatchPatternAt(frame, pattern, x, y, -1, best_err, err, samples))
						continue;
					if (samples <= 0)
						continue;
					if (err < best_err) {
						best_err = err;
						best_samples = samples;
						best_rect = RectC(x, y, pw, ph);
					}
				}
			}
		}
	}
	if (!found || best_samples <= 0)
		return false;
	double avg = (double)best_err / (double)best_samples;
	if (avg > 65.0)
		return false;
	out = best_rect;
	score = 1.0 / (1.0 + avg);
	return true;
}

bool g_cli_verbose = false;
bool g_cli_trace_api = false;
void ScriptLogStdout(String s) {
	if (!g_cli_verbose && !g_cli_trace_api) return;
	Cout() << s << "\n";
}

static void CliPrint(const String& s) {
	Cout() << s << "\n";
	Cout().Flush();
	VppLog() << s;
}

VectorMap<String, String> BuildDynamicPropsFromRects(const Vector<Rect>& rects) {
	VectorMap<String, String> out;
	out.Add("window_rects", AsString(rects.GetCount()));
	for (int i = 0; i < rects.GetCount(); i++) {
		const Rect& r = rects[i];
		String p = Format("window[%d].", i);
		out.Add(p + "x", AsString(r.left));
		out.Add(p + "y", AsString(r.top));
		out.Add(p + "w", AsString(r.GetWidth()));
		out.Add(p + "h", AsString(r.GetHeight()));
	}
	return out;
}

static void DrawRectBorder(ImageBuffer& ib, const Rect& rr, const RGBA& c) {
	Rect r = rr;
	r.Intersect(ib.GetSize());
	if (r.IsEmpty()) return;
	for (int x = r.left; x < r.right; x++) {
		ib[r.top][x] = c;
		ib[r.bottom - 1][x] = c;
	}
	for (int y = r.top; y < r.bottom; y++) {
		ib[y][r.left] = c;
		ib[y][r.right - 1] = c;
	}
}

static RGBA RuleColor(const String& rule_name) {
	RGBA c = RGBAZero();
	c.a = 255;
	if (rule_name == "top-left")      { c.r = 0;   c.g = 255; c.b = 0;   return c; }
	if (rule_name == "top-right")     { c.r = 0;   c.g = 160; c.b = 255; return c; }
	if (rule_name == "bottom-left 1") { c.r = 255; c.g = 255; c.b = 0;   return c; }
	if (rule_name == "bottom-right 1"){ c.r = 255; c.g = 0;   c.b = 255; return c; }
	if (rule_name == "window-size")   { c.r = 0;   c.g = 255; c.b = 255; return c; }
	c.r = 255; c.g = 160; c.b = 0;
	return c;
}

Image RenderPreviewWithRects(const Image& img, const Vector<Rect>& rects) {
	if (img.IsEmpty())
		return Image();
	Image src = img;
	ImageBuffer ib(src);
	RGBA red = RGBAZero();
	red.r = 255; red.g = 0; red.b = 0; red.a = 255;
	for (const Rect& r : rects) {
		DrawRectBorder(ib, r, red);
		DrawRectBorder(ib, r.Deflated(1), red);
	}
	return Image(ib);
}

String SavePreviewWithRectsJpg(const Image& img, const Vector<Rect>& rects, const String& export_path) {
	if (img.IsEmpty())
		return String();
	String out = export_path;
	if (out.IsEmpty()) {
		String dir = AppendFileName(GetCurrentDirectory(), "tmp");
		RealizeDirectory(dir);
		out = AppendFileName(dir, "exec_preview_rects.jpg");
	}
	Image preview = RenderPreviewWithRects(img, rects);
	JPGEncoder().Quality(100).SaveFile(out, preview);
	return out;
}

static String SavePreviewDebugJpg(const Image& img,
                                  const Vector<Rect>& window_rects,
                                  const VectorMap<String, Vector<Rect>>& rule_matches,
                                  const String& export_path) {
	if (img.IsEmpty())
		return String();
	String out = export_path;
	if (out.IsEmpty()) {
		String dir = AppendFileName(GetCurrentDirectory(), "tmp");
		RealizeDirectory(dir);
		out = AppendFileName(dir, "exec_preview_rects.jpg");
	}
	Image base = RenderPreviewWithRects(img, window_rects);
	if (base.IsEmpty())
		base = img;
	ImageBuffer ib(base);
	for (int i = 0; i < rule_matches.GetCount(); i++) {
		String rule = rule_matches.GetKey(i);
		const Vector<Rect>& rects = rule_matches[i];
		RGBA c = RuleColor(rule);
		for (const Rect& rr : rects) {
			Rect r = rr;
			if (r.GetWidth() <= 2 || r.GetHeight() <= 2) {
				int cx = r.left + r.GetWidth() / 2;
				int cy = r.top + r.GetHeight() / 2;
				r = RectC(cx - 4, cy - 4, 9, 9);
			}
			DrawRectBorder(ib, r, c);
		}
	}
	JPGEncoder().Quality(100).SaveFile(out, ib);
	return out;
}

static Vector<String> ParseCommandLineText(const String& text) {
	Vector<String> out;
	String cleaned;
	Vector<String> lines = Split(text, '\n');
	for (const String& line : lines) {
		String t = TrimBoth(line);
		if (t.IsEmpty()) continue;
		if (t.StartsWith("#") || t.StartsWith(";")) continue;
		cleaned << t << " ";
	}
	String cur;
	bool in_quote = false;
	int quote = 0;
	for (int i = 0; i < cleaned.GetCount(); i++) {
		int ch = cleaned[i];
		if (in_quote) {
			if (ch == quote) in_quote = false;
			else cur.Cat(ch);
			continue;
		}
		if (ch == '"' || ch == '\'') { in_quote = true; quote = ch; continue; }
		if (IsSpace(ch)) {
			if (!cur.IsEmpty()) { out.Add(cur); cur.Clear(); }
			continue;
		}
		cur.Cat(ch);
	}
	if (!cur.IsEmpty()) out.Add(cur);
	return out;
}

static String GetDefaultIniPath() {
	String exe = GetExeFilePath();
	String dir = GetFileDirectory(exe);
	String title = GetFileTitle(exe);
	if (title.IsEmpty()) title = "ScreenGame";
	return AppendFileName(dir, title + ".ini");
}

static bool IniValueTrue(const String& value) {
	String s = ToLower(TrimBoth(value));
	return s == "1" || s == "true" || s == "yes" || s == "on";
}

static Image MakeTestPattern48() {
	ImageBuffer ib(48, 48);
	for (int y = 0; y < 48; y++) {
		RGBA* row = ib[y];
		for (int x = 0; x < 48; x++) row[x] = White();
	}
	for (int y = 0; y < 6; y++)
	for (int x = 0; x < 6; x++)
	if ((x + y) & 1)
		for (int yy = 0; yy < 8; yy++) {
			RGBA* row = ib[y * 8 + yy];
			for (int xx = 0; xx < 8; xx++) row[x * 8 + xx] = Black();
		}
	for (int i = 0; i < 48; i++) { ib[i][i] = Black(); ib[i][47 - i] = Black(); }
	int cx = 24, cy = 24, r = 12;
	for (int y = 0; y < 48; y++)
	for (int x = 0; x < 48; x++) {
		int dx = x - cx, dy = y - cy;
		int d2 = dx * dx + dy * dy;
		if (d2 >= r * r - 2 && d2 <= r * r + 2) ib[y][x] = Black();
	}
	int seed = 12345;
	for (int i = 0; i < 200; i++) {
		seed = (seed * 1103515245 + 12345) & 0x7fffffff;
		int x = seed % 48;
		seed = (seed * 1103515245 + 12345) & 0x7fffffff;
		int y = seed % 48;
		ib[y][x] = Black();
	}
	return Image(ib);
}

static Image MakeTestScene(const Image& pattern, Point at) {
	ImageBuffer ib(640, 480);
	for (int y = 0; y < 480; y++) {
		RGBA* row = ib[y];
		for (int x = 0; x < 640; x++) row[x] = White();
	}
	for (int y = 0; y < pattern.GetHeight(); y++) {
		if (at.y + y < 0 || at.y + y >= 480) continue;
		RGBA* dst = ib[at.y + y];
		const RGBA* src = pattern[y];
		for (int x = 0; x < pattern.GetWidth(); x++) {
			int dx = at.x + x;
			if (dx < 0 || dx >= 640) continue;
			dst[dx] = src[x];
		}
	}
	return Image(ib);
}

static bool RunOrbTestCase(const Image& pattern, const Image& scene, const char* label, bool use_gpu = false) {
	Cout() << label << " pattern=" << pattern.GetSize() << " scene=" << scene.GetSize() << " gpu=" << (int)use_gpu << "\n";
	
	OrbSystem orb;
	orb.SetInput(pattern);
	orb.InitDefault();
	
	if (use_gpu) {
		const GpuPreprocessStats* p_stats = nullptr;
		int64 orb_us = 0;
		{
			GpuPreprocessEngine gpu;
			GpuPreprocessConfig cfg;
			cfg.backend = "egl-gl";
			cfg.route = "glx-x11";
			cfg.compact_readback_only = true;
			if (!gpu.Initialize(cfg)) {
				Cout() << label << " [GPU] FAILED to initialize; error: " << gpu.GetLastError() << "\n";
				return false;
			}
			if (!gpu.PrepareFrame(scene)) {
				Cout() << label << " [GPU] FAILED to prepare frame; error: " << gpu.GetLastError() << "\n";
				return false;
			}
			if (!gpu.ComputeScoreMaps()) {
				Cout() << label << " [GPU] FAILED to compute score maps; error: " << gpu.GetLastError() << "\n";
				return false;
			}
			
			Vector<GpuKp> gkps;
			if (!gpu.GetKeypoints(gkps)) {
				Cout() << label << " [GPU] FAILED to get keypoints; error: " << gpu.GetLastError() << "\n";
				return false;
			}
			
			Vector<BinDescriptor> descrs;
			if (!gpu.ComputeDescriptors(gkps, descrs)) {
				Cout() << label << " [GPU] FAILED to compute descriptors; error: " << gpu.GetLastError() << "\n";
				return false;
			}
			
			const GpuPreprocessStats& stats = gpu.GetStats();
			TimeStop ts;
			orb.ProcessGpu(gkps, descrs, Rect(0, 0, stats.width, stats.height));
			orb_us = ts.Elapsed();
			
			Cout() << label << " [GPU] matches=" << orb.GetLastMatchCount() 
			       << " good=" << orb.GetLastGoodMatches() 
			       << " corners=" << orb.GetLastCorners().GetCount() 
			       << " gpu_total_us=" << stats.total_us
			       << " scoremap_us=" << stats.scoremap_us
			       << " nms_us=" << stats.nms_us
			       << " describe_us=" << stats.describe_us
			       << " readback_us=" << stats.readback_us
			       << " orb_us=" << orb_us
			       << " readback_bytes=" << stats.readback_bytes 
			       << " backend=" << stats.backend << "/" << stats.backend_route << "\n";
		}
		return orb.GetLastMatchCount() > 0;
	} else {
		TimeStop ts;
		orb.SetInput(scene);
		orb.Process();
		int64 total_us = ts.Elapsed();
		Cout() << label << " [CPU] matches=" << orb.GetLastMatchCount() 
		       << " good=" << orb.GetLastGoodMatches() 
		       << " corners=" << orb.GetLastCorners().GetCount() 
		       << " total_us=" << total_us << "\n";
		return orb.GetLastMatchCount() > 0;
	}
}

int RunOrbSelfTest() {
	Image pattern = MakeTestPattern48();
	Image scene = MakeTestScene(pattern, Point(120, 160));
	Image rotated = RotateClockwise(pattern);
	Image scene_rot = MakeTestScene(rotated, Point(320, 220));
	
	// Base case: CPU only to check for pre-existing leaks
	bool ok1_cpu = RunOrbTestCase(pattern, scene, "orb_test_base_cpu_only", false);
	bool ok2_cpu = RunOrbTestCase(rotated, scene_rot, "orb_test_rot_cpu_only", false);
	
	bool ok1 = RunOrbTestCase(pattern, scene, "orb_test_base", false);
	bool ok1g = RunOrbTestCase(pattern, scene, "orb_test_base_gpu", true);
	bool ok2 = RunOrbTestCase(rotated, scene_rot, "orb_test_rot", false);
	bool ok2g = RunOrbTestCase(rotated, scene_rot, "orb_test_rot_gpu", true);
	bool all_ok = ok1 && ok1g && ok2 && ok2g;
	Cout() << "orb_test_result=" << (all_ok ? "ok" : "fail") << "\n";
	return all_ok ? 0 : 1;
}

#ifndef PLATFORM_WIN32
// V4L2 helpers moved to V4l2Capture.cpp

static bool ValidatePreparedDmaBufFrame(GpuPreprocessEngine& gpu, int width, int height, const char* tag) {
	const GpuPreprocessStats& stats = gpu.GetStats();
	Cout() << "Frame processing stats:\n";
	Cout() << "  upload_bytes: " << stats.upload_bytes << " (expected: 0)\n";
	Cout() << "  upload_copies: " << stats.upload_copies << " (expected: 0)\n";
	Cout() << "  upload_us: " << stats.upload_us << " us\n";
	Cout() << "  grayscale_us: " << stats.grayscale_us << " us\n";
	Cout() << "  pyramid_levels: " << stats.pyramid_levels << "\n";
	Cout() << "  total_us: " << stats.total_us << " us\n";
	Cout() << "  frame_prepared: " << (stats.frame_prepared ? "true" : "false") << "\n";

	bool test_ok = true;
	if (stats.upload_bytes != 0) {
		Cout() << "FAIL: upload_bytes should be 0 (zero-copy), got " << stats.upload_bytes << "\n";
		test_ok = false;
	}
	if (stats.upload_copies != 0) {
		Cout() << "FAIL: upload_copies should be 0, got " << stats.upload_copies << "\n";
		test_ok = false;
	}
	if (!stats.frame_prepared) {
		Cout() << "FAIL: frame should be marked as prepared\n";
		test_ok = false;
	}
	if (stats.pyramid_levels != 3) {
		Cout() << "FAIL: expected 3 pyramid levels, got " << stats.pyramid_levels << "\n";
		test_ok = false;
	}

	ByteMat gray;
	if (!gpu.GetGray(gray)) {
		Cout() << "FAIL: GetGray failed\n";
		test_ok = false;
	} else if (gray.cols != width || gray.rows != height) {
		Cout() << "FAIL: Gray dimensions wrong: " << gray.cols << "x" << gray.rows << "\n";
		test_ok = false;
	} else {
		int mid_y = height / 2;
		byte left_val = gray.data[mid_y * width + min(width - 1, 10)];
		byte mid_val = gray.data[mid_y * width + width / 2];
		byte right_val = gray.data[mid_y * width + max(0, width - 10)];
		Cout() << "Grayscale sample check: left=" << (int)left_val << " mid=" << (int)mid_val << " right=" << (int)right_val << "\n";
		if (tag && *tag)
			Cout() << tag << "_gray_mid=" << (int)mid_val << "\n";
		if (left_val == mid_val && mid_val == right_val) {
			Cout() << "FAIL: grayscale output appears flat\n";
			test_ok = false;
		}
	}
	return test_ok;
}

static bool PrepareGpuFromDmaBufFd(GpuPreprocessEngine& gpu, int dma_fd, int width, int height, int stride, const char* result_tag) {
	GpuFrame frame;
	frame.type = GpuFrame::DMA_BUF;
	frame.dmabuf.fd = dma_fd;
	frame.dmabuf.fourcc = DRM_FORMAT_ARGB8888;
	frame.dmabuf.width = width;
	frame.dmabuf.height = height;
	frame.dmabuf.stride = stride;
	frame.dmabuf.modifier = 0;
	frame.dmabuf.external_format = GpuFrame::EXTERNAL_RGBA8;

	Cout() << "Testing PrepareFrame(GpuFrame) with DMA_BUF...\n";
	if (!gpu.PrepareFrame(frame)) {
		Cout() << result_tag << "=fail (PrepareFrame failed: " << gpu.GetLastError() << ")\n";
		return false;
	}
	return ValidatePreparedDmaBufFrame(gpu, width, height, result_tag);
}

static bool InitGpuForEglImageTest(GpuPreprocessEngine& gpu, const char* result_tag) {
	GpuPreprocessConfig cfg;
	cfg.backend = "egl-gl";
	cfg.route = "glx-x11";
	cfg.compact_readback_only = false;
	cfg.pyramid_levels = 3;

	if (!gpu.Initialize(cfg)) {
		Cout() << result_tag << "=fail (GPU init failed: " << gpu.GetLastError() << ")\n";
		return false;
	}
	if (!gpu.IsAvailable()) {
		Cout() << result_tag << "=fail (GPU not available)\n";
		return false;
	}
	Cout() << "GPU backend initialized: egl-gl/glx-x11\n";
	return true;
}

static int CreateMemFdBuffer(const char* name, size_t size, void*& mapped) {
	#ifndef MFD_CLOEXEC
	#define MFD_CLOEXEC 0x0001U
	#endif

	mapped = MAP_FAILED;
	int memfd = memfd_create(name, MFD_CLOEXEC);
	if (memfd < 0)
		return -1;
	if (ftruncate(memfd, size) < 0) {
		close(memfd);
		return -1;
	}
	mapped = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0);
	if (mapped == MAP_FAILED) {
		close(memfd);
		return -1;
	}
	return memfd;
}

static int xioctl(int fd, unsigned long request, void* arg) {
	int rc;
	do {
		rc = ioctl(fd, request, arg);
	}
	while (rc < 0 && errno == EINTR);
	return rc;
}

// Phase 12 Stage 4: Zero-copy pipeline validation with memfd DMA-BUF
int RunEGLImageSelfTest() {
	Cout() << "=== Zero-Copy Pipeline Test (Phase 12 Stage 4) ===\n";

	GpuPreprocessEngine gpu;
	if (!InitGpuForEglImageTest(gpu, "eglimage_test_result"))
		return 1;

	// Create test pattern (640x480 RGBA)
	const int test_width = 640;
	const int test_height = 480;
	const size_t buffer_size = test_width * test_height * 4;
	void* mapped = MAP_FAILED;
	int memfd = CreateMemFdBuffer("zerocopy_test", buffer_size, mapped);
	if (memfd < 0) {
		Cout() << "eglimage_test_result=fail (memfd_create failed: " << strerror(errno) << ")\n";
		return 1;
	}

	byte* pixels = (byte*)mapped;
	for (int y = 0; y < test_height; y++) {
		for (int x = 0; x < test_width; x++) {
			int idx = (y * test_width + x) * 4;
			byte val = (byte)((x * 255) / test_width);  // Horizontal gradient
			pixels[idx + 0] = val;  // R
			pixels[idx + 1] = val;  // G
			pixels[idx + 2] = val;  // B
			pixels[idx + 3] = 255;  // A
		}
	}
	msync(mapped, buffer_size, MS_SYNC);  // Ensure written to backing store
	munmap(mapped, buffer_size);

	Cout() << "Created test DMA-BUF: fd=" << memfd << " size=" << buffer_size << "\n";
	bool test_ok = PrepareGpuFromDmaBufFd(gpu, memfd, test_width, test_height, test_width * 4, "eglimage_test_result");

	close(memfd);

	if (test_ok) {
		Cout() << "eglimage_test_result=ok (Zero-copy pipeline validated!)\n";
		Cout() << "SUCCESS: PrepareFrame(GpuFrame::DMA_BUF) works with upload_bytes=0\n";
		return 0;
	} else {
		Cout() << "eglimage_test_result=fail (validation checks failed)\n";
		return 1;
	}
}

int RunV4L2EGLImageSelfTest(const String& device_path, const String& format_policy) {
	Cout() << "=== V4L2 Webcam EGLImage Test (Phase 12 Stage 4B) ===\n";
	String dev = device_path.IsEmpty() ? String("/dev/video0") : device_path;
	String policy = ToLower(TrimBoth(format_policy));
	if (policy != "auto" && policy != "low-copy" && policy != "throughput")
		policy = "auto";
	Cout() << "Using V4L2 device: " << dev << "\n";
	Cout() << "Video format policy: " << policy << "\n";

	One<CaptureSource> source(CreateV4l2Capture(dev, policy));
	if (!source || !source->Open()) {
		Cout() << "v4l2_eglimage_test_result=fail (source open failed)\n";
		return 1;
	}

	int rc = 1;
	GpuPreprocessEngine gpu;
	if (!InitGpuForEglImageTest(gpu, "v4l2_direct_test_result")) {
		Cout() << "  stage4d_status: gpu_init_failed\n";
	} else {
		GpuFrame frame;
		if (source->GrabGpuFrame(frame)) {
			bool zero_copy = false;
			bool direct_ok = false;
			if (gpu.PrepareFrame(frame)) {
				const GpuPreprocessStats& stats = gpu.GetStats();
				zero_copy = stats.upload_copies == 0 && stats.upload_bytes == 0;
				Cout() << "Stage 4D: exported DMA-BUF frame prepared successfully\n";
				Cout() << "  stage4d_status: " << (zero_copy ? "direct_import_ok" : "safe_format_fallback_ok") << "\n";
				if (!zero_copy) {
					if (stats.upload_copies == 1 && stats.upload_bytes > 0)
						Cout() << "  stage4f_status: yuyv_lowcopy_manual_ok\n";
					else
						Cout() << "  stage4f_status: yuyv_cpu_conversion_fallback_ok\n";
				}
				Cout() << "  stage4d_upload_bytes: " << stats.upload_bytes << "\n";
				Cout() << "  stage4d_upload_copies: " << stats.upload_copies << "\n";
				direct_ok = true;
			} else {
				String last_err = gpu.GetLastError();
				if (frame.type == GpuFrame::DMA_BUF && last_err.Find("EGL bridge disabled") >= 0) {
					// Direct path failed because bridge is disabled, try GrabFrame as final fallback.
					Image img = source->GrabFrame();
					if (!img.IsEmpty() && gpu.PrepareFrame(img)) {
						const GpuPreprocessStats& stats = gpu.GetStats();
						Cout() << "Stage 4D: frame prepared successfully via GrabFrame fallback\n";
						Cout() << "  stage4d_status: safe_format_fallback_ok\n";
						Cout() << "  stage4f_status: grabframe_fallback_ok\n";
						Cout() << "  stage4d_upload_bytes: " << stats.upload_bytes << "\n";
						Cout() << "  stage4d_upload_copies: " << stats.upload_copies << "\n";
						direct_ok = true;
					}
				}
				
				if (!direct_ok) {
					Cout() << "Stage 4D: direct exported DMA-BUF import failed\n";
					Cout() << "  stage4d_status: direct_import_failed\n";
					Cout() << "  stage4d_error: " << last_err << "\n";
				}
			}
			
			if (frame.type == GpuFrame::DMA_BUF && frame.dmabuf.fd >= 0)
				close(frame.dmabuf.fd);
			
			if (direct_ok) rc = 0;
		} else {
			Cout() << "Stage 4D: GrabGpuFrame failed\n";
		}
	}

	source->Close();
	return rc;
}

 
#endif

int RunJpgSelfTest() {
	ImageBuffer ib(64, 64);
	for (int y = 0; y < 64; y++) {
		RGBA* row = ib[y];
		for (int x = 0; x < 64; x++) { row[x].r = 255; row[x].g = 0; row[x].b = 0; row[x].a = 255; }
	}
	Image img(ib);
	String dir = AppendFileName(GetCurrentDirectory(), "tmp");
	RealizeDirectory(dir);
	String path = AppendFileName(dir, "jpg_test_red.jpg");
	if (!JPGEncoder().Quality(100).SaveFile(path, img)) { Cout() << "jpg_test_result=save_fail\n"; return 1; }
	Image loaded = JPGRaster().LoadFile(path);
	if (loaded.IsEmpty()) { Cout() << "jpg_test_result=load_fail\n"; return 1; }
	const RGBA* p = loaded.Begin();
	if (!p) { Cout() << "jpg_test_result=load_empty\n"; return 1; }
	Cout() << "jpg_test_path=" << path << "\n";
	Cout() << "jpg_test_pixel=" << (int)p->r << "," << (int)p->g << "," << (int)p->b << "," << (int)p->a << "\n";
	bool ok = (p->r > p->g + 20) && (p->r > p->b + 20);
	Cout() << "jpg_test_result=" << (ok ? "ok" : "fail") << "\n";
	return ok ? 0 : 1;
}

int RunPngSelfTest() {
	ImageBuffer ib(64, 64);
	for (int y = 0; y < 64; y++) {
		RGBA* row = ib[y];
		for (int x = 0; x < 64; x++) { row[x].r = 255; row[x].g = 0; row[x].b = 0; row[x].a = 255; }
	}
	Image img(ib);
	String dir = AppendFileName(GetCurrentDirectory(), "tmp");
	RealizeDirectory(dir);
	String path = AppendFileName(dir, "png_test_red.png");
	if (!PNGEncoder().SaveFile(path, img)) { Cout() << "png_test_result=save_fail\n"; return 1; }
	Image loaded = PNGRaster().LoadFile(path);
	if (loaded.IsEmpty()) { Cout() << "png_test_result=load_fail\n"; return 1; }
	const RGBA* p = loaded.Begin();
	if (!p) { Cout() << "png_test_result=load_empty\n"; return 1; }
	Cout() << "png_test_path=" << path << "\n";
	Cout() << "png_test_pixel=" << (int)p->r << "," << (int)p->g << "," << (int)p->b << "," << (int)p->a << "\n";
	bool ok = (p->r > p->g + 20) && (p->r > p->b + 20);
	Cout() << "png_test_result=" << (ok ? "ok" : "fail") << "\n";
	return ok ? 0 : 1;
}

static String CompactOcrTextLocal(const String& s) {
	String out;
	out.Reserve(s.GetCount());
	for (int i = 0; i < s.GetCount(); i++) {
		char c = s[i];
		if (c == '\r' || c == '\n' || c == '\t')
			continue;
		if (c == ' ')
			continue;
		out.Cat(c);
	}
	return out;
}

struct TesseractRuntimeTest {
	typedef void* TessBaseAPI;
#ifdef PLATFORM_WIN32
	HMODULE hTess = NULL;
#else
	void* hTess = NULL;
#endif
	TessBaseAPI (*TessBaseAPICreate)() = nullptr;
	int (*TessBaseAPIInit3)(TessBaseAPI, const char*, const char*) = nullptr;
	void (*TessBaseAPISetImage)(TessBaseAPI, const unsigned char*, int, int, int, int) = nullptr;
	char* (*TessBaseAPIGetUTF8Text)(TessBaseAPI) = nullptr;
	void (*TessBaseAPIDelete)(TessBaseAPI) = nullptr;
	void (*TessDeleteText)(const char*) = nullptr;
	TessBaseAPI api = nullptr;
	bool loaded = false;
	String init_error;

	TesseractRuntimeTest() {
#ifdef PLATFORM_WIN32
		String dllPath = "E:/active/msys64/clang64/bin/libtesseract-5.5.dll";
		hTess = LoadLibraryA(~dllPath);
#else
		String dllPath = "/usr/lib64/libtesseract.so";
		hTess = dlopen(~dllPath, RTLD_LAZY);
#endif
		if (!hTess) {
			init_error = "library_missing";
			return;
		}
#ifdef PLATFORM_WIN32
		#define GET_PROC(x) Tess##x = (decltype(Tess##x))GetProcAddress(hTess, "Tess" #x)
#else
		#define GET_PROC(x) Tess##x = (decltype(Tess##x))dlsym(hTess, "Tess" #x)
#endif
		GET_PROC(BaseAPICreate);
		GET_PROC(BaseAPIInit3);
		GET_PROC(BaseAPISetImage);
		GET_PROC(BaseAPIGetUTF8Text);
		GET_PROC(BaseAPIDelete);
		GET_PROC(DeleteText);
#undef GET_PROC
		if (!TessBaseAPICreate || !TessBaseAPIInit3 || !TessBaseAPISetImage || !TessBaseAPIGetUTF8Text || !TessBaseAPIDelete || !TessDeleteText) {
			init_error = "symbols_missing";
			return;
		}
		api = TessBaseAPICreate();
		if (!api) {
			init_error = "api_create_failed";
			return;
		}
#ifdef PLATFORM_WIN32
		String tessDataPath = "E:/active/msys64/clang64/share/tessdata/";
#else
		String tessDataPath = "/usr/share/tessdata/";
#endif
		if (TessBaseAPIInit3(api, ~tessDataPath, "eng") != 0) {
			init_error = "api_init_failed";
			return;
		}
		loaded = true;
	}

	~TesseractRuntimeTest() {
		if (api && TessBaseAPIDelete)
			TessBaseAPIDelete(api);
		if (hTess) {
#ifdef PLATFORM_WIN32
			FreeLibrary(hTess);
#else
			dlclose(hTess);
#endif
		}
	}

	String Read(const Image& img) {
		if (!loaded || img.IsEmpty())
			return String();
		ImageBuffer ib(img.GetSize());
		for (int y = 0; y < img.GetHeight(); y++) {
			const RGBA* s = img[y];
			RGBA* d = ib[y];
			for (int x = 0; x < img.GetWidth(); x++) {
				int gray = (s[x].r * 77 + s[x].g * 150 + s[x].b * 29) >> 8;
				d[x] = (gray > 160 ? White() : Black());
			}
		}
		Image pre = ib;
		TessBaseAPISetImage(api, (const unsigned char*)~pre, pre.GetWidth(), pre.GetHeight(), 4, pre.GetWidth() * 4);
		char* outText = TessBaseAPIGetUTF8Text(api);
		String out(outText ? outText : "");
		if (outText)
			TessDeleteText(outText);
		return CompactOcrTextLocal(out);
	}
};

int RunTesseractRenderSelfTest() {
	String dir = AppendFileName(GetCurrentDirectory(), "tmp");
	RealizeDirectory(dir);
	String img_path = AppendFileName(dir, "tesseract_render_test.png");
	if (!ImageAnyDraw::IsAvailable()) {
		Cout() << "tesseract_render_test_result=image_draw_unavailable\n";
		return 1;
	}

	ImageAnyDraw iw(460, 140);
	iw.DrawRect(0, 0, 460, 140, White());
	iw.DrawText(20, 40, "HELLO123", Roman(42).Bold(), Black());
	iw.DrawText(250, 92, "POT50", Roman(28).Bold(), Color(180, 0, 0));
	Image img = iw;
	PNGEncoder().SaveFile(img_path, img);

	TesseractRuntimeTest tess;
	if (!tess.loaded) {
		Cout() << "tesseract_render_test_result=load_fail status=" << tess.init_error << "\n";
		return 1;
	}

	String txt = tess.Read(img);
	bool ok = txt.Find("HELLO123") >= 0 && txt.Find("POT") >= 0 && txt.Find("50") >= 0;
	Cout() << "tesseract_render_test_path=" << img_path << "\n";
	Cout() << "tesseract_render_test_text=" << txt << "\n";
	Cout() << "tesseract_render_test_result=" << (ok ? "ok" : "fail") << "\n";
	return ok ? 0 : 1;
}

static void PlotSafe(ImageBuffer& ib, int x, int y, Color c) {
	if (x < 0 || y < 0 || y >= ib.GetHeight() || x >= ib.GetWidth())
		return;
	RGBA& p = ib[y][x];
	p.r = c.GetR();
	p.g = c.GetG();
	p.b = c.GetB();
	p.a = 255;
}

static void DrawLineSimple(ImageBuffer& ib, int x0, int y0, int x1, int y1, Color c) {
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy;
	while (true) {
		PlotSafe(ib, x0, y0, c);
		if (x0 == x1 && y0 == y1)
			break;
		int e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; }
		if (e2 <= dx) { err += dx; y0 += sy; }
	}
}

static void DrawRectStroke(ImageBuffer& ib, int x, int y, int w, int h, Color c) {
	DrawLineSimple(ib, x, y, x + w, y, c);
	DrawLineSimple(ib, x, y + h, x + w, y + h, c);
	DrawLineSimple(ib, x, y, x, y + h, c);
	DrawLineSimple(ib, x + w, y, x + w, y + h, c);
}

static void DrawSegments(ImageBuffer& ib, int x, int y, int w, int h, const char* segs, Color c) {
	bool on[7] = {false,false,false,false,false,false,false}; // a,b,c,d,e,f,g
	for (int i = 0; segs[i]; i++) {
		char s = segs[i];
		if (s >= 'a' && s <= 'g')
			on[s - 'a'] = true;
	}
	if (on[0]) DrawLineSimple(ib, x + 2, y + 1, x + w - 2, y + 1, c);
	if (on[1]) DrawLineSimple(ib, x + w - 1, y + 2, x + w - 1, y + h / 2 - 1, c);
	if (on[2]) DrawLineSimple(ib, x + w - 1, y + h / 2 + 1, x + w - 1, y + h - 2, c);
	if (on[3]) DrawLineSimple(ib, x + 2, y + h - 1, x + w - 2, y + h - 1, c);
	if (on[4]) DrawLineSimple(ib, x + 1, y + h / 2 + 1, x + 1, y + h - 2, c);
	if (on[5]) DrawLineSimple(ib, x + 1, y + 2, x + 1, y + h / 2 - 1, c);
	if (on[6]) DrawLineSimple(ib, x + 2, y + h / 2, x + w - 2, y + h / 2, c);
}

static void DrawGlyphSymbol(ImageBuffer& ib, char ch, int x, int y, int w, int h, Color c) {
	switch (ch) {
	case '2': DrawSegments(ib, x, y, w, h, "abdeg", c); break;
	case '3': DrawSegments(ib, x, y, w, h, "abcdg", c); break;
	case '4': DrawSegments(ib, x, y, w, h, "bcfg", c); break;
	case '5': DrawSegments(ib, x, y, w, h, "acdfg", c); break;
	case '6': DrawSegments(ib, x, y, w, h, "acdefg", c); break;
	case '7': DrawSegments(ib, x, y, w, h, "abc", c); break;
	case '8': DrawSegments(ib, x, y, w, h, "abcdefg", c); break;
	case '9': DrawSegments(ib, x, y, w, h, "abcdfg", c); break;
	case 'T':
		DrawLineSimple(ib, x + 1, y + 1, x + w - 1, y + 1, c);
		DrawLineSimple(ib, x + w / 2, y + 1, x + w / 2, y + h - 1, c);
		break;
	case 'J':
		DrawLineSimple(ib, x + 1, y + 1, x + w - 1, y + 1, c);
		DrawLineSimple(ib, x + w / 2, y + 1, x + w / 2, y + h - 2, c);
		DrawLineSimple(ib, x + 2, y + h - 2, x + w / 2, y + h - 2, c);
		break;
	case 'Q':
		DrawRectStroke(ib, x + 2, y + 2, w - 4, h - 6, c);
		DrawLineSimple(ib, x + w / 2, y + h / 2, x + w - 1, y + h - 1, c);
		break;
	case 'K':
		DrawLineSimple(ib, x + 1, y + 1, x + 1, y + h - 1, c);
		DrawLineSimple(ib, x + 1, y + h / 2, x + w - 1, y + 1, c);
		DrawLineSimple(ib, x + 1, y + h / 2, x + w - 1, y + h - 1, c);
		break;
	case 'A':
		DrawLineSimple(ib, x + 1, y + h - 1, x + w / 2, y + 1, c);
		DrawLineSimple(ib, x + w - 1, y + h - 1, x + w / 2, y + 1, c);
		DrawLineSimple(ib, x + 3, y + h / 2, x + w - 3, y + h / 2, c);
		break;
	case 'S':
		DrawSegments(ib, x, y, w, h, "acdfg", c);
		break;
	case 'H':
		DrawLineSimple(ib, x + 1, y + 1, x + 1, y + h - 1, c);
		DrawLineSimple(ib, x + w - 1, y + 1, x + w - 1, y + h - 1, c);
		DrawLineSimple(ib, x + 1, y + h / 2, x + w - 1, y + h / 2, c);
		break;
	case 'D':
		DrawLineSimple(ib, x + 1, y + 1, x + 1, y + h - 1, c);
		DrawLineSimple(ib, x + 1, y + 1, x + w - 3, y + 3, c);
		DrawLineSimple(ib, x + 1, y + h - 1, x + w - 3, y + h - 3, c);
		DrawLineSimple(ib, x + w - 3, y + 3, x + w - 3, y + h - 3, c);
		break;
	case 'C':
		DrawSegments(ib, x, y, w, h, "adef", c);
		break;
	default:
		DrawRectStroke(ib, x + 2, y + 2, w - 4, h - 4, c);
		break;
	}
}

static Image MakeCardGlyphImage(const String& text, Color ink, int seed) {
	const int W = 32, H = 32;
	ImageBuffer ib(W, H);
	for (int y = 0; y < H; y++) {
		RGBA* row = ib[y];
		for (int x = 0; x < W; x++) {
			row[x].r = 255;
			row[x].g = 255;
			row[x].b = 255;
			row[x].a = 255;
		}
	}
	int jx = ((seed * 13 + 7) % 5) - 2;
	int jy = ((seed * 17 + 3) % 5) - 2;
	String t = ToUpper(TrimBoth(text));
	char ch = t.IsEmpty() ? '?' : t[0];
	DrawGlyphSymbol(ib, ch, 7 + jx, 5 + jy, 18, 22, ink);
	for (int i = 0; i < 12; i++) {
		int x = (seed * (29 + i * 7) + i * 19) % W;
		int y = (seed * (31 + i * 5) + i * 23) % H;
		int d = (seed + i) % 5;
		if (d == 0) PlotSafe(ib, x, y, Color(245, 245, 245));
		else if (d == 1) PlotSafe(ib, x, y, Color(230, 230, 230));
	}
	return Image(ib);
}

static void FillSampleFromImage(ConvNet::SessionData& d, bool test, int row, const Image& img) {
	const int w = img.GetWidth();
	const int h = img.GetHeight();
	for (int y = 0; y < h; y++) {
		const RGBA* src = img[y];
		for (int x = 0; x < w; x++) {
			int p = (y * w + x) * 3;
			double vr = (double)src[x].r / 255.0;
			double vg = (double)src[x].g / 255.0;
			double vb = (double)src[x].b / 255.0;
			if (test) {
				d.SetTestData(row, p + 0, vr);
				d.SetTestData(row, p + 1, vg);
				d.SetTestData(row, p + 2, vb);
			}
			else {
				d.SetData(row, p + 0, vr);
				d.SetData(row, p + 1, vg);
				d.SetData(row, p + 2, vb);
			}
		}
	}
}

static double EvalClassifierAccuracy(ConvNet::Session& ses) {
	ConvNet::SessionData& d = ses.Data();
	ConvNet::Volume x(d.GetDataWidth(), d.GetDataHeight(), d.GetDataDepth(), 0.0);
	ConvNet::Net& net = ses.GetNetwork();
	int ok = 0;
	int n = d.GetTestCount();
	for (int i = 0; i < n; i++) {
		x.SetData(d.GetTest(i));
		net.Forward(x);
		if (net.GetPrediction() == d.GetTestLabel(i))
			ok++;
	}
	return n > 0 ? (double)ok / (double)n : 0.0;
}

static void SaveSyntheticCardImage(const String& base_dir, const String& tag, const String& label, bool test_sample, int idx, const Image& img) {
	if (base_dir.IsEmpty() || img.IsEmpty())
		return;
	String dir = AppendFileName(base_dir, tag);
	RealizeDirectory(dir);
	String name = String(test_sample ? "test_" : "train_") + label + "_" + AsString(idx) + ".png";
	PNGEncoder().SaveFile(AppendFileName(dir, name), img);
}

static bool SaveSessionBinary(const String& path, ConvNet::Session& ses) {
	FileOut out(path);
	if (!out.IsOpen())
		return false;
	out % ses;
	return !out.IsError();
}

static bool SaveNetworkJson(const String& path, const String& tag, int W, int H, int D, int cls_count) {
	JsonArray ja;
	ja << Json("type", "input")("input_width", W)("input_height", H)("input_depth", D);
	ja << Json("type", "fc")("neuron_count", 64);
	ja << Json("type", "relu");
	ja << Json("type", "fc")("neuron_count", cls_count);
	ja << Json("type", "softmax")("class_count", cls_count);
	return SaveFile(path, ja.ToString());
}

static bool TrainSyntheticCardClassifier(const char* tag, const Vector<String>& labels, bool suit_mode, int iters, double& out_acc, const String& dump_dir) {
	const int W = 32, H = 32, D = 3;
	const int TRAIN_PER_CLASS = 8;
	const int TEST_PER_CLASS = 3;
	const int cls_count = labels.GetCount();
	const int train_count = cls_count * TRAIN_PER_CLASS;
	const int test_count = cls_count * TEST_PER_CLASS;
	ConvNet::Session ses;
	ConvNet::SessionData& data = ses.Data();
	data.BeginData(cls_count, train_count, W, H, D, test_count);
	for (int c = 0; c < cls_count; c++)
		data.SetClass(c, labels[c]);
	int ti = 0;
	int vi = 0;
	for (int c = 0; c < cls_count; c++) {
		for (int s = 0; s < TRAIN_PER_CLASS; s++) {
			bool red = suit_mode ? (labels[c] == "H" || labels[c] == "D") : ((c & 1) == 1);
			Color ink = red ? Color(220, 30, 30) : Black();
			Image img = MakeCardGlyphImage(labels[c], ink, 1000 + c * 100 + s);
			SaveSyntheticCardImage(dump_dir, tag, labels[c], false, s, img);
			FillSampleFromImage(data, false, ti, img);
			data.SetLabel(ti, c);
			ti++;
		}
		for (int s = 0; s < TEST_PER_CLASS; s++) {
			bool red = suit_mode ? (labels[c] == "H" || labels[c] == "D") : ((c & 1) == 1);
			Color ink = red ? Color(220, 30, 30) : Black();
			Image img = MakeCardGlyphImage(labels[c], ink, 2000 + c * 100 + s);
			SaveSyntheticCardImage(dump_dir, tag, labels[c], true, s, img);
			FillSampleFromImage(data, true, vi, img);
			data.SetTestLabel(vi, c);
			vi++;
		}
	}
	data.EndData();
	ses.ClearLayers();
	Cout() << "convnet_" << tag << "_train_begin iters=" << max(1, iters) << "\n";
	ses.AddInputLayer(W, H, D);
	ses.AddFullyConnLayer(64);
	ses.AddReluLayer();
	ses.AddFullyConnLayer(cls_count);
	ses.AddSoftmaxLayer(cls_count);
	ses.GetTrainer()
		.SetType(ConvNet::TRAINER_ADAM)
		.SetLearningRate(0.005)
		.SetBatchSize(5);
	ses.SetPredictInterval(1);
	ses.SetTestPredict(true);
	ses.TrainBegin();
	for (int i = 0; i < max(1, iters); i++) {
		ses.TrainIteration();
		Cout() << "convnet_" << tag << "_iter=" << (i + 1) << "\n";
	}
	ses.TrainEnd();
	out_acc = EvalClassifierAccuracy(ses);
	Cout() << "convnet_" << tag << "_classes=" << cls_count << " train=" << train_count << " test=" << test_count << "\n";
	Cout() << "convnet_" << tag << "_accuracy=" << Format("%.4f", out_acc) << "\n";
	
	if (out_acc >= 0.05 && !dump_dir.IsEmpty()) {
		String model_root = AppendFileName(dump_dir, "convnet_models");
		RealizeDirectory(model_root);
		String bin_path = AppendFileName(model_root, String(tag) + ".bin");
		String json_path = AppendFileName(model_root, String(tag) + ".json");
		SaveSessionBinary(bin_path, ses);
		SaveNetworkJson(json_path, tag, W, H, D, cls_count);
		Cout() << "convnet_" << tag << "_saved=" << bin_path << "\n";
	}
	
	return out_acc >= 0.05;
}

struct DiskCardSample : Moveable<DiskCardSample> {
	String label;
	Image img;
	bool is_test = false;
};

static bool TrainRealCardClassifier(const char* tag, const Vector<String>& labels, int iters, double& out_acc, const String& dump_dir) {
	String dataset_root = AppendFileName(GetExeDirFile("data"), "gfx/dataset");
	
	Vector<DiskCardSample> samples;
	
	// Scan all subdirectories (default_800x480, v13i, v5i, etc.)
	FindFile ff_sub(AppendFileName(dataset_root, "*"));
	while (ff_sub) {
		if (ff_sub.IsFolder()) {
			String tag_dir = AppendFileName(ff_sub.GetPath(), tag);
			if (DirectoryExists(tag_dir)) {
				FindFile ff(AppendFileName(tag_dir, "*.png"));
				while (ff) {
					String name = GetFileTitle(ff.GetName());
					DiskCardSample& s = samples.Add();
					s.img = PNGRaster().LoadFile(ff.GetPath());
					int us = name.Find('_');
					s.label = name.Left(us);
					ff.Next();
				}
			}
		}
		ff_sub.Next();
	}
	
	if (samples.IsEmpty()) {
		Cout() << "Error: No samples found for " << tag << " in " << dataset_root << "\n";
		return false;
	}
	
	Index<String> lbl_idx;
	for (const auto& l : labels) lbl_idx.Add(l);
	
	// Filter samples that match our target labels
	Vector<DiskCardSample> filtered;
	for (int i = 0; i < samples.GetCount(); i++) {
		if (lbl_idx.Find(samples[i].label) >= 0)
			filtered.Add(pick(samples[i]));
	}
	
	if (filtered.IsEmpty()) return false;
	
	// Cap samples for quick test
	if (iters == 1 && filtered.GetCount() > 1000) {
		filtered.SetCount(1000);
		Cout() << "Quick test mode: Capping samples to 1000\n";
	}
	
	// Determine common size (ConvNet needs consistent input size)
	// For this task, we resize all to a fixed size
	const int W = 20, H = 32, D = 3;
	const int cls_count = lbl_idx.GetCount();
	const int test_count = max(1, filtered.GetCount() / 10);
	const int train_count = filtered.GetCount() - test_count;
	
	ConvNet::Session ses;
	ConvNet::SessionData& data = ses.Data();
	data.BeginData(cls_count, train_count, W, H, D, test_count);
	for (int c = 0; c < cls_count; c++)
		data.SetClass(c, lbl_idx[c]);
		
	int ti = 0, vi = 0;
	for (int i = 0; i < filtered.GetCount(); i++) {
		int cls = lbl_idx.Find(filtered[i].label);
		// Resize to consistent size
		Image resized = Rescale(filtered[i].img, W, H);
		if (vi < test_count && (i % 10 == 0)) {
			FillSampleFromImage(data, true, vi, resized);
			data.SetTestLabel(vi, cls);
			vi++;
		} else if (ti < train_count) {
			FillSampleFromImage(data, false, ti, resized);
			data.SetLabel(ti, cls);
			ti++;
		}
	}
	data.EndData();
	
	ses.ClearLayers();
	Cout() << "convnet_real_" << tag << "_train_begin iters=" << max(1, iters) << "\n";
	ses.AddInputLayer(W, H, D);
	ses.AddFullyConnLayer(64);
	ses.AddReluLayer();
	ses.AddFullyConnLayer(cls_count);
	ses.AddSoftmaxLayer(cls_count);
	ses.GetTrainer()
		.SetType(ConvNet::TRAINER_ADAM)
		.SetLearningRate(0.01)
		.SetBatchSize(5);
	ses.TrainBegin();
	for (int i = 0; i < max(1, iters); i++) {
		ses.TrainIteration();
		Cout() << "convnet_real_" << tag << "_iter=" << (i + 1) << "\n";
		Cout().Flush();
	}
	ses.TrainEnd();
	out_acc = EvalClassifierAccuracy(ses);
	Cout() << "convnet_real_" << tag << "_accuracy=" << Format("%.4f", out_acc) << "\n";
	
	if (out_acc >= 0.05 && !dump_dir.IsEmpty()) {
		String model_root = AppendFileName(dump_dir, "convnet_models");
		RealizeDirectory(model_root);
		SaveSessionBinary(AppendFileName(model_root, String(tag) + ".bin"), ses);
		SaveNetworkJson(AppendFileName(model_root, String(tag) + ".json"), tag, W, H, D, cls_count);
	}
	return out_acc >= 0.05;
}

int RunConvNetCardsSelfTest(int iters, const String& dump_dir, bool real_mode ) {
	Vector<String> suits, ranks;
	suits << "S" << "H" << "D" << "C";
	ranks << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "T" << "J" << "Q" << "K" << "A";
	double acc_suit = 0.0, acc_rank = 0.0;
	if (!dump_dir.IsEmpty())
		Cout() << "convnet_dump_dir=" << dump_dir << "\n";
	
	bool ok_suit, ok_rank;
	if (real_mode) {
		ok_suit = TrainRealCardClassifier("suit", suits, iters, acc_suit, dump_dir);
		ok_rank = TrainRealCardClassifier("rank", ranks, iters, acc_rank, dump_dir);
	} else {
		ok_suit = TrainSyntheticCardClassifier("suit", suits, true, iters, acc_suit, dump_dir);
		ok_rank = TrainSyntheticCardClassifier("rank", ranks, false, iters, acc_rank, dump_dir);
	}
	
	ok_suit = ok_suit || (acc_suit >= 0.24);
	ok_rank = ok_rank || (acc_rank >= 0.07);
	Cout() << "convnet_cards_iters=" << max(1, iters) << "\n";
	Cout() << "convnet_cards_result=" << ((ok_suit && ok_rank) ? "ok" : "fail") << "\n";
	return (ok_suit && ok_rank) ? 0 : 1;
}

static bool ParseSyntheticCardDumpName(const String& file_name, bool& is_test, String& label) {
	String n = GetFileTitle(file_name);
	if (n.StartsWith("train_")) {
		is_test = false;
		n = n.Mid(6);
	}
	else if (n.StartsWith("test_")) {
		is_test = true;
		n = n.Mid(5);
	}
	else
		return false;
	int us = n.ReverseFind('_');
	if (us <= 0)
		return false;
	label = n.Left(us);
	return !label.IsEmpty();
}

static bool LoadCardDumpSamples(const String& dir, Vector<DiskCardSample>& out, Vector<String>& labels) {
	out.Clear();
	labels.Clear();
	if (!DirectoryExists(dir))
		return false;
	Index<String> lbls;
	FindFile ff(AppendFileName(dir, "*.png"));
	while (ff) {
		if (ff.IsFile()) {
			bool is_test = false;
			String label;
			if (ParseSyntheticCardDumpName(ff.GetName(), is_test, label)) {
				String path = AppendFileName(dir, ff.GetName());
				Image img = StreamRaster::LoadFileAny(path);
				if (!img.IsEmpty()) {
					DiskCardSample& s = out.Add();
					s.label = label;
					s.img = img;
					s.is_test = is_test;
					lbls.FindAdd(label);
				}
			}
		}
		ff.Next();
	}
	labels.SetCount(lbls.GetCount());
	for (int i = 0; i < lbls.GetCount(); i++)
		labels[i] = lbls[i];
	Sort(labels);
	return !out.IsEmpty() && !labels.IsEmpty();
}

static bool LoadSessionBinary(const String& path, ConvNet::Session& ses) {
	FileIn in(path);
	if (!in.IsOpen())
		return false;
	in % ses;
	return !in.IsError();
}

static bool TrainAndRoundtripCardModel(const String& tag, const String& data_dir, const String& model_dir, int iters, double& acc_before, double& acc_after) {
	Vector<DiskCardSample> samples;
	Vector<String> labels;
	if (!LoadCardDumpSamples(data_dir, samples, labels))
		return false;
	const int cls_count = labels.GetCount();
	if (cls_count <= 0)
		return false;
	int train_count = 0, test_count = 0;
	for (const DiskCardSample& s : samples) {
		if (s.is_test) test_count++;
		else train_count++;
	}
	if (train_count <= 0 || test_count <= 0)
		return false;
	const int w = samples[0].img.GetWidth();
	const int h = samples[0].img.GetHeight();
	if (w <= 0 || h <= 0)
		return false;
	Index<String> label_index;
	for (const String& l : labels)
		label_index.FindAdd(l);

	ConvNet::Session ses;
	ConvNet::SessionData& data = ses.Data();
	data.BeginData(cls_count, train_count, w, h, 3, test_count);
	for (int i = 0; i < labels.GetCount(); i++)
		data.SetClass(i, labels[i]);
	int ti = 0, vi = 0;
	for (const DiskCardSample& s : samples) {
		int cls = label_index.Find(s.label);
		if (cls < 0)
			continue;
		if (s.is_test) {
			FillSampleFromImage(data, true, vi, s.img);
			data.SetTestLabel(vi, cls);
			vi++;
		}
		else {
			FillSampleFromImage(data, false, ti, s.img);
			data.SetLabel(ti, cls);
			ti++;
		}
	}
	data.EndData();
	ses.ClearLayers();
	ses.AddInputLayer(w, h, 3);
	ses.AddFullyConnLayer(64);
	ses.AddReluLayer();
	ses.AddFullyConnLayer(cls_count);
	ses.AddSoftmaxLayer(cls_count);
	ses.GetTrainer()
		.SetType(ConvNet::TRAINER_ADAM)
		.SetLearningRate(0.005)
		.SetBatchSize(5);
	ses.SetPredictInterval(1);
	ses.SetTestPredict(true);
	ses.TrainBegin();
	for (int i = 0; i < max(1, iters); i++)
		ses.TrainIteration();
	ses.TrainEnd();
	acc_before = EvalClassifierAccuracy(ses);

	RealizeDirectory(model_dir);
	String model_path = AppendFileName(model_dir, tag + ".model.bin");
	String info_path = AppendFileName(model_dir, tag + ".model.json");
	if (!SaveSessionBinary(model_path, ses))
		return false;
	struct ModelInfo : Moveable<ModelInfo> {
		String tag;
		String model_file;
		int width = 0;
		int height = 0;
		int depth = 3;
		Vector<String> classes;
		int train_count = 0;
		int test_count = 0;
		double accuracy_before_reload = 0;
		double accuracy_after_reload = 0;
		void Jsonize(JsonIO& jio) {
			jio("tag", tag)
			   ("model_file", model_file)
			   ("width", width)
			   ("height", height)
			   ("depth", depth)
			   ("classes", classes)
			   ("train_count", train_count)
			   ("test_count", test_count)
			   ("accuracy_before_reload", accuracy_before_reload)
			   ("accuracy_after_reload", accuracy_after_reload);
		}
	};
	ModelInfo mi;
	mi.tag = tag;
	mi.model_file = GetFileName(model_path);
	mi.width = w;
	mi.height = h;
	mi.classes <<= labels;
	mi.train_count = train_count;
	mi.test_count = test_count;
	mi.accuracy_before_reload = acc_before;

	ConvNet::Session reloaded;
	if (!LoadSessionBinary(model_path, reloaded))
		return false;
	acc_after = EvalClassifierAccuracy(reloaded);
	mi.accuracy_after_reload = acc_after;
	if (!SaveFile(info_path, StoreAsJson(mi)))
		return false;
	Cout() << "convnet_disk_model tag=" << tag << " bin=" << model_path << " info=" << info_path << "\n";
	return true;
}

int RunConvNetDiskPipelineTest(const String& data_root, const String& model_root, int iters, const String& tag_filter) {
	String suit_dir = AppendFileName(data_root, "suit");
	String rank_dir = AppendFileName(data_root, "rank");
	double suit_before = 0, suit_after = 0, rank_before = 0, rank_after = 0;
	String f = ToLower(TrimBoth(tag_filter));
	if (f.IsEmpty())
		f = "all";
	bool run_suit = (f == "all" || f == "suit");
	bool run_rank = (f == "all" || f == "rank");
	bool ok_suit = run_suit ? TrainAndRoundtripCardModel("suit", suit_dir, model_root, iters, suit_before, suit_after) : true;
	bool ok_rank = run_rank ? TrainAndRoundtripCardModel("rank", rank_dir, model_root, iters, rank_before, rank_after) : true;
	Cout() << "convnet_disk_suit_accuracy_before=" << Format("%.4f", suit_before) << "\n";
	Cout() << "convnet_disk_suit_accuracy_after=" << Format("%.4f", suit_after) << "\n";
	Cout() << "convnet_disk_rank_accuracy_before=" << Format("%.4f", rank_before) << "\n";
	Cout() << "convnet_disk_rank_accuracy_after=" << Format("%.4f", rank_after) << "\n";
	bool acc_ok = false;
	if (run_suit && suit_after > 0)
		acc_ok = true;
	if (run_rank && rank_after > 0)
		acc_ok = true;
	if (!run_suit && !run_rank)
		acc_ok = false;
	bool ok = ok_suit && ok_rank && acc_ok;
	Cout() << "convnet_disk_pipeline_result=" << (ok ? "ok" : "fail") << "\n";
	return ok ? 0 : 1;
}

void AddScreenGameArgs(CommandLineArguments& cla) {
	cla.AddArg("project", 'p', "Project name", true, "name");
	cla.AddArg("remote", 'r', "Remote video server host:port", true, "host:port");
	cla.AddArg("platform", 'P', "Platform name", true, "name");
	cla.AddArg("provider", 'v', "Provider name for screenshots", true, "name");
	cla.AddArg("frame", 'f', "Frame path", true, "path");
	cla.AddArg("rule", 'R', "Rule name", true, "name");
	cla.AddArg("list-platforms", 'L', "List platforms", false);
	cla.AddArg("list-frames", 'F', "List captured frames", false);
	cla.AddArg("list-rules", 'U', "List rules", false);
	cla.AddArg("list-props", 'S', "List rule properties", false);
	cla.AddArg("list-images", 'I', "List rule images", false);
	cla.AddArg("run", 'x', "Run platform code on a frame", false);
	cla.AddArg("run-all", 'A', "Run all platform code on a frame", false);
	cla.AddArg("live-remote-test", 0, "Run live remote-feed processing benchmark", false);
	cla.AddArg("synthetic-window-motion-test", 0, "Run synthetic moving window-size slideshow tracking test", false);
	cla.AddArg("live-seconds", 0, "Duration for live remote test", true, "seconds");
	cla.AddArg("synthetic-fps", 0, "FPS for synthetic moving window test", true, "fps");
	cla.AddArg("live-warmup-frames", 0, "Warmup frames excluded from FPS stats", true, "count");
	cla.AddArg("temporal-local-dx", 0, "Local temporal ORB search half-width", true, "pixels");
	cla.AddArg("temporal-local-dy", 0, "Local temporal ORB search half-height", true, "pixels");
	cla.AddArg("decode-threads", 0, "Decode worker threads for live test", true, "count");
	cla.AddArg("queue-capacity", 0, "Fetch/decode queue capacity", true, "count");
	cla.AddArg("live-rule-cache-limit", 0, "Max cached rule images in live run (evict oldest)", true, "count");
	cla.AddArg("drop-policy", 0, "Queue drop policy: oldest|newest", true, "name");
	cla.AddArg("live-report-json", 0, "Write live benchmark summary to JSON file", true, "path");
	cla.AddArg("override-keep-keyframe", 0, "For live test, reuse previous frame matches without rematching", false);
	cla.AddArg("exec-script-debug", 0, "Run selected platform script on frame with detailed dump", false);
	cla.AddArg("validate-frame", 0, "Validate script execution outputs against expected minimum counts", false);
	cla.AddArg("trace-script-api", 0, "Print script API call sequence to log/stdout", false);
	cla.AddArg("expect-window-min", 0, "Validation: minimum window rect count", true, "count");
	cla.AddArg("expect-script-objects-min", 0, "Validation: minimum script object count", true, "count");
	cla.AddArg("expect-resolved-min", 0, "Validation: minimum resolved projected rect count", true, "count");
	cla.AddArg("expect-class-semantic-min", 0, "Validation: minimum class semantic count", true, "count");
	cla.AddArg("expect-text-semantic-min", 0, "Validation: minimum text semantic count", true, "count");
	cla.AddArg("expect-policy-semantic-min", 0, "Validation: minimum policy semantic count", true, "count");
	cla.AddArg("expect-text-backend-status-min", 0, "Validation: minimum text.backend_statuses count", true, "count");
	cla.AddArg("expect-class-orb-status-min", 0, "Validation: minimum class.orb_status count", true, "count");
	cla.AddArg("expect-class-nn-status-min", 0, "Validation: minimum class.nn_status count", true, "count");
	cla.AddArg("expect-class-alt-min", 0, "Validation: minimum class alternatives props count", true, "count");
	cla.AddArg("expect-threshold-pass-min", 0, "Validation: minimum class.threshold_pass=1 count", true, "count");
	cla.AddArg("expect-text-rect-total-min", 0, "Validation: minimum OCR objects with text rect diagnostics", true, "count");
	cla.AddArg("expect-text-rect-shifted-min", 0, "Validation: minimum OCR objects using shifted text rect", true, "count");
	cla.AddArg("expect-min-text-rect-shift-rate", 0, "Validation: minimum shifted OCR rect ratio [0..1]", true, "rate");
	cla.AddArg("expect-ocr-compare-exact-min", 0, "Validation: minimum exact OCR-vs-annotation matches", true, "count");
	cla.AddArg("expect-ocr-compare-normalized-min", 0, "Validation: minimum normalized OCR-vs-annotation matches", true, "count");
	cla.AddArg("expect-ocr-compare-missing-max", 0, "Validation: maximum missing OCR-vs-annotation entries", true, "count");
	cla.AddArg("expect-ocr-compare-total-min", 0, "Validation: minimum OCR-vs-annotation compared entries", true, "count");
	cla.AddArg("expect-ocr-compare-total-max", 0, "Validation: maximum OCR-vs-annotation compared entries", true, "count");
	cla.AddArg("expect-ocr-compare-normalized-rate-min", 0, "Validation: minimum normalized OCR match rate [0..1]", true, "rate");
	cla.AddArg("expect-ocr-compare-missing-rate-max", 0, "Validation: maximum missing OCR rate [0..1]", true, "rate");
	cla.AddArg("expect-text-backends", 0, "Validation: required backend tokens in text.backend_statuses (csv)", true, "csv");
	cla.AddArg("expect-max-text-total-us", 0, "Validation: maximum total text backend time (us)", true, "usec");
	cla.AddArg("expect-max-text-disagreement-rate", 0, "Validation: maximum text disagreement rate [0..1]", true, "rate");
	cla.AddArg("quit-after-run", 0, "Exit automatically after script execution", false);
	cla.AddArg("semantic-summary", 0, "Print semantic output summary for script objects", false);
	cla.AddArg("dump-ocr", 0, "Dump OCR text values from script objects after run", false);
	cla.AddArg("dump-ocr-shifted-only", 0, "Dump only OCR values where effective rect differs from base rect", false);
	cla.AddArg("dump-ocr-verbose", 0, "Include extra OCR diagnostics in dump output", false);
	cla.AddArg("compare-ocr-to-annotations", 0, "Compare OCR output to frame rule_data.ocr_text and print diff summary", false);
	cla.AddArg("compare-ocr-diff-only", 0, "Print only DIFF rows for OCR annotation compare", false);
	cla.AddArg("compare-ocr-filter", 0, "Filter OCR compare to annotation rule names by wildcard pattern", true, "pattern");
	cla.AddArg("export-ocr-compare-json", 0, "Save OCR annotation compare report as JSON", true, "path");
	cla.AddArg("dump-ocr-filter", 0, "Filter OCR dump by object wildcard pattern", true, "pattern");
	cla.AddArg("export-preview", 0, "Save preview image with dynamic rect overlay", true, "path");
	cla.AddArg("export-exec-json", 0, "Save execute output (windows/script/resolved) to JSON", true, "path");
	cla.AddArg("export-semantic-json", 0, "Save semantic summary metrics to JSON", true, "path");
	cla.AddArg("export-card-patches", 0, "Save resolved card suit/index patches and sidecars", true, "dir");
	cla.AddArg("harvest-text-patches", 0, "Save unique text OCR patches from runtime objects", true, "dir");
	cla.AddArg("harvest-filter", 0, "Wildcard filter for harvested object names", true, "pattern");
	cla.AddArg("harvest-include-orb-same", 0, "Also harvest objects marked text.orb_same=1", false);
	cla.AddArg("harvest-implicit-text", 0, "Harvest all text objects even without script harvest.capture flag", false);
	cla.AddArg("harvest-summary", 0, "Summarize harvested text patch dataset", true, "dir");
	cla.AddArg("harvest-summary-json", 0, "Save harvest summary as JSON", true, "path");
	cla.AddArg("harvest-status", 0, "Filter harvest summary/queue by status: any|unlabeled|labeled|reviewed|rejected", true, "name");
	cla.AddArg("harvest-queue", 0, "Print unlabeled queue from --harvest-summary dir", false);
	cla.AddArg("harvest-duplicates", 0, "List duplicate-suspect groups from --harvest-summary dir", false);
	cla.AddArg("harvest-duplicates-limit", 0, "Max duplicate groups to print", true, "count");
	cla.AddArg("harvest-dedup-reject", 0, "Mark duplicate-suspect items as rejected (keep first per hash)", false);
	cla.AddArg("harvest-dedup-reviewer", 0, "Reviewer value used by --harvest-dedup-reject", true, "name");
	cla.AddArg("harvest-edit-match", 0, "Wildcard pattern to edit harvest metadata files (*.png path)", true, "pattern");
	cla.AddArg("harvest-set-status", 0, "Set review_status for matched harvest items", true, "status");
	cla.AddArg("harvest-set-label", 0, "Set label for matched harvest items", true, "text");
	cla.AddArg("harvest-set-reviewer", 0, "Set reviewer for matched harvest items", true, "name");
	cla.AddArg("harvest-apply-labels", 0, "Apply labels from TSV: pattern<TAB>label<TAB>status<TAB>reviewer", true, "path");
	cla.AddArg("harvest-export-labels", 0, "Export TSV template for labeling matched harvest queue", true, "path");
	cla.AddArg("harvest-char-coverage", 0, "Print per-character counts from labeled harvest metadata", false);
	cla.AddArg("harvest-required-chars", 0, "Check required characters (string) against labeled harvest set", true, "text");
	cla.AddArg("harvest-readiness", 0, "Run readiness checks on --harvest-summary dir", false);
	cla.AddArg("harvest-required-labels", 0, "CSV list of required labels for readiness", true, "csv");
	cla.AddArg("harvest-min-per-label", 0, "Readiness minimum samples per label", true, "count");
	cla.AddArg("orb-find", 0, "Run ORB match on a frame with a pattern", false);
	cla.AddArg("pattern", 0, "Pattern image path for ORB", true, "path");
	cla.AddArg("verbose", 'V', "Verbose output", false);
	cla.AddArg("debug-orb", 0, "Print ORB match counts per rule", false);
	cla.AddArg("fast-exec", 0, "Use fast no-scale/no-rotation matcher path", false);
	cla.AddArg("profile-exec", 0, "Print execution timing logs", false);
	cla.AddArg("gpu-preprocess", 0, "Enable shared GPU preprocess path when available", false);
	cla.AddArg("gpu-probe", 0, "Probe GPU preprocess backends and print route summary", false);
	cla.AddArg("gpu-backend", 0, "GPU preprocess backend: stub|egl-gl|auto", true, "name");
	cla.AddArg("gpu-route", 0, "GPU preprocess route: auto|egl-surfaceless|egl-x11|glx-x11|stub-cpu", true, "name");
	cla.AddArg("fastcrash", 0, "Crash immediately on fatal errors/assertions (no message box)", false);
	cla.AddArg("save-frame-state", 0, "Save execute results into frame sidecar JSON", false);
	cla.AddArg("skip-autoexec", 0, "Skip autoexec cfg", false);
	cla.AddArg("test-orb", 0, "Run ORB self-test", false);
	cla.AddArg("test-jpg", 0, "Run JPG roundtrip self-test", false);
	cla.AddArg("test-png", 0, "Run PNG roundtrip self-test", false);
	cla.AddArg("test-eglimage", 0, "Run EGLImage DMA-BUF import test (Phase 12)", false);
	cla.AddArg("test-v4l2-eglimage", 0, "Run V4L2 webcam capture through EGLImage DMA-BUF test (Phase 12)", false);
	cla.AddArg("test-d3d11-interop", 0, "Run D3D11-GL texture sharing interop test (Phase 12 Windows)", false);
	cla.AddArg("video-device", 0, "V4L2 or MediaFoundation device path/name", true, "path");
	cla.AddArg("video-format-policy", 0, "Capture format policy: auto|low-copy|throughput", true, "name");
	cla.AddArg("test-process-screenshot", 0, "Process one screenshot with ScreenGame test runner and dump log", false);
	cla.AddArg("test-log-file", 0, "Output log file for --test-process-screenshot", true, "path");
	cla.AddArg("test-gui-edit-flow", 0, "Run GUI edit-flow smoke: edit one rule rect, save sidecar, reload and verify", false);
	cla.AddArg("test-edit-rule", 0, "Rule name for --test-gui-edit-flow", true, "name");
	cla.AddArg("test-edit-dx", 0, "X offset for --test-gui-edit-flow", true, "pixels");
	cla.AddArg("test-edit-dy", 0, "Y offset for --test-gui-edit-flow", true, "pixels");
	cla.AddArg("start-local-game-preview", 0, "Start CardEngine local scripted game for previewing theme/gameplay", false);
	cla.AddArg("quit-after-test", 0, "Exit after --test-process-screenshot", false);
	cla.AddArg("test-tesseract-render", 0, "Render synthetic text and OCR it with tesseract", false);
	cla.AddArg("test-convnet-cards", 0, "Run synthetic ConvNet suit/rank self-test", false);
	cla.AddArg("test-convnet-disk-pipeline", 0, "Run disk-based ConvNet train/save/load/eval for suit/rank dumps", false);
	cla.AddArg("convnet-iters", 0, "Training iterations for --test-convnet-cards", true, "count");
	cla.AddArg("train-cards", 0, "Train unified 52-class card classifier", false);
	cla.AddArg("training-minutes", 0, "Time limit for training (default 20)", true, "minutes");
	cla.AddArg("dataset-path", 0, "Path to the dataset root for training", true, "path");
	cla.AddArg("model-tag", 0, "Version tag for the model metadata", true, "tag");
	cla.AddArg("convnet-dump-dir", 0, "Optional dir to dump generated synthetic suit/rank images", true, "path");
	cla.AddArg("convnet-data-dir", 0, "Input dir for --test-convnet-disk-pipeline (expects suit/ and rank/)", true, "path");
	cla.AddArg("convnet-model-dir", 0, "Output dir for model bin/json files", true, "path");
	cla.AddArg("convnet-disk-tag", 0, "For --test-convnet-disk-pipeline: all|suit|rank", true, "name");
	cla.AddArg("openai-key-file", 0, "Path to text file containing OpenAI API key", true, "path");
	cla.AddArg("openai-model", 0, "OpenAI model for script text repair backend", true, "name");
	cla.AddArg("headless", 0, "Run without GUI", false);
	cla.SetDescription("ScreenGame CLI and GUI.");
}

bool ParseScreenGameCli(CommandLineArguments& cla, ScreenGameCliOptions& opt) {
	opt.headless = cla.IsArg("headless");
	bool cli_project = cla.IsArg("project");
	bool cli_remote = cla.IsArg("remote");
	bool cli_platform = cla.IsArg("platform");
	bool cli_provider = cla.IsArg("provider");
	bool cli_frame = cla.IsArg("frame");
	bool cli_rule = cla.IsArg("rule");
	bool cli_list_platforms = cla.IsArg("list-platforms");
	bool cli_list_frames = cla.IsArg("list-frames");
	bool cli_list_rules = cla.IsArg("list-rules");
	bool cli_list_props = cla.IsArg("list-props");
	bool cli_list_images = cla.IsArg("list-images");
	bool cli_run_one = cla.IsArg("run");
	bool cli_run_all = cla.IsArg("run-all");
	bool cli_live_remote_test = cla.IsArg("live-remote-test");
	bool cli_synthetic_window_motion_test = cla.IsArg("synthetic-window-motion-test");
	bool cli_override_keep_keyframe = cla.IsArg("override-keep-keyframe");
	bool cli_exec_script_debug = cla.IsArg("exec-script-debug");
	bool cli_validate_frame = cla.IsArg("validate-frame");
	bool cli_trace_script_api = cla.IsArg("trace-script-api");
	bool cli_quit_after_run = cla.IsArg("quit-after-run");
	bool cli_semantic_summary = cla.IsArg("semantic-summary");
	bool cli_dump_ocr = cla.IsArg("dump-ocr");
	bool cli_dump_ocr_shifted_only = cla.IsArg("dump-ocr-shifted-only");
	bool cli_dump_ocr_verbose = cla.IsArg("dump-ocr-verbose");
	bool cli_compare_ocr = cla.IsArg("compare-ocr-to-annotations");
	bool cli_compare_ocr_diff_only = cla.IsArg("compare-ocr-diff-only");
	bool cli_compare_ocr_filter = cla.IsArg("compare-ocr-filter");
	bool cli_export_ocr_compare_json = cla.IsArg("export-ocr-compare-json");
	bool cli_dump_ocr_filter = cla.IsArg("dump-ocr-filter");
	bool cli_harvest_text_patches = cla.IsArg("harvest-text-patches");
	bool cli_harvest_filter = cla.IsArg("harvest-filter");
	bool cli_harvest_include_orb_same = cla.IsArg("harvest-include-orb-same");
	bool cli_harvest_implicit_text = cla.IsArg("harvest-implicit-text");
	bool cli_harvest_summary = cla.IsArg("harvest-summary");
	bool cli_harvest_summary_json = cla.IsArg("harvest-summary-json");
	bool cli_harvest_status = cla.IsArg("harvest-status");
	bool cli_harvest_queue = cla.IsArg("harvest-queue");
	bool cli_harvest_duplicates = cla.IsArg("harvest-duplicates");
	bool cli_harvest_dedup_reject = cla.IsArg("harvest-dedup-reject");
	bool cli_harvest_dedup_reviewer = cla.IsArg("harvest-dedup-reviewer");
	bool cli_harvest_edit_match = cla.IsArg("harvest-edit-match");
	bool cli_harvest_set_status = cla.IsArg("harvest-set-status");
	bool cli_harvest_set_label = cla.IsArg("harvest-set-label");
	bool cli_harvest_set_reviewer = cla.IsArg("harvest-set-reviewer");
	bool cli_harvest_apply_labels = cla.IsArg("harvest-apply-labels");
	bool cli_harvest_export_labels = cla.IsArg("harvest-export-labels");
	bool cli_harvest_char_coverage = cla.IsArg("harvest-char-coverage");
	bool cli_harvest_required_chars = cla.IsArg("harvest-required-chars");
	bool cli_harvest_readiness = cla.IsArg("harvest-readiness");
	bool cli_harvest_required_labels = cla.IsArg("harvest-required-labels");
	bool cli_orb_find = cla.IsArg("orb-find");
	bool cli_verbose = cla.IsArg("verbose");
	bool cli_debug_orb = cla.IsArg("debug-orb");
	bool cli_fast_exec = cla.IsArg("fast-exec");
	bool cli_profile_exec = cla.IsArg("profile-exec");
	bool cli_gpu_preprocess = cla.IsArg("gpu-preprocess");
	bool cli_gpu_probe = cla.IsArg("gpu-probe");
	bool cli_save_frame_state = cla.IsArg("save-frame-state");
	bool cli_skip_autoexec = cla.IsArg("skip-autoexec");
	bool cli_test_orb = cla.IsArg("test-orb");
	bool cli_test_jpg = cla.IsArg("test-jpg");
	bool cli_test_png = cla.IsArg("test-png");
	bool cli_test_eglimage = cla.IsArg("test-eglimage");
	bool cli_test_v4l2_eglimage = cla.IsArg("test-v4l2-eglimage");
	bool cli_test_d3d11_interop = cla.IsArg("test-d3d11-interop");
	bool cli_test_tesseract_render = cla.IsArg("test-tesseract-render");
	bool cli_test_convnet_cards = cla.IsArg("test-convnet-cards");
	bool cli_test_convnet_disk = cla.IsArg("test-convnet-disk-pipeline");

	opt.project_name = cli_project ? cla.GetArg("project") : String();
	opt.remote_addr = cli_remote ? cla.GetArg("remote") : String();
	opt.platform_name = cli_platform ? cla.GetArg("platform") : String();
	opt.provider_name = cli_provider ? cla.GetArg("provider") : String();
	opt.frame_arg = cli_frame ? cla.GetArg("frame") : String();
	opt.rule_name = cli_rule ? cla.GetArg("rule") : String();
	opt.pattern_arg = cla.IsArg("pattern") ? cla.GetArg("pattern") : String();
	opt.export_preview_path = cla.IsArg("export-preview") ? cla.GetArg("export-preview") : String();
	opt.export_exec_json_path = cla.IsArg("export-exec-json") ? cla.GetArg("export-exec-json") : String();
	opt.export_semantic_json_path = cla.IsArg("export-semantic-json") ? cla.GetArg("export-semantic-json") : String();
	opt.export_card_patches_dir = cla.IsArg("export-card-patches") ? cla.GetArg("export-card-patches") : String();
	opt.harvest_text_patches_dir = cli_harvest_text_patches ? cla.GetArg("harvest-text-patches") : String();
	opt.harvest_filter = cli_harvest_filter ? TrimBoth(cla.GetArg("harvest-filter")) : String();
	opt.harvest_include_orb_same = cli_harvest_include_orb_same;
	opt.harvest_implicit_text = cli_harvest_implicit_text;
	opt.harvest_summary_dir = cli_harvest_summary ? cla.GetArg("harvest-summary") : String();
	opt.harvest_summary_json_path = cli_harvest_summary_json ? cla.GetArg("harvest-summary-json") : String();
	opt.harvest_status_filter = cli_harvest_status ? ToLower(TrimBoth(cla.GetArg("harvest-status"))) : String("any");
	opt.harvest_queue = cli_harvest_queue;
	opt.harvest_duplicates = cli_harvest_duplicates;
	opt.harvest_dedup_reject = cli_harvest_dedup_reject;
	opt.harvest_dedup_reviewer = cli_harvest_dedup_reviewer ? cla.GetArg("harvest-dedup-reviewer") : String("dedup");
	opt.harvest_edit_match = cli_harvest_edit_match ? TrimBoth(cla.GetArg("harvest-edit-match")) : String();
	opt.harvest_set_status = cli_harvest_set_status ? ToLower(TrimBoth(cla.GetArg("harvest-set-status"))) : String();
	opt.harvest_set_label = cli_harvest_set_label ? cla.GetArg("harvest-set-label") : String();
	opt.harvest_set_reviewer = cli_harvest_set_reviewer ? cla.GetArg("harvest-set-reviewer") : String();
	opt.harvest_apply_labels_path = cli_harvest_apply_labels ? cla.GetArg("harvest-apply-labels") : String();
	opt.harvest_export_labels_path = cli_harvest_export_labels ? cla.GetArg("harvest-export-labels") : String();
	opt.harvest_char_coverage = cli_harvest_char_coverage;
	opt.harvest_required_chars = cli_harvest_required_chars ? cla.GetArg("harvest-required-chars") : String();
	opt.harvest_readiness = cli_harvest_readiness;
	opt.harvest_required_labels = cli_harvest_required_labels ? TrimBoth(cla.GetArg("harvest-required-labels")) : String();
	if (cla.IsArg("harvest-min-per-label"))
		opt.harvest_min_per_label = max(1, StrInt(cla.GetArg("harvest-min-per-label")));
	if (cla.IsArg("harvest-duplicates-limit"))
		opt.harvest_duplicates_limit = max(1, StrInt(cla.GetArg("harvest-duplicates-limit")));
	opt.list_platforms = cli_list_platforms;
	opt.list_frames = cli_list_frames;
	opt.list_rules = cli_list_rules;
	opt.list_props = cli_list_props;
	opt.list_images = cli_list_images;
	opt.run_one = cli_run_one;
	opt.run_all = cli_run_all;
	opt.live_remote_test = cli_live_remote_test;
	opt.synthetic_window_motion_test = cli_synthetic_window_motion_test;
	if (cla.IsArg("live-seconds"))
		opt.live_seconds = max(1, StrInt(cla.GetArg("live-seconds")));
	if (cla.IsArg("synthetic-fps"))
		opt.synthetic_fps = max(1, StrInt(cla.GetArg("synthetic-fps")));
	if (cla.IsArg("live-warmup-frames"))
		opt.live_warmup_frames = max(0, StrInt(cla.GetArg("live-warmup-frames")));
	if (cla.IsArg("temporal-local-dx"))
		opt.temporal_local_dx = max(0, StrInt(cla.GetArg("temporal-local-dx")));
	if (cla.IsArg("temporal-local-dy"))
		opt.temporal_local_dy = max(0, StrInt(cla.GetArg("temporal-local-dy")));
	if (cla.IsArg("decode-threads"))
		opt.decode_threads = max(1, StrInt(cla.GetArg("decode-threads")));
	if (cla.IsArg("queue-capacity"))
		opt.queue_capacity = max(1, StrInt(cla.GetArg("queue-capacity")));
	if (cla.IsArg("live-rule-cache-limit"))
		opt.live_rule_cache_limit = max(8, StrInt(cla.GetArg("live-rule-cache-limit")));
	if (cla.IsArg("drop-policy")) {
		opt.drop_policy = ToLower(TrimBoth(cla.GetArg("drop-policy")));
		if (opt.drop_policy != "oldest" && opt.drop_policy != "newest")
			opt.drop_policy = "oldest";
	}
	if (cla.IsArg("live-report-json"))
		opt.live_report_json = cla.GetArg("live-report-json");
	opt.override_keep_keyframe = cli_override_keep_keyframe;
	opt.exec_script_debug = cli_exec_script_debug;
	opt.validate_frame = cli_validate_frame;
	opt.trace_script_api = cli_trace_script_api;
	if (cla.IsArg("expect-window-min"))
		opt.expect_window_min = max(0, StrInt(cla.GetArg("expect-window-min")));
	if (cla.IsArg("expect-script-objects-min"))
		opt.expect_script_objects_min = max(0, StrInt(cla.GetArg("expect-script-objects-min")));
	if (cla.IsArg("expect-resolved-min"))
		opt.expect_resolved_min = max(0, StrInt(cla.GetArg("expect-resolved-min")));
	if (cla.IsArg("expect-class-semantic-min"))
		opt.expect_class_semantic_min = max(0, StrInt(cla.GetArg("expect-class-semantic-min")));
	if (cla.IsArg("expect-text-semantic-min"))
		opt.expect_text_semantic_min = max(0, StrInt(cla.GetArg("expect-text-semantic-min")));
	if (cla.IsArg("expect-policy-semantic-min"))
		opt.expect_policy_semantic_min = max(0, StrInt(cla.GetArg("expect-policy-semantic-min")));
	if (cla.IsArg("expect-text-backend-status-min"))
		opt.expect_text_backend_status_min = max(0, StrInt(cla.GetArg("expect-text-backend-status-min")));
	if (cla.IsArg("expect-class-orb-status-min"))
		opt.expect_class_orb_status_min = max(0, StrInt(cla.GetArg("expect-class-orb-status-min")));
	if (cla.IsArg("expect-class-nn-status-min"))
		opt.expect_class_nn_status_min = max(0, StrInt(cla.GetArg("expect-class-nn-status-min")));
	if (cla.IsArg("expect-class-alt-min"))
		opt.expect_class_alt_min = max(0, StrInt(cla.GetArg("expect-class-alt-min")));
	if (cla.IsArg("expect-threshold-pass-min"))
		opt.expect_threshold_pass_min = max(0, StrInt(cla.GetArg("expect-threshold-pass-min")));
	if (cla.IsArg("expect-text-rect-total-min"))
		opt.expect_text_rect_total_min = max(0, StrInt(cla.GetArg("expect-text-rect-total-min")));
	if (cla.IsArg("expect-text-rect-shifted-min"))
		opt.expect_text_rect_shifted_min = max(0, StrInt(cla.GetArg("expect-text-rect-shifted-min")));
	if (cla.IsArg("expect-min-text-rect-shift-rate"))
		opt.expect_min_text_rect_shift_rate = minmax(StrDbl(cla.GetArg("expect-min-text-rect-shift-rate")), 0.0, 1.0);
	if (cla.IsArg("expect-ocr-compare-exact-min"))
		opt.expect_ocr_compare_exact_min = max(0, StrInt(cla.GetArg("expect-ocr-compare-exact-min")));
	if (cla.IsArg("expect-ocr-compare-normalized-min"))
		opt.expect_ocr_compare_normalized_min = max(0, StrInt(cla.GetArg("expect-ocr-compare-normalized-min")));
	if (cla.IsArg("expect-ocr-compare-missing-max"))
		opt.expect_ocr_compare_missing_max = max(0, StrInt(cla.GetArg("expect-ocr-compare-missing-max")));
	if (cla.IsArg("expect-ocr-compare-total-min"))
		opt.expect_ocr_compare_total_min = max(0, StrInt(cla.GetArg("expect-ocr-compare-total-min")));
	if (cla.IsArg("expect-ocr-compare-total-max"))
		opt.expect_ocr_compare_total_max = max(0, StrInt(cla.GetArg("expect-ocr-compare-total-max")));
	if (cla.IsArg("expect-ocr-compare-normalized-rate-min"))
		opt.expect_ocr_compare_normalized_rate_min = minmax(StrDbl(cla.GetArg("expect-ocr-compare-normalized-rate-min")), 0.0, 1.0);
	if (cla.IsArg("expect-ocr-compare-missing-rate-max"))
		opt.expect_ocr_compare_missing_rate_max = minmax(StrDbl(cla.GetArg("expect-ocr-compare-missing-rate-max")), 0.0, 1.0);
	if (cla.IsArg("expect-text-backends"))
		opt.expect_text_backends = TrimBoth(cla.GetArg("expect-text-backends"));
	if (cla.IsArg("expect-max-text-total-us"))
		opt.expect_max_text_total_us = max(0, StrInt(cla.GetArg("expect-max-text-total-us")));
	if (cla.IsArg("expect-max-text-disagreement-rate"))
		opt.expect_max_text_disagreement_rate = minmax(StrDbl(cla.GetArg("expect-max-text-disagreement-rate")), 0.0, 1.0);
	opt.quit_after_run = cli_quit_after_run;
	opt.semantic_summary = cli_semantic_summary;
	opt.dump_ocr = cli_dump_ocr;
	opt.dump_ocr_shifted_only = cli_dump_ocr_shifted_only;
	opt.dump_ocr_verbose = cli_dump_ocr_verbose;
	opt.compare_ocr_to_annotations = cli_compare_ocr;
	opt.compare_ocr_diff_only = cli_compare_ocr_diff_only;
	opt.compare_ocr_filter = cli_compare_ocr_filter ? TrimBoth(cla.GetArg("compare-ocr-filter")) : String();
	opt.export_ocr_compare_json_path = cli_export_ocr_compare_json ? cla.GetArg("export-ocr-compare-json") : String();
	opt.dump_ocr_filter = cli_dump_ocr_filter ? TrimBoth(cla.GetArg("dump-ocr-filter")) : String();
	opt.orb_find = cli_orb_find;
	opt.verbose = cli_verbose;
	opt.debug_orb = cli_debug_orb;
	opt.fast_exec = cli_fast_exec;
	opt.profile_exec = cli_profile_exec;
	opt.gpu_preprocess = cli_gpu_preprocess;
	opt.gpu_probe = cli_gpu_probe;
	opt.gpu_backend = cla.IsArg("gpu-backend") ? GpuPreprocessEngine::NormalizeBackend(cla.GetArg("gpu-backend")) : String("auto");
	opt.gpu_route = cla.IsArg("gpu-route") ? GpuPreprocessEngine::NormalizeRoute(cla.GetArg("gpu-route")) : String("auto");
	opt.save_frame_state = cli_save_frame_state;
	opt.skip_autoexec = cli_skip_autoexec;
	opt.test_orb = cli_test_orb;
	opt.test_eglimage = cli_test_eglimage;
	opt.test_v4l2_eglimage = cli_test_v4l2_eglimage;
	opt.test_d3d11_interop = cli_test_d3d11_interop;
	opt.test_jpg = cli_test_jpg;
	opt.test_png = cli_test_png;
	if (cla.IsArg("video-device"))
		opt.video_device = cla.GetArg("video-device");
	if (cla.IsArg("video-format-policy"))
		opt.video_format_policy = ToLower(TrimBoth(cla.GetArg("video-format-policy")));
	opt.test_tesseract_render = cli_test_tesseract_render;
	opt.test_convnet_cards = cli_test_convnet_cards;
	opt.test_convnet_disk_pipeline = cli_test_convnet_disk;
	if (opt.dump_ocr_shifted_only)
		opt.dump_ocr = true;
	if (opt.dump_ocr_verbose)
		opt.dump_ocr = true;
	if (opt.compare_ocr_to_annotations)
		opt.dump_ocr = true;
	if (!opt.compare_ocr_filter.IsEmpty()) {
		opt.compare_ocr_to_annotations = true;
		opt.dump_ocr = true;
	}
	if (opt.compare_ocr_diff_only) {
		opt.compare_ocr_to_annotations = true;
		opt.dump_ocr = true;
	}
	if (!opt.export_ocr_compare_json_path.IsEmpty()) {
		opt.compare_ocr_to_annotations = true;
		opt.dump_ocr = true;
	}
	if (opt.expect_ocr_compare_exact_min >= 0 || opt.expect_ocr_compare_normalized_min >= 0 || opt.expect_ocr_compare_missing_max >= 0 ||
	    opt.expect_ocr_compare_total_min >= 0 || opt.expect_ocr_compare_total_max >= 0 ||
	    opt.expect_ocr_compare_normalized_rate_min >= 0.0 || opt.expect_ocr_compare_missing_rate_max >= 0.0) {
		opt.compare_ocr_to_annotations = true;
		opt.dump_ocr = true;
	}
	if (!opt.dump_ocr_filter.IsEmpty())
		opt.dump_ocr = true;
	if (!opt.harvest_text_patches_dir.IsEmpty())
		opt.run_one = true;
	if (opt.dump_ocr && !opt.run_one && !opt.run_all && !opt.live_remote_test)
		opt.run_one = true;
	if (cla.IsArg("convnet-iters"))
		opt.convnet_iters = max(1, StrInt(cla.GetArg("convnet-iters")));
	if (cla.IsArg("train-cards")) opt.train_cards = true;
	if (cla.IsArg("training-minutes")) opt.training_minutes = max(1, StrInt(cla.GetArg("training-minutes")));
	if (cla.IsArg("dataset-path")) opt.dataset_path = cla.GetArg("dataset-path");
	if (cla.IsArg("model-tag")) opt.model_tag = cla.GetArg("model-tag");
	if (cla.IsArg("convnet-dump-dir"))
		opt.convnet_dump_dir = cla.GetArg("convnet-dump-dir");
	if (cla.IsArg("convnet-data-dir"))
		opt.convnet_data_dir = cla.GetArg("convnet-data-dir");
	if (cla.IsArg("convnet-model-dir"))
		opt.convnet_model_dir = cla.GetArg("convnet-model-dir");
	if (cla.IsArg("convnet-disk-tag"))
		opt.convnet_disk_tag = ToLower(TrimBoth(cla.GetArg("convnet-disk-tag")));
	if (cla.IsArg("openai-key-file"))
		opt.openai_key_file = cla.GetArg("openai-key-file");
	if (cla.IsArg("openai-model"))
		opt.openai_model = TrimBoth(cla.GetArg("openai-model"));

	String ini_path = GetDefaultIniPath();
	String ini_autoexec;
	if (FileExists(ini_path)) {
		VectorMap<String, String> ini = LoadIniFile(ini_path);
		auto ini_get = [&](const char* key) -> String { int q = ini.Find(key); return q >= 0 ? ini[q] : String(); };
		if (opt.project_name.IsEmpty()) opt.project_name = ini_get("project");
		if (opt.remote_addr.IsEmpty()) opt.remote_addr = ini_get("remote");
		if (opt.platform_name.IsEmpty()) opt.platform_name = ini_get("platform");
		if (opt.provider_name.IsEmpty()) opt.provider_name = ini_get("provider");
			if (opt.frame_arg.IsEmpty()) opt.frame_arg = ini_get("frame");
		if (opt.rule_name.IsEmpty()) opt.rule_name = ini_get("rule");
		if (opt.pattern_arg.IsEmpty()) opt.pattern_arg = ini_get("pattern");
			if (!opt.verbose) opt.verbose = IniValueTrue(ini_get("verbose"));
			if (!opt.gpu_preprocess) opt.gpu_preprocess = IniValueTrue(ini_get("gpu_preprocess"));
			if (opt.gpu_backend == "auto") {
				String v = ini_get("gpu_backend");
				if (!v.IsEmpty()) opt.gpu_backend = GpuPreprocessEngine::NormalizeBackend(v);
			}
			if (opt.gpu_route == "auto") {
				String v = ini_get("gpu_route");
				if (!v.IsEmpty()) opt.gpu_route = GpuPreprocessEngine::NormalizeRoute(v);
			}
			if (opt.openai_key_file.IsEmpty()) opt.openai_key_file = ini_get("openai_key_file");
			if (opt.openai_model.IsEmpty()) opt.openai_model = ini_get("openai_model");
			ini_autoexec = ini_get("autoexec");
	}

	if (!ini_autoexec.IsEmpty() && !opt.skip_autoexec) {
		String autoexec_path = ini_autoexec;
		if (!IsFullPath(autoexec_path)) autoexec_path = AppendFileName(GetFileDirectory(ini_path), autoexec_path);
		if (FileExists(autoexec_path)) {
			Vector<String> tokens = ParseCommandLineText(LoadFile(autoexec_path));
			CommandLineArguments auto_cla;
			AddScreenGameArgs(auto_cla);
			if (auto_cla.Parse(tokens)) {
				if (!cli_project && auto_cla.IsArg("project")) opt.project_name = auto_cla.GetArg("project");
				if (!cli_remote && auto_cla.IsArg("remote")) opt.remote_addr = auto_cla.GetArg("remote");
				if (!cli_platform && auto_cla.IsArg("platform")) opt.platform_name = auto_cla.GetArg("platform");
				if (!cli_provider && auto_cla.IsArg("provider")) opt.provider_name = auto_cla.GetArg("provider");
				if (!cli_frame && auto_cla.IsArg("frame")) opt.frame_arg = auto_cla.GetArg("frame");
				if (!cli_rule && auto_cla.IsArg("rule")) opt.rule_name = auto_cla.GetArg("rule");
				if (!cli_list_platforms && auto_cla.IsArg("list-platforms")) opt.list_platforms = true;
				if (!cli_list_frames && auto_cla.IsArg("list-frames")) opt.list_frames = true;
				if (!cli_list_rules && auto_cla.IsArg("list-rules")) opt.list_rules = true;
				if (!cli_list_props && auto_cla.IsArg("list-props")) opt.list_props = true;
				if (!cli_list_images && auto_cla.IsArg("list-images")) opt.list_images = true;
					if (!cli_run_one && auto_cla.IsArg("run")) opt.run_one = true;
					if (!cli_run_all && auto_cla.IsArg("run-all")) opt.run_all = true;
					if (!cli_live_remote_test && auto_cla.IsArg("live-remote-test")) opt.live_remote_test = true;
					if (!cli_synthetic_window_motion_test && auto_cla.IsArg("synthetic-window-motion-test")) opt.synthetic_window_motion_test = true;
					if (!cla.IsArg("live-seconds") && auto_cla.IsArg("live-seconds")) opt.live_seconds = max(1, StrInt(auto_cla.GetArg("live-seconds")));
					if (!cla.IsArg("synthetic-fps") && auto_cla.IsArg("synthetic-fps")) opt.synthetic_fps = max(1, StrInt(auto_cla.GetArg("synthetic-fps")));
				if (!cla.IsArg("live-warmup-frames") && auto_cla.IsArg("live-warmup-frames")) opt.live_warmup_frames = max(0, StrInt(auto_cla.GetArg("live-warmup-frames")));
				if (!cla.IsArg("temporal-local-dx") && auto_cla.IsArg("temporal-local-dx")) opt.temporal_local_dx = max(0, StrInt(auto_cla.GetArg("temporal-local-dx")));
				if (!cla.IsArg("temporal-local-dy") && auto_cla.IsArg("temporal-local-dy")) opt.temporal_local_dy = max(0, StrInt(auto_cla.GetArg("temporal-local-dy")));
				if (!cla.IsArg("decode-threads") && auto_cla.IsArg("decode-threads")) opt.decode_threads = max(1, StrInt(auto_cla.GetArg("decode-threads")));
				if (!cla.IsArg("queue-capacity") && auto_cla.IsArg("queue-capacity")) opt.queue_capacity = max(1, StrInt(auto_cla.GetArg("queue-capacity")));
				if (!cla.IsArg("live-rule-cache-limit") && auto_cla.IsArg("live-rule-cache-limit")) opt.live_rule_cache_limit = max(8, StrInt(auto_cla.GetArg("live-rule-cache-limit")));
				if (!cla.IsArg("drop-policy") && auto_cla.IsArg("drop-policy")) {
					opt.drop_policy = ToLower(TrimBoth(auto_cla.GetArg("drop-policy")));
					if (opt.drop_policy != "oldest" && opt.drop_policy != "newest")
						opt.drop_policy = "oldest";
				}
				if (opt.live_report_json.IsEmpty() && auto_cla.IsArg("live-report-json"))
					opt.live_report_json = auto_cla.GetArg("live-report-json");
				if (!cli_override_keep_keyframe && auto_cla.IsArg("override-keep-keyframe")) opt.override_keep_keyframe = true;
				if (!cli_exec_script_debug && auto_cla.IsArg("exec-script-debug")) opt.exec_script_debug = true;
				if (!cli_validate_frame && auto_cla.IsArg("validate-frame")) opt.validate_frame = true;
				if (!cli_trace_script_api && auto_cla.IsArg("trace-script-api")) opt.trace_script_api = true;
				if (!cla.IsArg("expect-window-min") && auto_cla.IsArg("expect-window-min")) opt.expect_window_min = max(0, StrInt(auto_cla.GetArg("expect-window-min")));
				if (!cla.IsArg("expect-script-objects-min") && auto_cla.IsArg("expect-script-objects-min")) opt.expect_script_objects_min = max(0, StrInt(auto_cla.GetArg("expect-script-objects-min")));
				if (!cla.IsArg("expect-resolved-min") && auto_cla.IsArg("expect-resolved-min")) opt.expect_resolved_min = max(0, StrInt(auto_cla.GetArg("expect-resolved-min")));
				if (!cla.IsArg("expect-class-semantic-min") && auto_cla.IsArg("expect-class-semantic-min")) opt.expect_class_semantic_min = max(0, StrInt(auto_cla.GetArg("expect-class-semantic-min")));
				if (!cla.IsArg("expect-text-semantic-min") && auto_cla.IsArg("expect-text-semantic-min")) opt.expect_text_semantic_min = max(0, StrInt(auto_cla.GetArg("expect-text-semantic-min")));
				if (!cla.IsArg("expect-policy-semantic-min") && auto_cla.IsArg("expect-policy-semantic-min")) opt.expect_policy_semantic_min = max(0, StrInt(auto_cla.GetArg("expect-policy-semantic-min")));
				if (!cla.IsArg("expect-text-backend-status-min") && auto_cla.IsArg("expect-text-backend-status-min")) opt.expect_text_backend_status_min = max(0, StrInt(auto_cla.GetArg("expect-text-backend-status-min")));
				if (!cla.IsArg("expect-class-orb-status-min") && auto_cla.IsArg("expect-class-orb-status-min")) opt.expect_class_orb_status_min = max(0, StrInt(auto_cla.GetArg("expect-class-orb-status-min")));
				if (!cla.IsArg("expect-class-nn-status-min") && auto_cla.IsArg("expect-class-nn-status-min")) opt.expect_class_nn_status_min = max(0, StrInt(auto_cla.GetArg("expect-class-nn-status-min")));
				if (!cla.IsArg("expect-class-alt-min") && auto_cla.IsArg("expect-class-alt-min")) opt.expect_class_alt_min = max(0, StrInt(auto_cla.GetArg("expect-class-alt-min")));
				if (!cla.IsArg("expect-threshold-pass-min") && auto_cla.IsArg("expect-threshold-pass-min")) opt.expect_threshold_pass_min = max(0, StrInt(auto_cla.GetArg("expect-threshold-pass-min")));
				if (!cla.IsArg("expect-text-rect-total-min") && auto_cla.IsArg("expect-text-rect-total-min")) opt.expect_text_rect_total_min = max(0, StrInt(auto_cla.GetArg("expect-text-rect-total-min")));
				if (!cla.IsArg("expect-text-rect-shifted-min") && auto_cla.IsArg("expect-text-rect-shifted-min")) opt.expect_text_rect_shifted_min = max(0, StrInt(auto_cla.GetArg("expect-text-rect-shifted-min")));
				if (!cla.IsArg("expect-min-text-rect-shift-rate") && auto_cla.IsArg("expect-min-text-rect-shift-rate")) opt.expect_min_text_rect_shift_rate = minmax(StrDbl(auto_cla.GetArg("expect-min-text-rect-shift-rate")), 0.0, 1.0);
				if (!cla.IsArg("expect-ocr-compare-exact-min") && auto_cla.IsArg("expect-ocr-compare-exact-min")) opt.expect_ocr_compare_exact_min = max(0, StrInt(auto_cla.GetArg("expect-ocr-compare-exact-min")));
				if (!cla.IsArg("expect-ocr-compare-normalized-min") && auto_cla.IsArg("expect-ocr-compare-normalized-min")) opt.expect_ocr_compare_normalized_min = max(0, StrInt(auto_cla.GetArg("expect-ocr-compare-normalized-min")));
				if (!cla.IsArg("expect-ocr-compare-missing-max") && auto_cla.IsArg("expect-ocr-compare-missing-max")) opt.expect_ocr_compare_missing_max = max(0, StrInt(auto_cla.GetArg("expect-ocr-compare-missing-max")));
				if (!cla.IsArg("expect-ocr-compare-total-min") && auto_cla.IsArg("expect-ocr-compare-total-min")) opt.expect_ocr_compare_total_min = max(0, StrInt(auto_cla.GetArg("expect-ocr-compare-total-min")));
				if (!cla.IsArg("expect-ocr-compare-total-max") && auto_cla.IsArg("expect-ocr-compare-total-max")) opt.expect_ocr_compare_total_max = max(0, StrInt(auto_cla.GetArg("expect-ocr-compare-total-max")));
				if (!cla.IsArg("expect-ocr-compare-normalized-rate-min") && auto_cla.IsArg("expect-ocr-compare-normalized-rate-min")) opt.expect_ocr_compare_normalized_rate_min = minmax(StrDbl(auto_cla.GetArg("expect-ocr-compare-normalized-rate-min")), 0.0, 1.0);
				if (!cla.IsArg("expect-ocr-compare-missing-rate-max") && auto_cla.IsArg("expect-ocr-compare-missing-rate-max")) opt.expect_ocr_compare_missing_rate_max = minmax(StrDbl(auto_cla.GetArg("expect-ocr-compare-missing-rate-max")), 0.0, 1.0);
				if (!cla.IsArg("expect-text-backends") && auto_cla.IsArg("expect-text-backends")) opt.expect_text_backends = TrimBoth(auto_cla.GetArg("expect-text-backends"));
				if (!cla.IsArg("expect-max-text-total-us") && auto_cla.IsArg("expect-max-text-total-us")) opt.expect_max_text_total_us = max(0, StrInt(auto_cla.GetArg("expect-max-text-total-us")));
				if (!cla.IsArg("expect-max-text-disagreement-rate") && auto_cla.IsArg("expect-max-text-disagreement-rate")) opt.expect_max_text_disagreement_rate = minmax(StrDbl(auto_cla.GetArg("expect-max-text-disagreement-rate")), 0.0, 1.0);
				if (!cli_quit_after_run && auto_cla.IsArg("quit-after-run")) opt.quit_after_run = true;
				if (!cli_semantic_summary && auto_cla.IsArg("semantic-summary")) opt.semantic_summary = true;
				if (!cli_dump_ocr && auto_cla.IsArg("dump-ocr")) opt.dump_ocr = true;
				if (!cli_dump_ocr_shifted_only && auto_cla.IsArg("dump-ocr-shifted-only")) opt.dump_ocr_shifted_only = true;
				if (!cli_dump_ocr_verbose && auto_cla.IsArg("dump-ocr-verbose")) opt.dump_ocr_verbose = true;
				if (!cli_compare_ocr && auto_cla.IsArg("compare-ocr-to-annotations")) opt.compare_ocr_to_annotations = true;
				if (!cli_compare_ocr_diff_only && auto_cla.IsArg("compare-ocr-diff-only")) opt.compare_ocr_diff_only = true;
				if (!cli_compare_ocr_filter && auto_cla.IsArg("compare-ocr-filter")) opt.compare_ocr_filter = TrimBoth(auto_cla.GetArg("compare-ocr-filter"));
				if (!cli_export_ocr_compare_json && auto_cla.IsArg("export-ocr-compare-json")) opt.export_ocr_compare_json_path = auto_cla.GetArg("export-ocr-compare-json");
				if (!cli_dump_ocr_filter && auto_cla.IsArg("dump-ocr-filter")) opt.dump_ocr_filter = TrimBoth(auto_cla.GetArg("dump-ocr-filter"));
				if (!cli_orb_find && auto_cla.IsArg("orb-find")) opt.orb_find = true;
				if (opt.export_preview_path.IsEmpty() && auto_cla.IsArg("export-preview")) opt.export_preview_path = auto_cla.GetArg("export-preview");
				if (opt.export_exec_json_path.IsEmpty() && auto_cla.IsArg("export-exec-json")) opt.export_exec_json_path = auto_cla.GetArg("export-exec-json");
				if (opt.export_semantic_json_path.IsEmpty() && auto_cla.IsArg("export-semantic-json")) opt.export_semantic_json_path = auto_cla.GetArg("export-semantic-json");
				if (opt.export_card_patches_dir.IsEmpty() && auto_cla.IsArg("export-card-patches")) opt.export_card_patches_dir = auto_cla.GetArg("export-card-patches");
				if (opt.harvest_text_patches_dir.IsEmpty() && auto_cla.IsArg("harvest-text-patches")) opt.harvest_text_patches_dir = auto_cla.GetArg("harvest-text-patches");
				if (!cli_harvest_filter && auto_cla.IsArg("harvest-filter")) opt.harvest_filter = TrimBoth(auto_cla.GetArg("harvest-filter"));
				if (!cli_harvest_include_orb_same && auto_cla.IsArg("harvest-include-orb-same")) opt.harvest_include_orb_same = true;
				if (!cli_harvest_implicit_text && auto_cla.IsArg("harvest-implicit-text")) opt.harvest_implicit_text = true;
				if (opt.harvest_summary_dir.IsEmpty() && auto_cla.IsArg("harvest-summary")) opt.harvest_summary_dir = auto_cla.GetArg("harvest-summary");
				if (opt.harvest_summary_json_path.IsEmpty() && auto_cla.IsArg("harvest-summary-json")) opt.harvest_summary_json_path = auto_cla.GetArg("harvest-summary-json");
				if (!cli_harvest_status && auto_cla.IsArg("harvest-status")) opt.harvest_status_filter = ToLower(TrimBoth(auto_cla.GetArg("harvest-status")));
				if (!cli_harvest_queue && auto_cla.IsArg("harvest-queue")) opt.harvest_queue = true;
				if (!cli_harvest_duplicates && auto_cla.IsArg("harvest-duplicates")) opt.harvest_duplicates = true;
				if (!cli_harvest_dedup_reject && auto_cla.IsArg("harvest-dedup-reject")) opt.harvest_dedup_reject = true;
				if (!cli_harvest_dedup_reviewer && auto_cla.IsArg("harvest-dedup-reviewer")) opt.harvest_dedup_reviewer = auto_cla.GetArg("harvest-dedup-reviewer");
				if (!cli_harvest_edit_match && auto_cla.IsArg("harvest-edit-match")) opt.harvest_edit_match = TrimBoth(auto_cla.GetArg("harvest-edit-match"));
				if (!cli_harvest_set_status && auto_cla.IsArg("harvest-set-status")) opt.harvest_set_status = ToLower(TrimBoth(auto_cla.GetArg("harvest-set-status")));
				if (!cli_harvest_set_label && auto_cla.IsArg("harvest-set-label")) opt.harvest_set_label = auto_cla.GetArg("harvest-set-label");
				if (!cli_harvest_set_reviewer && auto_cla.IsArg("harvest-set-reviewer")) opt.harvest_set_reviewer = auto_cla.GetArg("harvest-set-reviewer");
				if (!cli_harvest_apply_labels && auto_cla.IsArg("harvest-apply-labels")) opt.harvest_apply_labels_path = auto_cla.GetArg("harvest-apply-labels");
				if (!cli_harvest_export_labels && auto_cla.IsArg("harvest-export-labels")) opt.harvest_export_labels_path = auto_cla.GetArg("harvest-export-labels");
				if (!cli_harvest_char_coverage && auto_cla.IsArg("harvest-char-coverage")) opt.harvest_char_coverage = true;
				if (!cli_harvest_required_chars && auto_cla.IsArg("harvest-required-chars")) opt.harvest_required_chars = auto_cla.GetArg("harvest-required-chars");
				if (!cli_harvest_readiness && auto_cla.IsArg("harvest-readiness")) opt.harvest_readiness = true;
				if (!cli_harvest_required_labels && auto_cla.IsArg("harvest-required-labels")) opt.harvest_required_labels = TrimBoth(auto_cla.GetArg("harvest-required-labels"));
				if (!cla.IsArg("harvest-min-per-label") && auto_cla.IsArg("harvest-min-per-label")) opt.harvest_min_per_label = max(1, StrInt(auto_cla.GetArg("harvest-min-per-label")));
				if (!cla.IsArg("harvest-duplicates-limit") && auto_cla.IsArg("harvest-duplicates-limit")) opt.harvest_duplicates_limit = max(1, StrInt(auto_cla.GetArg("harvest-duplicates-limit")));
				if (!cli_verbose && auto_cla.IsArg("verbose")) opt.verbose = true;
				if (!cli_fast_exec && auto_cla.IsArg("fast-exec")) opt.fast_exec = true;
				if (!cli_profile_exec && auto_cla.IsArg("profile-exec")) opt.profile_exec = true;
				if (!cli_gpu_preprocess && auto_cla.IsArg("gpu-preprocess")) opt.gpu_preprocess = true;
				if (!cla.IsArg("gpu-route") && auto_cla.IsArg("gpu-route")) opt.gpu_route = GpuPreprocessEngine::NormalizeRoute(auto_cla.GetArg("gpu-route"));
				if (!cla.IsArg("gpu-backend") && auto_cla.IsArg("gpu-backend")) opt.gpu_backend = GpuPreprocessEngine::NormalizeBackend(auto_cla.GetArg("gpu-backend"));
				if (!cli_save_frame_state && auto_cla.IsArg("save-frame-state")) opt.save_frame_state = true;
				if (!cli_test_convnet_cards && auto_cla.IsArg("test-convnet-cards")) opt.test_convnet_cards = true;
				if (!cli_test_convnet_disk && auto_cla.IsArg("test-convnet-disk-pipeline")) opt.test_convnet_disk_pipeline = true;
				if (!cla.IsArg("convnet-iters") && auto_cla.IsArg("convnet-iters")) opt.convnet_iters = max(1, StrInt(auto_cla.GetArg("convnet-iters")));
				if (!cla.IsArg("convnet-dump-dir") && auto_cla.IsArg("convnet-dump-dir")) opt.convnet_dump_dir = auto_cla.GetArg("convnet-dump-dir");
				if (!cla.IsArg("convnet-data-dir") && auto_cla.IsArg("convnet-data-dir")) opt.convnet_data_dir = auto_cla.GetArg("convnet-data-dir");
					if (!cla.IsArg("convnet-model-dir") && auto_cla.IsArg("convnet-model-dir")) opt.convnet_model_dir = auto_cla.GetArg("convnet-model-dir");
					if (!cla.IsArg("convnet-disk-tag") && auto_cla.IsArg("convnet-disk-tag")) opt.convnet_disk_tag = ToLower(TrimBoth(auto_cla.GetArg("convnet-disk-tag")));
					if (!cla.IsArg("openai-key-file") && auto_cla.IsArg("openai-key-file")) opt.openai_key_file = auto_cla.GetArg("openai-key-file");
					if (!cla.IsArg("openai-model") && auto_cla.IsArg("openai-model")) opt.openai_model = TrimBoth(auto_cla.GetArg("openai-model"));
				}
			}
		}
	if (opt.validate_frame)
		opt.run_one = true;
	if (opt.dump_ocr)
		opt.run_one = true;

	opt.headless = opt.train_cards || opt.convnet_generate_dataset_texas || opt.list_platforms || opt.list_frames || opt.list_rules || opt.list_props || opt.list_images || opt.run_one || opt.run_all || opt.live_remote_test || opt.synthetic_window_motion_test || opt.exec_script_debug || opt.validate_frame || opt.semantic_summary || opt.dump_ocr || opt.orb_find || opt.test_orb || opt.test_jpg || opt.test_png || opt.test_eglimage || opt.test_v4l2_eglimage || opt.test_tesseract_render || opt.test_convnet_cards || opt.test_convnet_disk_pipeline || opt.gpu_probe || !opt.harvest_summary_dir.IsEmpty();
	if ((opt.harvest_queue || opt.harvest_duplicates || opt.harvest_dedup_reject || opt.harvest_readiness || !opt.harvest_required_labels.IsEmpty()) && opt.harvest_summary_dir.IsEmpty()) {
		Cout() << "Error: --harvest-summary is required with --harvest-queue/--harvest-duplicates/--harvest-dedup-reject/--harvest-readiness/--harvest-required-labels.\n";
		return false;
	}
	if ((!opt.harvest_edit_match.IsEmpty() || !opt.harvest_set_status.IsEmpty() || !opt.harvest_set_label.IsEmpty() || !opt.harvest_set_reviewer.IsEmpty() || !opt.harvest_apply_labels_path.IsEmpty() || !opt.harvest_export_labels_path.IsEmpty() || opt.harvest_char_coverage || !opt.harvest_required_chars.IsEmpty()) && opt.harvest_summary_dir.IsEmpty()) {
		Cout() << "Error: --harvest-summary is required with harvest edit args.\n";
		return false;
	}
	if (!opt.harvest_summary_json_path.IsEmpty() && opt.harvest_summary_dir.IsEmpty()) {
		Cout() << "Error: --harvest-summary is required with --harvest-summary-json.\n";
		return false;
	}
	if (opt.harvest_status_filter.IsEmpty())
		opt.harvest_status_filter = "any";
	if (opt.harvest_edit_match.IsEmpty() && (!opt.harvest_set_status.IsEmpty() || !opt.harvest_set_label.IsEmpty() || !opt.harvest_set_reviewer.IsEmpty()))
		opt.harvest_edit_match = "*";
	return true;
}






// Moved to HeadlessCardEngine.cpp

// Moved to HeadlessCardEngine.cpp

// Moved to HeadlessCardEngine.cpp
int RunScreenGameHeadless(const ScreenGameCliOptions& opt) {
	Upp::Cout() << "DEBUG: RunScreenGameHeadless platform_name=" << opt.platform_name << " list_rules=" << (int)opt.list_rules << " list_platforms=" << (int)opt.list_platforms << "\n";
	g_match_fast_exec = opt.fast_exec;
	g_match_profile_exec = opt.profile_exec;
	if (opt.gpu_probe) {
		GpuBackendProbe probe = GpuPreprocessEngine::ProbeCapabilities(true);
		Cout() << GpuPreprocessEngine::FormatProbeReport(probe);
		if (opt.gpu_route != "auto")
			Cout() << "gpu_probe requested_route=" << opt.gpu_route << " available=" <<
			         ((opt.gpu_route == "egl-surfaceless" && probe.egl_surfaceless_ok) ||
			          (opt.gpu_route == "egl-x11" && probe.egl_x11_ok) ||
			          (opt.gpu_route == "glx-x11" && probe.glx_x11_ok) ||
			          (opt.gpu_route == "stub-cpu") ? "1" : "0") << "\n";
		return 0;
	}
	if (opt.test_orb) return RunOrbSelfTest();
	#ifndef PLATFORM_WIN32
	if (opt.test_eglimage) return RunEGLImageSelfTest();
	if (opt.test_v4l2_eglimage) return RunV4L2EGLImageSelfTest(opt.video_device, opt.video_format_policy);
	if (opt.test_d3d11_interop) {
		Cout() << "d3d11_interop_test_result=skip (Linux not supported for this test)\n";
		return 0;
	}
	#else
	if (opt.test_eglimage) {
		Cout() << "eglimage_test_result=skip (Windows not supported for this test)\n";
		return 0;
	}
	if (opt.test_v4l2_eglimage) {
		Cout() << "v4l2_eglimage_test_result=skip (Windows not supported for this test)\n";
		return 0;
	}
	if (opt.test_d3d11_interop) {
		extern int RunD3D11InteropSelfTest();
		return RunD3D11InteropSelfTest();
	}
	#endif
	if (opt.test_jpg) return RunJpgSelfTest();
	if (opt.test_png) return RunPngSelfTest();
	if (opt.test_tesseract_render) return RunTesseractRenderSelfTest();
	if (opt.test_convnet_cards) {
		String dump_dir = opt.convnet_dump_dir;
		if (!dump_dir.IsEmpty() && !IsFullPath(dump_dir))
			dump_dir = AppendFileName(GetCurrentDirectory(), dump_dir);
		return RunConvNetCardsSelfTest(opt.convnet_iters, dump_dir);
	}
	if (opt.test_convnet_disk_pipeline) {
		String data_dir = opt.convnet_data_dir;
		if (data_dir.IsEmpty())
			data_dir = AppendFileName(AppendFileName(GetCurrentDirectory(), "tmp"), "convnet_cards");
		else if (!IsFullPath(data_dir))
			data_dir = AppendFileName(GetCurrentDirectory(), data_dir);
		String model_dir = opt.convnet_model_dir;
		if (model_dir.IsEmpty())
			model_dir = AppendFileName(AppendFileName(GetCurrentDirectory(), "tmp"), "convnet_models");
		else if (!IsFullPath(model_dir))
			model_dir = AppendFileName(GetCurrentDirectory(), model_dir);
		Cout() << "convnet_disk_data_dir=" << data_dir << "\n";
		Cout() << "convnet_disk_model_dir=" << model_dir << "\n";
		Cout() << "convnet_disk_tag=" << opt.convnet_disk_tag << "\n";
		return RunConvNetDiskPipelineTest(data_dir, model_dir, opt.convnet_iters, opt.convnet_disk_tag);
	}
	if (!opt.harvest_summary_dir.IsEmpty()) {
		String root = opt.harvest_summary_dir;
		if (!IsFullPath(root))
			root = AppendFileName(GetCurrentDirectory(), root);
		if (!opt.harvest_apply_labels_path.IsEmpty()) {
			String p = opt.harvest_apply_labels_path;
			if (!IsFullPath(p))
				p = AppendFileName(GetCurrentDirectory(), p);
			if (!FileExists(p)) {
				Cout() << "Error: harvest labels file not found: " << p << "\n";
				return 1;
			}
			Vector<String> lines = Split(LoadFile(p), '\n');
			int edited = 0;
			for (int li = 0; li < lines.GetCount(); li++) {
				String ln = TrimBoth(lines[li]);
				if (ln.IsEmpty() || ln.StartsWith("#"))
					continue;
				Vector<String> cols = Split(ln, '\t');
				if (cols.GetCount() < 2)
					continue;
				String patt = TrimBoth(cols[0]);
				String label = cols[1];
				String status = cols.GetCount() > 2 ? ToLower(TrimBoth(cols[2])) : String();
				String reviewer = cols.GetCount() > 3 ? cols[3] : String();
				if (TrimBoth(label).IsEmpty() && status.IsEmpty() && reviewer.IsEmpty())
					continue;
				if (status.IsEmpty() && !TrimBoth(label).IsEmpty())
					status = "labeled";
				if (patt.IsEmpty())
					patt = "*";
				FindFile dir_ff(AppendFileName(root, "*"));
				while (dir_ff) {
					if (dir_ff.IsFolder() && dir_ff.GetName()[0] != '.') {
						String bucket = dir_ff.GetName();
						String bucket_dir = AppendFileName(root, bucket);
						FindFile ff(AppendFileName(bucket_dir, "*.png"));
						while (ff) {
							if (ff.IsFile()) {
								String img_path = AppendFileName(bucket_dir, ff.GetName());
								if (!PatternMatchMulti(patt, img_path) &&
								    !PatternMatchMulti(patt, bucket + "/" + ff.GetName()) &&
								    !PatternMatchMulti(patt, ff.GetName())) {
									ff.Next();
									continue;
								}
								RuleImageMetadata md;
								if (!LoadRuleImageMetadata(img_path, md))
									md = RuleImageMetadata();
								if (!TrimBoth(label).IsEmpty())
									md.label = label;
								if (!status.IsEmpty())
									md.review_status = status;
								if (!reviewer.IsEmpty())
									md.reviewer = reviewer;
								if (md.captured_at.IsEmpty())
									md.captured_at = AsString(GetSysTime());
								if (SaveRuleImageMetadata(img_path, md))
									edited++;
							}
							ff.Next();
						}
					}
					dir_ff.Next();
				}
			}
			Cout() << "harvest_apply_labels edited=" << edited << " file=" << p << "\n";
		}
		if (!opt.harvest_edit_match.IsEmpty() || !opt.harvest_set_status.IsEmpty() || !opt.harvest_set_label.IsEmpty() || !opt.harvest_set_reviewer.IsEmpty()) {
			int edited = 0;
			FindFile dir_ff(AppendFileName(root, "*"));
			while (dir_ff) {
				if (dir_ff.IsFolder() && dir_ff.GetName()[0] != '.') {
					String bucket = dir_ff.GetName();
					String bucket_dir = AppendFileName(root, bucket);
					FindFile ff(AppendFileName(bucket_dir, "*.png"));
					while (ff) {
						if (ff.IsFile()) {
							String img_path = AppendFileName(bucket_dir, ff.GetName());
							if (!PatternMatchMulti(opt.harvest_edit_match, img_path) &&
							    !PatternMatchMulti(opt.harvest_edit_match, bucket + "/" + ff.GetName()) &&
							    !PatternMatchMulti(opt.harvest_edit_match, ff.GetName())) {
								ff.Next();
								continue;
							}
							RuleImageMetadata md;
							if (!LoadRuleImageMetadata(img_path, md))
								md = RuleImageMetadata();
							if (!opt.harvest_set_status.IsEmpty())
								md.review_status = opt.harvest_set_status;
							if (!opt.harvest_set_label.IsEmpty())
								md.label = opt.harvest_set_label;
							if (!opt.harvest_set_reviewer.IsEmpty())
								md.reviewer = opt.harvest_set_reviewer;
							if (md.captured_at.IsEmpty())
								md.captured_at = AsString(GetSysTime());
							if (SaveRuleImageMetadata(img_path, md))
								edited++;
						}
						ff.Next();
					}
				}
				dir_ff.Next();
			}
			Cout() << "harvest_edit edited=" << edited << " match=" << opt.harvest_edit_match << "\n";
		}
		HarvestDatasetStats st;
		Vector<String> queue_lines;
		bool need_queue = opt.harvest_queue || !opt.harvest_export_labels_path.IsEmpty();
		if (!CollectHarvestDatasetStats(root, opt.harvest_status_filter, st, need_queue ? &queue_lines : nullptr)) {
			Cout() << "Error: harvest summary dir not found: " << root << "\n";
			return 1;
		}
		Cout() << "harvest_summary dir=" << root
		       << " filter=" << opt.harvest_status_filter
		       << " buckets=" << st.buckets
		       << " total=" << st.total
		       << " unlabeled=" << st.unlabeled
		       << " labeled=" << st.labeled
		       << " reviewed=" << st.reviewed
		       << " rejected=" << st.rejected << "\n";
		for (int i = 0; i < st.status_counts.GetCount(); i++)
			Cout() << "harvest_status " << st.status_counts.GetKey(i) << "=" << st.status_counts[i] << "\n";
		for (int i = 0; i < st.label_counts.GetCount(); i++)
			Cout() << "harvest_label " << st.label_counts.GetKey(i) << "=" << st.label_counts[i] << "\n";
		if (opt.harvest_queue) {
			Cout() << "harvest_queue_count=" << queue_lines.GetCount() << "\n";
			for (int i = 0; i < queue_lines.GetCount(); i++)
				Cout() << "harvest_queue " << queue_lines[i] << "\n";
		}
		if (!opt.harvest_export_labels_path.IsEmpty()) {
			String out = opt.harvest_export_labels_path;
			if (!IsFullPath(out))
				out = AppendFileName(GetCurrentDirectory(), out);
			RealizeDirectory(GetFileDirectory(out));
			String tsv;
			tsv << "# pattern<TAB>label<TAB>status<TAB>reviewer\n";
			for (int i = 0; i < queue_lines.GetCount(); i++) {
				const String& q = queue_lines[i];
				int p = q.Find(' ');
				if (p < 0)
					continue;
				String path = q.Mid(p + 1);
				tsv << path << "\t\t\t\n";
			}
			if (SaveFile(out, tsv))
				Cout() << "harvest_export_labels=" << out << " rows=" << queue_lines.GetCount() << "\n";
		}
		int duplicate_groups = 0;
		int duplicate_items = 0;
		int dedup_rejected = 0;
		VectorMap<String, Vector<String>> dup_groups;
		if (opt.harvest_duplicates || opt.harvest_dedup_reject) {
			if (CollectHarvestDuplicateGroups(root, opt.harvest_status_filter, dup_groups, duplicate_items)) {
				duplicate_groups = dup_groups.GetCount();
				if (opt.harvest_dedup_reject) {
					for (int i = 0; i < dup_groups.GetCount(); i++) {
						Vector<String>& arr = dup_groups[i];
						Sort(arr);
						for (int j = 1; j < arr.GetCount(); j++) {
							RuleImageMetadata md;
							if (!LoadRuleImageMetadata(arr[j], md))
								continue;
							md.review_status = "rejected";
							md.reviewer = opt.harvest_dedup_reviewer;
							if (SaveRuleImageMetadata(arr[j], md))
								dedup_rejected++;
						}
					}
					Cout() << "harvest_dedup_reject rejected=" << dedup_rejected
					     << " reviewer=" << opt.harvest_dedup_reviewer << "\n";
				}
				if (opt.harvest_duplicates) {
				Cout() << "harvest_duplicates groups=" << duplicate_groups
				     << " items=" << duplicate_items
				     << " limit=" << opt.harvest_duplicates_limit << "\n";
				int lim = min(opt.harvest_duplicates_limit, dup_groups.GetCount());
				for (int i = 0; i < lim; i++) {
					const Vector<String>& arr = dup_groups[i];
					Cout() << "harvest_duplicate_group hash=" << dup_groups.GetKey(i)
					     << " count=" << arr.GetCount() << "\n";
					for (int j = 0; j < arr.GetCount(); j++)
						Cout() << "harvest_duplicate_item " << arr[j] << "\n";
				}
				}
			}
		}
		if (dedup_rejected > 0) {
			queue_lines.Clear();
			if (!CollectHarvestDatasetStats(root, opt.harvest_status_filter, st, need_queue ? &queue_lines : nullptr)) {
				Cout() << "Error: harvest summary refresh failed: " << root << "\n";
				return 1;
			}
			Cout() << "harvest_summary_refreshed total=" << st.total
			     << " unlabeled=" << st.unlabeled
			     << " labeled=" << st.labeled
			     << " reviewed=" << st.reviewed
			     << " rejected=" << st.rejected << "\n";
		}
		VectorMap<String, int> char_cov;
		int char_cov_rows = 0;
		bool required_chars_ok = true;
		if (opt.harvest_char_coverage || !opt.harvest_required_chars.IsEmpty()) {
			if (CollectHarvestCharCoverage(root, opt.harvest_status_filter, char_cov, char_cov_rows)) {
				Cout() << "harvest_char_coverage_rows=" << char_cov_rows
				     << " unique=" << char_cov.GetCount() << "\n";
				if (opt.harvest_char_coverage) {
					for (int i = 0; i < char_cov.GetCount(); i++)
						Cout() << "harvest_char \"" << char_cov.GetKey(i) << "\"=" << char_cov[i] << "\n";
				}
				if (!opt.harvest_required_chars.IsEmpty()) {
					Vector<String> missing;
					for (int i = 0; i < opt.harvest_required_chars.GetCount(); i++) {
						String ch = String(opt.harvest_required_chars.Mid(i, 1));
						bool already = false;
						for (int k = 0; k < missing.GetCount(); k++)
							if (missing[k] == ch) { already = true; break; }
						if (char_cov.Find(ch) < 0 && !already)
							missing.Add(ch);
					}
					Cout() << "harvest_required_chars missing=" << missing.GetCount()
					     << " result=" << (missing.IsEmpty() ? "PASS" : "FAIL") << "\n";
					if (!missing.IsEmpty())
						Cout() << "harvest_missing_chars=" << Join(missing, "") << "\n";
					if (!missing.IsEmpty())
						required_chars_ok = false;
				}
			}
		}
		bool summary_fail = false;
		if (opt.harvest_readiness) {
			bool ok = true;
			Vector<String> missing;
			Vector<String> low;
			Vector<String> req = Split(opt.harvest_required_labels, ",");
			for (int i = 0; i < req.GetCount(); i++) {
				String lbl = TrimBoth(req[i]);
				if (lbl.IsEmpty())
					continue;
				int q = st.label_counts.Find(lbl);
				int n = q >= 0 ? st.label_counts[q] : 0;
				if (n == 0) {
					missing.Add(lbl);
					ok = false;
				}
				else if (n < opt.harvest_min_per_label) {
					low.Add(lbl + ":" + AsString(n));
					ok = false;
				}
			}
			Cout() << "harvest_readiness min_per_label=" << opt.harvest_min_per_label
			       << " required=" << req.GetCount()
			       << " missing=" << missing.GetCount()
			       << " low=" << low.GetCount()
			       << " result=" << (ok ? "PASS" : "FAIL") << "\n";
			if (missing.GetCount())
				Cout() << "harvest_missing_labels=" << Join(missing, ",") << "\n";
			if (low.GetCount())
				Cout() << "harvest_low_labels=" << Join(low, ",") << "\n";
			if (!ok)
				summary_fail = true;
		}
		if (!required_chars_ok)
			summary_fail = true;
		if (!opt.harvest_summary_json_path.IsEmpty()) {
			struct HarvestSummaryJson : Moveable<HarvestSummaryJson> {
				String dir;
				String filter;
				int buckets = 0;
				int total = 0;
				int unlabeled = 0;
				int labeled = 0;
				int reviewed = 0;
				int rejected = 0;
				VectorMap<String, int> status_counts;
				VectorMap<String, int> label_counts;
						int queue_count = 0;
						int duplicate_groups = 0;
						int duplicate_items = 0;
						int dedup_rejected = 0;
						int char_rows = 0;
						VectorMap<String, int> char_counts;
						void Jsonize(JsonIO& jio) {
					jio("dir", dir)
					   ("filter", filter)
					   ("buckets", buckets)
					   ("total", total)
					   ("unlabeled", unlabeled)
					   ("labeled", labeled)
					   ("reviewed", reviewed)
					   ("rejected", rejected)
					   ("status_counts", status_counts)
					   ("label_counts", label_counts)
					   ("queue_count", queue_count)
					   ("duplicate_groups", duplicate_groups)
					   ("duplicate_items", duplicate_items)
					   ("dedup_rejected", dedup_rejected)
					   ("char_rows", char_rows)
					   ("char_counts", char_counts);
				}
			};
			HarvestSummaryJson sj;
			sj.dir = root;
			sj.filter = opt.harvest_status_filter;
			sj.buckets = st.buckets;
			sj.total = st.total;
			sj.unlabeled = st.unlabeled;
			sj.labeled = st.labeled;
			sj.reviewed = st.reviewed;
			sj.rejected = st.rejected;
			sj.status_counts <<= st.status_counts;
			sj.label_counts <<= st.label_counts;
			sj.queue_count = queue_lines.GetCount();
			sj.duplicate_groups = duplicate_groups;
			sj.duplicate_items = duplicate_items;
			sj.dedup_rejected = dedup_rejected;
			sj.char_rows = char_cov_rows;
			sj.char_counts <<= char_cov;
			String out = opt.harvest_summary_json_path;
			if (!IsFullPath(out))
				out = AppendFileName(GetCurrentDirectory(), out);
			RealizeDirectory(GetFileDirectory(out));
			if (SaveFile(out, StoreAsJson(sj)))
				Cout() << "harvest_summary_json=" << out << "\n";
		}
		return summary_fail ? 2 : 0;
	}
	RecognitionEngine rec;
	String rules_path = AppendFileName(GetExeDirFile(""), "pokerth.rules.json");
	Cout() << "Loading rules from: " << rules_path << "\n";
	rec.GetRuleManager().Load(rules_path);
	RuleManager& rm = rec.GetRuleManager();

	GpuPreprocessEngine gpu_pre;
	if (opt.gpu_preprocess) {
		GpuPreprocessConfig cfg;
		cfg.backend = opt.gpu_backend;
		cfg.route = opt.gpu_route;
		cfg.pyramid_levels = 3;
		cfg.compact_readback_only = true;
		if (!gpu_pre.Initialize(cfg)) {
			Cout() << "Error: failed to initialize GPU preprocess: " << gpu_pre.GetLastError() << "\n";
			return 1;
		}
	}

	if (opt.list_platforms) {
		Vector<String> platforms = rm.GetPlatformNames();
		for (const String& p : platforms) Cout() << p << "\n";
	}

	if (opt.list_rules) {
		if (!opt.platform_name.IsEmpty()) {
			rm.SetActivePlatform(opt.platform_name);
		}
		Cout() << "Platform: " << rm.GetActivePlatform() << "\n";
		const auto& rules = rm.GetRules();
		for (int i = 0; i < rules.GetCount(); i++) {
			const GameRule& r = rules[i];
			Cout() << "Rule: " << r.name << " Type: " << GetRuleClassName(r.type) << " Rect: " << r.rect << "\n";
		}
		Cout().Flush();
		return 0;
		}
	if (opt.list_platforms || opt.list_rules) return 0;

	if (opt.project_name.IsEmpty() && !opt.orb_find) {
		Cout() << "Error: --project is required for headless mode.\n";
		return 1;
	}
	if (opt.orb_find) {
		if (opt.frame_arg.IsEmpty() || opt.pattern_arg.IsEmpty()) {
			Cout() << "Error: --frame and --pattern are required for --orb-find.\n";
			return 1;
		}
		String frame_path = ResolveFramePath(opt.project_name, opt.platform_name, opt.provider_name, opt.frame_arg);
		if (frame_path.IsEmpty() || !FileExists(frame_path)) { Cout() << "Error: frame not found.\n"; return 1; }
		String pattern_path = IsFullPath(opt.pattern_arg) ? opt.pattern_arg : AppendFileName(GetProjectDirPath(opt.project_name), opt.pattern_arg);
		if (!FileExists(pattern_path)) { Cout() << "Error: pattern not found.\n"; return 1; }
		Image frame = LoadImageForPath(frame_path);
		Image pattern = LoadImageForPath(pattern_path);
		if (frame.IsEmpty() || pattern.IsEmpty()) { Cout() << "Error: failed to load frame or pattern.\n"; return 1; }
		OrbSystem orb;
		orb.SetInput(pattern);
		orb.InitDefault();
		orb.SetInput(frame);
		orb.Process();
		const Vector<Pointf>& corners = orb.GetLastCorners();
		Cout() << "orb_find_good=" << orb.GetLastGoodMatches() << "\n";
		Cout() << "orb_find_corners=" << corners.GetCount() << "\n";
		if (corners.GetCount() >= 4) {
			double minx = corners[0].x, miny = corners[0].y;
			double maxx = corners[0].x, maxy = corners[0].y;
			for (int i = 1; i < corners.GetCount(); i++) {
				minx = min(minx, (double)corners[i].x);
				miny = min(miny, (double)corners[i].y);
				maxx = max(maxx, (double)corners[i].x);
				maxy = max(maxy, (double)corners[i].y);
			}
			Rect r((int)floor(minx), (int)floor(miny), (int)ceil(maxx), (int)ceil(maxy));
			if (r.IsEmpty()) r = RectC((int)floor(minx), (int)floor(miny), 1, 1);
			Cout() << "orb_find_rect " << r.left << " " << r.top << " " << r.GetWidth() << " " << r.GetHeight() << "\n";
		}
		return 0;
	}

	ProjectFile pf;
	String project_path;
	if (!LoadProjectFileByName(opt.project_name, pf, project_path)) {
		Cout() << "Error: project not found.\n";
		return 1;
	}

	if (!opt.platform_name.IsEmpty()) {
		PlatformFile pfplat;
		if (!LoadPlatformFileByName(opt.project_name, opt.platform_name, pfplat)) {
			Cout() << "Error: platform not found.\n";
			return 1;
		}
		if (opt.list_frames) {
			String root = GetScreenshotsRoot(opt.project_name, opt.platform_name);
			if (opt.provider_name.IsEmpty()) {
				FindFile providers(AppendFileName(root, "*"));
				while (providers) {
					if (providers.IsFolder()) {
						String provider = providers.GetName();
						String dir = AppendFileName(root, provider);
						FindFile ff(AppendFileName(dir, "*.jpg"));
						while (ff) { if (ff.IsFile()) Cout() << AppendFileName(provider, ff.GetName()) << "\n"; ff.Next(); }
						FindFile ff2(AppendFileName(dir, "*.jpeg"));
						while (ff2) { if (ff2.IsFile()) Cout() << AppendFileName(provider, ff2.GetName()) << "\n"; ff2.Next(); }
					}
					providers.Next();
				}
			} else {
				String dir = GetScreenshotsDir(opt.project_name, opt.platform_name, opt.provider_name);
				FindFile ff(AppendFileName(dir, "*.jpg"));
				while (ff) { if (ff.IsFile()) Cout() << ff.GetName() << "\n"; ff.Next(); }
				FindFile ff2(AppendFileName(dir, "*.jpeg"));
				while (ff2) { if (ff2.IsFile()) Cout() << ff2.GetName() << "\n"; ff2.Next(); }
			}
		}
		if (opt.list_props) {
			for (const GameRule& r : pfplat.rules) {
				if (!opt.rule_name.IsEmpty() && r.name != opt.rule_name) continue;
				bool single_rule = !opt.rule_name.IsEmpty();
				if (!single_rule) Cout() << "rule " << r.name << "\n";
				String prefix = single_rule ? String() : "  ";
				Cout() << prefix << "type=" << r.type << "\n";
				Cout() << prefix << "rect=" << r.rect.left << " " << r.rect.top << " " << r.rect.right << " " << r.rect.bottom << "\n";
				if (!r.parent_name.IsEmpty()) Cout() << prefix << "parent=" << r.parent_name << "\n";
				for (int i = 0; i < r.props.GetCount(); i++) Cout() << prefix << r.props.GetKey(i) << "=" << r.props[i] << "\n";
			}
		}
			if (opt.list_images) {
			for (const GameRule& r : pfplat.rules) {
				if (!opt.rule_name.IsEmpty() && r.name != opt.rule_name) continue;
				bool single_rule = !opt.rule_name.IsEmpty();
				for (const String& img : GetRuleImagesRef(r)) {
					if (single_rule) Cout() << img;
					else Cout() << r.name << " " << img;
					if (opt.verbose) {
						RuleImageMetadata md;
						String abs = ResolveRuleImagePath(opt.project_name, img);
						if (LoadRuleImageMetadata(abs, md))
							Cout() << " status=" << md.review_status << " label=" << md.label;
						else
							Cout() << " status=missing-meta";
					}
					Cout() << "\n";
				}
				}
			}
			if (opt.synthetic_window_motion_test) {
				Array<GameRule> active_rules;
				for (int i = 0; i < pfplat.rules.GetCount(); i++) {
					const GameRule& src = pfplat.rules[i];
					GameRule& dst = active_rules.Add();
					dst.name = src.name;
					dst.type = src.type;
					dst.rect = src.rect;
					dst.parent_name = src.parent_name;
					dst.samples <<= src.samples;
					dst.images <<= src.images;
					dst.props <<= src.props;
				}
				FrameAnalysisData layout_ref;
				LoadUseLayoutFrameData(opt.project_name, opt.platform_name, opt.provider_name, layout_ref);
				ApplyFrameRuleDataToRules(active_rules, layout_ref);
				FillMissingRuleImagesFromDirs(active_rules, opt.project_name, opt.platform_name);
				String common_code = LoadProjectCommonCode(opt.project_name, pf.common_code);
				String platform_code = LoadPlatformCode(opt.project_name, opt.platform_name, pfplat.code);
				String full = common_code;
				if (!full.IsEmpty() && !platform_code.IsEmpty())
					full << "\n";
				full << platform_code;
				GameScript script;
				g_cli_verbose = opt.verbose;
				g_cli_trace_api = opt.trace_script_api;
				Callback1<String> logcb;
				logcb << ScriptLogStdout;
				script.SetLogCallback(logcb);
				script.SetTraceApi(opt.trace_script_api);
				script.SetOpenAIKeyFile(opt.openai_key_file);
				script.SetOpenAIModel(opt.openai_model);
				if (!script.LoadAndInit(full)) {
					Cout() << "Error: failed to load script.\n";
					return 1;
				}
				auto find_rule = [&](const String& name) -> const GameRule* {
					for (int i = 0; i < active_rules.GetCount(); i++)
						if (active_rules[i].name == name)
							return &active_rules[i];
					return nullptr;
				};
				auto get_layout_rule_rect = [&](const String& name, Rect& out) -> bool {
					for (const FrameRuleData& rd : layout_ref.rule_data) {
						if (rd.rule == name && rd.has_rect && !rd.rect.IsEmpty()) {
							out = rd.rect;
							return true;
						}
					}
					return false;
				};
				String ws_dir = GetRuleSamplesDir(opt.project_name, opt.platform_name, "window-size");
				Vector<String> ws_files;
				ListPngFiles(ws_dir, ws_files);
				if (ws_files.IsEmpty()) {
					Cout() << "Error: no window-size png files in " << ws_dir << "\n";
					return 1;
				}
				for (int i = 0; i < active_rules.GetCount(); i++) {
					if (active_rules[i].name != "window-size")
						continue;
					active_rules[i].images.Clear();
					for (const String& p : ws_files)
						active_rules[i].images.Add(MakeProjectRelative(opt.project_name, p));
					break;
				}
				Vector<Image> ws_imgs;
				for (const String& p : ws_files) {
					Image im = StreamRaster::LoadFileAny(p);
					if (!im.IsEmpty())
						ws_imgs.Add(im);
				}
				if (ws_imgs.IsEmpty()) {
					Cout() << "Error: failed to load window-size png files.\n";
					return 1;
				}
				Cout() << "synthetic_test_start seconds=" << opt.live_seconds
				       << " fps=" << opt.synthetic_fps
				       << " speed_px_s=30"
				       << " slideshow_s=1"
				       << " images=" << ws_imgs.GetCount() << "\n";
				VectorMap<String, Vector<Rect>> prev_rule_matches;
				VectorMap<String, Vector<double>> prev_rule_scores;
				Vector<Rect> prev_window_rects;
				VectorMap<String, Image> image_cache;
				OrbCacheStats orb_cache_stats;
					const int total_frames = max(1, opt.live_seconds * opt.synthetic_fps);
					const double fps = (double)max(1, opt.synthetic_fps);
					Vector<Image> pre_frames;
					Vector<int> pre_slide_idx;
					Vector<Point> pre_pos;
					pre_frames.SetCount(total_frames);
					pre_slide_idx.SetCount(total_frames, 0);
					pre_pos.SetCount(total_frames, Point(0, 0));
					String motion_dir = AppendFileName(AppendFileName(GetCurrentDirectory(), "tmp"), "motiontest");
					RealizeDirectory(motion_dir);
					for (int fi = 0; fi < total_frames; fi++) {
						double t = (double)fi / fps;
						int slide_idx = ((int)floor(t)) % ws_imgs.GetCount();
						const Image& overlay = ws_imgs[slide_idx];
						Size os = overlay.GetSize();
						const int fw = 1920, fh = 1080;
						int radius = min((fw - os.cx) / 2, (fh - os.cy) / 2) - 20;
						if (radius < 20)
							radius = 20;
						double angle = (30.0 * t) / (double)radius;
						int cx = fw / 2, cy = fh / 2;
						int x = (int)floor(cx + radius * cos(angle) - os.cx * 0.5 + 0.5);
						int y = (int)floor(cy + radius * sin(angle) - os.cy * 0.5 + 0.5);
						pre_frames[fi] = ComposeSyntheticFrame(overlay, x, y, Size(fw, fh), GrayColor(128));
						pre_slide_idx[fi] = slide_idx;
						pre_pos[fi] = Point(x, y);
						String out = AppendFileName(motion_dir, Format("frame_%04d_slide_%02d.png", fi, slide_idx));
						PNGEncoder().SaveFile(out, pre_frames[fi]);
					}
					Cout() << "synthetic_prerender_done frames=" << total_frames
					       << " out_dir=" << motion_dir << "\n";
					int tracking_miss_frames = 0;
					int log_change_count = 0;
					int card_change_count = 0;
					int prev_slide_idx = -1;
					String prev_log_text;
					String prev_card_text;
					Rect prev_main_window;
					bool prev_has_main_window = false;
					String prev_obj_area_sig;
					int64 total_frame_us = 0;
					TimeStop synthetic_ts;
					for (int fi = 0; fi < total_frames; fi++) {
						TimeStop frame_ts;
						double t = (double)fi / fps;
						int slide_idx = pre_slide_idx[fi];
						int x = pre_pos[fi].x;
						int y = pre_pos[fi].y;
						Image frame = pre_frames[fi];
						
						Vector<GpuKp> gkps;
						Vector<BinDescriptor> gdscs;
						if (gpu_pre.IsAvailable()) {
							if (gpu_pre.PrepareFrame(frame)) {
								if (gpu_pre.ComputeScoreMaps()) {
									gpu_pre.GetKeypoints(gkps);
									gpu_pre.ComputeDescriptors(gkps, gdscs);
								}
							}
						}

						if (prev_slide_idx != slide_idx) {
							const GameRule* wr = find_rule("window-size");
							if (wr) {
							Vector<Rect> dbg_rects;
							Vector<double> dbg_scores;
							bool dbg_found = FindOrbMatchesForRule(opt.project_name, *wr, frame, dbg_rects, dbg_scores, nullptr, nullptr, nullptr, &gkps, &gdscs);
							double bests = dbg_scores.IsEmpty() ? 0.0 : dbg_scores[0];
							Cout() << "synthetic_debug_window_orb frame=" << fi
							       << " slide_idx=" << slide_idx
							       << " found=" << (dbg_found ? 1 : 0)
							       << " matches=" << dbg_rects.GetCount()
							       << " best_score=" << bests << "\n";
						}
					}

					VectorMap<String, Vector<Rect>> rule_matches;
					VectorMap<String, Vector<double>> rule_scores_map;
					script.SetOrbFinder([&](const String& rule_name, const Image&, Vector<Rect>& rects, Vector<double>& scores, const Rect* search_hint) {
						rects.Clear();
						scores.Clear();
						int q = rule_matches.Find(rule_name);
						if (q >= 0) {
							rects.Append(rule_matches[q]);
							if (q < rule_scores_map.GetCount())
								scores.Append(rule_scores_map[q]);
							return !rects.IsEmpty();
						}
						const GameRule* r = find_rule(rule_name);
						if (!r)
							return false;
						while (image_cache.GetCount() > opt.live_rule_cache_limit) {
							image_cache.Remove(0);
							orb_cache_stats.evictions++;
						}
						bool found = false;
						if (search_hint && !search_hint->IsEmpty())
							found = FindOrbMatchesForRule(opt.project_name, *r, frame, rects, scores, search_hint, &image_cache, &orb_cache_stats, &gkps, &gdscs);
						if (!found) {
							int pq = prev_rule_matches.Find(rule_name);
							if (pq >= 0 && !prev_rule_matches[pq].IsEmpty()) {
								Rect local = prev_rule_matches[pq][0];
								local.Inflate(opt.temporal_local_dx, opt.temporal_local_dy);
								found = FindOrbMatchesForRule(opt.project_name, *r, frame, rects, scores, &local, &image_cache, &orb_cache_stats, &gkps, &gdscs);
							}
						}
						Rect predicted_roi;
						bool has_roi = false;
						if (!prev_window_rects.IsEmpty()) {
							Rect rel;
							if (get_layout_rule_rect(rule_name, rel) && !rel.IsEmpty()) {
								const Rect& w = prev_window_rects[0];
								predicted_roi = RectC(w.left + rel.left - 40, w.top + rel.top - 40, rel.GetWidth() + 80, rel.GetHeight() + 80);
								has_roi = true;
							}
						}
						if (!found && has_roi)
							found = FindOrbMatchesForRule(opt.project_name, *r, frame, rects, scores, &predicted_roi, &image_cache, &orb_cache_stats, &gkps, &gdscs);
						if (!found)
							found = FindOrbMatchesForRule(opt.project_name, *r, frame, rects, scores, nullptr, &image_cache, &orb_cache_stats, &gkps, &gdscs);
						if (!found && rule_name == "window-size") {
							Rect bf_out;
							double bf_score = 0.0;
							if (FindBruteforceMatchForRule(opt.project_name, *r, frame, nullptr, bf_out, bf_score)) {
								rects.Add(bf_out);
								scores.Add(max(0.0, min(1.0, bf_score)));
								found = true;
							}
						}
						if (found) {
							Vector<Rect> cached_rects;
							cached_rects <<= rects;
							rule_matches.Add(rule_name, pick(cached_rects));
							Vector<double> cached_scores;
							cached_scores <<= scores;
							rule_scores_map.Add(rule_name, pick(cached_scores));
						}
						return found;
					});
					script.SetBruteforceFinder([&](const String& rule_name, const Image& image, const Rect* search, Rect& out, double& score) {
						const GameRule* r = find_rule(rule_name);
						return r ? FindBruteforceMatchForRule(opt.project_name, *r, image, search, out, score) : false;
					});
					Vector<Rect> window_rects;
					script.SetWindowSetter([&](const Vector<Rect>& rects) {
						window_rects.Clear();
						window_rects.Append(rects);
					});
					script.SetRulePropGetter([&](const String& rule_name, const String& key, String& value) {
						for (int i = 0; i < active_rules.GetCount(); i++) {
							if (active_rules[i].name != rule_name)
								continue;
							int q = active_rules[i].props.Find(key);
							if (q < 0)
								return false;
							value = active_rules[i].props[q];
							return true;
						}
						return false;
					});
					Vector<Rect> inst;
					Vector<double> inst_scores;
					script.RunPlatformTracking(frame, inst, inst_scores);
					Vector<ScriptRuntimeObject> objs;
					script.GetRuntimeObjects(objs);

					if (window_rects.IsEmpty())
						tracking_miss_frames++;

					String log_text;
					String card_text;
					for (const ScriptRuntimeObject& o : objs) {
						int qt = o.props.Find("text");
						if (qt < 0)
							continue;
						String v = o.props[qt];
						if (o.name == "log")
							log_text = v;
						if (o.name.Find("card") >= 0 || o.name.Find("suit") >= 0 || o.name.Find("index") >= 0) {
							if (!card_text.IsEmpty())
								card_text << "|";
							card_text << o.name << "=" << v;
						}
					}
					if (prev_slide_idx != slide_idx) {
						Cout() << "synthetic_slide_change frame=" << fi
						       << " t=" << Format("%.3f", t)
						       << " slide_idx=" << slide_idx
						       << " pos=" << x << "," << y
						       << " window_rects=" << window_rects.GetCount() << "\n";
					}
					if (!prev_log_text.IsEmpty() && log_text != prev_log_text)
						log_change_count++;
						if (!prev_card_text.IsEmpty() && card_text != prev_card_text)
							card_change_count++;

						String obj_area_sig;
						for (const ScriptRuntimeObject& o : objs) {
							obj_area_sig << o.name << ":" << o.rect.left << "," << o.rect.top << "," << o.rect.GetWidth() << "," << o.rect.GetHeight() << ";";
						}
						bool obj_area_same = (!prev_obj_area_sig.IsEmpty() && obj_area_sig == prev_obj_area_sig);
						bool text_same = (!prev_log_text.IsEmpty() && log_text == prev_log_text);
						Rect mainw;
						bool has_mainw = !window_rects.IsEmpty();
						if (has_mainw)
							mainw = window_rects[0];
						bool window_same = false;
						if (prev_has_main_window && has_mainw) {
							int dx = abs(mainw.left - prev_main_window.left);
							int dy = abs(mainw.top - prev_main_window.top);
							int dw = abs(mainw.GetWidth() - prev_main_window.GetWidth());
							int dh = abs(mainw.GetHeight() - prev_main_window.GetHeight());
							window_same = dx <= 10 && dy <= 10 && dw <= 10 && dh <= 10;
						}
						String window_state = "new";
						if (!has_mainw)
							window_state = "missing";
						else if (window_same)
							window_state = "same";
						int64 frame_us = frame_ts.Elapsed();
						total_frame_us += frame_us;
						double inst_fps = frame_us > 0 ? (1000000.0 / (double)frame_us) : 0.0;
						double avg_fps = synthetic_ts.Elapsed() > 0 ? ((double)(fi + 1) * 1000000.0 / (double)synthetic_ts.Elapsed()) : 0.0;
						Cout() << "synthetic_frame idx=" << fi
						       << " t=" << Format("%.3f", t)
						       << " slide_idx=" << slide_idx
						       << " pos=" << x << "," << y
						       << " window_rects=" << window_rects.GetCount()
						       << " window_state=" << window_state
						       << " obj_count=" << objs.GetCount()
						       << " obj_area_same=" << (obj_area_same ? 1 : 0)
						       << " log_text_same=" << (text_same ? 1 : 0)
						       << " frame_us=" << frame_us
						       << " fps_inst=" << Format("%.2f", inst_fps)
						       << " fps_avg=" << Format("%.2f", avg_fps)
						       << "\n";

						prev_slide_idx = slide_idx;
						prev_log_text = log_text;
						prev_card_text = card_text;
						prev_obj_area_sig = obj_area_sig;
						prev_rule_matches <<= rule_matches;
						prev_rule_scores <<= rule_scores_map;
						prev_window_rects.Clear();
						prev_window_rects.Append(window_rects);
						prev_has_main_window = has_mainw;
						if (has_mainw)
							prev_main_window = mainw;
					}
					double avg_fps = total_frame_us > 0 ? ((double)total_frames * 1000000.0 / (double)total_frame_us) : 0.0;
					Cout() << "synthetic_test_done"
					       << " frames=" << total_frames
					       << " tracking_miss_frames=" << tracking_miss_frames
					       << " log_change_count=" << log_change_count
					       << " card_change_count=" << card_change_count
					       << " avg_fps=" << Format("%.2f", avg_fps)
					       << " fps_target_ok=" << (avg_fps >= 1.0 ? 1 : 0)
					       << "\n";
					if (avg_fps < 1.0)
						Cout() << "synthetic_test_problem avg_fps_below_1=1\n";
				}
			if (opt.live_remote_test) {
			if (opt.remote_addr.IsEmpty()) {
				Cout() << "Error: --remote is required for --live-remote-test.\n";
				return 1;
			}
			String host;
			int port = 0;
			if (!ParseHostPort(opt.remote_addr, host, port)) {
				Cout() << "Error: --remote must be host:port.\n";
				return 1;
			}
			Array<GameRule> active_rules;
			for (int i = 0; i < pfplat.rules.GetCount(); i++) {
				const GameRule& src = pfplat.rules[i];
				GameRule& dst = active_rules.Add();
				dst.name = src.name;
				dst.type = src.type;
				dst.rect = src.rect;
				dst.parent_name = src.parent_name;
				dst.samples <<= src.samples;
				dst.images <<= src.images;
				dst.props <<= src.props;
			}
			FrameAnalysisData layout_ref;
			LoadUseLayoutFrameData(opt.project_name, opt.platform_name, opt.provider_name, layout_ref);
			ApplyFrameRuleDataToRules(active_rules, layout_ref);
			FillMissingRuleImagesFromDirs(active_rules, opt.project_name, opt.platform_name);
			String common_code = LoadProjectCommonCode(opt.project_name, pf.common_code);
			String platform_code = LoadPlatformCode(opt.project_name, opt.platform_name, pfplat.code);
			String full = common_code;
			if (!full.IsEmpty() && !platform_code.IsEmpty())
				full << "\n";
			full << platform_code;
			GameScript script;
			g_cli_verbose = opt.verbose;
			g_cli_trace_api = opt.trace_script_api;
			Callback1<String> logcb;
			logcb << ScriptLogStdout;
			script.SetLogCallback(logcb);
			script.SetTraceApi(opt.trace_script_api);
			script.SetOpenAIKeyFile(opt.openai_key_file);
			script.SetOpenAIModel(opt.openai_model);
			if (!script.LoadAndInit(full)) {
				Cout() << "Error: failed to load script.\n";
				return 1;
			}
			auto find_rule = [&](const String& name) -> const GameRule* {
				for (int i = 0; i < active_rules.GetCount(); i++)
					if (active_rules[i].name == name)
						return &active_rules[i];
				return nullptr;
			};
			auto get_layout_rule_rect = [&](const String& name, Rect& out) -> bool {
				for (const FrameRuleData& rd : layout_ref.rule_data) {
					if (rd.rule == name && rd.has_rect && !rd.rect.IsEmpty()) {
						out = rd.rect;
						return true;
					}
				}
				return false;
			};
			const bool harvest_live = !opt.harvest_text_patches_dir.IsEmpty();
			String harvest_out_root;
			VectorMap<String, Index<uint64>> harvest_hashes;
			TextHarvestSummary harvest_total;
			int harvest_seq = 0;
			if (harvest_live) {
				harvest_out_root = opt.harvest_text_patches_dir;
				if (!IsFullPath(harvest_out_root))
					harvest_out_root = AppendFileName(GetCurrentDirectory(), harvest_out_root);
				RealizeDirectory(harvest_out_root);
			}
			Cout() << "live_test_start remote=" << host << ":" << port
			       << " seconds=" << opt.live_seconds
			       << " warmup_frames=" << opt.live_warmup_frames
			       << " temporal_local_dx=" << opt.temporal_local_dx
			       << " temporal_local_dy=" << opt.temporal_local_dy
			       << " decode_threads=" << opt.decode_threads
			       << " queue_capacity=" << opt.queue_capacity
			       << " drop_policy=" << opt.drop_policy
			       << " override_keep_keyframe=" << (opt.override_keep_keyframe ? 1 : 0)
			       << "\n";
			TcpSocket sock;
			if (!sock.Connect(host, port)) {
				Cout() << "Error: remote connect failed.\n";
				return 1;
			}
			sock.Timeout(500);
			uint32 last_id = 0;
			Mutex fetch_queue_lock, decode_queue_lock;
			Array<RemoteFramePacket> fetch_queue;
			Array<DecodedFrame> decode_queue;
			std::atomic<bool> pipeline_stop(false);
			std::atomic<bool> socket_error(false);
			std::atomic<int64> fetch_drops(0);
			std::atomic<int64> decode_drops(0);
			std::atomic<int64> fetched_packets(0);
			std::atomic<int64> decoded_packets(0);
			std::atomic<int> fetch_q_watermark(0);
			std::atomic<int> decode_q_watermark(0);
			Thread fetch_thread;
			Array<Thread> decode_threads;
			fetch_thread.Run([&] {
				while (!pipeline_stop.load(std::memory_order_relaxed)) {
					RemoteFramePacket pkt;
					if (!GrabRemoteFramePacket(sock, last_id, pkt)) {
						socket_error = true;
						break;
					}
					if (pkt.payload.IsEmpty()) {
						Sleep(1);
						continue;
					}
					Mutex::Lock __(fetch_queue_lock);
					if (fetch_queue.GetCount() >= opt.queue_capacity) {
						if (opt.drop_policy == "newest") {
							fetch_drops.fetch_add(1, std::memory_order_relaxed);
							continue;
						}
						fetch_queue.Remove(0);
						fetch_drops.fetch_add(1, std::memory_order_relaxed);
					}
					fetch_queue.Add(pick(pkt));
					int qsz = fetch_queue.GetCount();
					int prev = fetch_q_watermark.load(std::memory_order_relaxed);
					while (qsz > prev && !fetch_q_watermark.compare_exchange_weak(prev, qsz, std::memory_order_relaxed))
						;
					fetched_packets.fetch_add(1, std::memory_order_relaxed);
				}
			});
			decode_threads.SetCount(max(1, opt.decode_threads));
			for (int di = 0; di < decode_threads.GetCount(); di++) {
				decode_threads[di].Run([&] {
					while (!pipeline_stop.load(std::memory_order_relaxed)) {
						RemoteFramePacket pkt;
						bool have_pkt = false;
						{
							Mutex::Lock __(fetch_queue_lock);
							if (fetch_queue.GetCount()) {
								pkt = pick(fetch_queue[0]);
								fetch_queue.Remove(0);
								have_pkt = true;
							}
						}
						if (!have_pkt) {
							Sleep(1);
							continue;
						}
						DecodedFrame decoded;
						if (!DecodeRemoteFramePacket(pkt, decoded))
							continue;
						Mutex::Lock __(decode_queue_lock);
						if (decode_queue.GetCount() >= opt.queue_capacity) {
							if (opt.drop_policy == "newest") {
								decode_drops.fetch_add(1, std::memory_order_relaxed);
								continue;
							}
							decode_queue.Remove(0);
							decode_drops.fetch_add(1, std::memory_order_relaxed);
						}
						decode_queue.Add(pick(decoded));
						int qsz = decode_queue.GetCount();
						int prev = decode_q_watermark.load(std::memory_order_relaxed);
						while (qsz > prev && !decode_q_watermark.compare_exchange_weak(prev, qsz, std::memory_order_relaxed))
							;
						decoded_packets.fetch_add(1, std::memory_order_relaxed);
					}
				});
			}
			VectorMap<String, Image> image_cache;
			OrbCacheStats orb_cache_stats;
			VectorMap<String, Vector<Rect>> prev_rule_matches;
			VectorMap<String, Vector<double>> prev_rule_scores;
			Vector<int64> proc_us;
			int64 total_grab_us = 0;
			int64 total_fetch_us = 0;
			int64 total_decode_us = 0;
			int64 total_wait_us = 0;
			int64 total_script_us = 0;
			int64 total_orb_cb_us = 0;
			int64 total_orb_match_us = 0;
			int64 total_orb_cache_us = 0;
			int64 total_bruteforce_us = 0;
			int64 total_post_us = 0;
			Vector<Rect> prev_window_rects;
			int frames_received = 0;
			int frames_processed = 0;
			int frames_after_warmup = 0;
			int64 proc_us_sum = 0;
			int64 proc_us_sum_after_warmup = 0;
			TimeStop live_ts;
			TimeStop report_ts;
			const int64 duration_us = (int64)max(1, opt.live_seconds) * 1000000;
			while (live_ts.Elapsed() < duration_us) {
				TimeStop frame_total_ts;
				DecodedFrame frame_pkt;
				bool have_frame = false;
				TimeStop wait_ts;
				while (live_ts.Elapsed() < duration_us) {
					Mutex::Lock __(decode_queue_lock);
					if (decode_queue.GetCount()) {
						frame_pkt = pick(decode_queue[0]);
						decode_queue.Remove(0);
						have_frame = true;
						break;
					}
					if (socket_error.load(std::memory_order_relaxed))
						break;
					Sleep(1);
				}
				if (!have_frame) {
					if (socket_error.load(std::memory_order_relaxed)) {
						Cout() << "live_test_socket_error=1\n";
						break;
					}
					continue;
				}
				int64 wait_us = wait_ts.Elapsed();
				int64 fetch_us = frame_pkt.fetch_us;
				int64 decode_us = frame_pkt.decode_us;
				int64 grab_us = wait_us + fetch_us + decode_us;
				total_wait_us += wait_us;
				total_fetch_us += fetch_us;
				total_decode_us += decode_us;
				total_grab_us += grab_us;
				Image frame = frame_pkt.img;
				frames_received++;
				TimeStop frame_ts;
				VectorMap<String, Vector<Rect>> rule_matches;
				VectorMap<String, Vector<double>> rule_scores_map;
				int64 frame_orb_cb_us = 0;
				int64 frame_orb_match_us = 0;
				int64 frame_orb_cache_us = 0;
				int64 frame_bruteforce_us = 0;
				script.SetOrbFinder([&](const String& rule_name, const Image&, Vector<Rect>& rects, Vector<double>& scores, const Rect* search_hint) {
					TimeStop orb_cb_ts;
					rects.Clear();
					scores.Clear();
					int q = rule_matches.Find(rule_name);
					if (q >= 0) {
						rects.Append(rule_matches[q]);
						if (q < rule_scores_map.GetCount())
							scores.Append(rule_scores_map[q]);
						frame_orb_cache_us += orb_cb_ts.Elapsed();
						return !rects.IsEmpty();
					}
					if (opt.override_keep_keyframe) {
						int pq = prev_rule_matches.Find(rule_name);
						if (pq >= 0 && !prev_rule_matches[pq].IsEmpty()) {
							rects.Append(prev_rule_matches[pq]);
							int ps = prev_rule_scores.Find(rule_name);
							if (ps >= 0)
								scores.Append(prev_rule_scores[ps]);
							Vector<Rect> cached_rects;
							cached_rects <<= rects;
							rule_matches.Add(rule_name, pick(cached_rects));
							Vector<double> cached_scores;
							cached_scores <<= scores;
							rule_scores_map.Add(rule_name, pick(cached_scores));
							frame_orb_cache_us += orb_cb_ts.Elapsed();
							return true;
						}
					}
					const GameRule* r = find_rule(rule_name);
					if (!r)
						return false;
					while (image_cache.GetCount() > opt.live_rule_cache_limit) {
						image_cache.Remove(0);
						orb_cache_stats.evictions++;
					}
					TimeStop orb_match_ts;
					bool found = false;
					if (search_hint && !search_hint->IsEmpty())
						found = FindOrbMatchesForRule(opt.project_name, *r, frame, rects, scores, search_hint, &image_cache, &orb_cache_stats);
					if (!found && !opt.override_keep_keyframe) {
						int pq = prev_rule_matches.Find(rule_name);
						if (pq >= 0 && !prev_rule_matches[pq].IsEmpty()) {
							const Rect prev = prev_rule_matches[pq][0];
							Rect local = prev;
							local.Inflate(opt.temporal_local_dx, opt.temporal_local_dy);
							found = FindOrbMatchesForRule(opt.project_name, *r, frame, rects, scores, &local, &image_cache, &orb_cache_stats);
							if (!found && (opt.temporal_local_dx > 0 || opt.temporal_local_dy > 0)) {
								Rect local2 = prev;
								local2.Inflate(max(20, opt.temporal_local_dx * 3), max(20, opt.temporal_local_dy * 3));
								found = FindOrbMatchesForRule(opt.project_name, *r, frame, rects, scores, &local2, &image_cache, &orb_cache_stats);
							}
						}
					}
					Rect predicted_roi;
					bool has_roi = false;
					if (!prev_window_rects.IsEmpty()) {
						Rect rel;
						if (get_layout_rule_rect(rule_name, rel) && !rel.IsEmpty()) {
							const Rect& w = prev_window_rects[0];
							predicted_roi = RectC(w.left + rel.left - 40, w.top + rel.top - 40, rel.GetWidth() + 80, rel.GetHeight() + 80);
							has_roi = true;
						} else if (!r->rect.IsEmpty()) {
							const Rect& w = prev_window_rects[0];
							predicted_roi = RectC(w.left + r->rect.left - 40, w.top + r->rect.top - 40, r->rect.GetWidth() + 80, r->rect.GetHeight() + 80);
							has_roi = true;
						}
					}
					if (!found && has_roi)
						found = FindOrbMatchesForRule(opt.project_name, *r, frame, rects, scores, &predicted_roi, &image_cache, &orb_cache_stats);
					if (!found && has_roi) {
						Rect expanded = predicted_roi;
						expanded.Inflate(80);
						found = FindOrbMatchesForRule(opt.project_name, *r, frame, rects, scores, &expanded, &image_cache, &orb_cache_stats);
					}
					if (!found)
						found = FindOrbMatchesForRule(opt.project_name, *r, frame, rects, scores, nullptr, &image_cache, &orb_cache_stats);
					if (found) {
						Vector<Rect> cached_rects;
						cached_rects <<= rects;
						rule_matches.Add(rule_name, pick(cached_rects));
						Vector<double> cached_scores;
						cached_scores <<= scores;
						rule_scores_map.Add(rule_name, pick(cached_scores));
					}
					frame_orb_match_us += orb_match_ts.Elapsed();
					frame_orb_cb_us += orb_cb_ts.Elapsed();
					return found;
				});
				script.SetBruteforceFinder([&](const String& rule_name, const Image& image, const Rect* search, Rect& out, double& score) {
					TimeStop brute_ts;
					const GameRule* r = find_rule(rule_name);
					bool ok = r ? FindBruteforceMatchForRule(opt.project_name, *r, image, search, out, score) : false;
					frame_bruteforce_us += brute_ts.Elapsed();
					return ok;
				});
				Vector<Rect> window_rects;
				script.SetWindowSetter([&](const Vector<Rect>& rects) {
					window_rects.Clear();
					window_rects.Append(rects);
				});
				script.SetRulePropGetter([&](const String& rule_name, const String& key, String& value) {
					for (int i = 0; i < active_rules.GetCount(); i++) {
						if (active_rules[i].name != rule_name)
							continue;
						int q = active_rules[i].props.Find(key);
						if (q < 0)
							return false;
						value = active_rules[i].props[q];
						return true;
					}
					return false;
				});
				Vector<Rect> inst;
				Vector<double> inst_scores;
				TimeStop script_ts;
				script.RunPlatformTracking(frame, inst, inst_scores);
				Vector<ScriptRuntimeObject> script_objects;
				script.GetRuntimeObjects(script_objects);
				const int64 script_us = script_ts.Elapsed();
				total_script_us += script_us;
				if (harvest_live && !frame.IsEmpty()) {
					for (int oi = 0; oi < script_objects.GetCount(); oi++) {
						const ScriptRuntimeObject& o = script_objects[oi];
						int qtext = o.props.Find("text");
						if (qtext < 0)
							continue;
						int qcap = o.props.Find("harvest.capture");
						bool script_marked = qcap >= 0 && (o.props[qcap] == "1" || ToLower(o.props[qcap]) == "true");
						if (!opt.harvest_implicit_text && !script_marked)
							continue;
						harvest_total.seen++;
						if (!opt.harvest_filter.IsEmpty() && !PatternMatchMulti(opt.harvest_filter, o.name)) {
							harvest_total.filtered++;
							continue;
						}
						if (!opt.harvest_include_orb_same) {
							int qsame = o.props.Find("text.orb_same");
							if (qsame >= 0 && (o.props[qsame] == "1" || ToLower(o.props[qsame]) == "true")) {
								harvest_total.orb_same_skipped++;
								continue;
							}
						}
						Rect rr;
						int qocr = o.props.Find("text.ocr_rect");
						if (qocr >= 0 && ParseRectString(o.props[qocr], rr)) {
						}
						else if (!o.rect.IsEmpty())
							rr = o.rect;
						if (rr.IsEmpty()) {
							harvest_total.missing_rect++;
							continue;
						}
						rr.Intersect(frame.GetSize());
						if (rr.IsEmpty()) {
							harvest_total.missing_rect++;
							continue;
						}
						Image patch = Crop(frame, rr);
						if (patch.IsEmpty()) {
							harvest_total.empty_patch++;
							continue;
						}
						String bucket_name = SanitizeNameForPath(o.name);
						String bucket_dir = AppendFileName(harvest_out_root, bucket_name);
						RealizeDirectory(bucket_dir);
						int bi = harvest_hashes.Find(bucket_name);
						if (bi < 0) {
							Index<uint64>& idx = harvest_hashes.Add(bucket_name);
							LoadExistingHarvestHashes(bucket_dir, idx);
							bi = harvest_hashes.GetCount() - 1;
						}
						uint64 h = HashImagePixels(patch);
						if (harvest_hashes[bi].Find(h) >= 0) {
							harvest_total.dup_hash++;
							continue;
						}
						String stamp = BuildHarvestStamp(harvest_seq++);
						String patch_path = AppendFileName(bucket_dir, "patch_" + stamp + ".png");
						if (!PNGEncoder().SaveFile(patch_path, patch))
							continue;
						RuleImageMetadata md;
						md.source_frame = "live_frame_" + AsString(frames_processed + 1);
						md.source_rule = o.name;
						md.source_rect = rr;
						md.source_provider = opt.provider_name;
						md.source_platform = opt.platform_name;
						md.label = String();
						md.label_confidence = 0.0;
						md.review_status = "unlabeled";
						md.value_text = o.props[qtext];
						md.dedupe_hash = HashHex(h);
						md.capture_kind = "text_patch";
						md.reviewer = String();
						md.captured_at = AsString(GetSysTime());
						SaveRuleImageMetadata(patch_path, md);
						harvest_hashes[bi].FindAdd(h);
						harvest_total.kept++;
					}
				}
				prev_rule_matches.Clear();
				prev_rule_scores.Clear();
				TimeStop post_ts;
				for (int i = 0; i < rule_matches.GetCount(); i++) {
					Vector<Rect> rr;
					rr <<= rule_matches[i];
					prev_rule_matches.Add(rule_matches.GetKey(i), pick(rr));
				}
				for (int i = 0; i < rule_scores_map.GetCount(); i++) {
					Vector<double> ss;
					ss <<= rule_scores_map[i];
					prev_rule_scores.Add(rule_scores_map.GetKey(i), pick(ss));
				}
				if (!window_rects.IsEmpty())
					prev_window_rects <<= window_rects;
				const int64 post_us = post_ts.Elapsed();
				total_post_us += post_us;
				int64 us = frame_ts.Elapsed();
				proc_us.Add(us);
				frames_processed++;
				proc_us_sum += us;
				total_orb_cb_us += frame_orb_cb_us;
				total_orb_match_us += frame_orb_match_us;
				total_orb_cache_us += frame_orb_cache_us;
				total_bruteforce_us += frame_bruteforce_us;
				if (opt.profile_exec) {
					Cout() << "live_frame_timing idx=" << frames_processed
					       << " grab_us=" << grab_us
					       << " fetch_us=" << fetch_us
					       << " decode_us=" << decode_us
					       << " wait_us=" << wait_us
					       << " script_us=" << script_us
					       << " orb_cb_us=" << frame_orb_cb_us
					       << " orb_match_us=" << frame_orb_match_us
					       << " orb_cache_us=" << frame_orb_cache_us
					       << " brute_us=" << frame_bruteforce_us
					       << " post_us=" << post_us
					       << " total_us=" << frame_total_ts.Elapsed()
					       << "\n";
				}
				if (frames_processed > opt.live_warmup_frames) {
					frames_after_warmup++;
					proc_us_sum_after_warmup += us;
				}
				if (report_ts.Elapsed() >= 1000000) {
					double secs = (double)live_ts.Elapsed() / 1000000.0;
					double fps = secs > 0.0 ? (double)frames_processed / secs : 0.0;
					int64 ff = fetched_packets.load(std::memory_order_relaxed);
					int64 dd = decoded_packets.load(std::memory_order_relaxed);
					int64 dpf = fetch_drops.load(std::memory_order_relaxed);
					int64 dpd = decode_drops.load(std::memory_order_relaxed);
					Cout() << "live_progress frames=" << frames_processed
					       << " fetched=" << ff
					       << " decoded=" << dd
					       << " drop_fetch=" << dpf
					       << " drop_decode=" << dpd
					       << " orb_cache_hits=" << orb_cache_stats.hits
					       << " orb_cache_misses=" << orb_cache_stats.misses
					       << " orb_cache_evictions=" << orb_cache_stats.evictions
					       << " fps=" << Format("%.2f", fps) << "\n";
					report_ts.Reset();
				}
			}
			pipeline_stop = true;
			fetch_thread.Wait();
			for (int di = 0; di < decode_threads.GetCount(); di++)
				decode_threads[di].Wait();
			double secs = max(0.001, (double)live_ts.Elapsed() / 1000000.0);
			double fps_total = (double)frames_processed / secs;
			double avg_ms = frames_processed > 0 ? ((double)proc_us_sum / (double)frames_processed) / 1000.0 : 0.0;
			double avg_ms_after = frames_after_warmup > 0 ? ((double)proc_us_sum_after_warmup / (double)frames_after_warmup) / 1000.0 : 0.0;
			Sort(proc_us);
			double p50 = proc_us.IsEmpty() ? 0.0 : (double)proc_us[proc_us.GetCount() / 2] / 1000.0;
			double p95 = proc_us.IsEmpty() ? 0.0 : (double)proc_us[min(proc_us.GetCount() - 1, (int)floor(proc_us.GetCount() * 0.95))] / 1000.0;
			Cout() << "live_summary duration_s=" << Format("%.3f", secs)
			       << " received=" << frames_received
			       << " processed=" << frames_processed
			       << " fetched=" << fetched_packets.load(std::memory_order_relaxed)
			       << " decoded=" << decoded_packets.load(std::memory_order_relaxed)
			       << " drop_fetch=" << fetch_drops.load(std::memory_order_relaxed)
			       << " drop_decode=" << decode_drops.load(std::memory_order_relaxed)
			       << " fetch_q_wm=" << fetch_q_watermark.load(std::memory_order_relaxed)
			       << " decode_q_wm=" << decode_q_watermark.load(std::memory_order_relaxed)
			       << " fps=" << Format("%.2f", fps_total)
			       << " avg_ms=" << Format("%.3f", avg_ms)
			       << " avg_ms_after_warmup=" << Format("%.3f", avg_ms_after)
			       << " p50_ms=" << Format("%.3f", p50)
			       << " p95_ms=" << Format("%.3f", p95)
			       << " orb_cache_hits=" << orb_cache_stats.hits
			       << " orb_cache_misses=" << orb_cache_stats.misses
			       << " orb_cache_evictions=" << orb_cache_stats.evictions
			       << "\n";
			if (harvest_live) {
				Cout() << "harvest_text_patches_summary"
				       << " seen=" << harvest_total.seen
				       << " kept=" << harvest_total.kept
				       << " filtered=" << harvest_total.filtered
				       << " missing_rect=" << harvest_total.missing_rect
				       << " empty_patch=" << harvest_total.empty_patch
				       << " dup_hash=" << harvest_total.dup_hash
				       << " orb_same_skipped=" << harvest_total.orb_same_skipped
				       << " dir=" << harvest_out_root
				       << "\n";
			}
			if (frames_processed > 0) {
				Cout() << "live_phase_avg_ms"
				       << " grab=" << Format("%.3f", (double)total_grab_us / (double)frames_processed / 1000.0)
				       << " fetch=" << Format("%.3f", (double)total_fetch_us / (double)frames_processed / 1000.0)
				       << " decode=" << Format("%.3f", (double)total_decode_us / (double)frames_processed / 1000.0)
				       << " wait=" << Format("%.3f", (double)total_wait_us / (double)frames_processed / 1000.0)
				       << " script=" << Format("%.3f", (double)total_script_us / (double)frames_processed / 1000.0)
				       << " orb_cb=" << Format("%.3f", (double)total_orb_cb_us / (double)frames_processed / 1000.0)
				       << " orb_match=" << Format("%.3f", (double)total_orb_match_us / (double)frames_processed / 1000.0)
				       << " orb_cache=" << Format("%.3f", (double)total_orb_cache_us / (double)frames_processed / 1000.0)
				       << " bruteforce=" << Format("%.3f", (double)total_bruteforce_us / (double)frames_processed / 1000.0)
				       << " post=" << Format("%.3f", (double)total_post_us / (double)frames_processed / 1000.0)
				       << "\n";
			}
			if (!opt.live_report_json.IsEmpty()) {
				struct LiveReport : Moveable<LiveReport> {
					double duration_s = 0;
					int received = 0;
					int processed = 0;
					int fetched = 0;
					int decoded = 0;
					int drop_fetch = 0;
					int drop_decode = 0;
					int fetch_q_wm = 0;
					int decode_q_wm = 0;
					double fps = 0;
					double avg_ms = 0;
					double avg_ms_after_warmup = 0;
					double p50_ms = 0;
					double p95_ms = 0;
					double phase_grab_ms = 0;
					double phase_fetch_ms = 0;
					double phase_decode_ms = 0;
					double phase_wait_ms = 0;
					double phase_script_ms = 0;
					double phase_orb_cb_ms = 0;
					double phase_orb_match_ms = 0;
					double phase_orb_cache_ms = 0;
					double phase_bruteforce_ms = 0;
					double phase_post_ms = 0;
					int orb_cache_hits = 0;
					int orb_cache_misses = 0;
					int orb_cache_evictions = 0;
					void Jsonize(JsonIO& jio) {
						jio("duration_s", duration_s)
						   ("received", received)
						   ("processed", processed)
						   ("fetched", fetched)
						   ("decoded", decoded)
						   ("drop_fetch", drop_fetch)
						   ("drop_decode", drop_decode)
						   ("fetch_q_wm", fetch_q_wm)
						   ("decode_q_wm", decode_q_wm)
						   ("fps", fps)
						   ("avg_ms", avg_ms)
						   ("avg_ms_after_warmup", avg_ms_after_warmup)
						   ("p50_ms", p50_ms)
						   ("p95_ms", p95_ms)
						   ("phase_grab_ms", phase_grab_ms)
						   ("phase_fetch_ms", phase_fetch_ms)
						   ("phase_decode_ms", phase_decode_ms)
						   ("phase_wait_ms", phase_wait_ms)
						   ("phase_script_ms", phase_script_ms)
						   ("phase_orb_cb_ms", phase_orb_cb_ms)
						   ("phase_orb_match_ms", phase_orb_match_ms)
						   ("phase_orb_cache_ms", phase_orb_cache_ms)
						   ("phase_bruteforce_ms", phase_bruteforce_ms)
						   ("phase_post_ms", phase_post_ms)
						   ("orb_cache_hits", orb_cache_hits)
						   ("orb_cache_misses", orb_cache_misses)
						   ("orb_cache_evictions", orb_cache_evictions);
					}
				};
				LiveReport rep;
				rep.duration_s = secs;
				rep.received = frames_received;
				rep.processed = frames_processed;
				rep.fetched = (int)fetched_packets.load(std::memory_order_relaxed);
				rep.decoded = (int)decoded_packets.load(std::memory_order_relaxed);
				rep.drop_fetch = (int)fetch_drops.load(std::memory_order_relaxed);
				rep.drop_decode = (int)decode_drops.load(std::memory_order_relaxed);
				rep.fetch_q_wm = fetch_q_watermark.load(std::memory_order_relaxed);
				rep.decode_q_wm = decode_q_watermark.load(std::memory_order_relaxed);
				rep.fps = fps_total;
				rep.avg_ms = avg_ms;
				rep.avg_ms_after_warmup = avg_ms_after;
				rep.p50_ms = p50;
				rep.p95_ms = p95;
				if (frames_processed > 0) {
					double denom = (double)frames_processed * 1000.0;
					rep.phase_grab_ms = (double)total_grab_us / denom;
					rep.phase_fetch_ms = (double)total_fetch_us / denom;
					rep.phase_decode_ms = (double)total_decode_us / denom;
					rep.phase_wait_ms = (double)total_wait_us / denom;
					rep.phase_script_ms = (double)total_script_us / denom;
					rep.phase_orb_cb_ms = (double)total_orb_cb_us / denom;
					rep.phase_orb_match_ms = (double)total_orb_match_us / denom;
					rep.phase_orb_cache_ms = (double)total_orb_cache_us / denom;
					rep.phase_bruteforce_ms = (double)total_bruteforce_us / denom;
					rep.phase_post_ms = (double)total_post_us / denom;
				}
				rep.orb_cache_hits = (int)orb_cache_stats.hits;
				rep.orb_cache_misses = (int)orb_cache_stats.misses;
				rep.orb_cache_evictions = (int)orb_cache_stats.evictions;
				String out = opt.live_report_json;
				if (!IsFullPath(out))
					out = AppendFileName(GetCurrentDirectory(), out);
				RealizeDirectory(GetFileDirectory(out));
				if (SaveFile(out, StoreAsJson(rep)))
					Cout() << "live_report_json=" << out << "\n";
			}
			return 0;
		}
		if (opt.run_one || opt.exec_script_debug) {
			TimeStop exec_ts;
			TimeStop stage_ts;
			String frame_path = ResolveFramePath(opt.project_name, opt.platform_name, opt.provider_name, opt.frame_arg);
			if (frame_path.IsEmpty() || !FileExists(frame_path)) { Cout() << "Error: frame not found.\n"; return 1; }
			FrameAnalysisData frame_data;
			LoadFrameAnalysisData(frame_path, frame_data);
			if (opt.profile_exec)
				CliPrint("perf stage_sidecar_load_us=" + AsString(stage_ts.Elapsed()));
			stage_ts.Reset();
			if (opt.exec_script_debug)
				CliPrint("exec_debug frame_rule_data=" + AsString(frame_data.rule_data.GetCount()));
			Array<GameRule> active_rules;
			for (int i = 0; i < pfplat.rules.GetCount(); i++) {
				const GameRule& src = pfplat.rules[i];
				GameRule& dst = active_rules.Add();
				dst.name = src.name;
				dst.type = src.type;
				dst.rect = src.rect;
				dst.parent_name = src.parent_name;
				dst.samples <<= src.samples;
				dst.images <<= src.images;
				dst.props <<= src.props;
			}
			ApplyFrameRuleDataToRules(active_rules, frame_data);
			FillMissingRuleImagesFromDirs(active_rules, opt.project_name, opt.platform_name);
			if (opt.exec_script_debug) {
				CliPrint("exec_debug project=" + opt.project_name);
				CliPrint("exec_debug platform=" + opt.platform_name);
				CliPrint("exec_debug provider=" + opt.provider_name);
				CliPrint("exec_debug frame=" + frame_path);
			}
			String common_code = LoadProjectCommonCode(opt.project_name, pf.common_code);
			String platform_code = LoadPlatformCode(opt.project_name, opt.platform_name, pfplat.code);
			String full = common_code;
			if (!full.IsEmpty() && !platform_code.IsEmpty()) full << "\n";
			full << platform_code;
			GameScript script;
			g_cli_verbose = opt.verbose;
			g_cli_trace_api = opt.trace_script_api;
			Callback1<String> logcb;
			logcb << ScriptLogStdout;
			script.SetLogCallback(logcb);
			script.SetTraceApi(opt.trace_script_api);
			script.SetOpenAIKeyFile(opt.openai_key_file);
			script.SetOpenAIModel(opt.openai_model);
			bool load_ok = script.LoadAndInit(full);
			if (opt.profile_exec)
				CliPrint("perf stage_script_init_us=" + AsString(stage_ts.Elapsed()));
			stage_ts.Reset();
			if (opt.exec_script_debug)
				CliPrint("exec_debug script_loaded=" + AsString((int)load_ok));
			Image img = LoadImageForPath(frame_path);
			if (opt.profile_exec)
				CliPrint("perf stage_frame_load_us=" + AsString(stage_ts.Elapsed()));
			stage_ts.Reset();
			if (opt.exec_script_debug)
				CliPrint("exec_debug frame_size=" + AsString(img.GetWidth()) + "x" + AsString(img.GetHeight()));
			GpuPreprocessEngine gpu_pre;
			Vector<GpuKp> gkps;
			Vector<BinDescriptor> gdscs;
			if (opt.gpu_preprocess) {
				GpuPreprocessConfig gpu_cfg;
				gpu_cfg.backend = opt.gpu_backend;
				gpu_cfg.route = opt.gpu_route;
				gpu_cfg.headless = true;
				if (gpu_pre.Initialize(gpu_cfg) && gpu_pre.PrepareFrame(img)) {
					if (gpu_pre.ComputeScoreMaps()) {
						gpu_pre.GetKeypoints(gkps);
						gpu_pre.ComputeDescriptors(gkps, gdscs);
					}
				}
				const GpuPreprocessStats& gst = gpu_pre.GetStats();
				if (opt.profile_exec || opt.verbose || opt.exec_script_debug) {
					CliPrint("gpu_preprocess enabled=1 backend=" + gst.backend + " requested_backend=" + gst.requested_backend +
					         " requested_route=" + gst.requested_route +
					         " route=" + gst.backend_route + " ok=" + AsString((int)gpu_pre.IsAvailable()));
					CliPrint("gpu_preprocess total_us=" + AsString(gst.total_us) +
					         " upload_us=" + AsString(gst.upload_us) +
					         " gray_us=" + AsString(gst.grayscale_us) +
					         " blur_us=" + AsString(gst.blur_us) +
					         " pyramid_us=" + AsString(gst.pyramid_us) +
					         " upload_copies=" + AsString(gst.upload_copies) +
					         " readback_copies=" + AsString(gst.readback_copies) +
					         " upload_bytes=" + AsString(gst.upload_bytes) +
					         " readback_bytes=" + AsString(gst.readback_bytes) +
					         " renderer=\"" + gst.renderer + "\"" +
					         " transfer_model=" + gst.transfer_model +
					         " levels=" + AsString(gst.pyramid_levels));
					if (!gst.last_error.IsEmpty())
						CliPrint("gpu_preprocess error=" + gst.last_error);
				}
			}
			VectorMap<String, Vector<Rect>> rule_matches;
			VectorMap<String, Vector<double>> rule_scores_map;
			VectorMap<String, Image> image_cache;
			script.SetOrbFinder([&](const String& rule_name, const Image&, Vector<Rect>& rects, Vector<double>& scores, const Rect* search_hint) {
				rects.Clear();
				scores.Clear();
				int q = rule_matches.Find(rule_name);
				if (q >= 0) {
					rects.Append(rule_matches[q]);
					if (q < rule_scores_map.GetCount())
						scores.Append(rule_scores_map[q]);
					return !rects.IsEmpty();
				}
				int ri = -1;
				for (int i = 0; i < active_rules.GetCount(); i++) {
					if (active_rules[i].name == rule_name) {
						ri = i;
						break;
					}
				}
				if (ri < 0)
					return false;
				const GameRule& r = active_rules[ri];
				bool found = FindOrbMatchesForRule(opt.project_name, r, img, rects, scores, search_hint, &image_cache, nullptr, &gkps, &gdscs);
				if (found) {
					Vector<Rect> cached_rects;
					cached_rects <<= rects;
					rule_matches.Add(rule_name, pick(cached_rects));
					Vector<double> cached_scores;
					cached_scores <<= scores;
					rule_scores_map.Add(rule_name, pick(cached_scores));
				}
				if (opt.exec_script_debug) {
					CliPrint("exec_debug rule=" + rule_name + " images=" + AsString(GetRuleImagesRef(r).GetCount()));
					for (int i = 0; i < rects.GetCount(); i++)
						CliPrint("exec_debug orb_rect " + rule_name + " " + AsString(rects[i].left) + " " + AsString(rects[i].top) + " " + AsString(rects[i].GetWidth()) + " " + AsString(rects[i].GetHeight()) + " score=" + AsString(i < scores.GetCount() ? scores[i] : 0.0));
				}
				if (opt.debug_orb)
					Cout() << "orb_rule " << rule_name << " matches=" << rects.GetCount() << "\n";
				return found;
			});
			script.SetBruteforceFinder([&](const String& rule_name, const Image& image, const Rect* search, Rect& out, double& score) {
				int ri = -1;
				for (int i = 0; i < active_rules.GetCount(); i++) {
					if (active_rules[i].name == rule_name) {
						ri = i;
						break;
					}
				}
				if (ri < 0)
					return false;
				return FindBruteforceMatchForRule(opt.project_name, active_rules[ri], image, search, out, score);
			});
			Vector<Rect> window_rects;
			script.SetWindowSetter([&](const Vector<Rect>& rects) { window_rects.Clear(); window_rects.Append(rects); });
			script.SetRulePropGetter([&](const String& rule_name, const String& key, String& value) {
				for (int i = 0; i < active_rules.GetCount(); i++) {
					if (active_rules[i].name != rule_name)
						continue;
					int q = active_rules[i].props.Find(key);
					if (q < 0)
						return false;
					value = active_rules[i].props[q];
					return true;
				}
				return false;
			});
			script.SetRulePropGetter([&](const String& rule_name, const String& key, String& value) {
				for (int i = 0; i < active_rules.GetCount(); i++) {
					if (active_rules[i].name != rule_name)
						continue;
					int q = active_rules[i].props.Find(key);
					if (q < 0)
						return false;
					value = active_rules[i].props[q];
					return true;
				}
				return false;
			});
			Vector<Rect> inst; Vector<double> scores;
			bool run_ok = script.RunPlatformTracking(img, inst, scores);
			Vector<ScriptRuntimeObject> script_objects;
			script.GetRuntimeObjects(script_objects);
			if (opt.profile_exec)
				CliPrint("perf stage_script_run_us=" + AsString(stage_ts.Elapsed()));
			stage_ts.Reset();
			if (opt.exec_script_debug)
				CliPrint("exec_debug run_ok=" + AsString((int)run_ok));
			if (opt.verbose || opt.exec_script_debug) {
				CliPrint("instances=" + AsString(inst.GetCount()));
				for (int i = 0; i < inst.GetCount(); i++)
					CliPrint("instance_rect " + AsString(inst[i].left) + " " + AsString(inst[i].top) + " " + AsString(inst[i].GetWidth()) + " " + AsString(inst[i].GetHeight()) + " score=" + AsString(i < scores.GetCount() ? scores[i] : 0.0));
				CliPrint("window_rects=" + AsString(window_rects.GetCount()));
				for (const Rect& r : window_rects)
					CliPrint("window_rect " + AsString(r.left) + " " + AsString(r.top) + " " + AsString(r.GetWidth()) + " " + AsString(r.GetHeight()));
			}
			auto is_descendant = [&](const String& child_name, const String& ancestor_name) -> bool {
				String cur = child_name;
				for (int guard = 0; guard < 64; guard++) {
					int ci = -1;
					for (int i = 0; i < active_rules.GetCount(); i++) {
						if (active_rules[i].name == cur) {
							ci = i;
							break;
						}
					}
					if (ci < 0)
						return false;
					String parent = active_rules[ci].parent_name;
					if (parent.IsEmpty())
						return false;
					if (parent == ancestor_name)
						return true;
					cur = parent;
				}
				return false;
			};
			int resolved_count = 0;
			Vector<String> resolved_names;
			Vector<int> resolved_window_idx;
			Vector<Rect> resolved_rects;
			for (int wi = 0; wi < window_rects.GetCount(); wi++) {
				for (int ri = 0; ri < active_rules.GetCount(); ri++) {
					const GameRule& r = active_rules[ri];
					if (r.name == "window-size")
						continue;
					if (!is_descendant(r.name, "window-size"))
						continue;
					Rect resolved;
					if (!ResolveRuntimeRuleRect(frame_data, active_rules, "window-size", window_rects[wi], r.name, resolved))
						continue;
					resolved_count++;
					resolved_names.Add(r.name);
					resolved_window_idx.Add(wi);
					resolved_rects.Add(resolved);
				}
			}
			int sem_conf_total = 0;
			int sem_conf_low = 0;
			int sem_disagree_total = 0;
			int sem_disagree_count = 0;
			int sem_text_disagree_total = 0;
			int sem_text_disagree_count = 0;
			int sem_class_count = 0;
			int sem_text_count = 0;
			int sem_policy_count = 0;
			int sem_text_backend_status_count = 0;
			int sem_class_orb_status_count = 0;
			int sem_class_nn_status_count = 0;
			int sem_class_alt_count = 0;
			int sem_threshold_pass_count = 0;
			int sem_text_total_us = 0;
			int sem_text_rect_total = 0;
			int sem_text_rect_shifted = 0;
			Index<String> sem_text_backend_tokens;
			int ocr_cmp_exact = -1;
			int ocr_cmp_normalized = -1;
			int ocr_cmp_missing = -1;
			int ocr_cmp_total = -1;
			double ocr_cmp_normalized_rate = -1.0;
			double ocr_cmp_missing_rate = -1.0;
			for (int i = 0; i < script_objects.GetCount(); i++) {
				const ScriptRuntimeObject& o = script_objects[i];
				for (int j = 0; j < o.props.GetCount(); j++) {
					String k = o.props.GetKey(j);
					String v = o.props[j];
					if (k == "class" && !TrimBoth(v).IsEmpty())
						sem_class_count++;
					if (k == "text")
						sem_text_count++;
					if (k == "class.policy.primary")
						sem_policy_count++;
					if (k == "text.backend_statuses") {
						sem_text_backend_status_count++;
						Vector<String> toks = Split(v, ",");
						for (int q = 0; q < toks.GetCount(); q++) {
							String tok = TrimBoth(toks[q]);
							if (tok.IsEmpty())
								continue;
							int p = tok.Find(':');
							String name = p >= 0 ? TrimBoth(tok.Left(p)) : tok;
							if (!name.IsEmpty())
								sem_text_backend_tokens.FindAdd(name);
						}
					}
					if (k == "class.orb_status" && !TrimBoth(v).IsEmpty())
						sem_class_orb_status_count++;
					if (k == "class.nn_status" && !TrimBoth(v).IsEmpty())
						sem_class_nn_status_count++;
					if ((k == "class.orb_alternatives" || k == "class.nn_alternatives") && !TrimBoth(v).IsEmpty())
						sem_class_alt_count++;
					if (k == "class.threshold_pass" && (v == "1" || v == "true" || v == "True"))
						sem_threshold_pass_count++;
					if (k.EndsWith(".confidence")) {
						double c = StrDbl(v);
						sem_conf_total++;
						if (c < 0.60)
							sem_conf_low++;
					}
					if (k.EndsWith(".disagree")) {
						sem_disagree_total++;
						if (v == "1" || v == "true" || v == "True")
							sem_disagree_count++;
						if (k.StartsWith("text.")) {
							sem_text_disagree_total++;
							if (v == "1" || v == "true" || v == "True")
								sem_text_disagree_count++;
						}
					}
					if (k == "text.total_elapsed_us")
						sem_text_total_us += StrInt(v);
					if (k == "text.ocr_rect") {
						sem_text_rect_total++;
						int qb = o.props.Find("text.base_rect");
						if (qb >= 0 && o.props[qb] != v)
							sem_text_rect_shifted++;
					}
				}
			}
			double sem_text_disagree_rate = sem_text_disagree_total > 0
				? (double)sem_text_disagree_count / (double)sem_text_disagree_total
				: 0.0;
			double sem_text_rect_shift_rate = sem_text_rect_total > 0
				? (double)sem_text_rect_shifted / (double)sem_text_rect_total
				: 0.0;
			if (opt.semantic_summary || opt.exec_script_debug || opt.verbose) {
				CliPrint("semantic_summary class=" + AsString(sem_class_count) +
				         " text=" + AsString(sem_text_count) +
				         " policy=" + AsString(sem_policy_count));
				CliPrint("semantic_summary text_backend_status=" + AsString(sem_text_backend_status_count) +
				         " threshold_pass=" + AsString(sem_threshold_pass_count));
				CliPrint("semantic_summary class_orb_status=" + AsString(sem_class_orb_status_count) +
				         " class_nn_status=" + AsString(sem_class_nn_status_count) +
				         " class_alternatives=" + AsString(sem_class_alt_count));
				CliPrint("semantic_summary confidence_total=" + AsString(sem_conf_total) +
				         " low=" + AsString(sem_conf_low));
				CliPrint("semantic_summary disagree_total=" + AsString(sem_disagree_total) +
				         " disagree_count=" + AsString(sem_disagree_count));
				CliPrint("semantic_summary text_disagree_total=" + AsString(sem_text_disagree_total) +
				         " text_disagree_count=" + AsString(sem_text_disagree_count) +
				         " text_disagree_rate=" + AsString(sem_text_disagree_rate));
				CliPrint("semantic_summary text_total_us=" + AsString(sem_text_total_us));
				CliPrint("semantic_summary text_rect_total=" + AsString(sem_text_rect_total) +
				         " shifted=" + AsString(sem_text_rect_shifted) +
				         " shift_rate=" + AsString(sem_text_rect_shift_rate));
			}
			if (opt.dump_ocr) {
				int dumped = 0;
				for (int i = 0; i < script_objects.GetCount(); i++) {
					const ScriptRuntimeObject& o = script_objects[i];
					int q = o.props.Find("text");
					if (q < 0)
						continue;
					String text = o.props[q];
					String conf, status;
					int qc = o.props.Find("text.confidence");
					int qs = o.props.Find("text.status");
					int qbr = o.props.Find("text.base_rect");
					int qor = o.props.Find("text.ocr_rect");
					int qbe = o.props.Find("text.backends");
					int qet = o.props.Find("text.elapsed_us");
					int qtt = o.props.Find("text.total_elapsed_us");
					int qbs = o.props.Find("text.backend_statuses");
					int qbc = o.props.Find("text.backend_confidences");
					int qop = o.props.Find("text.orb_has_prev");
					int qos = o.props.Find("text.orb_same");
					int qog = o.props.Find("text.orb_good_matches");
					int qom = o.props.Find("text.orb_matches");
					if (qc >= 0) conf = o.props[qc];
					if (qs >= 0) status = o.props[qs];
					String base_rect = qbr >= 0 ? o.props[qbr] : String();
					String ocr_rect = qor >= 0 ? o.props[qor] : String();
					if (!opt.dump_ocr_filter.IsEmpty() && !PatternMatchMulti(opt.dump_ocr_filter, o.name))
						continue;
					if (opt.dump_ocr_shifted_only && (base_rect.IsEmpty() || ocr_rect.IsEmpty() || base_rect == ocr_rect))
						continue;
					String line = "ocr_value " + o.name + " text=\"" + text + "\" confidence=" + conf + " status=" + status +
					              " base_rect=\"" + base_rect + "\" ocr_rect=\"" + ocr_rect + "\"";
					if (opt.dump_ocr_verbose) {
						String backends = qbe >= 0 ? o.props[qbe] : String();
						String elapsed = qet >= 0 ? o.props[qet] : String();
						String total_elapsed = qtt >= 0 ? o.props[qtt] : String();
						String bstatuses = qbs >= 0 ? o.props[qbs] : String();
						String bconfs = qbc >= 0 ? o.props[qbc] : String();
						String orb_prev = qop >= 0 ? o.props[qop] : String();
						String orb_same = qos >= 0 ? o.props[qos] : String();
						String orb_good = qog >= 0 ? o.props[qog] : String();
						String orb_matches = qom >= 0 ? o.props[qom] : String();
						line << " backends=\"" + backends + "\" elapsed_us=" + elapsed + " total_elapsed_us=" + total_elapsed +
						        " backend_statuses=\"" + bstatuses + "\" backend_confidences=\"" + bconfs + "\"" +
						        " orb_has_prev=" + orb_prev + " orb_same=" + orb_same +
						        " orb_good_matches=" + orb_good + " orb_matches=" + orb_matches;
					}
					CliPrint(line);
					dumped++;
				}
				CliPrint("ocr_values=" + AsString(dumped));
			}
			if (!opt.harvest_text_patches_dir.IsEmpty()) {
				String out_root = opt.harvest_text_patches_dir;
				if (!IsFullPath(out_root))
					out_root = AppendFileName(GetCurrentDirectory(), out_root);
				TextHarvestSummary hs;
				HarvestTextPatches(out_root, opt.platform_name, opt.provider_name,
				                  frame_path, img, script_objects, opt.harvest_filter, opt.harvest_include_orb_same, opt.harvest_implicit_text, hs);
				CliPrint("harvest_text_patches seen=" + AsString(hs.seen) +
				         " kept=" + AsString(hs.kept) +
				         " filtered=" + AsString(hs.filtered) +
				         " missing_rect=" + AsString(hs.missing_rect) +
				         " empty_patch=" + AsString(hs.empty_patch) +
				         " dup_hash=" + AsString(hs.dup_hash) +
				         " orb_same_skipped=" + AsString(hs.orb_same_skipped) +
				         " dir=" + out_root);
			}
			if (opt.compare_ocr_to_annotations) {
				auto normalize_text = [&](const String& s) -> String {
					String out;
					out.Reserve(s.GetCount());
					for (int i = 0; i < s.GetCount(); i++) {
						int c = (byte)s[i];
						if (c == '\r' || c == '\n' || c == '\t' || c == ' ')
							continue;
						out.Cat(ToLower((char)c));
					}
					return out;
				};
				auto normalize_money = [&](const String& s) -> String {
					String out;
					out.Reserve(s.GetCount());
					for (int i = 0; i < s.GetCount(); i++) {
						byte c = (byte)s[i];
						if (c >= '0' && c <= '9') {
							out.Cat((char)c);
							continue;
						}
						if (c >= 'a' && c <= 'z')
							c = (byte)(c - 'a' + 'A');
						// Common OCR confusions in numeric money fields.
						if (c == 'O' || c == 'Q' || c == 'D')
							out.Cat('0');
						else if (c == 'B')
							out.Cat('8');
						else if (c == 'I' || c == 'L' || c == '|')
							out.Cat('1');
					}
					return out;
				};
				auto edit_distance_leq1 = [&](const String& a, const String& b) -> bool {
					int na = a.GetCount();
					int nb = b.GetCount();
					if (abs(na - nb) > 1)
						return false;
					if (a == b)
						return true;
					if (na == nb) {
						int diff = 0;
						for (int i = 0; i < na; i++) {
							if (a[i] != b[i]) {
								diff++;
								if (diff > 1)
									return false;
							}
						}
						return diff <= 1;
					}
					const String& s = na < nb ? a : b;
					const String& t = na < nb ? b : a;
					int i = 0, j = 0, edits = 0;
					while (i < s.GetCount() && j < t.GetCount()) {
						if (s[i] == t[j]) { i++; j++; continue; }
						edits++;
						if (edits > 1)
							return false;
						j++; // one insertion/deletion
					}
					return true;
				};
				auto is_money_rule = [&](const String& name) -> bool {
					return name == "total-pot" || name == "round-bets" ||
					       PatternMatch("*-account", name) || PatternMatch("*-bet", name);
				};
				VectorMap<String, String> expected;
				for (int i = 0; i < frame_data.rule_data.GetCount(); i++) {
					const FrameRuleData& rd = frame_data.rule_data[i];
					if (!opt.compare_ocr_filter.IsEmpty() && !PatternMatchMulti(opt.compare_ocr_filter, rd.rule))
						continue;
					String t = TrimBoth(rd.ocr_text);
					if (!rd.rule.IsEmpty() && !t.IsEmpty())
						expected.GetAdd(rd.rule) = t;
				}
				VectorMap<String, String> got_text;
				VectorMap<String, String> got_conf;
				VectorMap<String, String> got_status;
				for (int i = 0; i < script_objects.GetCount(); i++) {
					const ScriptRuntimeObject& o = script_objects[i];
					int qt = o.props.Find("text");
					if (qt < 0)
						continue;
					String txt = o.props[qt];
					String conf, status;
					int qc = o.props.Find("text.confidence");
					int qs = o.props.Find("text.status");
					if (qc >= 0) conf = o.props[qc];
					if (qs >= 0) status = o.props[qs];
					got_text.GetAdd(o.name) = txt;
					got_conf.GetAdd(o.name) = conf;
						got_status.GetAdd(o.name) = status;
				}
				int exact = 0;
				int normalized = 0;
				int missing = 0;
				int emitted = 0;
				Vector<String> diff_names;
				Vector<String> missing_names;
				struct OcrCompareRow : Moveable<OcrCompareRow> {
					String name;
					String expected;
					String got;
					String confidence;
					String status;
					bool exact = false;
					bool normalized = false;
					void Jsonize(JsonIO& jio) {
						jio("name", name)
						   ("expected", expected)
						   ("got", got)
						   ("confidence", confidence)
						   ("status", status)
						   ("exact", exact)
						   ("normalized", normalized);
					}
				};
				Vector<OcrCompareRow> cmp_rows;
				ocr_cmp_total = expected.GetCount();
				CliPrint("ocr_compare_begin expected=" + AsString(expected.GetCount()));
				for (int i = 0; i < expected.GetCount(); i++) {
					String name = expected.GetKey(i);
					String exp = expected[i];
					int q = got_text.Find(name);
					String got = q >= 0 ? got_text[q] : String();
					String conf = q >= 0 && q < got_conf.GetCount() ? got_conf[q] : String();
					String status = q >= 0 && q < got_status.GetCount() ? got_status[q] : "missing";
					bool ok = TrimBoth(got) == TrimBoth(exp);
					bool ok_norm = normalize_text(got) == normalize_text(exp);
					if (!ok_norm && is_money_rule(name)) {
						String gn = normalize_money(got);
						String en = normalize_money(exp);
						if (!en.IsEmpty() && gn == en)
							ok_norm = true;
						else if (!en.IsEmpty() && !gn.IsEmpty() && edit_distance_leq1(gn, en))
							ok_norm = true;
					}
					if (ok) exact++;
					if (ok_norm) normalized++;
					if (!ok)
						diff_names.Add(name);
					if (status == "missing") {
						missing++;
						missing_names.Add(name);
					}
					if (!opt.compare_ocr_diff_only || !ok) {
						OcrCompareRow& row = cmp_rows.Add();
						row.name = name;
						row.expected = exp;
						row.got = got;
						row.confidence = conf;
						row.status = status;
						row.exact = ok;
						row.normalized = ok_norm;
						emitted++;
						CliPrint("ocr_compare " + name +
						         " result=" + String(ok ? "OK" : "DIFF") +
						         " norm=" + String(ok_norm ? "OK" : "DIFF") +
						         " expected=\"" + exp + "\"" +
						         " got=\"" + got + "\"" +
						         " confidence=" + conf +
						         " status=" + status);
					}
				}
				ocr_cmp_exact = exact;
				ocr_cmp_normalized = normalized;
				ocr_cmp_missing = missing;
				ocr_cmp_normalized_rate = expected.GetCount() > 0 ? (double)normalized / (double)expected.GetCount() : 0.0;
				ocr_cmp_missing_rate = expected.GetCount() > 0 ? (double)missing / (double)expected.GetCount() : 0.0;
				int diff_count = expected.GetCount() - exact;
				CliPrint("ocr_compare_summary exact=" + AsString(exact) +
				         " normalized=" + AsString(normalized) +
				         " diff=" + AsString(diff_count) +
				         " missing=" + AsString(missing) +
				         " normalized_rate=" + AsString(ocr_cmp_normalized_rate) +
				         " missing_rate=" + AsString(ocr_cmp_missing_rate) +
				         " total=" + AsString(expected.GetCount()));
				if (diff_names.GetCount())
					CliPrint("ocr_compare_diff_names=" + Join(diff_names, ","));
				if (missing_names.GetCount())
					CliPrint("ocr_compare_missing_names=" + Join(missing_names, ","));
				if (!opt.export_ocr_compare_json_path.IsEmpty()) {
					struct OcrCompareDump : Moveable<OcrCompareDump> {
						String project;
						String platform;
						String provider;
						String frame;
						String filter;
						bool diff_only = false;
						int exact = 0;
						int normalized = 0;
						int missing = 0;
						int diff = 0;
						int total = 0;
						int emitted = 0;
						double normalized_rate = 0.0;
						double missing_rate = 0.0;
						Vector<String> diff_names;
						Vector<String> missing_names;
						Vector<OcrCompareRow> rows;
						void Jsonize(JsonIO& jio) {
							jio("project", project)
							   ("platform", platform)
							   ("provider", provider)
							   ("frame", frame)
							   ("filter", filter)
							   ("diff_only", diff_only)
							   ("exact", exact)
							   ("normalized", normalized)
							   ("missing", missing)
							   ("diff", diff)
							   ("total", total)
							   ("emitted", emitted)
							   ("normalized_rate", normalized_rate)
							   ("missing_rate", missing_rate)
							   ("diff_names", diff_names)
							   ("missing_names", missing_names)
							   ("rows", rows);
						}
					};
					OcrCompareDump dump;
					dump.project = opt.project_name;
					dump.platform = opt.platform_name;
					dump.provider = opt.provider_name;
					dump.frame = frame_path;
					dump.filter = opt.compare_ocr_filter;
					dump.diff_only = opt.compare_ocr_diff_only;
					dump.exact = exact;
					dump.normalized = normalized;
					dump.missing = missing;
					dump.diff = diff_count;
					dump.total = expected.GetCount();
					dump.emitted = emitted;
					dump.normalized_rate = ocr_cmp_normalized_rate;
					dump.missing_rate = ocr_cmp_missing_rate;
					dump.diff_names <<= diff_names;
					dump.missing_names <<= missing_names;
					dump.rows <<= cmp_rows;
					String out = opt.export_ocr_compare_json_path;
					if (!IsFullPath(out))
						out = AppendFileName(GetCurrentDirectory(), out);
					RealizeDirectory(GetFileDirectory(out));
					if (SaveFile(out, StoreAsJson(dump)))
						CliPrint("ocr_compare_export_json=" + out);
				}
			}
			if (opt.validate_frame) {
				bool ok = true;
				if (window_rects.GetCount() < opt.expect_window_min) {
					CliPrint("validate_fail windows got=" + AsString(window_rects.GetCount()) + " expected_min=" + AsString(opt.expect_window_min));
					ok = false;
				}
				if (script_objects.GetCount() < opt.expect_script_objects_min) {
					CliPrint("validate_fail script_objects got=" + AsString(script_objects.GetCount()) + " expected_min=" + AsString(opt.expect_script_objects_min));
					ok = false;
				}
				if (resolved_count < opt.expect_resolved_min) {
					CliPrint("validate_fail resolved got=" + AsString(resolved_count) + " expected_min=" + AsString(opt.expect_resolved_min));
					ok = false;
				}
				if (opt.expect_class_semantic_min >= 0 && sem_class_count < opt.expect_class_semantic_min) {
					CliPrint("validate_fail class_semantic got=" + AsString(sem_class_count) + " expected_min=" + AsString(opt.expect_class_semantic_min));
					ok = false;
				}
				if (opt.expect_text_semantic_min >= 0 && sem_text_count < opt.expect_text_semantic_min) {
					CliPrint("validate_fail text_semantic got=" + AsString(sem_text_count) + " expected_min=" + AsString(opt.expect_text_semantic_min));
					ok = false;
				}
				if (opt.expect_policy_semantic_min >= 0 && sem_policy_count < opt.expect_policy_semantic_min) {
					CliPrint("validate_fail policy_semantic got=" + AsString(sem_policy_count) + " expected_min=" + AsString(opt.expect_policy_semantic_min));
					ok = false;
				}
				if (opt.expect_text_backend_status_min >= 0 && sem_text_backend_status_count < opt.expect_text_backend_status_min) {
					CliPrint("validate_fail text_backend_status got=" + AsString(sem_text_backend_status_count) + " expected_min=" + AsString(opt.expect_text_backend_status_min));
					ok = false;
				}
				if (opt.expect_class_orb_status_min >= 0 && sem_class_orb_status_count < opt.expect_class_orb_status_min) {
					CliPrint("validate_fail class_orb_status got=" + AsString(sem_class_orb_status_count) + " expected_min=" + AsString(opt.expect_class_orb_status_min));
					ok = false;
				}
				if (opt.expect_class_nn_status_min >= 0 && sem_class_nn_status_count < opt.expect_class_nn_status_min) {
					CliPrint("validate_fail class_nn_status got=" + AsString(sem_class_nn_status_count) + " expected_min=" + AsString(opt.expect_class_nn_status_min));
					ok = false;
				}
				if (opt.expect_class_alt_min >= 0 && sem_class_alt_count < opt.expect_class_alt_min) {
					CliPrint("validate_fail class_alternatives got=" + AsString(sem_class_alt_count) + " expected_min=" + AsString(opt.expect_class_alt_min));
					ok = false;
				}
				if (opt.expect_threshold_pass_min >= 0 && sem_threshold_pass_count < opt.expect_threshold_pass_min) {
					CliPrint("validate_fail threshold_pass got=" + AsString(sem_threshold_pass_count) + " expected_min=" + AsString(opt.expect_threshold_pass_min));
					ok = false;
				}
				if (opt.expect_text_rect_total_min >= 0 && sem_text_rect_total < opt.expect_text_rect_total_min) {
					CliPrint("validate_fail text_rect_total got=" + AsString(sem_text_rect_total) + " expected_min=" + AsString(opt.expect_text_rect_total_min));
					ok = false;
				}
				if (opt.expect_text_rect_shifted_min >= 0 && sem_text_rect_shifted < opt.expect_text_rect_shifted_min) {
					CliPrint("validate_fail text_rect_shifted got=" + AsString(sem_text_rect_shifted) + " expected_min=" + AsString(opt.expect_text_rect_shifted_min));
					ok = false;
				}
				if (opt.expect_min_text_rect_shift_rate >= 0.0 && sem_text_rect_shift_rate < opt.expect_min_text_rect_shift_rate) {
					CliPrint("validate_fail text_rect_shift_rate got=" + AsString(sem_text_rect_shift_rate) + " expected_min=" + AsString(opt.expect_min_text_rect_shift_rate));
					ok = false;
				}
				if (opt.expect_ocr_compare_exact_min >= 0 && ocr_cmp_exact >= 0 && ocr_cmp_exact < opt.expect_ocr_compare_exact_min) {
					CliPrint("validate_fail ocr_compare_exact got=" + AsString(ocr_cmp_exact) + " expected_min=" + AsString(opt.expect_ocr_compare_exact_min));
					ok = false;
				}
				if (opt.expect_ocr_compare_normalized_min >= 0 && ocr_cmp_normalized >= 0 && ocr_cmp_normalized < opt.expect_ocr_compare_normalized_min) {
					CliPrint("validate_fail ocr_compare_normalized got=" + AsString(ocr_cmp_normalized) + " expected_min=" + AsString(opt.expect_ocr_compare_normalized_min));
					ok = false;
				}
				if (opt.expect_ocr_compare_missing_max >= 0 && ocr_cmp_missing >= 0 && ocr_cmp_missing > opt.expect_ocr_compare_missing_max) {
					CliPrint("validate_fail ocr_compare_missing got=" + AsString(ocr_cmp_missing) + " expected_max=" + AsString(opt.expect_ocr_compare_missing_max));
					ok = false;
				}
				if (opt.expect_ocr_compare_total_min >= 0 && ocr_cmp_total >= 0 && ocr_cmp_total < opt.expect_ocr_compare_total_min) {
					CliPrint("validate_fail ocr_compare_total got=" + AsString(ocr_cmp_total) + " expected_min=" + AsString(opt.expect_ocr_compare_total_min));
					ok = false;
				}
				if (opt.expect_ocr_compare_total_max >= 0 && ocr_cmp_total >= 0 && ocr_cmp_total > opt.expect_ocr_compare_total_max) {
					CliPrint("validate_fail ocr_compare_total got=" + AsString(ocr_cmp_total) + " expected_max=" + AsString(opt.expect_ocr_compare_total_max));
					ok = false;
				}
				if (opt.expect_ocr_compare_normalized_rate_min >= 0.0 && ocr_cmp_normalized_rate >= 0.0 && ocr_cmp_normalized_rate < opt.expect_ocr_compare_normalized_rate_min) {
					CliPrint("validate_fail ocr_compare_normalized_rate got=" + AsString(ocr_cmp_normalized_rate) + " expected_min=" + AsString(opt.expect_ocr_compare_normalized_rate_min));
					ok = false;
				}
				if (opt.expect_ocr_compare_missing_rate_max >= 0.0 && ocr_cmp_missing_rate >= 0.0 && ocr_cmp_missing_rate > opt.expect_ocr_compare_missing_rate_max) {
					CliPrint("validate_fail ocr_compare_missing_rate got=" + AsString(ocr_cmp_missing_rate) + " expected_max=" + AsString(opt.expect_ocr_compare_missing_rate_max));
					ok = false;
				}
				if (!opt.expect_text_backends.IsEmpty()) {
					Vector<String> req = Split(opt.expect_text_backends, ",");
					for (int q = 0; q < req.GetCount(); q++) {
						String name = TrimBoth(req[q]);
						if (name.IsEmpty())
							continue;
						if (sem_text_backend_tokens.Find(name) < 0) {
							CliPrint("validate_fail text_backend_missing name=" + name);
							ok = false;
						}
					}
				}
				if (opt.expect_max_text_total_us >= 0 && sem_text_total_us > opt.expect_max_text_total_us) {
					CliPrint("validate_fail text_total_us got=" + AsString(sem_text_total_us) + " expected_max=" + AsString(opt.expect_max_text_total_us));
					ok = false;
				}
				if (opt.expect_max_text_disagreement_rate >= 0.0 && sem_text_disagree_rate > opt.expect_max_text_disagreement_rate) {
					CliPrint("validate_fail text_disagreement_rate got=" + AsString(sem_text_disagree_rate) + " expected_max=" + AsString(opt.expect_max_text_disagreement_rate));
					ok = false;
				}
				CliPrint(String("validate_result=") + (ok ? "PASS" : "FAIL"));
				if (!ok)
					return 2;
			}
			if (opt.exec_script_debug) {
				CliPrint("script_objects=" + AsString(script_objects.GetCount()));
				for (int i = 0; i < script_objects.GetCount(); i++) {
					const ScriptRuntimeObject& o = script_objects[i];
					CliPrint("script_object " + o.name + " " + AsString(o.rect.left) + " " + AsString(o.rect.top) + " " +
					         AsString(o.rect.GetWidth()) + " " + AsString(o.rect.GetHeight()));
					for (int j = 0; j < o.props.GetCount(); j++)
						CliPrint("script_object_prop " + o.name + "." + o.props.GetKey(j) + "=" + o.props[j]);
				}
				int debug_resolved_count = 0;
				for (int wi = 0; wi < window_rects.GetCount(); wi++) {
					for (int ri = 0; ri < active_rules.GetCount(); ri++) {
						const GameRule& r = active_rules[ri];
						if (r.name == "window-size")
							continue;
						if (!is_descendant(r.name, "window-size"))
							continue;
						Rect resolved;
						if (!ResolveRuntimeRuleRect(frame_data, active_rules, "window-size", window_rects[wi], r.name, resolved))
							continue;
						CliPrint("resolved_rect " + r.name + " window=" + AsString(wi) + " " +
						         AsString(resolved.left) + " " + AsString(resolved.top) + " " +
						         AsString(resolved.GetWidth()) + " " + AsString(resolved.GetHeight()));
						debug_resolved_count++;
					}
				}
				CliPrint("resolved_rects=" + AsString(debug_resolved_count));
				VectorMap<String, String> dyn = BuildDynamicPropsFromRects(window_rects);
				CliPrint("dynamic_rules_count=" + AsString(window_rects.GetCount()));
				for (int i = 0; i < window_rects.GetCount(); i++) {
					const Rect& r = window_rects[i];
					CliPrint(Format("dynamic_rule window[%d] rect=%d %d %d %d", i, r.left, r.top, r.GetWidth(), r.GetHeight()));
				}
				CliPrint("dynamic_props_count=" + AsString(dyn.GetCount()));
				for (int i = 0; i < dyn.GetCount(); i++)
					CliPrint("dynamic_prop " + dyn.GetKey(i) + "=" + dyn[i]);
				if (!opt.export_preview_path.IsEmpty()) {
					String preview = SavePreviewDebugJpg(img, window_rects, rule_matches, opt.export_preview_path);
					if (!preview.IsEmpty())
						CliPrint("preview_export=" + preview);
				}
			}
			if (opt.profile_exec)
				CliPrint("perf stage_preview_export_us=" + AsString(stage_ts.Elapsed()));
			stage_ts.Reset();
			if (!opt.export_exec_json_path.IsEmpty()) {
				struct ExecResolvedRow : Moveable<ExecResolvedRow> {
					String name;
					int window_index = -1;
					Rect rect;
					void Jsonize(JsonIO& jio) { jio("name", name)("window_index", window_index)("rect", rect); }
				};
				struct ExecObjectRow : Moveable<ExecObjectRow> {
					String name;
					Rect rect;
					VectorMap<String, String> props;
					void Jsonize(JsonIO& jio) { jio("name", name)("rect", rect)("props", props); }
				};
				struct ExecDump : Moveable<ExecDump> {
					String project;
					String platform;
					String provider;
					String frame;
					Vector<Rect> windows;
					Vector<ExecObjectRow> script_objects;
					Vector<ExecResolvedRow> resolved;
					void Jsonize(JsonIO& jio) {
						jio("project", project)
						   ("platform", platform)
						   ("provider", provider)
						   ("frame", frame)
						   ("windows", windows)
						   ("script_objects", script_objects)
						   ("resolved", resolved);
					}
				};
				ExecDump dump;
				dump.project = opt.project_name;
				dump.platform = opt.platform_name;
				dump.provider = opt.provider_name;
				dump.frame = frame_path;
				dump.windows <<= window_rects;
				for (int i = 0; i < script_objects.GetCount(); i++) {
					ExecObjectRow& row = dump.script_objects.Add();
					row.name = script_objects[i].name;
					row.rect = script_objects[i].rect;
					row.props <<= script_objects[i].props;
				}
				for (int i = 0; i < resolved_rects.GetCount(); i++) {
					ExecResolvedRow& rr = dump.resolved.Add();
					rr.name = resolved_names[i];
					rr.window_index = resolved_window_idx[i];
					rr.rect = resolved_rects[i];
				}
				String out = opt.export_exec_json_path;
				if (!IsFullPath(out))
					out = AppendFileName(GetCurrentDirectory(), out);
				RealizeDirectory(GetFileDirectory(out));
				if (SaveFile(out, StoreAsJson(dump)))
					CliPrint("exec_export_json=" + out);
			}
			if (!opt.export_card_patches_dir.IsEmpty()) {
				String out_root = opt.export_card_patches_dir;
				if (!IsFullPath(out_root))
					out_root = AppendFileName(GetCurrentDirectory(), out_root);
				int saved = 0;
				for (int i = 0; i < resolved_rects.GetCount(); i++) {
					String rn = ToLower(resolved_names[i]);
					if (rn.Find("card") < 0 && rn.Find("-suit") < 0 && rn.Find("-index") < 0)
						continue;
					Rect rr = resolved_rects[i];
					rr.Intersect(img.GetSize());
					if (rr.IsEmpty())
						continue;
					Image patch = Crop(img, rr);
					if (patch.IsEmpty())
						continue;
					String rule_dir = AppendFileName(out_root, resolved_names[i]);
					RealizeDirectory(rule_dir);
					String stamp = Format("%04d%02d%02d_%02d%02d%02d_%03d",
					              GetSysTime().year, GetSysTime().month, GetSysTime().day,
					              GetSysTime().hour, GetSysTime().minute, GetSysTime().second, i);
					String img_path = AppendFileName(rule_dir, "patch_" + stamp + ".png");
					if (!PNGEncoder().SaveFile(img_path, patch))
						continue;
					RuleImageMetadata md;
					md.source_frame = frame_path;
					md.source_rule = resolved_names[i];
					md.source_rect = rr;
					md.label = String();
					md.label_confidence = 0.0;
					md.review_status = "unlabeled";
					md.reviewer = String();
					md.captured_at = AsString(GetSysTime());
					SaveRuleImageMetadata(img_path, md);
					saved++;
				}
				CliPrint("export_card_patches saved=" + AsString(saved) + " dir=" + out_root);
			}
			if (!opt.export_semantic_json_path.IsEmpty()) {
				struct SemanticDump : Moveable<SemanticDump> {
					String project;
					String platform;
					String provider;
					String frame;
					int confidence_total = 0;
					int low_confidence = 0;
					int disagreement_total = 0;
					int disagreement_count = 0;
					int text_disagreement_total = 0;
					int text_disagreement_count = 0;
					int class_semantic = 0;
					int text_semantic = 0;
					int policy_semantic = 0;
					int text_backend_status = 0;
					int class_orb_status = 0;
					int class_nn_status = 0;
					int class_alternatives = 0;
					int threshold_pass = 0;
					Vector<String> text_backend_tokens;
					int text_total_us = 0;
					int text_rect_total = 0;
					int text_rect_shifted = 0;
					double text_rect_shift_rate = 0.0;
					int ocr_compare_exact = -1;
					int ocr_compare_normalized = -1;
					int ocr_compare_missing = -1;
					int ocr_compare_total = -1;
					double ocr_compare_normalized_rate = -1.0;
					double ocr_compare_missing_rate = -1.0;
					void Jsonize(JsonIO& jio) {
						jio("project", project)
						   ("platform", platform)
						   ("provider", provider)
						   ("frame", frame)
						   ("confidence_total", confidence_total)
						   ("low_confidence", low_confidence)
						   ("disagreement_total", disagreement_total)
						   ("disagreement_count", disagreement_count)
						   ("text_disagreement_total", text_disagreement_total)
						   ("text_disagreement_count", text_disagreement_count)
						   ("class_semantic", class_semantic)
						   ("text_semantic", text_semantic)
						   ("policy_semantic", policy_semantic)
						   ("text_backend_status", text_backend_status)
						   ("class_orb_status", class_orb_status)
						   ("class_nn_status", class_nn_status)
						   ("class_alternatives", class_alternatives)
						   ("threshold_pass", threshold_pass)
						   ("text_backend_tokens", text_backend_tokens)
						   ("text_total_us", text_total_us)
						   ("text_rect_total", text_rect_total)
						   ("text_rect_shifted", text_rect_shifted)
						   ("text_rect_shift_rate", text_rect_shift_rate)
						   ("ocr_compare_exact", ocr_compare_exact)
						   ("ocr_compare_normalized", ocr_compare_normalized)
						   ("ocr_compare_missing", ocr_compare_missing)
						   ("ocr_compare_total", ocr_compare_total)
						   ("ocr_compare_normalized_rate", ocr_compare_normalized_rate)
						   ("ocr_compare_missing_rate", ocr_compare_missing_rate);
					}
				};
				SemanticDump s;
				s.project = opt.project_name;
				s.platform = opt.platform_name;
				s.provider = opt.provider_name;
				s.frame = frame_path;
				s.confidence_total = sem_conf_total;
				s.low_confidence = sem_conf_low;
				s.disagreement_total = sem_disagree_total;
				s.disagreement_count = sem_disagree_count;
				s.text_disagreement_total = sem_text_disagree_total;
				s.text_disagreement_count = sem_text_disagree_count;
				s.class_semantic = sem_class_count;
				s.text_semantic = sem_text_count;
				s.policy_semantic = sem_policy_count;
				s.text_backend_status = sem_text_backend_status_count;
				s.class_orb_status = sem_class_orb_status_count;
				s.class_nn_status = sem_class_nn_status_count;
				s.class_alternatives = sem_class_alt_count;
				s.threshold_pass = sem_threshold_pass_count;
				for (int i = 0; i < sem_text_backend_tokens.GetCount(); i++)
					s.text_backend_tokens.Add(sem_text_backend_tokens[i]);
				s.text_total_us = sem_text_total_us;
				s.text_rect_total = sem_text_rect_total;
				s.text_rect_shifted = sem_text_rect_shifted;
				s.text_rect_shift_rate = sem_text_rect_total > 0 ? (double)sem_text_rect_shifted / (double)sem_text_rect_total : 0.0;
				s.ocr_compare_exact = ocr_cmp_exact;
				s.ocr_compare_normalized = ocr_cmp_normalized;
				s.ocr_compare_missing = ocr_cmp_missing;
				s.ocr_compare_total = ocr_cmp_total;
				s.ocr_compare_normalized_rate = ocr_cmp_normalized_rate;
				s.ocr_compare_missing_rate = ocr_cmp_missing_rate;
				String out = opt.export_semantic_json_path;
				if (!IsFullPath(out))
					out = AppendFileName(GetCurrentDirectory(), out);
				RealizeDirectory(GetFileDirectory(out));
				if (SaveFile(out, StoreAsJson(s)))
					CliPrint("semantic_export_json=" + out);
			}
			if (opt.save_frame_state) {
				FrameAnalysisData sidecar;
				LoadFrameAnalysisData(frame_path, sidecar);
				String before_json = StoreAsJson(sidecar);
				sidecar.window_rects <<= window_rects;
				sidecar.props <<= BuildDynamicPropsFromRects(window_rects);
				sidecar.rule_rects.Clear();
				for (int i = 0; i < rule_matches.GetCount(); i++) {
					FrameRuleRects& rr = sidecar.rule_rects.Add();
					rr.rule = rule_matches.GetKey(i);
					rr.rects <<= rule_matches[i];
				}
				String after_json = StoreAsJson(sidecar);
				if (after_json != before_json)
					SaveFrameAnalysisData(frame_path, sidecar);
			}
			if (opt.profile_exec)
				CliPrint("perf stage_sidecar_save_us=" + AsString(stage_ts.Elapsed()));
			if (opt.quit_after_run)
				CliPrint("exec_debug quit_after_run=1");
			if (opt.profile_exec)
				CliPrint("perf execute_total_us=" + AsString(exec_ts.Elapsed()));
		}
	}

	if (opt.run_all) {
		if (opt.frame_arg.IsEmpty()) { Cout() << "Error: --frame is required for --run-all.\n"; return 1; }
		String frame_path = ResolveFramePath(opt.project_name, opt.platform_name, opt.provider_name, opt.frame_arg);
		if (frame_path.IsEmpty() || !FileExists(frame_path)) { Cout() << "Error: frame not found.\n"; return 1; }
		for (const ProjectPlatform& pp : pf.platforms) {
			PlatformFile pfplat;
			if (!LoadPlatformFileByName(opt.project_name, pp.name, pfplat)) continue;
			String common_code = LoadProjectCommonCode(opt.project_name, pf.common_code);
			String platform_code = LoadPlatformCode(opt.project_name, pp.name, pfplat.code);
			String full = common_code;
			if (!full.IsEmpty() && !platform_code.IsEmpty()) full << "\n";
			full << platform_code;
			GameScript script;
			g_cli_verbose = opt.verbose;
			g_cli_trace_api = opt.trace_script_api;
			Callback1<String> logcb;
			logcb << ScriptLogStdout;
			script.SetLogCallback(logcb);
			script.SetTraceApi(opt.trace_script_api);
			script.SetOpenAIKeyFile(opt.openai_key_file);
			script.SetOpenAIModel(opt.openai_model);
			script.LoadAndInit(full);
			Image img = LoadImageForPath(frame_path);
			FrameAnalysisData frame_data;
			LoadFrameAnalysisData(frame_path, frame_data);
			Array<GameRule> active_rules;
			for (int i = 0; i < pfplat.rules.GetCount(); i++) {
				const GameRule& src = pfplat.rules[i];
				GameRule& dst = active_rules.Add();
				dst.name = src.name;
				dst.type = src.type;
				dst.rect = src.rect;
				dst.parent_name = src.parent_name;
				dst.samples <<= src.samples;
				dst.images <<= src.images;
				dst.props <<= src.props;
			}
			ApplyFrameRuleDataToRules(active_rules, frame_data);
			FillMissingRuleImagesFromDirs(active_rules, opt.project_name, pp.name);
			VectorMap<String, Vector<Rect>> rule_matches;
			VectorMap<String, Vector<double>> rule_scores_map;
			VectorMap<String, Image> image_cache;
			script.SetOrbFinder([&](const String& rule_name, const Image&, Vector<Rect>& rects, Vector<double>& scores, const Rect* search_hint) {
				rects.Clear();
				scores.Clear();
				int q = rule_matches.Find(rule_name);
				if (q >= 0) {
					rects.Append(rule_matches[q]);
					if (q < rule_scores_map.GetCount())
						scores.Append(rule_scores_map[q]);
					return !rects.IsEmpty();
				}
				int ri = -1;
				for (int i = 0; i < active_rules.GetCount(); i++) {
					if (active_rules[i].name == rule_name) {
						ri = i;
						break;
					}
				}
				if (ri < 0)
					return false;
				const GameRule& r = active_rules[ri];
				bool found = FindOrbMatchesForRule(opt.project_name, r, img, rects, scores, search_hint, &image_cache);
				if (found) {
					Vector<Rect> cached_rects;
					cached_rects <<= rects;
					rule_matches.Add(rule_name, pick(cached_rects));
					Vector<double> cached_scores;
					cached_scores <<= scores;
					rule_scores_map.Add(rule_name, pick(cached_scores));
				}
				if (opt.debug_orb)
					Cout() << "orb_rule " << rule_name << " matches=" << rects.GetCount() << "\n";
				return found;
			});
			script.SetBruteforceFinder([&](const String& rule_name, const Image& image, const Rect* search, Rect& out, double& score) {
				int ri = -1;
				for (int i = 0; i < active_rules.GetCount(); i++) {
					if (active_rules[i].name == rule_name) {
						ri = i;
						break;
					}
				}
				if (ri < 0)
					return false;
				return FindBruteforceMatchForRule(opt.project_name, active_rules[ri], image, search, out, score);
			});
			Vector<Rect> window_rects;
			script.SetWindowSetter([&](const Vector<Rect>& rects) { window_rects.Clear(); window_rects.Append(rects); });
			Vector<Rect> inst; Vector<double> scores;
			script.RunPlatformTracking(img, inst, scores);
			Cout() << "platform " << pp.name << " instances=" << inst.GetCount() << "\n";
			if (opt.verbose)
				for (const Rect& r : window_rects)
					Cout() << "window_rect " << r.left << " " << r.top << " " << r.GetWidth() << " " << r.GetHeight() << "\n";
		}
	}
	return 0;
}

String MsTextFromUs(int64 us) {
	return Format("%.1f", us / 1000.0);
}

}

bool g_match_fast_exec = false;
bool g_match_profile_exec = false;
bool g_cli_verbose = false;
bool g_cli_trace_api = false;
