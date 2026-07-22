#include "AmpAtlasRuntimeProbe.h"

CONSOLE_APP_MAIN {
	Upp::RunAmpAtlasRuntimeProbe();
}

NAMESPACE_UPP

static bool ReadArguments(String& atlas_path, String& manifest_path,
                       String& inventory_path, int& device_index, bool& exact_selftest,
                       bool& frame_selftest, int& threshold, String& frame_report_path)
{
	const Vector<String>& args = CommandLine();
	for(int i = 0; i + 1 < args.GetCount(); i++) {
		if(args[i] == "--atlas")
			atlas_path = args[i + 1];
		else if(args[i] == "--manifest")
			manifest_path = args[i + 1];
		else if(args[i] == "--amp-device-inventory")
			inventory_path = args[i + 1];
		else if(args[i] == "--amp-device-index")
			device_index = StrInt(args[i + 1]);
		else if(args[i] == "--amp-match-threshold")
			threshold = StrInt(args[i + 1]);
		else if(args[i] == "--amp-frame-report")
			frame_report_path = args[i + 1];
	}
	for(const String& arg : args)
		exact_selftest |= arg == "--amp-atlas-selftest";
	for(const String& arg : args)
		frame_selftest |= arg == "--amp-frame-selftest";
	return !atlas_path.IsEmpty() && !manifest_path.IsEmpty();
}

static byte Gray(const RGBA& p)
{
	return (byte)((p.r * 77 + p.g * 150 + p.b * 29) >> 8);
}

static bool BuildCompactTemplates(const Image& atlas,
	                              const AmpTemplateAtlasManifest& manifest,
	                              Vector<int>& pixels, int& nonempty, String& error)
{
	if(atlas.IsEmpty() || atlas.GetWidth() != manifest.atlas_width ||
	   atlas.GetHeight() != manifest.atlas_height) {
		error = "atlas dimensions do not match manifest";
		return false;
	}
	nonempty = 0;
	for(const AmpTemplateAtlasEntry& entry : manifest.entries) {
		int entry_nonempty = 0;
		for(int y = entry.y; y < entry.y + entry.height; y++) {
			const RGBA* row = atlas[y];
			for(int x = entry.x; x < entry.x + entry.width; x++) {
				byte value = row[x].a ? Gray(row[x]) : 0;
				pixels.Add((int)value);
				entry_nonempty += value != 0;
			}
		}
		if(entry_nonempty == 0) {
			error = Format("empty template entry: %s", ~entry.id);
			return false;
		}
		nonempty += entry_nonempty;
		COUTLOG(Format("amp_atlas_entry id=%s pixels=%d nonempty=%d",
		               ~entry.id, entry.width * entry.height, entry_nonempty));
	}
	return true;
}

static void BuildAtlasPixels(const Image& atlas, Vector<int>& pixels)
{
	pixels.SetCount(atlas.GetWidth() * atlas.GetHeight());
	for(int y = 0; y < atlas.GetHeight(); y++) {
		const RGBA* row = atlas[y];
		for(int x = 0; x < atlas.GetWidth(); x++)
			pixels[y * atlas.GetWidth() + x] = row[x].a ? Gray(row[x]) : 0;
	}
}

static String EscapeHtml(String text)
{
	text.Replace("&", "&amp;");
	text.Replace("<", "&lt;");
	text.Replace(">", "&gt;");
	text.Replace("\"", "&quot;");
	return text;
}

static Image MakeGrayImage(const Vector<int>& pixels, int width, int height)
{
	ImageBuffer buffer(width, height);
	for(int y = 0; y < height; y++) {
		RGBA* row = buffer[y];
		for(int x = 0; x < width; x++) {
			byte value = (byte)clamp(pixels[y * width + x], 0, 255);
			row[x].r = value;
			row[x].g = value;
			row[x].b = value;
			row[x].a = 255;
		}
	}
	return Image(buffer);
}

static void Outline(ImageBuffer& image, int x, int y, int width, int height, Color color)
{
	for(int xx = x; xx < x + width; xx++) {
		if(xx >= 0 && xx < image.GetWidth()) {
			if(y >= 0 && y < image.GetHeight()) image[y][xx] = color;
			if(y + height - 1 >= 0 && y + height - 1 < image.GetHeight())
				image[y + height - 1][xx] = color;
		}
	}
	for(int yy = y; yy < y + height; yy++) {
		if(yy >= 0 && yy < image.GetHeight()) {
			if(x >= 0 && x < image.GetWidth()) image[yy][x] = color;
			if(x + width - 1 >= 0 && x + width - 1 < image.GetWidth())
				image[yy][x + width - 1] = color;
		}
	}
}

static bool SaveFrameEvidence(const String& report_path, const Vector<int>& frame,
	                           int frame_width, int frame_height,
	                           const AmpTemplateAtlasManifest& manifest,
	                           const AmpTemplateMatchResult& result,
	                           const String& backend, int threshold,
	                           int64 cpu_ms, int64 amp_ms, bool equal,
	                           int64& report_ms)
{
	int64 started = msecs();
	if(report_path.IsEmpty()) {
		report_ms = 0;
		return true;
	}
	String folder = GetFileFolder(report_path);
	RealizeDirectory(folder);
	String input_name = "frame_input.png";
	String overlay_name = "frame_matches.png";
	String input_path = AppendFileName(folder, input_name);
	String overlay_path = AppendFileName(folder, overlay_name);
	Image input = MakeGrayImage(frame, frame_width, frame_height);
	if(!PNGEncoder().SaveFile(input_path, input))
		return false;
	ImageBuffer overlay(input);
	for(const AmpTemplateMatchHit& hit : result.entries) {
		if(hit.entry_index < 0 || hit.x < 0 || hit.y < 0)
			continue;
		const AmpTemplateAtlasEntry& entry = manifest.entries[hit.entry_index];
		Outline(overlay, hit.x, hit.y, entry.width, entry.height,
		        hit.accepted ? Color(70, 230, 100) : Color(220, 70, 70));
	}
	if(!PNGEncoder().SaveFile(overlay_path, Image(overlay)))
		return false;
	String html;
	html << "<!doctype html><meta charset=\"utf-8\"><title>AMP frame evidence</title>\n"
	      << "<h1>AMP frame-template evidence</h1>\n"
	      << "<p>backend=" << EscapeHtml(backend) << " threshold=" << threshold
	      << " reference_match=" << (equal ? "pass" : "fail") << "</p>\n"
	      << "<p><img src=\"" << input_name << "\" alt=\"input frame\"></p>\n"
	      << "<p><img src=\"" << overlay_name << "\" alt=\"match overlay\"></p>\n"
	      << "<table><tr><th>id</th><th>accepted</th><th>score</th><th>x</th><th>y</th></tr>\n";
	for(const AmpTemplateMatchHit& hit : result.entries)
		html << "<tr><td>" << EscapeHtml(hit.id) << "</td><td>"
		      << (hit.accepted ? "yes" : "no") << "</td><td>" << hit.score
		      << "</td><td>" << hit.x << "</td><td>" << hit.y << "</td></tr>\n";
	int64 report_elapsed_ms = msecs() - started;
	if(report_elapsed_ms < 1)
		report_elapsed_ms = 1;
	html << "</table><p>candidates=" << result.candidate_count
	      << " accepted=" << result.accepted << " rejected=" << result.rejected
	      << " checksum=" << result.checksum << "</p>\n"
	      << "<p>cpu_ms=" << cpu_ms << " amp_ms=" << amp_ms
	      << " report_ms=" << report_elapsed_ms << "</p>\n";
	if(!SaveFile(report_path, html))
		return false;
	report_ms = msecs() - started;
	if(report_ms < 1)
		report_ms = 1;
	return true;
}

static bool RunFrameSelftest(const Vector<int>& atlas_pixels,
	                           const AmpTemplateAtlasManifest& manifest,
	                           int threshold, const String& inventory_path,
	                           int device_index, const String& report_path)
{
	if(manifest.entries.GetCount() < 2)
		return false;
	const AmpTemplateAtlasEntry& first = manifest.entries[0];
	const AmpTemplateAtlasEntry& second = manifest.entries[1];
	int frame_width = first.width + second.width + 4;
	int frame_height = max(first.height, second.height) + 2;
	Vector<int> frame;
	frame.SetCount(frame_width * frame_height, 0);
	for(int y = 0; y < first.height; y++)
		for(int x = 0; x < first.width; x++)
			frame[(y + 1) * frame_width + x + 1] =
				atlas_pixels[(first.y + y) * manifest.atlas_width + first.x + x];
	for(int y = 0; y < second.height; y++)
		for(int x = 0; x < second.width; x++)
			frame[(y + 1) * frame_width + x + first.width + 2] =
				atlas_pixels[(second.y + y) * manifest.atlas_width + second.x + x];
	AmpTemplateMatchResult result, reference;
	String error;
	int64 cpu_started = msecs();
	if(!MatchAmpTemplatesCpu(frame, frame_width, frame_height, atlas_pixels,
	                         manifest, threshold, reference, error)) {
		COUTLOG(Format("amp_frame_selftest=fail error=%s", ~error));
		return false;
	}
	int64 cpu_ms = msecs() - cpu_started;
	String device_path;
#ifdef HAVE_SYSTEM_AMP
	Vector<AmpDeviceInfo> devices;
	AmpDeviceInfo selected;
	if(!LoadAmpDeviceInventory(inventory_path, devices, error) ||
	   !SelectAmpDevice(devices, device_index, selected, error)) {
		COUTLOG(Format("amp_frame_selftest=fail device=%s", ~error));
		return false;
	}
	device_path = selected.device_path;
#endif
	int64 amp_started = msecs();
	if(!MatchAmpTemplatesAmp(frame, frame_width, frame_height, atlas_pixels,
	                         manifest, threshold, device_path, result, error)) {
		COUTLOG(Format("amp_frame_selftest=fail kernel=%s", ~error));
		return false;
	}
	int64 amp_ms = msecs() - amp_started;
	bool equal = result.entries.GetCount() == reference.entries.GetCount();
	for(int i = 0; equal && i < result.entries.GetCount(); i++)
		equal = result.entries[i].id == reference.entries[i].id &&
		         result.entries[i].score == reference.entries[i].score &&
		         result.entries[i].x == reference.entries[i].x &&
		         result.entries[i].y == reference.entries[i].y;
	if(!equal) {
		COUTLOG("amp_frame_selftest=fail reference-mismatch");
		return false;
	}
	String backend;
#ifdef HAVE_SYSTEM_AMP
	backend = "native-amp-kernel";
#else
	backend = "compat-cpu";
#endif
	int64 report_ms = 0;
	if(!SaveFrameEvidence(report_path, frame, frame_width, frame_height, manifest,
	                      result, backend, threshold, cpu_ms, amp_ms, equal, report_ms)) {
		COUTLOG("amp_frame_report=fail write");
		return false;
	}
	COUTLOG(Format("amp_frame_selftest=pass backend=%s frame=%d`x%d threshold=%d "
	               "candidates=%d accepted=%d rejected=%d checksum=%d winner=%s score=%d",
	               ~backend,
	               frame_width, frame_height, threshold, result.candidate_count,
	               result.accepted, result.rejected, result.checksum,
	               result.winner_index >= 0 ? ~result.entries[result.winner_index].id : "none",
	               result.winner_score));
	COUTLOG(Format("amp_frame_timing cpu_ms=%d amp_ms=%d report_ms=%d",
	               cpu_ms, amp_ms, report_ms));
	if(!report_path.IsEmpty())
		COUTLOG(Format("amp_frame_report=pass path=%s", ~report_path));
	for(const AmpTemplateMatchHit& hit : result.entries)
		COUTLOG(Format("amp_frame_hit id=%s accepted=%d score=%d at=%d,%d",
		               ~hit.id, hit.accepted, hit.score, hit.x, hit.y));
	return result.entries[0].accepted && result.entries[1].accepted &&
	       result.entries[0].score == 0 && result.entries[1].score == 0;
}

static int RunCompatAtlasRuntime(const Image& atlas,
	                             const AmpTemplateAtlasManifest& manifest,
	                             const Vector<int>& pixels,
	                             int nonempty,
	                             bool exact_selftest,
	                             int64 started,
	                             int64 prepare_ms)
{
	Vector<int> match_pixels;
	match_pixels.Append(pixels);
	for(int i = 0; i < match_pixels.GetCount(); i++)
		match_pixels[i] += i & 7;

	int checksum = 0;
	for(int value : match_pixels)
		checksum += value;

	int failed = 0;
	if(exact_selftest) {
		Vector<int> atlas_pixels;
		BuildAtlasPixels(atlas, atlas_pixels);
		int offset = 0;
		for(int i = 0; i < manifest.entries.GetCount(); i++) {
			const AmpTemplateAtlasEntry& entry = manifest.entries[i];
			int score = 0;
			for(int y = 0; y < entry.height; y++)
				for(int x = 0; x < entry.width; x++) {
					int a = atlas_pixels[(entry.y + y) * atlas.GetWidth() + entry.x + x];
					int b = pixels[offset + y * entry.width + x];
					int d = a - b;
					score += d < 0 ? -d : d;
				}
			if(score != 0) {
				failed++;
				COUTLOG(Format("amp_atlas_selftest_fail id=%s score=%d", ~entry.id, score));
			}
			offset += entry.width * entry.height;
		}
	}
	int64 total_ms = msecs() - started;
	COUTLOG(Format("amp_atlas_runtime=pass backend=compat-cpu entries=%d nonempty=%d "
	               "pixels=%d checksum=%d selftest=%d selftest_failed=%d "
	               "prepare_ms=%d total_ms=%d",
	               manifest.entries.GetCount(), nonempty, pixels.GetCount(), checksum,
	               exact_selftest, failed, prepare_ms, total_ms));
	return checksum > 0 && failed == 0 ? 0 : 1;
}

int RunAmpAtlasRuntimeProbe()
{
	String atlas_path, manifest_path, inventory_path;
	int device_index = 0;
	bool exact_selftest = false;
	bool frame_selftest = false;
	int threshold = 0;
	String frame_report_path;
	if(!ReadArguments(atlas_path, manifest_path, inventory_path, device_index,
                  exact_selftest, frame_selftest, threshold, frame_report_path)) {
		COUTLOG("usage=--atlas <atlas.png> --manifest <manifest.json> "
		        "[--amp-device-inventory <inventory.json>] [--amp-device-index n] "
		        "[--amp-frame-selftest] [--amp-match-threshold n] "
		        "[--amp-frame-report report.htm]");
		return 2;
	}
	int64 started = msecs();
	AmpTemplateAtlasManifest manifest;
	String error;
	if(!manifest.Load(manifest_path, error)) {
		COUTLOG(Format("amp_atlas_runtime=fail manifest=%s", ~error));
		return 1;
	}
	Image atlas = StreamRaster::LoadFileAny(atlas_path);
	Vector<int> pixels;
	int nonempty = 0;
	if(!BuildCompactTemplates(atlas, manifest, pixels, nonempty, error)) {
		COUTLOG(Format("amp_atlas_runtime=fail pixels=%s", ~error));
		return 1;
	}
	if(frame_selftest) {
		Vector<int> atlas_pixels;
		BuildAtlasPixels(atlas, atlas_pixels);
		if(!RunFrameSelftest(atlas_pixels, manifest, threshold, inventory_path,
		                     device_index, frame_report_path))
			return 1;
	}
	int64 prepare_ms = msecs() - started;
#ifndef HAVE_SYSTEM_AMP
	return RunCompatAtlasRuntime(atlas, manifest, pixels, nonempty, exact_selftest,
	                             started, prepare_ms);
#else
	if(inventory_path.IsEmpty()) {
		COUTLOG("amp_atlas_runtime=fail device=inventory-required-for-native");
		return 2;
	}
	Vector<int> atlas_pixels;
	Vector<int> offsets, widths, heights, xs, ys;
	if(exact_selftest) {
		BuildAtlasPixels(atlas, atlas_pixels);
		int offset = 0;
		for(const AmpTemplateAtlasEntry& entry : manifest.entries) {
			offsets.Add(offset);
			widths.Add(entry.width);
			heights.Add(entry.height);
			xs.Add(entry.x);
			ys.Add(entry.y);
			offset += entry.width * entry.height;
		}
	}
	Vector<AmpDeviceInfo> devices;
	AmpDeviceInfo selected;
	if(!LoadAmpDeviceInventory(inventory_path, devices, error) ||
	   !SelectAmpDevice(devices, device_index, selected, error)) {
		COUTLOG(Format("amp_atlas_runtime=fail device=%s", ~error));
		return 1;
	}
	try {
		concurrency::accelerator device(selected.device_path.ToWString().ToStd());
		concurrency::array_view<int, 1> view(pixels.GetCount(), pixels.Begin());
		Vector<int> match_pixels;
		match_pixels.Append(pixels);
		COUTLOG(Format("amp_atlas_upload=launch device=%s pixels=%d",
		               ~selected.description, pixels.GetCount()));
		concurrency::parallel_for_each(device.default_view, view.extent,
		                               [=](concurrency::index<1> index) PARALLEL_AMP {
			view[index] = view[index] + (index[0] & 7);
		});
		view.synchronize();
		int checksum = 0;
		for(int value : pixels)
			checksum += value;
		int failed = 0;
		if(exact_selftest) {
			Vector<int> scores;
			scores.SetCount(manifest.entries.GetCount(), 0);
			concurrency::array_view<int, 1> full(atlas_pixels.GetCount(), atlas_pixels.Begin());
			concurrency::array_view<int, 1> compact(match_pixels.GetCount(), match_pixels.Begin());
			concurrency::array_view<int, 1> score_view(scores.GetCount(), scores.Begin());
			concurrency::array_view<int, 1> offset_view(offsets.GetCount(), offsets.Begin());
			concurrency::array_view<int, 1> width_view(widths.GetCount(), widths.Begin());
			concurrency::array_view<int, 1> height_view(heights.GetCount(), heights.Begin());
			concurrency::array_view<int, 1> x_view(xs.GetCount(), xs.Begin());
			concurrency::array_view<int, 1> y_view(ys.GetCount(), ys.Begin());
			int atlas_width = atlas.GetWidth();
			parallel_for_each(device.default_view, score_view.extent,
				[=](concurrency::index<1> index) PARALLEL_AMP {
					int id = index[0];
					int score = 0;
					for(int y = 0; y < height_view[id]; y++)
						for(int x = 0; x < width_view[id]; x++) {
							int a = full[(y_view[id] + y) * atlas_width + x_view[id] + x];
							int b = compact[offset_view[id] + y * width_view[id] + x];
							int d = a - b;
							score += d < 0 ? -d : d;
						}
					score_view[id] = score;
				});
			score_view.synchronize();
			for(int i = 0; i < scores.GetCount(); i++) {
				if(scores[i] != 0) {
					failed++;
					COUTLOG(Format("amp_atlas_selftest_fail id=%s score=%d",
					               ~manifest.entries[i].id, scores[i]));
				}
			}
		}
		int64 total_ms = msecs() - started;
		COUTLOG(Format("amp_atlas_runtime=pass backend=native-amp entries=%d nonempty=%d pixels=%d "
		               "checksum=%d selftest=%d selftest_failed=%d prepare_ms=%d total_ms=%d",
		               manifest.entries.GetCount(), nonempty, pixels.GetCount(), checksum,
		               exact_selftest, failed, prepare_ms, total_ms));
		return checksum > 0 && failed == 0 ? 0 : 1;
	}
	catch(const std::exception& e) {
		COUTLOG(Format("amp_atlas_runtime=native-failure error=%s", e.what()));
		return 1;
	}
#endif
}

END_UPP_NAMESPACE
