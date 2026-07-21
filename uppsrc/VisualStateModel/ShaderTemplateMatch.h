#ifndef _VisualStateModel_ShaderTemplateMatch_h_
#define _VisualStateModel_ShaderTemplateMatch_h_

namespace Upp {

struct VsmPackedWindow : Moveable<VsmPackedWindow> {
	String id;
	int source_x = 0, source_y = 0;
	int width = 0, height = 0;
	int64 timestamp_ms = 0;
	void Jsonize(JsonIO& json) { json("id", id)("source_x", source_x)("source_y", source_y)
		("width", width)("height", height)("timestamp_ms", timestamp_ms); }
};

bool VsmPackTwoWindows(const VsmImageBuffer& first, const VsmImageBuffer& second,
	                   VsmImageBuffer& packed, Vector<VsmPackedWindow>& windows,
	                   String& error);

struct VsmShaderTemplate : Moveable<VsmShaderTemplate> {
	String id;
	String label;
	int x = 0, y = 0, w = 0, h = 0;
	int foreground_x = 0, foreground_y = 0;
	int foreground_w = 0, foreground_h = 0;
	int hotspot_x = 0, hotspot_y = 0;
	int class_id = 0;
	int polarity = 0;
	double expected_scale = 1.0;

	Rect GetRect() const { return Rect(x, y, x + w, y + h); }
	Rect GetForegroundRect() const { return Rect(foreground_x, foreground_y,
		foreground_x + foreground_w, foreground_y + foreground_h); }
	void Jsonize(JsonIO& json);
};

struct VsmShaderTemplateManifest {
	int crop_map_width = 0;
	int crop_map_height = 0;
	int version = 1;
	Vector<VsmShaderTemplate> templates;

	void Jsonize(JsonIO& json) { json("version", version)("crop_map_width", crop_map_width)
		("crop_map_height", crop_map_height)("templates", templates); }
	bool Validate(String& error) const;
	bool Save(const String& path) const;
	bool Load(const String& path);
};

struct VsmShaderMatchHit : Moveable<VsmShaderMatchHit> {
	int template_index = -1;
	int x = -1, y = -1;
	double score = 0.0;
	void Jsonize(JsonIO& json);
};

struct VsmShaderEvidence {
	VsmImageBuffer image;
	Vector<VsmShaderMatchHit> best_hits;
	Vector<double> best_scores;
	Vector<double> mean_scores;

	bool Save(const String& path) const { return image.Save(path); }
};

class VsmCpuShaderTemplateMatcher {
public:
	bool Match(const VsmImageBuffer& frame,
	           const VsmImageBuffer& crop_map,
	           const VsmShaderTemplateManifest& manifest,
	           VsmShaderEvidence& output,
	           String& error) const;

	static double ScoreAt(const VsmImageBuffer& frame,
	                     const VsmImageBuffer& crop_map,
	                     const VsmShaderTemplate& templ,
	                     int x, int y);
	static String FragmentShaderSource();
};

struct VsmThresholdResult {
	double threshold = 0.0;
	int accepted = 0;
	int rejected = 0;
	Vector<int> histogram;
	void Jsonize(JsonIO& json);
};

VsmThresholdResult VsmAnalyzeEvidenceThreshold(const VsmImageBuffer& evidence,
	                                             int channel = 0,
	                                             int bins = 256);

struct VsmOccupancyMask {
	int width = 0, height = 0;
	Vector<byte> pixels;
	bool IsSet(int x, int y) const { return pixels[y * width + x] != 0; }
};

VsmOccupancyMask VsmBuildOccupancyMask(const VsmImageBuffer& evidence,
	                                    byte threshold);
Rect VsmFindOccupancyBounds(const VsmOccupancyMask& mask);

struct VsmPackedOccupancyMask {
	int width = 0, height = 0;
	Vector<byte> bits;
	bool IsSet(int x, int y) const { int i = y * width + x; return (bits[i >> 3] & (1 << (i & 7))) != 0; }
};

VsmPackedOccupancyMask VsmBuildPackedOccupancyMask(const VsmImageBuffer& evidence,
	                                               byte threshold);

struct VsmTileOccupancyMask {
	int width = 0, height = 0, tile_size = 8;
	Vector<byte> tiles;
	bool IsSet(int x, int y) const { return tiles[y * width + x] != 0; }
};

VsmTileOccupancyMask VsmBuildTileOccupancyMask(const VsmImageBuffer& evidence,
	                                           byte threshold, int tile_size = 8);

struct VsmOccupancyBenchmark {
	int iterations = 0;
	dword packed_ms = 0, tiles_ms = 0, xy_ms = 0;
	int packed_bytes = 0, tile_bytes = 0, xy_bytes = 0;
	int packed_occupied = 0, tile_occupied = 0, xy_occupied = 0;
	void Jsonize(JsonIO& json) { json("iterations", iterations)("packed_ms", packed_ms)
		("tiles_ms", tiles_ms)("xy_ms", xy_ms)("packed_bytes", packed_bytes)
		("tile_bytes", tile_bytes)("xy_bytes", xy_bytes)("packed_occupied", packed_occupied)
		("tile_occupied", tile_occupied)("xy_occupied", xy_occupied); }
};

VsmOccupancyBenchmark VsmBenchmarkOccupancy(const VsmImageBuffer& evidence,
	                                         byte threshold, int iterations = 1);

struct VsmEvidenceGlyph : Moveable<VsmEvidenceGlyph> {
	String template_id;
	String label;
	int template_index = -1;
	Rect bounds;
	double score = 0.0;
	void Jsonize(JsonIO& json) { int w = bounds.Width(), h = bounds.Height();
		json("template_id", template_id)("label", label)("template_index", template_index)
		("x", bounds.left)("y", bounds.top)("w", w)("h", h)("score", score); }
};

struct VsmEvidenceTextRun : Moveable<VsmEvidenceTextRun> {
	String text;
	Rect bounds;
	double confidence = 0.0;
	bool ambiguous = false;
	Vector<VsmEvidenceGlyph> glyphs;
	void Jsonize(JsonIO& json) { int w = bounds.Width(), h = bounds.Height();
		json("text", text)("x", bounds.left)("y", bounds.top)
		("w", w)("h", h)("confidence", confidence)
		("ambiguous", ambiguous)("glyphs", glyphs); }
};

struct VsmShaderRecognitionService {
	VsmShaderTemplateManifest manifest;
	VsmImageBuffer crop_map;
	byte threshold = 0;
	bool use_threshold = false;

	bool Process(const VsmImageBuffer& frame, VsmShaderEvidence& evidence,
	             Vector<VsmEvidenceTextRun>& runs, String& error) const;
};

Vector<VsmEvidenceTextRun> VsmReconstructEvidence(const VsmImageBuffer& evidence,
	                                               const VsmShaderTemplateManifest& manifest,
	                                               byte threshold,
	                                               const Rect& search_area = Null);

} // namespace Upp

#endif
