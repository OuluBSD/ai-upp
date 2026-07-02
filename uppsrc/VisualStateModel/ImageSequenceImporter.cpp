#include "VisualStateModel.h"

namespace Upp {

// ---------------------------------------------------------------------------
// File collection helpers

int VsmImageSequenceImporter::ExtractNumericPrefix(const String& filename)
{
	// Extract leading digits from the base name (e.g. "00000003.vsm" → 3)
	const char* p = GetFileName(filename);
	while(*p && !IsDigit(*p)) p++;
	int n = 0;
	while(*p && IsDigit(*p)) { n = n * 10 + (*p - '0'); p++; }
	return n;
}

Vector<String> VsmImageSequenceImporter::CollectFiles(const String& dir) const
{
	Vector<String> files;
	FindFile ff;
	if(!ff.Search(AppendFileName(dir, "*.vsm"))) return files;
	do {
		if(!ff.IsDirectory())
			files.Add(ff.GetPath());
	} while(ff.Next());
	// Sort by numeric prefix
	Sort(files, [](const String& a, const String& b) {
		return ExtractNumericPrefix(GetFileName(a)) <
		       ExtractNumericPrefix(GetFileName(b));
	});
	return files;
}

// ---------------------------------------------------------------------------
// VsmImageSequenceImporter::Import

VsmImageSequenceImportResult VsmImageSequenceImporter::Import(
	const VsmImageSequenceImportOptions& opts)
{
	VsmImageSequenceImportResult result;

	if(opts.source_dir.IsEmpty()) {
		LogError(log_, "Importer", "source_dir is empty");
		return result;
	}
	if(opts.output_dir.IsEmpty()) {
		LogError(log_, "Importer", "output_dir is empty");
		return result;
	}
	if(!DirectoryExists(opts.source_dir)) {
		LogError(log_, "Importer", "source_dir not found: " + opts.source_dir);
		return result;
	}

	// Collect source files
	Vector<String> files = CollectFiles(opts.source_dir);
	result.frames_scanned = files.GetCount();
	LogInfo(log_, "Importer", "Scanned " + IntStr(result.frames_scanned) +
	        " .vsm files in: " + opts.source_dir);

	if(files.IsEmpty()) {
		LogWarn(log_, "Importer", "No .vsm files found in: " + opts.source_dir);
		result.success = true; // empty import is not an error
		return result;
	}

	// Determine frame dimensions from the first readable file
	int width = 0, height = 0;
	for(const String& f : files) {
		VsmImageBuffer probe;
		if(probe.Load(f)) {
			width  = probe.width;
			height = probe.height;
			break;
		}
	}
	if(width == 0 || height == 0) {
		LogError(log_, "Importer", "Cannot determine frame dimensions from any file");
		return result;
	}

	// Determine session_id
	String session_id = opts.session_id.IsEmpty()
	                  ? "import-" + IntStr((int)GetTickCount())
	                  : opts.session_id;

	// Create output session
	VsmSessionStore out_store;
	out_store.SetLog(log_.GetSink());
	if(!out_store.Create(opts.output_dir, session_id, width, height, "image-sequence")) {
		LogError(log_, "Importer", "Cannot create output session at: " + opts.output_dir);
		return result;
	}

	// Import frames
	int frame_idx = 0;
	int fps = opts.fps > 0 ? opts.fps : 30;
	for(const String& src_path : files) {
		VsmImageBuffer img;
		if(!img.Load(src_path)) {
			VsmImportWarning& w = result.warnings.Add();
			w.filename = GetFileName(src_path);
			w.message  = "Cannot load file; skipped";
			result.frames_skipped++;
			LogWarn(log_, "Importer", "Skip: " + GetFileName(src_path));
			continue;
		}
		if(img.width != width || img.height != height) {
			VsmImportWarning& w = result.warnings.Add();
			w.filename = GetFileName(src_path);
			w.message  = "Dimensions mismatch (" + IntStr(img.width) + "x" +
			             IntStr(img.height) + " vs " + IntStr(width) + "x" +
			             IntStr(height) + "); skipped";
			result.frames_skipped++;
			LogWarn(log_, "Importer", w.message);
			continue;
		}
		VsmAssetRef ref = out_store.SaveFrameImage(frame_idx, img);
		if(ref.IsEmpty()) {
			VsmImportWarning& w = result.warnings.Add();
			w.filename = GetFileName(src_path);
			w.message  = "Write failed; skipped";
			result.frames_skipped++;
			continue;
		}
		frame_idx++;
		result.frames_imported++;
	}

	// Persist manifest
	if(!out_store.SaveManifest()) {
		LogError(log_, "Importer", "Cannot save manifest");
		return result;
	}

	result.success     = true;
	result.session_id  = session_id;
	result.output_dir  = opts.output_dir;

	LogInfo(log_, "Importer",
	        "Import complete: " + IntStr(result.frames_imported) + " frames into " +
	        opts.output_dir + " session=" + session_id);
	return result;
}

} // namespace Upp
