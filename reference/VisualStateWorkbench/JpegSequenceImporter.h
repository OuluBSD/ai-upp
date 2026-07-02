#ifndef _VisualStateWorkbench_JpegSequenceImporter_h_
#define _VisualStateWorkbench_JpegSequenceImporter_h_

#include <VisualStateModel/VisualStateModel.h>
#include <plugin/jpg/jpg.h>

using namespace Upp;

// ---------------------------------------------------------------------------
// Options for importing a JPEG/PNG image sequence

struct VsmJpegImportOptions : Moveable<VsmJpegImportOptions> {
	String source_dir;
	String output_dir;
	String session_id;     // leave empty to auto-generate
	int    fps          = 30;
	bool   sort_numeric = true;  // sort by numeric prefix in filename
	bool   grayscale    = true;  // convert to grayscale VsmImageBuffer
};

// ---------------------------------------------------------------------------
// Import result

struct VsmJpegImportResult : Moveable<VsmJpegImportResult> {
	bool           success         = false;
	int            frames_scanned  = 0;
	int            frames_imported = 0;
	int            frames_skipped  = 0;
	String         session_id;
	String         output_dir;
	Vector<String> warnings;
};

// ---------------------------------------------------------------------------
// JpegSequenceImporter
//
// Draw-dependent importer for .jpg/.jpeg/.png sequences.
// Uses JPGRaster/PNGRaster from the Draw package — NOT headless.

class JpegSequenceImporter {
public:
	void SetLog(AppLog* sink) { log_.SetSink(sink); }
	VsmJpegImportResult Import(const VsmJpegImportOptions& opts);

private:
	CoreLog log_;
	static int     ExtractNumericPrefix(const String& filename);
	Vector<String> CollectFiles(const String& dir, bool sort_numeric) const;
	bool TryImportFile(const String& path, int frame_idx,
	                   VsmSessionStore& store,
	                   const VsmJpegImportOptions& opts,
	                   VsmJpegImportResult& result);
};

#endif
