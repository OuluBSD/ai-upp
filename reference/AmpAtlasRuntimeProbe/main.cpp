#include "AmpAtlasRuntimeProbe.h"
#include <VisualStateModel/VisualStateModel.h>

CONSOLE_APP_MAIN {
	Upp::RunAmpAtlasRuntimeProbe();
}

NAMESPACE_UPP

static void FinalizeAmpFixture(AmpTemplatePixelBuffer& pixels)
{
	pixels.otsu_threshold = AmpOtsuThreshold(pixels.gray);
	pixels.otsu.SetCount(pixels.gray.GetCount());
	for(int i = 0; i < pixels.gray.GetCount(); i++)
		pixels.otsu[i] = pixels.gray[i] > pixels.otsu_threshold ? 255 : 0;
}

static bool ReadArguments(String& atlas_path, String& manifest_path,
                       String& inventory_path, int& device_index, bool& exact_selftest,
                       bool& frame_selftest, bool& pixel_selftest, bool& dual_selftest,
                       int& threshold, String& frame_report_path,
                       String& frame_path, String& match_scale, String& match_kind)
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
		else if(args[i] == "--frame")
			frame_path = args[i + 1];
		else if(args[i] == "--amp-match-scale")
			match_scale = args[i + 1];
		else if(args[i] == "--amp-match-kind")
			match_kind = args[i + 1];
	}
	for(const String& arg : args)
		exact_selftest |= arg == "--amp-atlas-selftest";
	for(const String& arg : args)
		frame_selftest |= arg == "--amp-frame-selftest";
	for(const String& arg : args)
		pixel_selftest |= arg == "--amp-pixel-selftest";
	for(const String& arg : args)
		dual_selftest |= arg == "--amp-dual-selftest";
	frame_selftest |= !frame_path.IsEmpty();
	return pixel_selftest || dual_selftest || frame_selftest ||
	       (!atlas_path.IsEmpty() && !manifest_path.IsEmpty());
}

static Image MakeGrayDiagnostic(const Vector<int>& values, int width, int height)
{
	ImageBuffer buffer(width, height);
	for(int y = 0; y < height; y++)
		for(int x = 0; x < width; x++) {
			int value = clamp(values[y * width + x], 0, 255);
			RGBA pixel;
			pixel.r = pixel.g = pixel.b = (byte)value;
			pixel.a = 255;
			buffer[y][x] = pixel;
		}
	return Image(buffer);
}

static int RunLocalOtsuSelftest(const String& report_path)
{
	Vector<int> gray;
	gray << 0 << 0 << 30 << 200 << 200;
	Vector<int> triangular, gaussian;
	String error;
	if(!BuildAmpLocalOtsu(gray, 5, 1, 1, false, triangular, error) ||
	   !BuildAmpLocalOtsu(gray, 5, 1, 1, true, gaussian, error)) {
		COUTLOG(Format("amp_local_otsu_selftest=fail error=%s", ~error));
		return 1;
	}
	bool shape = triangular.GetCount() == gray.GetCount() &&
	             gaussian.GetCount() == gray.GetCount();
	int gray_checksum = 0, triangular_checksum = 0, gaussian_checksum = 0;
	for(int value : gray) gray_checksum += value;
	for(int value : triangular) triangular_checksum += value;
	for(int value : gaussian) gaussian_checksum += value;
	COUTLOG(Format("amp_local_otsu_selftest=%s radius=1 variants=triangular,gaussian "
	               "gray_checksum=%d triangular_checksum=%d gaussian_checksum=%d",
               shape ? "pass" : "fail", gray_checksum, triangular_checksum,
		               gaussian_checksum));
	if(!report_path.IsEmpty()) {
		String folder = GetFileFolder(report_path);
		RealizeDirectory(folder);
		String gray_name = "gray.png", triangular_name = "triangular.png", gaussian_name = "gaussian.png";
		bool saved = PNGEncoder().SaveFile(AppendFileName(folder, gray_name), MakeGrayDiagnostic(gray, 5, 1)) &&
		             PNGEncoder().SaveFile(AppendFileName(folder, triangular_name), MakeGrayDiagnostic(triangular, 5, 1)) &&
		             PNGEncoder().SaveFile(AppendFileName(folder, gaussian_name), MakeGrayDiagnostic(gaussian, 5, 1));
		String html = "<!doctype html><meta charset=\"utf-8\"><title>AMP local Otsu evidence</title>\n"
		              "<h1>Local Otsu evidence</h1><p>Each output pixel uses its own crop-local weighted window; no atlas-wide histogram is used.</p>\n";
		html << "<p><img src=\"" << gray_name << "\" alt=\"gray input\"></p>"
		      << "<p><img src=\"" << triangular_name << "\" alt=\"triangular local Otsu\"></p>"
		      << "<p><img src=\"" << gaussian_name << "\" alt=\"Gaussian local Otsu\"></p>"
		      << "<p>radius=1 status=" << (saved && shape ? "pass" : "fail") << "</p>\n";
		SaveFile(report_path, html);
		COUTLOG(Format("amp_local_otsu_report=%s path=%s", saved && shape ? "pass" : "fail", ~report_path));
	}
	return shape ? 0 : 1;
}

static bool RunDualSelftest(const String& device_path, String& error)
{
	AmpTemplatePixelBuffer frame, atlas;
	frame.width = 2;
	frame.height = 1;
	frame.rgb.Add(0xFF8000);
	frame.rgb.Add(0x0000FF);
	frame.gray.Add(AmpRgbGray(frame.rgb[0]));
	frame.gray.Add(AmpRgbGray(frame.rgb[1]));
	atlas.width = 1;
	atlas.height = 1;
	atlas.rgb.Add(frame.rgb[1]);
	atlas.gray.Add(frame.gray[1]);
	FinalizeAmpFixture(frame);
	FinalizeAmpFixture(atlas);
	AmpTemplateAtlasManifest manifest;
	manifest.atlas_width = 1;
	manifest.atlas_height = 1;
	AmpTemplateAtlasEntry& entry = manifest.entries.Add();
	entry.id = "dual-fixture";
	entry.preprocessing = "color";
	entry.width = entry.height = 1;
	AmpTemplateMatchResult cpu, amp;
	if(!MatchAmpTemplatePixelsCpu(frame, atlas, manifest, 0, cpu, error) ||
	   !MatchAmpTemplatePixelsAmp(frame, atlas, manifest, 0, device_path, amp, error))
		return false;
	bool equal = cpu.winner_index == amp.winner_index &&
	             cpu.winner_score == amp.winner_score &&
	             cpu.entries.GetCount() == amp.entries.GetCount() &&
	             cpu.entries[0].x == amp.entries[0].x &&
	             cpu.entries[0].score == amp.entries[0].score;
	entry.preprocessing = "otsu";
	AmpTemplateMatchResult otsu_cpu, otsu_amp;
	bool otsu_equal = MatchAmpTemplatePixelsCpu(frame, atlas, manifest, 0,
	                                             otsu_cpu, error) &&
	                  MatchAmpTemplatePixelsAmp(frame, atlas, manifest, 0,
	                                             device_path, otsu_amp, error) &&
	                  otsu_cpu.winner_index == otsu_amp.winner_index &&
	                  otsu_cpu.winner_score == otsu_amp.winner_score &&
	                  otsu_cpu.entries[0].score == 0;
	equal &= otsu_equal;
	AmpTemplatePixelBuffer crop_frame, crop_atlas;
	crop_frame.width = 2;
	crop_frame.height = crop_atlas.height = 1;
	crop_atlas.width = 22;
	for(int value : {20, 200}) {
		crop_frame.gray.Add(value);
		crop_frame.rgb.Add(value | value << 8 | value << 16);
	}
	for(int i = 0; i < crop_atlas.width; i++) {
		int value = i == 20 ? 20 : i == 21 ? 200 : 0;
		crop_atlas.gray.Add(value);
		crop_atlas.rgb.Add(value | value << 8 | value << 16);
	}
	FinalizeAmpFixture(crop_frame);
	FinalizeAmpFixture(crop_atlas);
	AmpTemplateAtlasManifest crop_manifest;
	crop_manifest.atlas_width = crop_atlas.width;
	crop_manifest.atlas_height = crop_atlas.height;
	AmpTemplateAtlasEntry& crop_entry = crop_manifest.entries.Add();
	crop_entry.id = "otsu-black-canvas-fixture";
	crop_entry.preprocessing = "otsu";
	crop_entry.x = 20;
	crop_entry.width = 2;
	crop_entry.height = 1;
	AmpTemplateMatchResult crop_cpu, crop_amp;
	int crop_threshold = AmpOtsuThresholdCrop(crop_atlas.gray, crop_atlas.width,
	                                          crop_entry.x, crop_entry.y,
	                                          crop_entry.width, crop_entry.height);
	bool crop_equal = crop_threshold == 20 &&
		MatchAmpTemplatePixelsCpu(crop_frame, crop_atlas, crop_manifest, 0,
		                          crop_cpu, error) &&
		MatchAmpTemplatePixelsAmp(crop_frame, crop_atlas, crop_manifest, 0,
		                          device_path, crop_amp, error) &&
		crop_cpu.entries[0].score == 0 && crop_amp.entries[0].score == 0 &&
		crop_cpu.entries[0].x == crop_amp.entries[0].x;
	equal &= crop_equal;
	AmpTemplatePixelBuffer malformed;
	malformed.width = frame.width;
	malformed.height = frame.height;
	malformed.rgb.Append(frame.rgb);
	AmpTemplateMatchResult ignored;
	String malformed_error;
	bool malformed_rejected = !MatchAmpTemplatePixelsCpu(malformed, atlas, manifest,
	                                                     0, ignored, malformed_error);
	equal &= malformed_rejected;
	COUTLOG(Format("amp_dual_selftest=%s backend=%s cpu_score=%d amp_score=%d otsu=%s otsu_threshold=%d crop_canvas=%s crop_threshold=%d x=%d malformed=%s",
	               equal ? "pass" : "fail", device_path.IsEmpty() ? "compat-cpu" : "native-amp-kernel",
	               cpu.entries[0].score, amp.entries[0].score,
	               otsu_equal ? "pass" : "fail", frame.otsu_threshold,
	               crop_equal ? "pass" : "fail", crop_threshold,
	               amp.entries[0].x,
	               malformed_rejected ? "rejected" : "accepted"));
	return equal;
}

static int RunPixelSelftest()
{
	ImageBuffer buffer(2, 1);
	buffer[0][0].r = 255;
	buffer[0][0].g = 0;
	buffer[0][0].b = 0;
	buffer[0][0].a = 255;
	buffer[0][1].r = 0;
	buffer[0][1].g = 128;
	buffer[0][1].b = 255;
	buffer[0][1].a = 255;
	Image test_image(buffer);
	AmpTemplatePixelBuffer pixels;
	String error;
	if(!BuildAmpPixelBuffer(test_image, pixels, error)) {
		COUTLOG(Format("amp_pixel_selftest=fail error=%s", ~error));
		return 1;
	}
	AmpTemplatePreprocessing mode;
	bool valid = pixels.IsValid() && pixels.rgb[0] == 255 &&
	             AmpRgbRed(pixels.rgb[1]) == 0 &&
	             AmpRgbGreen(pixels.rgb[1]) == 128 &&
	             AmpRgbBlue(pixels.rgb[1]) == 255 &&
	             pixels.gray[0] == AmpRgbGray(pixels.rgb[0]) &&
	             pixels.gray[1] == AmpRgbGray(pixels.rgb[1]) &&
	             ParseAmpTemplatePreprocessing("RGB", mode, error) &&
	             mode == AMP_TEMPLATE_RGB &&
	             ParseAmpTemplatePreprocessing("grayscale", mode, error) &&
	             mode == AMP_TEMPLATE_GRAY &&
	             ParseAmpTemplatePreprocessing("otsu", mode, error) &&
	             mode == AMP_TEMPLATE_OTSU;
	String invalid_error;
	valid &= !ParseAmpTemplatePreprocessing("unsupported", mode, invalid_error) &&
	          !invalid_error.IsEmpty();
	AmpTemplatePixelBuffer frame;
	frame.width = 2;
	frame.height = 1;
	frame.rgb.Add(PackAmpRgb(test_image[0][1]));
	frame.rgb.Add(PackAmpRgb(test_image[0][0]));
	frame.gray.Add(AmpRgbGray(frame.rgb[0]));
	frame.gray.Add(AmpRgbGray(frame.rgb[1]));
	AmpTemplatePixelBuffer atlas;
	atlas.width = 1;
	atlas.height = 1;
	atlas.rgb.Add(frame.rgb[1]);
	atlas.gray.Add(frame.gray[1]);
	FinalizeAmpFixture(frame);
	FinalizeAmpFixture(atlas);
	AmpTemplateAtlasManifest manifest;
	manifest.atlas_width = 1;
	manifest.atlas_height = 1;
	AmpTemplateAtlasEntry& entry = manifest.entries.Add();
	entry.id = "rgb-fixture";
	entry.preprocessing = "rgb";
	entry.width = entry.height = 1;
	AmpTemplateMatchResult match;
	String match_error;
	valid &= MatchAmpTemplatePixelsCpu(frame, atlas, manifest, 0, match,
	                                   match_error) && match.winner_index == 0 &&
	          match.entries.GetCount() == 1 && match.entries[0].x == 1 &&
	          match.entries[0].score == 0;
	COUTLOG(Format("amp_pixel_selftest=%s size=%d`x%d rgb_checksum=%d gray_checksum=%d",
	               valid ? "pass" : "fail", pixels.width, pixels.height,
	               pixels.rgb[0] + pixels.rgb[1], pixels.gray[0] + pixels.gray[1]));
	return valid ? 0 : 1;
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

static bool FilterManifest(const AmpTemplateAtlasManifest& source,
	                       const String& scale, const String& kind,
	                       AmpTemplateAtlasManifest& filtered, String& error)
{
	if(!scale.IsEmpty() && scale != "board" && scale != "hand") {
		error = "match scale must be board or hand";
		return false;
	}
	if(!kind.IsEmpty() && kind != "rank" && kind != "suit") {
		error = "match kind must be rank or suit";
		return false;
	}
	filtered.atlas_name = source.atlas_name;
	filtered.atlas_width = source.atlas_width;
	filtered.atlas_height = source.atlas_height;
	filtered.entries.Clear();
	for(const AmpTemplateAtlasEntry& entry : source.entries)
		if((scale.IsEmpty() || entry.scale == scale) &&
		   (kind.IsEmpty() || entry.kind == kind))
			filtered.entries.Add(entry);
	if(filtered.entries.IsEmpty()) {
		error = "match scope has no entries";
		return false;
	}
	return true;
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

static Image MakeRgbImage(const Vector<int>& pixels, int width, int height)
{
	ImageBuffer buffer(width, height);
	for(int y = 0; y < height; y++) {
		RGBA* row = buffer[y];
		for(int x = 0; x < width; x++) {
			int value = pixels[y * width + x];
			row[x].r = AmpRgbRed(value);
			row[x].g = AmpRgbGreen(value);
			row[x].b = AmpRgbBlue(value);
			row[x].a = 255;
		}
	}
	return Image(buffer);
}

static Image MakePerEntryOtsuAtlasImage(const AmpTemplatePixelBuffer& atlas,
	                                    const AmpTemplateAtlasManifest& manifest)
{
	ImageBuffer buffer(atlas.width, atlas.height);
	for(int y = 0; y < atlas.height; y++)
		for(int x = 0; x < atlas.width; x++) {
			buffer[y][x].r = buffer[y][x].g = buffer[y][x].b = 0;
			buffer[y][x].a = 255;
		}
	for(const AmpTemplateAtlasEntry& entry : manifest.entries) {
		int threshold = AmpOtsuThresholdCrop(atlas.gray, atlas.width,
		                                    entry.x, entry.y,
		                                    entry.width, entry.height);
		for(int y = 0; y < entry.height; y++)
			for(int x = 0; x < entry.width; x++) {
				byte value = atlas.gray[(entry.y + y) * atlas.width + entry.x + x] > threshold
			           ? 255 : 0;
				buffer[entry.y + y][entry.x + x].r = value;
				buffer[entry.y + y][entry.x + x].g = value;
				buffer[entry.y + y][entry.x + x].b = value;
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

static bool SaveFrameEvidence(const String& report_path,
	                           const AmpTemplatePixelBuffer& frame,
	                           const AmpTemplatePixelBuffer& atlas,
	                           const AmpTemplateAtlasManifest& manifest,
	                           const AmpTemplateAtlasManifest& evidence_manifest,
                           const AmpTemplateMatchResult& result,
                           const String& backend, int threshold,
                           const String& frame_source, const String& scope,
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
	String input_name = "frame_input_rgb.png";
	String gray_name = "frame_input_gray.png";
	String otsu_name = "frame_input_otsu.png";
	String atlas_name = "atlas_input_rgb.png";
	String atlas_gray_name = "atlas_input_gray.png";
	String atlas_otsu_name = "atlas_input_otsu.png";
	String overlay_name = "frame_matches.png";
	String input_path = AppendFileName(folder, input_name);
	String overlay_path = AppendFileName(folder, overlay_name);
	Image input = MakeRgbImage(frame.rgb, frame.width, frame.height);
	Image gray = MakeGrayImage(frame.gray, frame.width, frame.height);
	Image otsu = MakeGrayImage(frame.otsu, frame.width, frame.height);
	Image atlas_input = MakeRgbImage(atlas.rgb, atlas.width, atlas.height);
	Image atlas_gray_input = MakeGrayImage(atlas.gray, atlas.width, atlas.height);
	Image atlas_otsu_input = MakePerEntryOtsuAtlasImage(atlas, evidence_manifest);
	if(!PNGEncoder().SaveFile(input_path, input) ||
	   !PNGEncoder().SaveFile(AppendFileName(folder, gray_name), gray) ||
	   !PNGEncoder().SaveFile(AppendFileName(folder, otsu_name), otsu) ||
	   !PNGEncoder().SaveFile(AppendFileName(folder, atlas_name), atlas_input) ||
	   !PNGEncoder().SaveFile(AppendFileName(folder, atlas_gray_name), atlas_gray_input) ||
	   !PNGEncoder().SaveFile(AppendFileName(folder, atlas_otsu_name), atlas_otsu_input))
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
	      << " reference_match=" << (equal ? "pass" : "fail")
	      << " source=" << EscapeHtml(frame_source)
	      << " scope=" << EscapeHtml(scope) << "</p>\n"
	      << "<p>atlas_evidence_entries=" << evidence_manifest.entries.GetCount()
	      << " match_scope_entries=" << manifest.entries.GetCount() << "</p>\n"
	      << "<p><img src=\"" << input_name << "\" alt=\"RGB input frame\"></p>\n"
	      << "<p><img src=\"" << gray_name << "\" alt=\"grayscale input frame\"></p>\n"
	      << "<p><img src=\"" << otsu_name << "\" alt=\"global diagnostic Otsu frame\"></p>\n"
	      << "<h2>Atlas inputs</h2>\n"
	      << "<p><img src=\"" << atlas_name << "\" alt=\"RGB atlas input\"></p>\n"
	      << "<p><img src=\"" << atlas_gray_name << "\" alt=\"grayscale atlas input\"></p>\n"
	      << "<p><img src=\"" << atlas_otsu_name << "\" alt=\"per-entry Otsu atlas input\"></p>\n"
	      << "<p><img src=\"" << overlay_name << "\" alt=\"match overlay\"></p>\n"
	      << "<table><tr><th>id</th><th>preprocessing</th><th>otsu_threshold</th><th>accepted</th>"
	      << "<th>score</th><th>x</th><th>y</th></tr>\n";
	for(const AmpTemplateMatchHit& hit : result.entries) {
		html << "<tr><td>" << EscapeHtml(hit.id) << "</td><td>"
		      << EscapeHtml(manifest.entries[hit.entry_index].preprocessing) << "</td><td>";
		AmpTemplatePreprocessing mode;
		String mode_error;
		if(ParseAmpTemplatePreprocessing(manifest.entries[hit.entry_index].preprocessing,
		                                  mode, mode_error) && mode == AMP_TEMPLATE_OTSU)
			html << AmpOtsuThresholdCrop(atlas.gray, atlas.width,
		                              manifest.entries[hit.entry_index].x,
		                              manifest.entries[hit.entry_index].y,
		                              manifest.entries[hit.entry_index].width,
		                              manifest.entries[hit.entry_index].height);
		else
			html << "n/a";
		html << "</td><td>" << (hit.accepted ? "yes" : "no") << "</td><td>" << hit.score
		      << "</td><td>" << hit.x << "</td><td>" << hit.y << "</td></tr>\n";
	}
	int64 report_elapsed_ms = msecs() - started;
	if(report_elapsed_ms < 1)
		report_elapsed_ms = 1;
	html << "</table><p>candidates=" << result.candidate_count
	      << " accepted=" << result.accepted << " rejected=" << result.rejected
	      << " checksum=" << result.checksum << "</p>\n"
	      << "<p>cpu_ms=" << cpu_ms << " amp_ms=" << amp_ms
	      << " report_ms=" << report_elapsed_ms << "</p>\n";
	int rgb_count = 0, gray_count = 0, otsu_count = 0;
	for(const AmpTemplateAtlasEntry& entry : manifest.entries) {
		AmpTemplatePreprocessing mode;
		String mode_error;
		if(ParseAmpTemplatePreprocessing(entry.preprocessing, mode, mode_error)) {
			if(mode == AMP_TEMPLATE_RGB) rgb_count++;
			else if(mode == AMP_TEMPLATE_OTSU) otsu_count++;
			else gray_count++;
		}
	}
	html << "<p>preprocessing_rgb=" << rgb_count << " preprocessing_gray=" << gray_count
	     << " preprocessing_otsu=" << otsu_count
	     << " otsu_available=1 production_manifest_otsu=" << (otsu_count ? 1 : 0)
	     << "</p>\n";
	html << "<p>frame_otsu_threshold=" << frame.otsu_threshold
	     << " (global diagnostic only); atlas_otsu_threshold=per-entry crop</p>\n";
	if(!SaveFile(report_path, html))
		return false;
	report_ms = msecs() - started;
	if(report_ms < 1)
		report_ms = 1;
	return true;
}

static bool RunFrameSelftest(const Vector<int>& atlas_pixels,
	                           const Image& atlas_image,
	                           const AmpTemplateAtlasManifest& manifest,
	                           const AmpTemplateAtlasManifest& evidence_manifest,
	                           int threshold, const String& inventory_path,
	                           int device_index, const String& report_path,
                           const Image& input_image, const String& frame_source,
                           const String& scope)
{
	if(manifest.entries.GetCount() < 2)
		return false;
	const AmpTemplateAtlasEntry& first = manifest.entries[0];
	const AmpTemplateAtlasEntry& second = manifest.entries[1];
	String error;
	int frame_width = input_image.IsEmpty() ? first.width + second.width + 4 : input_image.GetWidth();
	int frame_height = input_image.IsEmpty() ? max(first.height, second.height) + 2 : input_image.GetHeight();
	Image synthetic_frame;
	if(input_image.IsEmpty()) {
		ImageBuffer synthetic_buffer(frame_width, frame_height);
		for(int y = 0; y < first.height; y++)
			for(int x = 0; x < first.width; x++)
				synthetic_buffer[y + 1][x + 1] =
					atlas_image[first.y + y][first.x + x];
		for(int y = 0; y < second.height; y++)
			for(int x = 0; x < second.width; x++)
				synthetic_buffer[y + 1][x + first.width + 2] =
					atlas_image[second.y + y][second.x + x];
		synthetic_frame = Image(synthetic_buffer);
	}
	AmpTemplatePixelBuffer frame_pixels, atlas_pixels_dual;
	if(!BuildAmpPixelBuffer(input_image.IsEmpty() ? synthetic_frame : input_image,
	                        frame_pixels, error) ||
	   !BuildAmpPixelBuffer(atlas_image, atlas_pixels_dual, error)) {
		COUTLOG(Format("amp_frame_input=fail error=%s", ~error));
		return false;
	}
	AmpTemplateMatchResult result, reference;
	int64 cpu_started = msecs();
	if(!MatchAmpTemplatePixelsCpu(frame_pixels, atlas_pixels_dual, manifest,
	                              threshold, reference, error)) {
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
	if(!MatchAmpTemplatePixelsAmp(frame_pixels, atlas_pixels_dual, manifest,
	                              threshold, device_path, result, error)) {
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
	if(!SaveFrameEvidence(report_path, frame_pixels, atlas_pixels_dual, manifest,
	                      evidence_manifest,
	                      result, backend, threshold, frame_source,
	                      scope,
                      cpu_ms, amp_ms, equal, report_ms)) {
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
	if(!input_image.IsEmpty())
		return equal;
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

static int RunRealFrameRegression(const String& atlas_path, const String& manifest_path,
                                  const String& frame_list_path, const String& report_path,
                                  const String& inventory_path, int device_index,
                                  int threshold)
{
	AmpTemplateAtlasManifest manifest;
	String error;
	if(!manifest.Load(manifest_path, error)) {
		COUTLOG(Format("amp_real_frame_regression=fail manifest=%s", ~error));
		return 1;
	}
	Image atlas = StreamRaster::LoadFileAny(atlas_path);
	if(atlas.IsEmpty()) {
		COUTLOG(Format("amp_real_frame_regression=fail atlas=%s", ~atlas_path));
		return 1;
	}
	Vector<String> frame_paths;
	for(const String& line : Split(LoadFile(frame_list_path), "\n", false)) {
		String path = TrimBoth(line);
		if(!path.IsEmpty() && path[0] != '#') frame_paths.Add(path);
	}
	if(frame_paths.IsEmpty()) {
		COUTLOG("amp_real_frame_regression=fail reason=empty-frame-list");
		return 1;
	}
	String device_path;
#ifdef HAVE_SYSTEM_AMP
	Vector<AmpDeviceInfo> devices;
	AmpDeviceInfo selected;
	if(!LoadAmpDeviceInventory(inventory_path, devices, error) ||
	   !SelectAmpDevice(devices, device_index, selected, error)) {
		COUTLOG(Format("amp_real_frame_regression=fail device=%s", ~error));
		return 1;
	}
	device_path = selected.device_path;
#endif
	RealizeDirectory(report_path);
	String html = "<!doctype html><meta charset=\"utf-8\"><title>AMP real-frame regression</title>\n";
	html << "<h1>AMP real-frame grayscale regression</h1><p>atlas="
	      << EscapeHtml(atlas_path) << " manifest=" << EscapeHtml(manifest_path)
	      << "</p><table><tr><th>frame</th><th>cpu/amp parity</th><th>cpu_ms</th><th>amp_ms</th><th>report</th></tr>\n";
	int failed = 0;
	for(int i = 0; i < frame_paths.GetCount(); i++) {
		Image frame_image = StreamRaster::LoadFileAny(frame_paths[i]);
		String item_dir = AppendFileName(report_path, Format("frame_%04d", i));
		String item_report = AppendFileName(item_dir, "report.htm");
		if(frame_image.IsEmpty()) {
			failed++;
			html << "<tr><td>" << EscapeHtml(frame_paths[i]) << "</td><td>decode-fail</td><td>-</td><td>-</td><td>-</td></tr>\n";
			continue;
		}
		AmpTemplatePixelBuffer frame_pixels, atlas_pixels;
		if(!BuildAmpPixelBuffer(frame_image, frame_pixels, error) ||
		   !BuildAmpPixelBuffer(atlas, atlas_pixels, error)) {
			failed++;
			html << "<tr><td>" << EscapeHtml(frame_paths[i]) << "</td><td>prepare-fail</td><td>-</td><td>-</td><td>-</td></tr>\n";
			continue;
		}
		AmpTemplateMatchResult cpu, amp;
		int64 cpu_started = msecs();
		bool ok = MatchAmpTemplatePixelsCpu(frame_pixels, atlas_pixels, manifest,
		                                    threshold, cpu, error);
		int64 cpu_ms = msecs() - cpu_started;
		int64 amp_started = msecs();
		ok = ok && MatchAmpTemplatePixelsAmp(frame_pixels, atlas_pixels, manifest,
		                                    threshold, device_path, amp, error);
		int64 amp_ms = msecs() - amp_started;
		bool equal = ok && cpu.entries.GetCount() == amp.entries.GetCount() &&
		             cpu.winner_index == amp.winner_index &&
		             cpu.winner_score == amp.winner_score;
		if(equal)
			for(int j = 0; j < cpu.entries.GetCount(); j++)
				equal &= cpu.entries[j].id == amp.entries[j].id &&
				          cpu.entries[j].score == amp.entries[j].score &&
				          cpu.entries[j].x == amp.entries[j].x &&
				          cpu.entries[j].y == amp.entries[j].y;
		if(!equal) failed++;
		int64 report_ms = 0;
		if(ok && !SaveFrameEvidence(item_report, frame_pixels, atlas_pixels, manifest,
		                            manifest, amp, "native-or-compat", threshold,
		                            frame_paths[i], "all", cpu_ms, amp_ms, equal,
		                            report_ms)) {
			failed++;
			equal = false;
		}
		html << "<tr><td>" << EscapeHtml(frame_paths[i]) << "</td><td>"
		      << (equal ? "pass" : "fail") << "</td><td>" << cpu_ms << "</td><td>"
		      << amp_ms << "</td><td><a href=\"frame_" << Format("%04d", i)
		      << "/report.htm\">evidence</a></td></tr>\n";
		COUTLOG(Format("amp_real_frame frame=%d/%d path=%s parity=%s cpu_ms=%d amp_ms=%d",
		               i + 1, frame_paths.GetCount(), ~frame_paths[i],
		               equal ? "pass" : "fail", cpu_ms, amp_ms));
	}
	html << "</table><p>frames=" << frame_paths.GetCount() << " failed=" << failed
	      << " status=" << (failed ? "fail" : "pass") << "</p>\n";
	SaveFile(AppendFileName(report_path, "index.htm"), html);
	COUTLOG(Format("amp_real_frame_regression=%s frames=%d failed=%d report=%s",
	               failed ? "fail" : "pass", frame_paths.GetCount(), failed,
	               ~AppendFileName(report_path, "index.htm")));
	return failed ? 1 : 0;
}

static Image AmpImageFromVsm(const VsmImageBuffer& source)
{
	if(source.IsEmpty() || source.channels < 3)
		return Image();
	ImageBuffer output(Size(source.width, source.height));
	for(int y = 0; y < source.height; y++) {
		RGBA *row = output[y];
		for(int x = 0; x < source.width; x++) {
			row[x].r = source.Get(x, y, 0);
			row[x].g = source.Get(x, y, 1);
			row[x].b = source.Get(x, y, 2);
			row[x].a = source.channels > 3 ? source.Get(x, y, 3) : 255;
		}
	}
	return output;
}

static int RunRealVideoRegression(const String& atlas_path, const String& manifest_path,
                                  const String& video_path, const String& times_path,
                                  const String& report_path, const String& inventory_path,
                                  int device_index, int threshold,
                                  const String& focus_rect_spec)
{
	Vector<String> times;
	for(const String& line : Split(LoadFile(times_path), "\n", false)) {
		String value = TrimBoth(line);
		if(!value.IsEmpty() && value[0] != '#') times.Add(value);
	}
	if(times.IsEmpty()) {
		COUTLOG("amp_real_video_regression=fail reason=empty-time-list");
		return 1;
	}
	String error;
	AmpTemplateAtlasManifest manifest;
	if(!manifest.Load(manifest_path, error)) {
		COUTLOG(Format("amp_real_video_regression=fail manifest=%s", ~error));
		return 1;
	}
	Image atlas = StreamRaster::LoadFileAny(atlas_path);
	if(atlas.IsEmpty()) {
		COUTLOG(Format("amp_real_video_regression=fail atlas=%s", ~atlas_path));
		return 1;
	}
	String device_path;
#ifdef HAVE_SYSTEM_AMP
	Vector<AmpDeviceInfo> devices;
	AmpDeviceInfo selected;
	if(!LoadAmpDeviceInventory(inventory_path, devices, error) ||
	   !SelectAmpDevice(devices, device_index, selected, error)) {
		COUTLOG(Format("amp_real_video_regression=fail device=%s", ~error));
		return 1;
	}
	device_path = selected.device_path;
#endif
	VsmVideoFileFrameSource source;
	if(!source.Open(video_path)) {
		COUTLOG(Format("amp_real_video_regression=fail decode=%s", ~source.GetLastError()));
		return 1;
	}
	RealizeDirectory(report_path);
	String html = "<!doctype html><meta charset=\"utf-8\"><title>AMP direct video regression</title>\n";
	html << "<h1>AMP direct libavcodec frame regression</h1><p>video="
	      << EscapeHtml(video_path) << " atlas=" << EscapeHtml(atlas_path)
	      << " manifest=" << EscapeHtml(manifest_path)
	      << "</p><table><tr><th>requested_ms</th><th>window</th><th>decoded_ms</th><th>parity</th><th>evidence</th></tr>\n";
	int failed = 0;
	for(int i = 0; i < times.GetCount(); i++) {
		Vector<String> spec = Split(times[i], " ", false);
		int64 requested_ms = StrInt64(spec[0]);
		String window_id = spec.GetCount() > 1 ? spec[1] : "full";
		String item_dir = AppendFileName(report_path, Format("frame_%04d", i));
		String item_report = AppendFileName(item_dir, "report.htm");
		if(!source.SeekMs(requested_ms)) {
			failed++;
			html << "<tr><td>" << requested_ms << "</td><td>-</td><td>seek-fail</td><td>-</td></tr>\n";
			continue;
		}
		VsmImageBuffer decoded;
		int64 decoded_ms = 0;
		if(!source.ReadFrame(decoded, decoded_ms)) {
			failed++;
			html << "<tr><td>" << requested_ms << "</td><td>-</td><td>decode-fail</td><td>-</td></tr>\n";
			continue;
		}
		Image frame = AmpImageFromVsm(decoded);
		if(window_id == "L" || window_id == "R") {
			int half_width = frame.GetWidth() / 2;
			int table_height = min(frame.GetHeight(), 682);
			Rect window_rect = window_id == "L"
			                 ? Rect(0, 0, half_width, table_height)
			                 : Rect(half_width, 0, frame.GetWidth(), table_height);
			frame = Crop(frame, window_rect);
		}
		if(!focus_rect_spec.IsEmpty()) {
			Vector<String> values = Split(focus_rect_spec, ",", false);
			if(values.GetCount() != 4) {
				COUTLOG("amp_real_video_regression=fail reason=invalid-focus-rect");
				return 1;
			}
			Rect focus(StrInt(values[0]), StrInt(values[1]),
			           StrInt(values[0]) + StrInt(values[2]),
			           StrInt(values[1]) + StrInt(values[3]));
			focus &= Rect(0, 0, frame.GetWidth(), frame.GetHeight());
			if(focus.IsEmpty()) {
				COUTLOG("amp_real_video_regression=fail reason=empty-focus-rect");
				return 1;
			}
			frame = Crop(frame, focus);
		}
		AmpTemplatePixelBuffer frame_pixels, atlas_pixels;
		if(frame.IsEmpty() || !BuildAmpPixelBuffer(frame, frame_pixels, error) ||
		   !BuildAmpPixelBuffer(atlas, atlas_pixels, error)) {
			failed++;
			html << "<tr><td>" << requested_ms << "</td><td>" << decoded_ms << "</td><td>prepare-fail</td><td>-</td></tr>\n";
			continue;
		}
		AmpTemplateMatchResult cpu, amp;
		int64 cpu_started = msecs();
		bool ok = MatchAmpTemplatePixelsCpu(frame_pixels, atlas_pixels, manifest,
		                                    threshold, cpu, error);
		int64 cpu_ms = msecs() - cpu_started;
		int64 amp_started = msecs();
		ok = ok && MatchAmpTemplatePixelsAmp(frame_pixels, atlas_pixels, manifest,
		                                    threshold, device_path, amp, error);
		int64 amp_ms = msecs() - amp_started;
		bool equal = ok && cpu.entries.GetCount() == amp.entries.GetCount() &&
		             cpu.winner_index == amp.winner_index && cpu.winner_score == amp.winner_score;
		if(equal)
			for(int j = 0; j < cpu.entries.GetCount(); j++)
				equal &= cpu.entries[j].id == amp.entries[j].id &&
				          cpu.entries[j].score == amp.entries[j].score &&
				          cpu.entries[j].x == amp.entries[j].x && cpu.entries[j].y == amp.entries[j].y;
		if(!equal) failed++;
		int64 report_ms = 0;
		if(ok && !SaveFrameEvidence(item_report, frame_pixels, atlas_pixels, manifest,
		                            manifest, amp, "direct-libavcodec-native-amp", threshold,
		                            video_path, Format("timestamp_ms=%lld", decoded_ms),
		                            cpu_ms, amp_ms, equal, report_ms)) {
			failed++;
			equal = false;
		}
		html << "<tr><td>" << requested_ms << "</td><td>" << window_id << "</td><td>" << decoded_ms << "</td><td>"
		      << (equal ? "pass" : "fail") << "</td><td><a href=\"frame_"
		      << Format("%04d", i) << "/report.htm\">evidence</a></td></tr>\n";
		COUTLOG(Format("amp_real_video frame=%d/%d requested_ms=%lld window=%s decoded_ms=%lld parity=%s cpu_ms=%d amp_ms=%d",
		               i + 1, times.GetCount(), requested_ms, ~window_id, decoded_ms,
		               equal ? "pass" : "fail", cpu_ms, amp_ms));
	}
	html << "</table><p>frames=" << times.GetCount() << " failed=" << failed
	      << " status=" << (failed ? "fail" : "pass") << "</p>\n";
	SaveFile(AppendFileName(report_path, "index.htm"), html);
	COUTLOG(Format("amp_real_video_regression=%s frames=%d failed=%d report=%s",
	               failed ? "fail" : "pass", times.GetCount(), failed,
	               ~AppendFileName(report_path, "index.htm")));
	return failed ? 1 : 0;
}

int RunAmpAtlasRuntimeProbe()
{
	String local_otsu_report;
	for(int i = 0; i + 1 < CommandLine().GetCount(); i++)
		if(CommandLine()[i] == "--amp-local-otsu-report") local_otsu_report = CommandLine()[i + 1];
	for(const String& arg : CommandLine())
		if(arg == "--amp-local-otsu-selftest")
			return RunLocalOtsuSelftest(local_otsu_report);
	String regression_list, regression_video, regression_times, regression_report, regression_rect;
	for(int i = 0; i + 1 < CommandLine().GetCount(); i++) {
		if(CommandLine()[i] == "--amp-real-frame-regression") regression_list = CommandLine()[i + 1];
		if(CommandLine()[i] == "--amp-real-video-regression") regression_video = CommandLine()[i + 1];
		if(CommandLine()[i] == "--amp-regression-times") regression_times = CommandLine()[i + 1];
		if(CommandLine()[i] == "--amp-video-focus-rect") regression_rect = CommandLine()[i + 1];
		if(CommandLine()[i] == "--amp-regression-report") regression_report = CommandLine()[i + 1];
	}
	if(!regression_video.IsEmpty()) {
		String atlas_path, manifest_path, inventory_path;
		int device_index = 0, threshold = 0;
		for(int i = 0; i + 1 < CommandLine().GetCount(); i++) {
			if(CommandLine()[i] == "--atlas") atlas_path = CommandLine()[i + 1];
			else if(CommandLine()[i] == "--manifest") manifest_path = CommandLine()[i + 1];
			else if(CommandLine()[i] == "--amp-device-inventory") inventory_path = CommandLine()[i + 1];
			else if(CommandLine()[i] == "--amp-device-index") device_index = StrInt(CommandLine()[i + 1]);
			else if(CommandLine()[i] == "--amp-match-threshold") threshold = StrInt(CommandLine()[i + 1]);
		}
		if(atlas_path.IsEmpty() || manifest_path.IsEmpty() || regression_times.IsEmpty() ||
		   regression_report.IsEmpty()) {
			COUTLOG("usage=--amp-real-video-regression video.mp4 --amp-regression-times times.txt "
			        "--amp-regression-report report_dir --atlas atlas.png --manifest manifest.json");
			return 2;
		}
		return RunRealVideoRegression(atlas_path, manifest_path, regression_video,
		                              regression_times, regression_report, inventory_path,
		                              device_index, threshold, regression_rect);
	}
	if(!regression_list.IsEmpty()) {
		String atlas_path, manifest_path, inventory_path;
		int device_index = 0, threshold = 0;
		for(int i = 0; i + 1 < CommandLine().GetCount(); i++) {
			if(CommandLine()[i] == "--atlas") atlas_path = CommandLine()[i + 1];
			else if(CommandLine()[i] == "--manifest") manifest_path = CommandLine()[i + 1];
			else if(CommandLine()[i] == "--amp-device-inventory") inventory_path = CommandLine()[i + 1];
			else if(CommandLine()[i] == "--amp-device-index") device_index = StrInt(CommandLine()[i + 1]);
			else if(CommandLine()[i] == "--amp-match-threshold") threshold = StrInt(CommandLine()[i + 1]);
		}
		if(atlas_path.IsEmpty() || manifest_path.IsEmpty() || regression_report.IsEmpty()) {
			COUTLOG("usage=--amp-real-frame-regression list.txt --amp-regression-report report_dir --atlas atlas.png --manifest manifest.json");
			return 2;
		}
		return RunRealFrameRegression(atlas_path, manifest_path, regression_list,
		                              regression_report, inventory_path, device_index,
		                              threshold);
	}
	bool requested_dual_selftest = false;
	for(const String& arg : CommandLine())
		requested_dual_selftest |= arg == "--amp-dual-selftest";
	if(requested_dual_selftest) {
		bool has_atlas = false, has_manifest = false;
		for(int i = 0; i + 1 < CommandLine().GetCount(); i++) {
			has_atlas |= CommandLine()[i] == "--atlas";
			has_manifest |= CommandLine()[i] == "--manifest";
		}
		if(!has_atlas || !has_manifest) {
			COUTLOG("usage=--amp-dual-selftest --atlas atlas.png --manifest manifest.json "
			        "--amp-device-inventory inventory.json");
			return 2;
		}
	}
	String atlas_path, manifest_path, inventory_path;
	int device_index = 0;
	bool exact_selftest = false;
	bool frame_selftest = false;
	bool pixel_selftest = false;
	bool dual_selftest = false;
	int threshold = 0;
	String frame_report_path;
	String frame_path;
	String match_scale, match_kind;
	if(!ReadArguments(atlas_path, manifest_path, inventory_path, device_index,
                  exact_selftest, frame_selftest, pixel_selftest, dual_selftest, threshold,
                  frame_report_path,
                  frame_path, match_scale, match_kind)) {
		if(pixel_selftest)
			return RunPixelSelftest();
		COUTLOG("usage=--atlas <atlas.png> --manifest <manifest.json> "
		        "[--amp-device-inventory <inventory.json>] [--amp-device-index n] "
		        "[--amp-pixel-selftest] [--amp-dual-selftest] [--amp-frame-selftest] [--amp-match-threshold n] "
		        "[--amp-frame-report report.htm] [--frame frame.png] "
		        "[--amp-match-scale board|hand] [--amp-match-kind rank|suit]");
		return 2;
	}
	if(pixel_selftest)
		return RunPixelSelftest();
	int64 started = msecs();
	AmpTemplateAtlasManifest manifest;
	String error;
	if(!manifest.Load(manifest_path, error)) {
		COUTLOG(Format("amp_atlas_runtime=fail manifest=%s", ~error));
		return 1;
	}
	Image atlas = StreamRaster::LoadFileAny(atlas_path);
	Image frame_image;
	if(!frame_path.IsEmpty()) {
		frame_image = StreamRaster::LoadFileAny(frame_path);
		if(frame_image.IsEmpty()) {
			COUTLOG(Format("amp_frame_input=fail path=%s reason=decode", ~frame_path));
			return 1;
		}
		COUTLOG(Format("amp_frame_input=pass path=%s size=%d`x%d",
		               ~frame_path, frame_image.GetWidth(), frame_image.GetHeight()));
	}
	Vector<int> pixels;
	int nonempty = 0;
	if(!BuildCompactTemplates(atlas, manifest, pixels, nonempty, error)) {
		COUTLOG(Format("amp_atlas_runtime=fail pixels=%s", ~error));
		return 1;
	}
	if(frame_selftest) {
		Vector<int> atlas_pixels;
		BuildAtlasPixels(atlas, atlas_pixels);
		AmpTemplateAtlasManifest match_manifest;
		if(!FilterManifest(manifest, match_scale, match_kind, match_manifest, error)) {
			COUTLOG(Format("amp_frame_scope=fail error=%s", ~error));
			return 1;
		}
		String scope = match_scale.IsEmpty() ? "all" : match_scale;
		scope << "/" << (match_kind.IsEmpty() ? "all" : match_kind);
		COUTLOG(Format("amp_frame_scope=pass scope=%s entries=%d",
		               ~scope, match_manifest.entries.GetCount()));
		if(!RunFrameSelftest(atlas_pixels, atlas, match_manifest, manifest, threshold, inventory_path,
		                     device_index, frame_report_path, frame_image,
	                     frame_path.IsEmpty() ? "synthetic-fixture" : frame_path,
	                     scope))
			return 1;
	}
	int64 prepare_ms = msecs() - started;
#ifndef HAVE_SYSTEM_AMP
	if(dual_selftest && !RunDualSelftest(String(), error))
		return 1;
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
	if(dual_selftest && !RunDualSelftest(selected.device_path, error))
		return 1;
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
