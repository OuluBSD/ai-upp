#include "MainWindow.h"
#include "JpegSequenceImporter.h"

// ---------------------------------------------------------------------------
// Decode helpers — use Draw rasters directly (not headless-compatible)

static Image LoadJpegImage(const String& path)
{
	FileIn fi(path);
	if(!fi) return Image();
	JPGRaster r;
	if(!r.Open(fi)) return Image();
	int w = r.GetWidth(), h = r.GetHeight();
	if(w <= 0 || h <= 0) return Image();
	ImageBuffer buf(w, h);
	for(int y = 0; y < h; y++) {
		Raster::Line line = r.GetLine(y);
		const RGBA* px = line;
		if(!px) return Image();
		memcpy(buf[y], px, w * sizeof(RGBA));
	}
	return buf;
}

static Image LoadPngImage(const String& path)
{
	FileIn fi(path);
	if(!fi) return Image();
	PNGRaster r;
	if(!r.Open(fi)) return Image();
	int w = r.GetWidth(), h = r.GetHeight();
	if(w <= 0 || h <= 0) return Image();
	ImageBuffer buf(w, h);
	for(int y = 0; y < h; y++) {
		Raster::Line line = r.GetLine(y);
		const RGBA* px = line;
		if(!px) return Image();
		memcpy(buf[y], px, w * sizeof(RGBA));
	}
	return buf;
}

static VsmImageBuffer ImageToVsmBuffer(const Image& img, bool grayscale)
{
	int w = img.GetWidth(), h = img.GetHeight();
	VsmImageBuffer buf;
	if(grayscale) {
		buf.Create(w, h, 1);
		for(int y = 0; y < h; y++) {
			const RGBA* row = img[y];
			for(int x = 0; x < w; x++) {
				// Luminance (BT.601 integer approximation)
				int g = (row[x].r * 77 + row[x].g * 150 + row[x].b * 29) >> 8;
				buf.Set(x, y, (byte)g);
			}
		}
	} else {
		buf.Create(w, h, 3);
		for(int y = 0; y < h; y++) {
			const RGBA* row = img[y];
			for(int x = 0; x < w; x++) {
				buf.Set(x, y, row[x].r, 0);
				buf.Set(x, y, row[x].g, 1);
				buf.Set(x, y, row[x].b, 2);
			}
		}
	}
	return buf;
}

// ---------------------------------------------------------------------------
// JpegSequenceImporter internals

int JpegSequenceImporter::ExtractNumericPrefix(const String& filename)
{
	const char* p = GetFileName(filename);
	while(*p && !IsDigit(*p)) p++;
	int n = 0;
	while(*p && IsDigit(*p)) { n = n * 10 + (*p - '0'); p++; }
	return n;
}

Vector<String> JpegSequenceImporter::CollectFiles(const String& dir,
                                                   bool sort_numeric) const
{
	static const char* patterns[] = { "*.jpg", "*.jpeg", "*.png", nullptr };

	Index<String> seen;
	Vector<String> files;
	for(int i = 0; patterns[i]; i++) {
		FindFile ff;
		if(ff.Search(AppendFileName(dir, patterns[i]))) {
			do {
				if(!ff.IsDirectory()) {
					String p   = ff.GetPath();
					String key = ToLower(p);
					if(seen.Find(key) < 0) {
						seen.Add(key);
						files.Add(p);
					}
				}
			} while(ff.Next());
		}
	}

	if(sort_numeric) {
		Sort(files, [](const String& a, const String& b) {
			int na = ExtractNumericPrefix(GetFileName(a));
			int nb = ExtractNumericPrefix(GetFileName(b));
			return na != nb ? na < nb : GetFileName(a) < GetFileName(b);
		});
	} else {
		Sort(files, [](const String& a, const String& b) {
			return GetFileName(a) < GetFileName(b);
		});
	}
	return files;
}

bool JpegSequenceImporter::TryImportFile(const String& path, int frame_idx,
    VsmSessionStore& store, const VsmJpegImportOptions& opts,
    VsmJpegImportResult& result)
{
	String ext = ToLower(GetFileExt(path));

	Image img;
	if(ext == ".jpg" || ext == ".jpeg") img = LoadJpegImage(path);
	else if(ext == ".png")              img = LoadPngImage(path);

	if(img.GetWidth() == 0 || img.GetHeight() == 0) {
		result.warnings.Add(GetFileName(path) + ": decode failed");
		result.frames_skipped++;
		return false;
	}

	VsmImageBuffer buf = ImageToVsmBuffer(img, opts.grayscale);

	int   fps   = opts.fps > 0 ? opts.fps : 30;
	int64 ts_ms = (int64)frame_idx * (1000 / fps);
	VsmAssetRef ref = store.SaveFrameImage(frame_idx, buf, ts_ms);
	if(ref.IsEmpty()) {
		result.warnings.Add(GetFileName(path) + ": write failed");
		result.frames_skipped++;
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------
// Public entry point

VsmJpegImportResult JpegSequenceImporter::Import(const VsmJpegImportOptions& opts)
{
	VsmJpegImportResult result;

	if(opts.source_dir.IsEmpty()) {
		LogError(log_, "JpegImporter", "source_dir is empty");
		return result;
	}
	if(opts.output_dir.IsEmpty()) {
		LogError(log_, "JpegImporter", "output_dir is empty");
		return result;
	}
	if(!DirectoryExists(opts.source_dir)) {
		LogError(log_, "JpegImporter", "source_dir not found: " + opts.source_dir);
		return result;
	}

	Vector<String> files = CollectFiles(opts.source_dir, opts.sort_numeric);
	result.frames_scanned = files.GetCount();
	LogInfo(log_, "JpegImporter",
	        Format("Scanned %d file(s) in: %s", result.frames_scanned, opts.source_dir));

	if(files.IsEmpty()) {
		LogWarn(log_, "JpegImporter", "No image files found in: " + opts.source_dir);
		result.success = true;
		return result;
	}

	// Determine frame dimensions from the first decodable file
	int width = 0, height = 0;
	for(const String& f : files) {
		String ext = ToLower(GetFileExt(f));
		Image probe;
		if(ext == ".jpg" || ext == ".jpeg") probe = LoadJpegImage(f);
		else if(ext == ".png")              probe = LoadPngImage(f);
		if(probe.GetWidth() > 0) {
			width  = probe.GetWidth();
			height = probe.GetHeight();
			break;
		}
	}
	if(width == 0 || height == 0) {
		LogError(log_, "JpegImporter", "Cannot determine frame dimensions");
		return result;
	}

	String session_id = opts.session_id.IsEmpty()
	                  ? "import-" + IntStr((int)GetTickCount())
	                  : opts.session_id;

	VsmSessionStore out_store;
	out_store.SetLog(log_.GetSink());
	if(!out_store.Create(opts.output_dir, session_id, width, height, "image-sequence")) {
		LogError(log_, "JpegImporter", "Cannot create session at: " + opts.output_dir);
		return result;
	}

	int frame_idx = 0;
	for(const String& path : files) {
		if(TryImportFile(path, frame_idx, out_store, opts, result)) {
			result.frames_imported++;
			frame_idx++;
		}
	}

	if(!out_store.SaveManifest()) {
		LogError(log_, "JpegImporter", "SaveManifest failed");
		return result;
	}

	result.success    = true;
	result.session_id = session_id;
	result.output_dir = opts.output_dir;

	LogInfo(log_, "JpegImporter",
	        Format("Import complete: %d/%d frames → %s",
	               result.frames_imported, result.frames_scanned, opts.output_dir));
	return result;
}
