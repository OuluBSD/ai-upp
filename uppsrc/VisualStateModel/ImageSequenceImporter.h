#ifndef _VisualStateModel_ImageSequenceImporter_h_
#define _VisualStateModel_ImageSequenceImporter_h_

namespace Upp {

// ---------------------------------------------------------------------------
// Options for importing an image sequence into a new session

struct VsmImageSequenceImportOptions : Moveable<VsmImageSequenceImportOptions> {
	String source_dir;        // directory to scan for .vsm files
	String output_dir;        // where to create the new session
	String session_id;        // leave empty to auto-generate
	int    fps          = 30; // assumed frame rate for timestamp assignment
	bool   sort_numeric = true; // sort by numeric prefix in filename

	void Jsonize(JsonIO& json) {
		json("source_dir",   source_dir)  ("output_dir",   output_dir)
		    ("session_id",   session_id)  ("fps",          fps)
		    ("sort_numeric", sort_numeric);
	}
};

// ---------------------------------------------------------------------------
// Per-file warning recorded during import

struct VsmImportWarning : Moveable<VsmImportWarning> {
	String filename;
	String message;

	void Jsonize(JsonIO& json) { json("filename", filename)("message", message); }
};

// ---------------------------------------------------------------------------
// Import result

struct VsmImageSequenceImportResult : Moveable<VsmImageSequenceImportResult> {
	bool               success        = false;
	int                frames_scanned = 0;
	int                frames_imported= 0;
	int                frames_skipped = 0;
	String             session_id;
	String             output_dir;
	Vector<VsmImportWarning> warnings;

	bool HasWarnings() const { return !warnings.IsEmpty(); }
	void Jsonize(JsonIO& json) {
		json("success",          success)
		    ("frames_scanned",   frames_scanned)
		    ("frames_imported",  frames_imported)
		    ("frames_skipped",   frames_skipped)
		    ("session_id",       session_id)
		    ("output_dir",       output_dir)
		    ("warnings",         warnings);
	}
};

// ---------------------------------------------------------------------------
// VsmImageSequenceImporter
//
// Scans a directory for .vsm files, sorts them in natural/numeric order,
// and imports them as a new VisualStateModel session.
//
// Only .vsm (VSM1 binary) files are supported by this sequence importer.
// PNG/JPEG decode is available headlessly via PngFrame.h's VsmLoadPngFrame
// (Draw-gated, not GUI-gated — VisualStateModel.upp depends on Draw only,
// no CtrlCore/CtrlLib); it is not yet wired into this importer's file scan.

class VsmImageSequenceImporter {
public:
	void SetLog(AppLog* sink) { log_.SetSink(sink); }

	VsmImageSequenceImportResult Import(const VsmImageSequenceImportOptions& opts);

private:
	CoreLog log_;

	// Collect .vsm files from source_dir, sorted by numeric prefix
	Vector<String> CollectFiles(const String& dir) const;
	// Natural/numeric sort: "00000003.vsm" < "00000010.vsm"
	static int ExtractNumericPrefix(const String& filename);
};

} // namespace Upp

#endif
