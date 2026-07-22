#include "AmpTemplateAtlasPack.h"

CONSOLE_APP_MAIN {
	Upp::RunAmpTemplateAtlasPack();
}

NAMESPACE_UPP

struct SourceTemplate : Moveable<SourceTemplate> {
	String path;
	String kind;
	String scale;
	String symbol;
	Image image;
	int x = 0;
	int y = 0;
};

static bool IsPng(const String& path)
{
	return ToLower(GetFileExt(path)) == ".png";
}

static void DiscoverTemplates(const String& root, const String& directory,
	                          const String& inherited_scale, Vector<SourceTemplate>& out)
{
	FindFile ff(AppendFileName(directory, "*"));
	while(ff) {
		String path = ff.GetPath();
		if(ff.IsFolder()) {
			String name = ToLower(GetFileName(path));
			String scale = inherited_scale;
			if(name == "ranks" || name == "suits")
				scale = directory == root ? "board" : "hand";
			DiscoverTemplates(root, path, scale, out);
		}
		else if(IsPng(path)) {
			String parent = ToLower(GetFileName(GetFileFolder(path)));
			if(parent != "ranks" && parent != "suits") {
				ff.Next();
				continue;
			}
			Image image = StreamRaster::LoadFileAny(path);
			if(image.IsEmpty()) {
				COUTLOG(Format("amp_atlas_skip path=%s reason=decode", ~path));
				ff.Next();
				continue;
			}
			SourceTemplate& item = out.Add();
			item.path = path;
			item.kind = parent == "ranks" ? "rank" : "suit";
			item.scale = inherited_scale.IsEmpty() ? "board" : inherited_scale;
			item.symbol = GetFileTitle(path);
			item.image = image;
		}
		ff.Next();
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

static Image MakeCanvas(int width, int height, Color color)
{
	ImageBuffer buffer(width, height);
	for(int y = 0; y < height; y++) {
		RGBA* row = buffer[y];
		for(int x = 0; x < width; x++) {
			row[x].r = color.GetR();
			row[x].g = color.GetG();
			row[x].b = color.GetB();
			row[x].a = 255;
		}
	}
	return Image(buffer);
}

static void Blit(ImageBuffer& destination, int dx, int dy, const Image& source)
{
	for(int y = 0; y < source.GetHeight(); y++)
		memcpy(destination[dy + y] + dx, source[y], source.GetWidth() * sizeof(RGBA));
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

static bool PackAtlas(const String& root, const String& atlas_path,
                  const String& manifest_path, const String& report_path)
{
	int64 started_ms = msecs();
	Vector<SourceTemplate> source;
	DiscoverTemplates(root, root, String(), source);
	int64 discover_ms = msecs() - started_ms;
	if(source.IsEmpty()) {
		COUTLOG(Format("amp_atlas_pack=fail root=%s reason=no-templates", ~root));
		return false;
	}

	Index<String> identities;
	int width = 1024;
	int x = 2, y = 2, row_height = 0;
	for(SourceTemplate& item : source) {
		int w = item.image.GetWidth();
		int h = item.image.GetHeight();
		if(x + w + 2 > width) {
			x = 2;
			y += row_height + 2;
			row_height = 0;
		}
		item.x = x;
		item.y = y;
		x += w + 2;
		row_height = max(row_height, h);
		String identity = item.kind + "-" + item.symbol + "-" + item.scale;
		if(identities.Find(identity) >= 0) {
			COUTLOG(Format("amp_atlas_pack=fail duplicate=%s", ~identity));
			return false;
		}
		identities.Add(identity);
	}
	int height = y + row_height + 2;
	int64 layout_ms = msecs() - started_ms - discover_ms;
	Image atlas = MakeCanvas(width, height, Color(18, 18, 18));
	ImageBuffer atlas_buffer(atlas);
	AmpTemplateAtlasManifest manifest;
	manifest.atlas_name = GetFileName(atlas_path);
	manifest.atlas_width = width;
	manifest.atlas_height = height;
	for(const SourceTemplate& item : source) {
		Blit(atlas_buffer, item.x, item.y, item.image);
		AmpTemplateAtlasEntry& entry = manifest.entries.Add();
		entry.id = item.kind + "-" + item.symbol + "-" + item.scale;
		entry.kind = item.kind;
		entry.scale = item.scale;
		entry.x = item.x;
		entry.y = item.y;
		entry.width = item.image.GetWidth();
		entry.height = item.image.GetHeight();
		entry.preprocessing = item.kind == "rank" ? "grayscale" : "color";
		entry.threshold = 0;
	}
	atlas = atlas_buffer;
	String error;
	if(!manifest.Validate(error)) {
		COUTLOG(Format("amp_atlas_pack=fail manifest=%s", ~error));
		return false;
	}
	RealizeDirectory(GetFileFolder(atlas_path));
	RealizeDirectory(GetFileFolder(manifest_path));
	RealizeDirectory(GetFileFolder(report_path));
	if(!PNGEncoder().SaveFile(atlas_path, atlas)) {
		COUTLOG(Format("amp_atlas_pack=fail output=%s reason=atlas-write", ~atlas_path));
		return false;
	}
	if(!manifest.Save(manifest_path))
		return false;
	AmpTemplateAtlasManifest roundtrip;
	String roundtrip_error;
	bool roundtrip_ok = roundtrip.Load(manifest_path, roundtrip_error);
	if(!roundtrip_ok) {
		COUTLOG(Format("amp_atlas_pack=fail roundtrip=%s", ~roundtrip_error));
		return false;
	}

	Image annotated = MakeCanvas(width, height + 36, Color(18, 18, 18));
	ImageBuffer annotated_buffer(annotated);
	Blit(annotated_buffer, 0, 0, atlas);
	for(const SourceTemplate& item : source) {
		Color outline = item.kind == "rank" ? Color(80, 180, 255) : Color(255, 180, 70);
		Outline(annotated_buffer, item.x - 1, item.y - 1, item.image.GetWidth() + 2,
		        item.image.GetHeight() + 2, outline);
	}
	annotated = annotated_buffer;
	String annotated_path = AppendFileName(GetFileFolder(report_path), "atlas_annotated.png");
	PNGEncoder().SaveFile(annotated_path, annotated);
	String html;
	html << "<!doctype html><meta charset=\"utf-8\"><title>AMP atlas evidence</title>\n"
	      << "<h1>Template atlas evidence</h1>\n"
	      << "<p>entries=" << source.GetCount() << " size=" << width << "x" << height << "</p>\n"
	      << "<p><img src=\"" << EscapeHtml(GetFileName(atlas_path)) << "\" alt=\"atlas\"></p>\n"
	      << "<p><img src=\"atlas_annotated.png\" alt=\"annotated atlas\"></p>\n"
	      << "<table><tr><th>id</th><th>source</th><th>rect</th><th>preprocessing</th></tr>\n";
	for(const SourceTemplate& item : source)
		html << "<tr><td>" << EscapeHtml(item.kind + "-" + item.symbol + "-" + item.scale)
		      << "</td><td>" << EscapeHtml(item.path) << "</td><td>"
			  << Format("%d,%d %d`x%d", item.x, item.y, item.image.GetWidth(), item.image.GetHeight())
		      << "</td><td>" << (item.kind == "rank" ? "grayscale" : "color") << "</td></tr>\n";
	html << "</table>\n";
	if(!SaveFile(report_path, html))
		return false;
	int64 total_ms = msecs() - started_ms;
	String timing = Format("discover_ms=%d layout_ms=%d output_ms=%d total_ms=%d",
	                       discover_ms, layout_ms, total_ms - discover_ms - layout_ms, total_ms);
	html = LoadFile(report_path);
	html.Replace("</table>", "</table><p>" + timing + "</p>");
	SaveFile(report_path, html);
	COUTLOG(Format("amp_atlas_pack=pass entries=%d size=%d`x%d %s atlas=%s manifest=%s report=%s",
	               source.GetCount(), width, height, ~timing, ~atlas_path, ~manifest_path,
	               ~report_path));
	return true;
}

int RunAmpTemplateAtlasPack()
{
	const Vector<String>& args = CommandLine();
	for(int i = 0; i + 4 < args.GetCount(); i++) {
		if(args[i] == "--pack")
			return PackAtlas(args[i + 1], args[i + 2], args[i + 3], args[i + 4]) ? 0 : 1;
	}
	COUTLOG("usage=--pack <template-root> <atlas.png> <manifest.json> <report.htm>");
	return 2;
}

END_UPP_NAMESPACE
