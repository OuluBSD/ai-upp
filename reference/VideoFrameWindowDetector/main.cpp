#include "VideoFrameWindowDetector.h"

NAMESPACE_UPP

static void PrintHelp()
{
	Cout() << "VideoFrameWindowDetector\n\n"
	       << "Usage: VideoFrameWindowDetector --input <frame.jpg> [options]\n\n"
	       << "Options:\n"
	       << "  --input <path>             Captured frame image\n"
	       << "  --out <dir>                Output directory (default <input>_windows)\n"
	       << "  --min-title-width <px>     Minimum white titlebar width (default 300)\n"
	       << "  --min-window-height <px>   Minimum inferred window height (default 120)\n"
	       << "  --help, -h                 Show help\n";
}

static DetectorOptions ParseOptions(const Vector<String>& args)
{
	DetectorOptions opt;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--input" && i + 1 < args.GetCount())
			opt.input = args[++i];
		else if(args[i] == "--out" && i + 1 < args.GetCount())
			opt.out_dir = args[++i];
		else if(args[i] == "--min-title-width" && i + 1 < args.GetCount())
			opt.min_title_width = max(50, StrInt(args[++i]));
		else if(args[i] == "--min-window-height" && i + 1 < args.GetCount())
			opt.min_window_height = max(50, StrInt(args[++i]));
		else if(args[i] == "--help" || args[i] == "-h")
			opt.help = true;
	}
	if(opt.out_dir.IsEmpty() && !opt.input.IsEmpty())
		opt.out_dir = opt.input + "_windows";
	return opt;
}

static bool IsTitleWhite(const RGBA& p)
{
	return p.r >= 235 && p.g >= 235 && p.b >= 235;
}

static bool IsNotBlack(const RGBA& p)
{
	return p.r > 28 || p.g > 28 || p.b > 28;
}

static bool RunsOverlapEnough(const Rect& a, const Rect& b)
{
	int overlap = min(a.right, b.right) - max(a.left, b.left);
	if(overlap <= 0)
		return false;
	int minw = min(a.Width(), b.Width());
	return overlap * 3 >= minw * 2;
}

static Vector<Rect> FindTitleBars(const Image& img, int min_title_width)
{
	Vector<Rect> bars;
	for(int y = 0; y < img.GetHeight(); y++) {
		const RGBA* row = img[y];
		Vector<Rect> runs;
		int x = 0;
		while(x < img.GetWidth()) {
			while(x < img.GetWidth() && !IsTitleWhite(row[x]))
				x++;
			int start = x;
			while(x < img.GetWidth() && IsTitleWhite(row[x]))
				x++;
			if(x - start >= min_title_width)
				runs.Add(Rect(start, y, x, y + 1));
		}
		for(const Rect& run : runs) {
			bool merged = false;
			for(Rect& bar : bars) {
				if(y <= bar.bottom + 2 && RunsOverlapEnough(bar, run)) {
					bar.left = min(bar.left, run.left);
					bar.right = max(bar.right, run.right);
					bar.bottom = y + 1;
					merged = true;
					break;
				}
			}
			if(!merged)
				bars.Add(run);
		}
	}
	Vector<Rect> out;
	for(const Rect& b : bars) {
		if(b.Height() >= 12 && b.Height() <= 55 && b.Width() >= min_title_width)
			out.Add(b);
	}
	return out;
}

static Rect InferWindowRect(const Image& img, const Rect& title, int min_window_height)
{
	int last = title.bottom;
	int band = min(100, max(10, title.Width() / 5));
	for(int y = title.top; y < img.GetHeight(); y++) {
		const RGBA* row = img[y];
		int left_count = 0;
		int right_count = 0;
		for(int x = title.left; x < title.right; x++) {
			if(x < title.left + band && IsNotBlack(row[x]))
				left_count++;
			if(x >= title.right - band && IsNotBlack(row[x]))
				right_count++;
		}
		bool left_live = left_count * 100 >= band * 2;
		bool right_live = right_count * 100 >= band * 2;
		if(left_live && right_live)
			last = y;
	}
	Rect r(title.left, title.top, title.right, min(img.GetHeight(), last + 1));
	if(r.Height() < min_window_height)
		return Rect(0, 0, 0, 0);
	return r;
}

static String ClassifyWindow(const Rect& r, const Size& frame_size)
{
	if(r.top <= 40 && r.Height() >= frame_size.cy / 2)
		return "table_candidate";
	if(r.Width() >= frame_size.cx / 2 && r.Height() >= frame_size.cy / 2)
		return "large_foreground";
	if(r.Height() < frame_size.cy / 4)
		return "dialog";
	return "window";
}

static bool SameHorizontalTitle(const WindowCandidate& a, const WindowCandidate& b)
{
	return abs(a.title.left - b.title.left) <= 12 &&
	       abs(a.title.Width() - b.title.Width()) <= 24;
}

static void RemoveNestedDuplicateTitles(Vector<WindowCandidate>& windows)
{
	Vector<WindowCandidate> out;
	for(int i = 0; i < windows.GetCount(); i++) {
		bool duplicate = false;
		for(int j = 0; j < windows.GetCount(); j++) {
			if(i == j)
				continue;
			if(windows[j].title.top < windows[i].title.top &&
			   SameHorizontalTitle(windows[j], windows[i]) &&
			   windows[j].rect.Contains(windows[i].title)) {
				duplicate = true;
				break;
			}
		}
		if(!duplicate)
			out.Add(windows[i]);
	}
	windows = pick(out);
}

static Image CropImage(const Image& img, const Rect& r)
{
	ImageBuffer ib(r.Width(), r.Height());
	for(int y = 0; y < r.Height(); y++) {
		const RGBA* src = img[r.top + y] + r.left;
		RGBA* dst = ib[y];
		memcpy(dst, src, r.Width() * sizeof(RGBA));
	}
	return ib;
}

static void DrawBorder(ImageBuffer& ib, const Rect& r, const RGBA& color)
{
	Rect rr = r & RectC(0, 0, ib.GetSize().cx, ib.GetSize().cy);
	if(rr.IsEmpty())
		return;
	for(int x = rr.left; x < rr.right; x++) {
		ib[rr.top][x] = color;
		ib[rr.bottom - 1][x] = color;
	}
	for(int y = rr.top; y < rr.bottom; y++) {
		ib[y][rr.left] = color;
		ib[y][rr.right - 1] = color;
	}
}

static Image MakeOverlay(const Image& img, const Vector<WindowCandidate>& windows)
{
	ImageBuffer ib(img.GetSize());
	for(int y = 0; y < img.GetHeight(); y++)
		memcpy(ib[y], img[y], img.GetWidth() * sizeof(RGBA));
	RGBA table = { 0, 255, 0, 255 };
	RGBA other = { 255, 32, 32, 255 };
	RGBA title = { 255, 255, 0, 255 };
	for(const WindowCandidate& w : windows) {
		DrawBorder(ib, w.rect, w.role == "table_candidate" ? table : other);
		DrawBorder(ib, w.title, title);
	}
	return ib;
}

static String JsonString(const String& s)
{
	String out;
	for(int i = 0; i < s.GetCount(); i++) {
		byte c = s[i];
		if(c == '\\')
			out << "\\\\";
		else if(c == '"')
			out << "\\\"";
		else if(c == '\n')
			out << "\\n";
		else if(c == '\r')
			out << "\\r";
		else if(c == '\t')
			out << "\\t";
		else
			out.Cat(c);
	}
	return out;
}

static void AppendRectJson(String& out, const char *name, const Rect& r)
{
	out << "\"" << name << "\": {"
	    << "\"x\": " << r.left << ", "
	    << "\"y\": " << r.top << ", "
	    << "\"w\": " << r.Width() << ", "
	    << "\"h\": " << r.Height() << "}";
}

static String BuildJson(const DetectorOptions& opt, const Size& size,
                        const Vector<WindowCandidate>& windows,
                        const String& overlay_path)
{
	int table_count = 0;
	for(const WindowCandidate& w : windows)
		if(w.role == "table_candidate")
			table_count++;
	String out;
	out << "{\n";
	out << "  \"input\": \"" << JsonString(opt.input) << "\",\n";
	out << "  \"width\": " << size.cx << ",\n";
	out << "  \"height\": " << size.cy << ",\n";
	out << "  \"window_count\": " << windows.GetCount() << ",\n";
	out << "  \"table_candidate_count\": " << table_count << ",\n";
	out << "  \"overlay\": \"" << JsonString(overlay_path) << "\",\n";
	out << "  \"windows\": [\n";
	for(int i = 0; i < windows.GetCount(); i++) {
		const WindowCandidate& w = windows[i];
		out << "    {\"index\": " << w.index
		    << ", \"role\": \"" << JsonString(w.role) << "\", ";
		AppendRectJson(out, "title", w.title);
		out << ", ";
		AppendRectJson(out, "rect", w.rect);
		out << ", \"crop\": \"" << JsonString(w.crop_path) << "\"}";
		if(i + 1 < windows.GetCount())
			out << ",";
		out << "\n";
	}
	out << "  ]\n";
	out << "}\n";
	return out;
}

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN
{
	DetectorOptions opt = ParseOptions(CommandLine());
	if(opt.help || opt.input.IsEmpty()) {
		PrintHelp();
		if(opt.input.IsEmpty() && !opt.help)
			SetExitCode(1);
		return;
	}
	Image img = StreamRaster::LoadFileAny(opt.input);
	if(img.IsEmpty()) {
		Cerr() << "ERROR: failed to load input image: " << opt.input << "\n";
		SetExitCode(1);
		return;
	}
	RealizeDirectory(opt.out_dir);
	Vector<Rect> titles = FindTitleBars(img, opt.min_title_width);
	Vector<WindowCandidate> windows;
	for(const Rect& title : titles) {
		Rect rect = InferWindowRect(img, title, opt.min_window_height);
		if(rect.IsEmpty())
			continue;
		WindowCandidate& w = windows.Add();
		w.index = windows.GetCount() - 1;
		w.title = title;
		w.rect = rect;
		w.role = ClassifyWindow(rect, img.GetSize());
		w.crop_path = AppendFileName(opt.out_dir, Format("window_%02d_%s.jpg", w.index, w.role));
	}
	RemoveNestedDuplicateTitles(windows);
	Sort(windows, [](const WindowCandidate& a, const WindowCandidate& b) {
		if(a.rect.top != b.rect.top)
			return a.rect.top < b.rect.top;
		return a.rect.left < b.rect.left;
	});
	for(int i = 0; i < windows.GetCount(); i++) {
		windows[i].index = i;
		windows[i].crop_path = AppendFileName(opt.out_dir, Format("window_%02d_%s.jpg", windows[i].index, windows[i].role));
		Image crop = CropImage(img, windows[i].rect);
		if(!JPGEncoder().Quality(95).SaveFile(windows[i].crop_path, crop)) {
			Cerr() << "ERROR: failed to save crop: " << windows[i].crop_path << "\n";
			SetExitCode(1);
			return;
		}
	}
	String overlay_path = AppendFileName(opt.out_dir, "overlay.jpg");
	JPGEncoder().Quality(95).SaveFile(overlay_path, MakeOverlay(img, windows));
	String json_path = AppendFileName(opt.out_dir, "windows.json");
	SaveFile(json_path, BuildJson(opt, img.GetSize(), windows, overlay_path));
	int table_count = 0;
	for(const WindowCandidate& w : windows) {
		if(w.role == "table_candidate")
			table_count++;
		Cout() << "window index=" << w.index
		       << " role=" << w.role
		       << " rect=" << w.rect.left << "," << w.rect.top
		       << " " << w.rect.Width() << "x" << w.rect.Height()
		       << " crop=" << w.crop_path << "\n";
	}
	Cout() << "windows=" << windows.GetCount()
	       << " table_candidates=" << table_count
	       << " overlay=" << overlay_path
	       << " json=" << json_path << "\n";
}
