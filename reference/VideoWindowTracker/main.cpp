#include "VideoWindowTracker.h"

NAMESPACE_UPP

static void PrintHelp()
{
	Cout() << "VideoWindowTracker\n\n"
	       << "Usage: VideoWindowTracker --input-dir <recorder-dir> [options]\n\n"
	       << "Options:\n"
	       << "  --input-dir <dir>          Directory containing frame_*.jpg files\n"
	       << "  --out <dir>                Output directory (default <input-dir>_tracked)\n"
	       << "  --block <px>               Change detection block size (default 24)\n"
	       << "  --pixel-threshold <v>      RGB diff threshold per pixel (default 35)\n"
	       << "  --min-changed-pixels <n>   Minimum changed pixels per block (default 24)\n"
	       << "  --table-mode <mode>        Table perspective: unknown, hero, observer (default unknown)\n"
	       << "  --help, -h                 Show help\n";
}

static bool IsValidTableMode(const String& mode)
{
	return mode == "unknown" || mode == "hero" || mode == "observer";
}

static TrackerOptions ParseOptions(const Vector<String>& args)
{
	TrackerOptions opt;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--input-dir" && i + 1 < args.GetCount())
			opt.input_dir = args[++i];
		else if(args[i] == "--out" && i + 1 < args.GetCount())
			opt.out_dir = args[++i];
		else if(args[i] == "--block" && i + 1 < args.GetCount())
			opt.block = max(4, StrInt(args[++i]));
		else if(args[i] == "--pixel-threshold" && i + 1 < args.GetCount())
			opt.pixel_threshold = max(1, StrInt(args[++i]));
		else if(args[i] == "--min-changed-pixels" && i + 1 < args.GetCount())
			opt.min_changed_pixels = max(1, StrInt(args[++i]));
		else if(args[i] == "--table-mode" && i + 1 < args.GetCount())
			opt.table_mode = ToLower(args[++i]);
		else if(args[i] == "--help" || args[i] == "-h")
			opt.help = true;
	}
	if(!IsValidTableMode(opt.table_mode))
		opt.table_mode = "unknown";
	if(opt.out_dir.IsEmpty() && !opt.input_dir.IsEmpty())
		opt.out_dir = opt.input_dir + "_tracked";
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
		int x = 0;
		while(x < img.GetWidth()) {
			while(x < img.GetWidth() && !IsTitleWhite(row[x]))
				x++;
			int start = x;
			while(x < img.GetWidth() && IsTitleWhite(row[x]))
				x++;
			if(x - start < min_title_width)
				continue;
			Rect run(start, y, x, y + 1);
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
	for(const Rect& b : bars)
		if(b.Height() >= 12 && b.Height() <= 55 && b.Width() >= min_title_width)
			out.Add(b);
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
		if(left_count * 100 >= band * 2 && right_count * 100 >= band * 2)
			last = y;
	}
	Rect r(title.left, title.top, title.right, min(img.GetHeight(), last + 1));
	return r.Height() >= min_window_height ? r : Rect(0, 0, 0, 0);
}

static Vector<TrackerWindow> DetectTables(const Image& img, const TrackerOptions& opt)
{
	Vector<TrackerWindow> out;
	for(const Rect& title : FindTitleBars(img, opt.min_title_width)) {
		Rect r = InferWindowRect(img, title, opt.min_window_height);
		if(r.IsEmpty())
			continue;
		if(r.top <= 40 && r.Height() >= img.GetHeight() / 2) {
			TrackerWindow& w = out.Add();
			w.title = title;
			w.rect = r;
		}
	}
	Sort(out, [](const TrackerWindow& a, const TrackerWindow& b) {
		return a.rect.left < b.rect.left;
	});
	for(int i = 0; i < out.GetCount(); i++)
		out[i].id = i;
	return out;
}

static Image CropImage(const Image& img, const Rect& r)
{
	ImageBuffer ib(r.Width(), r.Height());
	for(int y = 0; y < r.Height(); y++)
		memcpy(ib[y], img[r.top + y] + r.left, r.Width() * sizeof(RGBA));
	return ib;
}

static int PixelDiff(const RGBA& a, const RGBA& b)
{
	return max(max(abs((int)a.r - (int)b.r), abs((int)a.g - (int)b.g)), abs((int)a.b - (int)b.b));
}

static Vector<TrackerChange> DetectChanges(const Image& prev, const Image& curr,
                                           int window_id, const TrackerOptions& opt)
{
	Vector<TrackerChange> changes;
	if(prev.GetSize() != curr.GetSize())
		return changes;
	for(int y = 0; y < curr.GetHeight(); y += opt.block) {
		for(int x = 0; x < curr.GetWidth(); x += opt.block) {
			Rect block = RectC(x, y, min(opt.block, curr.GetWidth() - x),
			                  min(opt.block, curr.GetHeight() - y));
			int changed = 0;
			for(int yy = block.top; yy < block.bottom; yy++) {
				const RGBA* pr = prev[yy];
				const RGBA* cr = curr[yy];
				for(int xx = block.left; xx < block.right; xx++)
					if(PixelDiff(pr[xx], cr[xx]) >= opt.pixel_threshold)
						changed++;
			}
			if(changed >= opt.min_changed_pixels) {
				TrackerChange& c = changes.Add();
				c.window_id = window_id;
				c.rect = block;
				c.changed_pixels = changed;
			}
		}
	}
	return changes;
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

static Image MakeFrameOverlay(const Image& frame, const Vector<TrackerWindow>& windows)
{
	ImageBuffer ib(frame.GetSize());
	for(int y = 0; y < frame.GetHeight(); y++)
		memcpy(ib[y], frame[y], frame.GetWidth() * sizeof(RGBA));
	RGBA green = { 0, 255, 0, 255 };
	for(const TrackerWindow& w : windows)
		DrawBorder(ib, w.rect, green);
	return ib;
}

static Image MakeChangeOverlay(const Image& crop, const Vector<TrackerChange>& changes)
{
	ImageBuffer ib(crop.GetSize());
	for(int y = 0; y < crop.GetHeight(); y++)
		memcpy(ib[y], crop[y], crop.GetWidth() * sizeof(RGBA));
	RGBA red = { 255, 0, 0, 255 };
	for(const TrackerChange& c : changes)
		DrawBorder(ib, c.rect, red);
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

static void AppendRect(String& out, const Rect& r)
{
	out << "{\"x\": " << r.left << ", \"y\": " << r.top
	    << ", \"w\": " << r.Width() << ", \"h\": " << r.Height() << "}";
}

static int Part(int total, int permille)
{
	return total * permille / 1000;
}

static Rect CropRect(const Size& sz, int left, int top, int right, int bottom)
{
	Rect bounds = RectC(0, 0, sz.cx, sz.cy);
	Rect r(Part(sz.cx, left), Part(sz.cy, top), Part(sz.cx, right), Part(sz.cy, bottom));
	r = r & bounds;
	return r.Width() > 0 && r.Height() > 0 ? r : Rect(0, 0, 0, 0);
}

static void AddSemanticCrop(Vector<SemanticCrop>& crops, const String& name, const Rect& rect)
{
	ASSERT(!name.IsEmpty());
	ASSERT(!rect.IsEmpty());
	SemanticCrop& crop = crops.Add();
	crop.name = name;
	crop.rect = rect;
}

static Vector<SemanticCrop> GetSemanticCrops(const TrackerWindow& window, const Image& crop)
{
	Vector<SemanticCrop> out;
	Size sz = crop.GetSize();
	Rect bounds = RectC(0, 0, sz.cx, sz.cy);
	Rect title(window.title.left - window.rect.left, window.title.top - window.rect.top,
	           window.title.right - window.rect.left, window.title.bottom - window.rect.top);
	title = title & bounds;
	if(title.IsEmpty())
		title = CropRect(sz, 0, 0, 1000, 45);

	AddSemanticCrop(out, "title", title);
	AddSemanticCrop(out, "table_area", CropRect(sz, 0, 45, 1000, 1000));
	AddSemanticCrop(out, "board_cards", CropRect(sz, 340, 350, 660, 485));
	AddSemanticCrop(out, "pot_label", CropRect(sz, 405, 270, 595, 350));
	AddSemanticCrop(out, "center_chips", CropRect(sz, 420, 410, 580, 545));
	AddSemanticCrop(out, "bottom_button", CropRect(sz, 380, 560, 475, 690));
	AddSemanticCrop(out, "left_seats", CropRect(sz, 35, 190, 310, 810));
	AddSemanticCrop(out, "right_seats", CropRect(sz, 690, 190, 965, 810));
	AddSemanticCrop(out, "top_seat", CropRect(sz, 350, 115, 650, 300));
	AddSemanticCrop(out, "bottom_seat", CropRect(sz, 330, 650, 670, 870));
	return out;
}

static Vector<SemanticCrop> SaveSemanticCrops(const Image& crop, const TrackerWindow& window,
                                              const String& semantic_dir, int frame_index)
{
	String table_dir = AppendFileName(semantic_dir, Format("frame_%06d_table_%d", frame_index, window.id));
	RealizeDirectory(table_dir);
	Vector<SemanticCrop> crops = GetSemanticCrops(window, crop);
	for(SemanticCrop& semantic : crops) {
		semantic.path = AppendFileName(table_dir, semantic.name + ".jpg");
		JPGEncoder().Quality(95).SaveFile(semantic.path, CropImage(crop, semantic.rect));
	}
	return crops;
}

static void AppendSemanticCrops(String& out, const Vector<SemanticCrop>& crops)
{
	out << "{";
	for(int i = 0; i < crops.GetCount(); i++) {
		if(i)
			out << ", ";
		out << "\"" << JsonString(crops[i].name) << "\": {\"path\": \""
		    << JsonString(crops[i].path) << "\", \"rect\": ";
		AppendRect(out, crops[i].rect);
		out << "}";
	}
	out << "}";
}

struct SemanticChange : Moveable<SemanticChange> {
	String name;
	int    change_blocks = 0;
	Rect   change_union;
};

static Vector<SemanticChange> SummarizeSemanticChanges(const Vector<SemanticCrop>& crops,
                                                       const Vector<TrackerChange>& changes)
{
	Vector<SemanticChange> out;
	for(const SemanticCrop& crop : crops) {
		SemanticChange& semantic = out.Add();
		semantic.name = crop.name;
		for(const TrackerChange& change : changes) {
			Rect overlap = crop.rect & change.rect;
			if(overlap.IsEmpty())
				continue;
			semantic.change_blocks++;
			semantic.change_union = semantic.change_blocks == 1 ? overlap : semantic.change_union | overlap;
		}
	}
	return out;
}

static void AppendSemanticChanges(String& out, const Vector<SemanticChange>& changes)
{
	out << "{";
	for(int i = 0; i < changes.GetCount(); i++) {
		if(i)
			out << ", ";
		out << "\"" << JsonString(changes[i].name) << "\": {\"change_blocks\": "
		    << changes[i].change_blocks << ", \"union\": ";
		AppendRect(out, changes[i].change_union);
		out << "}";
	}
	out << "}";
}

static String FormatSemanticChangesLine(const Vector<SemanticChange>& changes)
{
	String out;
	for(const SemanticChange& change : changes) {
		if(change.change_blocks <= 0)
			continue;
		if(!out.IsEmpty())
			out << " ";
		out << change.name << "=" << change.change_blocks;
	}
	return out;
}

static int SemanticBlocks(const Vector<SemanticChange>& changes, const char *name)
{
	for(const SemanticChange& change : changes)
		if(change.name == name)
			return change.change_blocks;
	return 0;
}

static void AddEvent(Vector<SemanticEvent>& events, int frame_index, int table_id,
                     const String& type, const String& reason, const String& semantic,
                     int change_blocks, double confidence)
{
	SemanticEvent& event = events.Add();
	event.frame_index = frame_index;
	event.table_id = table_id;
	event.type = type;
	event.reason = reason;
	event.semantic = semantic;
	event.change_blocks = change_blocks;
	event.confidence = confidence;
}

static Vector<SemanticEvent> DetectSemanticEvents(int frame_index, int table_id,
                                                  const Vector<SemanticChange>& changes,
                                                  const TrackerOptions& opt)
{
	Vector<SemanticEvent> events;
	int board = SemanticBlocks(changes, "board_cards");
	int pot = SemanticBlocks(changes, "pot_label");
	int center = SemanticBlocks(changes, "center_chips");
	int left = SemanticBlocks(changes, "left_seats");
	int right = SemanticBlocks(changes, "right_seats");
	int top = SemanticBlocks(changes, "top_seat");
	int bottom = SemanticBlocks(changes, "bottom_seat");
	int buttons = SemanticBlocks(changes, "bottom_button");
	int seat = left + right + top + bottom;

	if(board >= 12 || (board >= 2 && (center >= 1 || pot >= 1)))
		AddEvent(events, frame_index, table_id, "board_changed",
		         Format("board_cards=%d center_chips=%d pot_label=%d", board, center, pot),
		         "board_cards", board, board >= 12 ? 0.75 : 0.55);
	if(pot >= 2)
		AddEvent(events, frame_index, table_id, "pot_changed",
		         Format("pot_label=%d center_chips=%d", pot, center),
		         "pot_label", pot, 0.55);
	if(center >= 2)
		AddEvent(events, frame_index, table_id, "chips_changed",
		         Format("center_chips=%d pot_label=%d", center, pot),
		         "center_chips", center, 0.55);
	if(seat >= 6)
		AddEvent(events, frame_index, table_id, "seat_activity",
		         Format("left=%d right=%d top=%d bottom=%d", left, right, top, bottom),
		         "seat_regions", seat, 0.35);
	if(buttons >= 2) {
		if(opt.table_mode == "observer")
			AddEvent(events, frame_index, table_id, "observer_bottom_region_changed",
			         Format("observer_nohero bottom_button=%d bottom_seat=%d", buttons, bottom),
			         "bottom_button", buttons, 0.30);
		else
			AddEvent(events, frame_index, table_id, "button_changed",
			         Format("bottom_button=%d bottom_seat=%d", buttons, bottom),
			         "bottom_button", buttons, 0.45);
	}
	return events;
}

static Vector<SemanticEvent> DebounceEvents(const Vector<SemanticEvent>& events,
                                            VectorMap<String, int>& last_event_frame)
{
	Vector<SemanticEvent> out;
	for(const SemanticEvent& event : events) {
		String key = AsString(event.table_id) + ":" + event.type;
		int ix = last_event_frame.Find(key);
		if(ix >= 0 && event.frame_index - last_event_frame[ix] <= 3)
			continue;
		last_event_frame.GetAdd(key) = event.frame_index;
		out.Add(event);
	}
	return out;
}

static String FormatEventsLine(const Vector<SemanticEvent>& events)
{
	String out;
	for(const SemanticEvent& event : events) {
		if(!out.IsEmpty())
			out << " ";
		out << event.type;
	}
	return out;
}

static void AppendEvents(String& out, const Vector<SemanticEvent>& events)
{
	out << "[";
	for(int i = 0; i < events.GetCount(); i++) {
		if(i)
			out << ", ";
		out << "{\"frame_index\": " << events[i].frame_index
		    << ", \"table_id\": " << events[i].table_id
		    << ", \"type\": \"" << JsonString(events[i].type)
		    << "\", \"reason\": \"" << JsonString(events[i].reason)
		    << "\", \"confidence\": " << Format("%.2f", events[i].confidence)
		    << ", \"evidence\": {\"semantic\": \"" << JsonString(events[i].semantic)
		    << "\", \"change_blocks\": " << events[i].change_blocks << "}}";
	}
	out << "]";
}

static Rect GetChangeUnion(const Vector<TrackerChange>& changes)
{
	Rect out(0, 0, 0, 0);
	for(int i = 0; i < changes.GetCount(); i++)
		out = i ? out | changes[i].rect : changes[i].rect;
	return out;
}

static Vector<String> FindFrames(const String& dir)
{
	Vector<String> files;
	FindFile ff(AppendFileName(dir, "frame_*.jpg"));
	while(ff) {
		if(ff.IsFile())
			files.Add(AppendFileName(dir, ff.GetName()));
		ff.Next();
	}
	Sort(files);
	return files;
}

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN
{
	TrackerOptions opt = ParseOptions(CommandLine());
	if(opt.help || opt.input_dir.IsEmpty()) {
		PrintHelp();
		if(opt.input_dir.IsEmpty() && !opt.help)
			SetExitCode(1);
		return;
	}
	Vector<String> frames = FindFrames(opt.input_dir);
	if(frames.IsEmpty()) {
		Cerr() << "ERROR: no frame_*.jpg files in " << opt.input_dir << "\n";
		SetExitCode(1);
		return;
	}
	RealizeDirectory(opt.out_dir);
	String crops_dir = AppendFileName(opt.out_dir, "crops");
	String overlays_dir = AppendFileName(opt.out_dir, "overlays");
	String semantic_dir = AppendFileName(opt.out_dir, "semantic");
	RealizeDirectory(crops_dir);
	RealizeDirectory(overlays_dir);
	RealizeDirectory(semantic_dir);

	Vector<Image> previous_crops;
	String json;
	json << "{\n";
	json << "  \"input_dir\": \"" << JsonString(opt.input_dir) << "\",\n";
	json << "  \"table_mode\": \"" << JsonString(opt.table_mode) << "\",\n";
	json << "  \"hero_cards_expected\": " << (opt.table_mode == "hero" ? "true" : "false") << ",\n";
	json << "  \"observer_nohero\": " << (opt.table_mode == "observer" ? "true" : "false") << ",\n";
	json << "  \"frame_count\": " << frames.GetCount() << ",\n";
	json << "  \"frames\": [\n";

	String summary;
	summary << "{\n";
	summary << "  \"input_dir\": \"" << JsonString(opt.input_dir) << "\",\n";
	summary << "  \"table_mode\": \"" << JsonString(opt.table_mode) << "\",\n";
	summary << "  \"hero_cards_expected\": " << (opt.table_mode == "hero" ? "true" : "false") << ",\n";
	summary << "  \"observer_nohero\": " << (opt.table_mode == "observer" ? "true" : "false") << ",\n";
	summary << "  \"frame_count\": " << frames.GetCount() << ",\n";
	summary << "  \"frames\": [\n";

	Vector<SemanticEvent> all_events;
	VectorMap<String, int> last_event_frame;
	for(int fi = 0; fi < frames.GetCount(); fi++) {
		Image frame = StreamRaster::LoadFileAny(frames[fi]);
		if(frame.IsEmpty()) {
			Cerr() << "ERROR: failed to load frame " << frames[fi] << "\n";
			SetExitCode(1);
			return;
		}
		Vector<TrackerWindow> windows = DetectTables(frame, opt);
		String frame_overlay = AppendFileName(overlays_dir, Format("frame_%06d_windows.jpg", fi));
		JPGEncoder().Quality(95).SaveFile(frame_overlay, MakeFrameOverlay(frame, windows));
		Cout() << "frame=" << fi << " tables=" << windows.GetCount()
		       << " overlay=" << frame_overlay << "\n";

		json << "    {\"index\": " << fi
		     << ", \"path\": \"" << JsonString(frames[fi]) << "\""
		     << ", \"table_count\": " << windows.GetCount()
		     << ", \"overlay\": \"" << JsonString(frame_overlay) << "\""
		     << ", \"windows\": [";

		summary << "    {\"index\": " << fi
		        << ", \"path\": \"" << JsonString(frames[fi]) << "\""
		        << ", \"table_count\": " << windows.GetCount()
		        << ", \"overlay\": \"" << JsonString(frame_overlay) << "\""
		        << ", \"tables\": [";

		Vector<Image> current_crops;
		for(int wi = 0; wi < windows.GetCount(); wi++) {
			TrackerWindow& w = windows[wi];
			Image crop = CropImage(frame, w.rect);
			current_crops.Add(crop);
			w.crop_path = AppendFileName(crops_dir, Format("frame_%06d_table_%d.jpg", fi, w.id));
			JPGEncoder().Quality(95).SaveFile(w.crop_path, crop);
			Vector<SemanticCrop> semantic_crops = SaveSemanticCrops(crop, w, semantic_dir, fi);
			Vector<TrackerChange> changes;
			String change_overlay;
			if(fi > 0 && wi < previous_crops.GetCount()) {
				changes = DetectChanges(previous_crops[wi], crop, w.id, opt);
				change_overlay = AppendFileName(overlays_dir, Format("frame_%06d_table_%d_changes.jpg", fi, w.id));
				JPGEncoder().Quality(95).SaveFile(change_overlay, MakeChangeOverlay(crop, changes));
			}
			Vector<SemanticChange> semantic_changes = SummarizeSemanticChanges(semantic_crops, changes);
			Vector<SemanticEvent> events = DebounceEvents(DetectSemanticEvents(fi, w.id, semantic_changes, opt),
			                                              last_event_frame);
			for(const SemanticEvent& event : events)
				all_events.Add(event);
			String semantic_change_line = FormatSemanticChangesLine(semantic_changes);
			String event_line = FormatEventsLine(events);
			Cout() << "  table=" << w.id
			       << " rect=" << w.rect.left << "," << w.rect.top
			       << " " << w.rect.Width() << "x" << w.rect.Height()
			       << " changes=" << changes.GetCount()
			       << " semantic=" << semantic_crops.GetCount()
			       << " crop=" << w.crop_path;
			if(!semantic_change_line.IsEmpty())
				Cout() << " semantic_changes=" << semantic_change_line;
			if(!event_line.IsEmpty())
				Cout() << " events=" << event_line;
			Cout() << "\n";

			if(wi)
				json << ", ";
			json << "{\"id\": " << w.id << ", \"rect\": ";
			AppendRect(json, w.rect);
			json << ", \"crop\": \"" << JsonString(w.crop_path) << "\""
			     << ", \"semantic\": ";
			AppendSemanticCrops(json, semantic_crops);
			json << ", \"change_count\": " << changes.GetCount();
			if(!change_overlay.IsEmpty())
				json << ", \"change_overlay\": \"" << JsonString(change_overlay) << "\"";
			json << ", \"changes\": [";
			for(int ci = 0; ci < changes.GetCount(); ci++) {
				if(ci)
					json << ", ";
				json << "{\"rect\": ";
				AppendRect(json, changes[ci].rect);
				json << ", \"changed_pixels\": " << changes[ci].changed_pixels << "}";
			}
			json << "]}";

			if(wi)
				summary << ", ";
			summary << "{\"id\": " << w.id << ", \"rect\": ";
			AppendRect(summary, w.rect);
			summary << ", \"crop\": \"" << JsonString(w.crop_path) << "\""
			        << ", \"semantic\": ";
			AppendSemanticCrops(summary, semantic_crops);
			summary << ", \"change_count\": " << changes.GetCount()
			        << ", \"change_union\": ";
			AppendRect(summary, GetChangeUnion(changes));
			summary << ", \"semantic_changes\": ";
			AppendSemanticChanges(summary, semantic_changes);
			summary << ", \"events\": ";
			AppendEvents(summary, events);
			summary << ", \"change_overlay\": \"" << JsonString(change_overlay) << "\"}";
		}
		json << "]}";
		summary << "]}";
		if(fi + 1 < frames.GetCount())
			json << ",";
		if(fi + 1 < frames.GetCount())
			summary << ",";
		json << "\n";
		summary << "\n";
		previous_crops = pick(current_crops);
	}
	json << "  ]\n";
	json << "}\n";
	summary << "  ]\n";
	summary << "}\n";
	String events_json;
	events_json << "{\n";
	events_json << "  \"input_dir\": \"" << JsonString(opt.input_dir) << "\",\n";
	events_json << "  \"table_mode\": \"" << JsonString(opt.table_mode) << "\",\n";
	events_json << "  \"hero_cards_expected\": " << (opt.table_mode == "hero" ? "true" : "false") << ",\n";
	events_json << "  \"observer_nohero\": " << (opt.table_mode == "observer" ? "true" : "false") << ",\n";
	events_json << "  \"frame_count\": " << frames.GetCount() << ",\n";
	events_json << "  \"event_count\": " << all_events.GetCount() << ",\n";
	events_json << "  \"events\": ";
	AppendEvents(events_json, all_events);
	events_json << "\n}\n";
	String json_path = AppendFileName(opt.out_dir, "tracking.json");
	String summary_path = AppendFileName(opt.out_dir, "tracking_summary.json");
	String events_path = AppendFileName(opt.out_dir, "events.json");
	SaveFile(json_path, json);
	SaveFile(summary_path, summary);
	SaveFile(events_path, events_json);
	Cout() << "tracking_json=" << json_path << "\n";
	Cout() << "tracking_summary_json=" << summary_path << "\n";
	Cout() << "events_json=" << events_path << "\n";
}
