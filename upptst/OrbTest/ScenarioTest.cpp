#include <ComputerVision/ComputerVision.h>
#include <plugin/jpg/jpg.h>
#include <plugin/png/png.h>

using namespace Upp;

struct RuleMatch {
	String name;
	Point  expected_pos;
	Rect   roi;
	Rect   found_rect;
	int    good_matches;
	bool   fallback_used;
	bool   success;
};

void RunScenario(const String& frame_path, const String& window_pattern_path, const VectorMap<String, String>& corner_patterns) {
	Image frame = StreamRaster::LoadFileAny(frame_path);
	if (frame.IsEmpty()) {
		Cout() << "Error: Failed to load frame: " << frame_path << "
";
		return;
	}

	Image window_pattern = StreamRaster::LoadFileAny(window_pattern_path);
	if (window_pattern.IsEmpty()) {
		Cout() << "Error: Failed to load window pattern: " << window_pattern_path << "
";
		return;
	}

	Cout() << "--- SCENARIO START ---
";
	Cout() << "Frame: " << frame.GetSize() << "
";
	
	OrbSystem orb;
	orb.InitDefault();
	orb.SetInput(window_pattern);
	orb.TrainPattern();
	
	orb.SetInput(frame);
	orb.Process();
	
	int w_good = orb.GetLastGoodMatches();
	if (w_good < 4) {
		Cout() << "Error: window-size not found (good matches=" << w_good << ")
";
		return;
	}
	
	const Vector<Pointf>& w_corners = orb.GetLastCorners();
	Pointf w_tl = w_corners[0];
	for(const auto& p : w_corners) {
		w_tl.x = min(w_tl.x, p.x);
		w_tl.y = min(w_tl.y, p.y);
	}
	
	Cout() << "Window found at: " << w_tl << " (good matches=" << w_good << ")

";
	
	// Hardcoded relative offsets for this specific frame/ruleset (derived from previous successful runs)
	VectorMap<String, Point> rel_offsets;
	rel_offsets.Add("top-left", Point(43, 7));
	rel_offsets.Add("top-right", Point(514 - 450, 157 - 182));
	rel_offsets.Add("bottom-left 1", Point(74 - 450, 1064 - 182)); // Using the noisy values from JSON to see if ROI helps
	rel_offsets.Add("bottom-right 1", Point(758, 421)); // From my previous manual calc
	
	for (int i = 0; i < corner_patterns.GetCount(); i++) {
		String name = corner_patterns.GetKey(i);
		String path = corner_patterns[i];
		
		Image pattern = StreamRaster::LoadFileAny(path);
		if (pattern.IsEmpty()) {
			Cout() << "Rule [" << name << "]: pattern missing at " << path << "
";
			continue;
		}
		
		Point rel_pos = rel_offsets.Get(name, Point(0, 0));
		Point expected = Point((int)w_tl.x + rel_pos.x, (int)w_tl.y + rel_pos.y);
		
		// 1. Run in ROI
		Rect roi = RectC(expected.x - 50, expected.y - 50, 100 + pattern.GetWidth(), 100 + pattern.GetHeight());
		
		orb.SetInput(pattern);
		orb.TrainPattern();
		orb.SetInput(frame);
		orb.ProcessROI(roi);
		
		bool success = (orb.GetLastGoodMatches() >= 4);
		bool fallback = false;
		
		// 2. Expand ROI if failed
		if (!success) {
			roi = RectC(expected.x - 150, expected.y - 150, 300 + pattern.GetWidth(), 300 + pattern.GetHeight());
			orb.ProcessROI(roi);
			success = (orb.GetLastGoodMatches() >= 4);
		}
		
		// 3. Fallback to full frame
		if (!success) {
			fallback = true;
			orb.Process();
			success = (orb.GetLastGoodMatches() >= 4);
		}
		
		Cout() << "Rule [" << name << "]:
";
		Cout() << "  Expected: " << expected << "
";
		Cout() << "  ROI used: " << (fallback ? "FULL FRAME" : AsString(roi)) << "
";
		
		if (success) {
			const auto& c = orb.GetLastCorners();
			Pointf found_tl = c[0];
			for(const auto& p : c) {
				found_tl.x = min(found_tl.x, p.x);
				found_tl.y = min(found_tl.y, p.y);
			}
			Cout() << "  Match: " << found_tl << " (good=" << orb.GetLastGoodMatches() << ")
";
			Cout() << "  Status: SUCCESS" << (fallback ? " (via FALLBACK)" : "") << "
";
		} else {
			Cout() << "  Status: FAILED
";
		}
		Cout() << "
";
	}
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);

	CommandLineArguments cla;
	cla.AddArg("frame", 0, "Path to the frame image (JPG/PNG)", true);
	cla.AddArg("window", 0, "Path to the window-size pattern", true);
	cla.AddArg("tl", 0, "Path to top-left rule", false);
	cla.AddArg("tr", 0, "Path to top-right rule", false);
	cla.AddArg("bl", 0, "Path to bottom-left rule", false);
	cla.AddArg("br", 0, "Path to bottom-right rule", false);
	
	if (cla.Parse()) {
		String frame_path = cla.GetArg("frame");
		String window_path = cla.GetArg("window");
		
		VectorMap<String, String> corners;
		if (cla.IsArg("tl")) corners.Add("top-left", cla.GetArg("tl"));
		if (cla.IsArg("tr")) corners.Add("top-right", cla.GetArg("tr"));
		if (cla.IsArg("bl")) corners.Add("bottom-left 1", cla.GetArg("bl"));
		if (cla.IsArg("br")) corners.Add("bottom-right 1", cla.GetArg("br"));
		
		if (!frame_path.IsEmpty() && !window_path.IsEmpty()) {
			RunScenario(frame_path, window_path, corners);
			return;
		}
	}

	Cout() << "Usage: OrbTest --frame <path> --window <path> [--tl <path>] [--tr <path>] ...
";
}
