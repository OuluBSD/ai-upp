#include "VisualStateModel.h"

namespace Upp {

bool VsmPackTwoWindows(const VsmImageBuffer& first, const VsmImageBuffer& second,
	                   VsmImageBuffer& packed, Vector<VsmPackedWindow>& windows,
	                   String& error)
{
	if(first.IsEmpty() || second.IsEmpty()) { error = "window input is empty"; return false; }
	if(first.height != second.height || first.channels != second.channels) {
		error = "windows must have equal height and channel count";
		return false;
	}
	packed.Create(first.width + second.width, first.height, first.channels);
	for(int y = 0; y < first.height; y++) {
		for(int x = 0; x < first.width; x++)
			for(int c = 0; c < first.channels; c++)
				packed.Set(x, y, first.Get(x, y, c), c);
		for(int x = 0; x < second.width; x++)
			for(int c = 0; c < second.channels; c++)
				packed.Set(first.width + x, y, second.Get(x, y, c), c);
	}
	windows.Clear();
	VsmPackedWindow& a = windows.Add(); a.id = "window0"; a.width = first.width; a.height = first.height;
	VsmPackedWindow& b = windows.Add(); b.id = "window1"; b.source_x = first.width;
	b.width = second.width; b.height = second.height;
	return true;
}

void VsmShaderTemplate::Jsonize(JsonIO& json)
{
	json("id", id)("label", label)("x", x)("y", y)("w", w)("h", h)
	    ("foreground_x", foreground_x)("foreground_y", foreground_y)
	    ("foreground_w", foreground_w)("foreground_h", foreground_h)
	    ("hotspot_x", hotspot_x)("hotspot_y", hotspot_y)
	    ("class_id", class_id)("polarity", polarity)
	    ("expected_scale", expected_scale);
}

bool VsmShaderTemplateManifest::Validate(String& error) const
{
	if(crop_map_width <= 0 || crop_map_height <= 0) {
		error = "crop map dimensions must be positive";
		return false;
	}
	Vector<String> ids;
	for(const VsmShaderTemplate& t : templates) {
		if(t.id.IsEmpty()) { error = "template id is empty"; return false; }
		bool duplicate = false;
		for(const String& id : ids)
			if(id == t.id) { duplicate = true; break; }
		if(duplicate) { error = "duplicate template id: " + t.id; return false; }
		ids.Add(t.id);
		if(t.w <= 0 || t.h <= 0 || t.x < 0 || t.y < 0 ||
		   t.x + t.w > crop_map_width || t.y + t.h > crop_map_height) {
			error = "template crop is outside crop map: " + t.id;
			return false;
		}
		if(t.foreground_w < 0 || t.foreground_h < 0 ||
		   t.foreground_x < 0 || t.foreground_y < 0 ||
		   t.foreground_x + t.foreground_w > t.w ||
		   t.foreground_y + t.foreground_h > t.h) {
			error = "foreground crop is outside template: " + t.id;
			return false;
		}
	}
	return true;
}

bool VsmShaderTemplateManifest::Save(const String& path) const
{
	String error;
	if(!Validate(error)) return false;
	return SaveFile(path, StoreAsJson(*this, true));
}

bool VsmShaderTemplateManifest::Load(const String& path)
{
	String json = LoadFile(path);
	if(json.IsEmpty()) return false;
	if(!LoadFromJson(*this, json)) return false;
	String error;
	return Validate(error);
}

void VsmShaderMatchHit::Jsonize(JsonIO& json)
{
	json("template_index", template_index)("x", x)("y", y)("score", score);
}

static byte ShaderGray(const VsmImageBuffer& image, int x, int y)
{
	x = clamp(x, 0, image.width - 1);
	y = clamp(y, 0, image.height - 1);
	if(image.channels == 1) return image.Get(x, y);
	int r = image.Get(x, y, 0), g = image.Get(x, y, min(1, image.channels - 1));
	int b = image.Get(x, y, min(2, image.channels - 1));
	return (byte)clamp((299 * r + 587 * g + 114 * b) / 1000, 0, 255);
}

double VsmCpuShaderTemplateMatcher::ScoreAt(const VsmImageBuffer& frame,
	                                           const VsmImageBuffer& crop_map,
	                                           const VsmShaderTemplate& t,
	                                           int x, int y)
{
	Rect fr = t.GetForegroundRect();
	if(fr.IsEmpty()) fr = Rect(0, 0, t.w, t.h);
	double sum = 0.0, sum_sq = 0.0;
	int count = 0;
	for(int ty = fr.top; ty < fr.bottom; ty++)
		for(int tx = fr.left; tx < fr.right; tx++) {
			byte a = ShaderGray(frame, x + tx, y + ty);
			byte b = ShaderGray(crop_map, t.x + tx, t.y + ty);
			if(t.polarity) b = 255 - b;
			double d = (double)a - b;
			sum += d;
			sum_sq += d * d;
			count++;
		}
	if(count == 0) return 0.0;
	// Normalized SSD is stable for different crop sizes and bounded in [0,1].
	double mse = sum_sq / (count * 255.0 * 255.0);
	return 1.0 - clamp(mse, 0.0, 1.0);
}

bool VsmCpuShaderTemplateMatcher::Match(const VsmImageBuffer& frame,
	                                       const VsmImageBuffer& crop_map,
	                                       const VsmShaderTemplateManifest& manifest,
	                                       VsmShaderEvidence& output,
	                                       String& error) const
{
	if(frame.IsEmpty() || crop_map.IsEmpty()) { error = "frame or crop map is empty"; return false; }
	if(crop_map.width != manifest.crop_map_width || crop_map.height != manifest.crop_map_height) {
		error = "crop map dimensions do not match manifest";
		return false;
	}
	if(manifest.templates.IsEmpty()) { error = "manifest has no templates"; return false; }
	if(!manifest.Validate(error)) return false;
	output.image.Create(frame.width, frame.height, 3);
	output.best_hits.SetCount(manifest.templates.GetCount());
	output.best_scores.SetCount(manifest.templates.GetCount(), 0.0);
	output.mean_scores.SetCount(manifest.templates.GetCount(), 0.0);
	Vector<int> score_counts;
	score_counts.SetCount(manifest.templates.GetCount(), 0);
	for(int y = 0; y < frame.height; y++)
		for(int x = 0; x < frame.width; x++) {
			double best = 0.0, total = 0.0;
			int best_id = 0;
			for(int i = 0; i < manifest.templates.GetCount(); i++) {
				const VsmShaderTemplate& t = manifest.templates[i];
				if(x + t.w > frame.width || y + t.h > frame.height) continue;
				double score = ScoreAt(frame, crop_map, t, x, y);
				total += score;
				if(score > output.best_scores[i]) {
					output.best_scores[i] = score;
					VsmShaderMatchHit& hit = output.best_hits[i];
					hit.template_index = i;
					hit.x = x;
					hit.y = y;
					hit.score = score;
				}
				if(score > best) { best = score; best_id = i; }
				output.mean_scores[i] += score;
				score_counts[i]++;
			}
			byte* p = output.image.pixels.Begin() + (y * frame.width + x) * 3;
			p[0] = (byte)clamp((int)floor(best * 255.0 + 0.5), 0, 255);
			p[1] = (byte)clamp(best_id, 0, 255);
			p[2] = (byte)clamp((int)floor((total / max(1, manifest.templates.GetCount())) * 255.0 + 0.5), 0, 255);
		}
	for(int i = 0; i < manifest.templates.GetCount(); i++)
		output.mean_scores[i] /= max(1, score_counts[i]);
	return true;
}

String VsmCpuShaderTemplateMatcher::FragmentShaderSource()
{
	return "#version 330 core\n"
	       "uniform sampler2D frame_image;\n"
	       "uniform sampler2D crop_map;\n"
	       "uniform ivec2 frame_size;\n"
	       "uniform ivec2 crop_map_size;\n"
	       "uniform int template_count;\n"
	       "uniform ivec4 template_rects[64];\n"
	       "uniform ivec4 foreground_rects[64];\n"
	       "uniform int template_polarity[64];\n"
	       "layout(location=0) out vec4 evidence;\n"
	       "float gray(vec4 c) { return c.r; }\n"
	       "void main() {\n"
	       "  ivec2 p = ivec2(gl_FragCoord.xy); float best=0.0; float mean=0.0; int best_id=0;\n"
	       "  int used=0;\n"
	       "  for(int i=0; i<64; i++) { if(i>=template_count) break;\n"
	       "    ivec4 tr=template_rects[i], fr=foreground_rects[i];\n"
	       "    if(fr.z<=0 || fr.w<=0) fr=ivec4(0,0,tr.z,tr.w);\n"
	       "    if(p.x+tr.z>frame_size.x || p.y+tr.w>frame_size.y) continue;\n"
	       "    float error=0.0; int count=0;\n"
	       "    for(int y=0; y<64; y++) { if(y>=fr.w) break; for(int x=0; x<64; x++) { if(x>=fr.z) break;\n"
	       "      float a=gray(texelFetch(frame_image,p+fr.xy+ivec2(x,y),0));\n"
	       "      float b=gray(texelFetch(crop_map,tr.xy+fr.xy+ivec2(x,y),0));\n"
	       "      if(template_polarity[i]!=0) b=1.0-b;\n"
	       "      float d=a-b; error += d*d; count++;\n"
	       "    }}\n"
	       "    float score=1.0-clamp(error/max(1.0,float(count)),0.0,1.0); mean+=score; used++;\n"
	       "    if(score>best) { best=score; best_id=i; }\n"
	       "  }\n"
	       "  evidence=vec4(best,float(best_id)/255.0,mean/max(1,used),1.0);\n"
	       "}\n";
}

String VsmCpuShaderTemplateMatcher::VertexShaderSource()
{
	return "#version 330 core\n"
	       "layout(location=0) in vec2 position;\n"
	       "out vec2 uv;\n"
	       "void main() { uv = position * 0.5 + 0.5; gl_Position = vec4(position, 0.0, 1.0); }\n";
}

bool VsmShaderRecognitionService::Process(const VsmImageBuffer& frame,
	                                         VsmShaderEvidence& evidence,
	                                         Vector<VsmEvidenceTextRun>& runs,
	                                         String& error) const
{
	VsmCpuShaderTemplateMatcher matcher;
	if(!matcher.Match(frame, crop_map, manifest, evidence, error)) return false;
	byte selected = threshold;
	if(!use_threshold) {
		VsmThresholdResult analysis = VsmAnalyzeEvidenceThreshold(evidence.image, 0);
		selected = (byte)clamp((int)floor(analysis.threshold * 255.0 + 0.5), 0, 255);
	}
	runs = VsmReconstructEvidence(evidence.image, manifest, selected);
	return true;
}

void VsmThresholdResult::Jsonize(JsonIO& json)
{
	json("threshold", threshold)("accepted", accepted)("rejected", rejected)
	    ("histogram", histogram);
}

VsmThresholdResult VsmAnalyzeEvidenceThreshold(const VsmImageBuffer& evidence,
	                                              int channel, int bins)
{
	VsmThresholdResult out;
	out.histogram.SetCount(max(1, bins), 0);
	if(evidence.IsEmpty() || channel < 0 || channel >= evidence.channels) return out;
	for(int y = 0; y < evidence.height; y++)
		for(int x = 0; x < evidence.width; x++) {
			int value = evidence.Get(x, y, channel);
			out.histogram[min(out.histogram.GetCount() - 1, value * out.histogram.GetCount() / 256)]++;
		}
	// Conservative first-pass threshold: midpoint between the highest populated
	// low-score bin and the highest populated score bin is intentionally exposed
	// as evidence, not hidden in the shader.
	int first = 0, last = out.histogram.GetCount() - 1;
	while(first < last && out.histogram[first] == 0) first++;
	while(last > first && out.histogram[last] == 0) last--;
	out.threshold = (double)(first + last) / (2.0 * max(1, out.histogram.GetCount() - 1));
	for(int y = 0; y < evidence.height; y++)
		for(int x = 0; x < evidence.width; x++)
			if(evidence.Get(x, y, channel) / 255.0 >= out.threshold) out.accepted++;
	out.rejected = evidence.width * evidence.height - out.accepted;
	return out;
}

VsmOccupancyMask VsmBuildOccupancyMask(const VsmImageBuffer& evidence, byte threshold)
{
	VsmOccupancyMask out;
	out.width = evidence.width; out.height = evidence.height;
	out.pixels.SetCount(out.width * out.height, 0);
	if(evidence.channels == 0) return out;
	for(int y = 0; y < out.height; y++)
		for(int x = 0; x < out.width; x++)
			out.pixels[y * out.width + x] = evidence.Get(x, y) >= threshold;
	return out;
}

Rect VsmFindOccupancyBounds(const VsmOccupancyMask& mask)
{
	int left = mask.width, top = mask.height, right = 0, bottom = 0;
	for(int y = 0; y < mask.height; y++)
		for(int x = 0; x < mask.width; x++) if(mask.IsSet(x, y)) {
			left = min(left, x); top = min(top, y);
			right = max(right, x + 1); bottom = max(bottom, y + 1);
		}
	return left == mask.width ? Rect(0, 0, 0, 0) : Rect(left, top, right, bottom);
}

VsmPackedOccupancyMask VsmBuildPackedOccupancyMask(const VsmImageBuffer& evidence, byte threshold)
{
	VsmPackedOccupancyMask out;
	out.width = evidence.width; out.height = evidence.height;
	out.bits.SetCount((out.width * out.height + 7) / 8, 0);
	for(int y = 0; y < out.height; y++) for(int x = 0; x < out.width; x++)
		if(evidence.Get(x, y) >= threshold) {
			int i = y * out.width + x;
			out.bits[i >> 3] |= (byte)(1 << (i & 7));
		}
	return out;
}

VsmTileOccupancyMask VsmBuildTileOccupancyMask(const VsmImageBuffer& evidence,
	                                             byte threshold, int tile_size)
{
	VsmTileOccupancyMask out;
	out.tile_size = max(1, tile_size);
	out.width = (evidence.width + out.tile_size - 1) / out.tile_size;
	out.height = (evidence.height + out.tile_size - 1) / out.tile_size;
	out.tiles.SetCount(out.width * out.height, 0);
	for(int y = 0; y < evidence.height; y++) for(int x = 0; x < evidence.width; x++)
		if(evidence.Get(x, y) >= threshold)
			out.tiles[(y / out.tile_size) * out.width + x / out.tile_size] = 1;
	return out;
}

VsmOccupancyBenchmark VsmBenchmarkOccupancy(const VsmImageBuffer& evidence,
	                                           byte threshold, int iterations)
{
	VsmOccupancyBenchmark out;
	out.iterations = max(1, iterations);
	dword start = GetTickCount();
	for(int i = 0; i < out.iterations; i++) {
		VsmPackedOccupancyMask mask = VsmBuildPackedOccupancyMask(evidence, threshold);
		out.packed_occupied += mask.bits.GetCount();
		out.packed_bytes = mask.bits.GetCount();
	}
	out.packed_ms = GetTickCount() - start;
	start = GetTickCount();
	for(int i = 0; i < out.iterations; i++) {
		VsmTileOccupancyMask mask = VsmBuildTileOccupancyMask(evidence, threshold);
		out.tile_occupied += mask.tiles.GetCount();
		out.tile_bytes = mask.tiles.GetCount();
	}
	out.tiles_ms = GetTickCount() - start;
	start = GetTickCount();
	for(int i = 0; i < out.iterations; i++) {
		VsmOccupancyMask mask = VsmBuildOccupancyMask(evidence, threshold);
		out.xy_occupied += mask.pixels.GetCount();
		out.xy_bytes = mask.pixels.GetCount();
		}
	out.xy_ms = GetTickCount() - start;
	return out;
}

Vector<VsmEvidenceTextRun> VsmReconstructEvidence(const VsmImageBuffer& evidence,
	                                               const VsmShaderTemplateManifest& manifest,
	                                               byte threshold, const Rect& requested)
{
	Vector<VsmEvidenceTextRun> out;
	static const int max_runs = 4096;
	if(evidence.IsEmpty() || evidence.channels < 2 || manifest.templates.IsEmpty()) return out;
	Rect area = requested.IsEmpty() ? Rect(0, 0, evidence.width, evidence.height)
	                              : requested & Rect(0, 0, evidence.width, evidence.height);
	if(area.IsEmpty()) return out;
	VsmOccupancyMask mask = VsmBuildOccupancyMask(evidence, threshold);
	Vector<byte> visited;
	visited.SetCount(evidence.width * evidence.height, 0);
	bool limit_reached = false;
	for(int sy = area.top; sy < area.bottom && !limit_reached; sy++)
		for(int sx = area.left; sx < area.right; sx++) {
			int offset = sy * evidence.width + sx;
			if(visited[offset] || !mask.IsSet(sx, sy)) continue;
			if(out.GetCount() >= max_runs) {
				limit_reached = true;
				break;
			}
			Vector<Point> queue;
			queue.Add(Point(sx, sy)); visited[offset] = 1;
			VsmEvidenceTextRun& run = out.Add();
			run.bounds = Rect(sx, sy, sx + 1, sy + 1);
			for(int qi = 0; qi < queue.GetCount(); qi++) {
				Point p = queue[qi];
				run.bounds.left = min(run.bounds.left, p.x); run.bounds.top = min(run.bounds.top, p.y);
				run.bounds.right = max(run.bounds.right, p.x + 1); run.bounds.bottom = max(run.bounds.bottom, p.y + 1);
				int id = evidence.Get(p.x, p.y, 1);
				if(id >= manifest.templates.GetCount()) { run.ambiguous = true; continue; }
				const VsmShaderTemplate& templ = manifest.templates[id];
				bool found = false;
				for(VsmEvidenceGlyph& glyph : run.glyphs)
					if(glyph.template_index == id) { glyph.bounds = glyph.bounds | Rect(p.x, p.y, p.x + 1, p.y + 1); found = true; break; }
				if(!found) {
					VsmEvidenceGlyph& glyph = run.glyphs.Add();
					glyph.template_index = id; glyph.template_id = templ.id; glyph.label = templ.label;
					glyph.bounds = Rect(p.x, p.y, p.x + 1, p.y + 1);
				}
				run.confidence += evidence.Get(p.x, p.y, 0) / 255.0;
				for(int dy = -1; dy <= 1; dy++) for(int dx = -1; dx <= 1; dx++) {
					int nx = p.x + dx, ny = p.y + dy;
					if(nx < area.left || ny < area.top || nx >= area.right || ny >= area.bottom) continue;
					int no = ny * evidence.width + nx;
					if(!visited[no] && mask.IsSet(nx, ny)) { visited[no] = 1; queue.Add(Point(nx, ny)); }
				}
			}
			run.confidence /= max(1, queue.GetCount());
			Sort(run.glyphs, [](const VsmEvidenceGlyph& a, const VsmEvidenceGlyph& b) {
				return a.bounds.left < b.bounds.left;
			});
			for(const VsmEvidenceGlyph& glyph : run.glyphs) {
				if(glyph.label.GetCount() == 1 && IsDigit(glyph.label[0])) run.text.Cat(glyph.label);
				else if(glyph.label == "dot") run.text.Cat('.');
				else run.ambiguous = true;
			}
			if(run.text.IsEmpty()) run.ambiguous = true;
		}
	return out;
}

} // namespace Upp
