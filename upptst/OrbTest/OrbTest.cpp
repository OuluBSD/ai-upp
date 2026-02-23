#include <ComputerVision/ComputerVision.h>
#include <plugin/jpg/jpg.h>
#include <plugin/png/png.h>

using namespace Upp;

Image CreatePattern(int w, int h) {
	SImageDraw iw(w, h);
	iw.DrawRect(0, 0, w, h, White());
	// Add some distinct patterns for keypoints
	iw.DrawRect(5, 5, 10, 10, Black());
	iw.DrawRect(20, 10, 20, 5, Blue());
	iw.DrawEllipse(10, 30, 15, 10, Red());
	iw.DrawRect(30, 30, 5, 15, Green());
	iw.DrawRect(15, 15, 2, 2, Magenta());
	iw.DrawRect(40, 5, 3, 3, Cyan());
	return iw;
}

Image CreateScene(const Image& pattern, int sw, int sh, int px, int py, double scale = 1.0) {
	SImageDraw iw(sw, sh);
	iw.DrawRect(0, 0, sw, sh, GrayColor(128));
	Size psz = pattern.GetSize();
	int tw = (int)(psz.cx * scale);
	int th = (int)(psz.cy * scale);
	iw.DrawImage(px, py, tw, th, pattern);
	return iw;
}

void RunTest(OrbSystem& orb, const Image& scene, const char* name) {
	orb.SetInput(scene);
	orb.Process();

	int matches = orb.GetLastMatchCount();
	int good = orb.GetLastGoodMatches();
	int corners = orb.GetLastCorners().GetCount();

	Cout() << "TEST [" << name << "]: matches=" << matches << " good=" << good << " corners=" << corners << "\n";
	
	const Vector<float>& H = orb.GetHomo();
	Cout() << "Homography: [" << H[0] << ", " << H[1] << ", " << H[2] << "; "
	       << H[3] << ", " << H[4] << ", " << H[5] << "; "
	       << H[6] << ", " << H[7] << ", " << H[8] << "]\n";

	if (good >= 4 && corners == 4) {
		Cout() << " - OK\n";
	} else {
		Cout() << " - FAILED\n";
	}
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);

	CommandLineArguments cla;
	cla.AddArg("frame", 0, "Path to the frame image (JPG/PNG)", true);
	cla.AddArg("pattern", 0, "Path to the pattern image (JPG/PNG)", true);
	cla.AddArg("test-random", 0, "Extract random 120x48 from frame and find it", false);
	
	if (cla.Parse()) {
		String frame_path = cla.GetArg("frame");
		String pattern_path = cla.GetArg("pattern");
		bool test_random = cla.IsArg("test-random");
		
		if (!frame_path.IsEmpty()) {
			Image frame = StreamRaster::LoadFileAny(frame_path);
			if (frame.IsEmpty()) {
				Cout() << "Error: Failed to load frame: " << frame_path << "\n";
				return;
			}
			
			if (test_random) {
				Size fsz = frame.GetSize();
				int pw = 120;
				int ph = 48;
				if (fsz.cx < pw || fsz.cy < ph) {
					Cout() << "Error: Frame too small for random test (" << fsz << " < " << pw << "x" << ph << ")\n";
					return;
				}
				
				int rx = Random(fsz.cx - pw);
				int ry = Random(fsz.cy - ph);
				
				ImageBuffer ib(pw, ph);
				Copy(ib, Point(0, 0), frame, RectC(rx, ry, pw, ph));
				Image pattern = ib;
				
				Cout() << "Random test: extracted " << pw << "x" << ph << " pattern from (" << rx << ", " << ry << ")\n";
				
				OrbSystem orb;
				orb.SetInput(pattern);
				orb.InitDefault();
				
				RunTest(orb, frame, "Random Pattern Find");
				
				const Vector<Pointf>& corners = orb.GetLastCorners();
				if (corners.GetCount() >= 4) {
					double minx = corners[0].x, miny = corners[0].y;
					for (int i = 1; i < corners.GetCount(); i++) {
						minx = min(minx, (double)corners[i].x);
						miny = min(miny, (double)corners[i].y);
					}
					Cout() << "Found at approx: (" << (int)minx << ", " << (int)miny << ")\n";
					Cout() << "Original was: (" << rx << ", " << ry << ")\n";
				}
				return;
			}
			
			if (!pattern_path.IsEmpty()) {
				Image pattern = StreamRaster::LoadFileAny(pattern_path);
				if (pattern.IsEmpty()) {
					Cout() << "Error: Failed to load pattern: " << pattern_path << "\n";
				} else {
					Cout() << "Loaded frame " << frame.GetSize() << " and pattern " << pattern.GetSize() << "\n";
					OrbSystem orb;
					orb.SetInput(pattern);
					orb.InitDefault();
					
					RunTest(orb, frame, "CLI Repro");
				
				const Vector<float>& H = orb.GetHomo();
				Cout() << "Homography: [" << H[0] << ", " << H[1] << ", " << H[2] << "; "
				       << H[3] << ", " << H[4] << ", " << H[5] << "; "
				       << H[6] << ", " << H[7] << ", " << H[8] << "]\n";
				
				const Vector<Pointf>& corners = orb.GetLastCorners();
				if (corners.GetCount() >= 4) {
					double minx = corners[0].x, miny = corners[0].y;
					for (int i = 1; i < corners.GetCount(); i++) {
						minx = min(minx, (double)corners[i].x);
						miny = min(miny, (double)corners[i].y);
					}
					Cout() << "Found at approx: (" << (int)minx << ", " << (int)miny << ")\n";
				}
				}
				return;
			}
		}
	}

	// Default synthetic tests
	OrbSystem orb;

	int pw = 48, ph = 48;
	Image pattern = CreatePattern(pw, ph);

	Cout() << "Training pattern " << pw << "x" << ph << "...\n";
	orb.SetInput(pattern);
	orb.InitDefault();

	int sw = 640, sh = 480;

	RunTest(orb, CreateScene(pattern, sw, sh, 100, 100, 1.0), "Identity");
	RunTest(orb, CreateScene(pattern, sw, sh, 300, 200, 1.0), "Translation");
	RunTest(orb, CreateScene(pattern, sw, sh, 100, 100, 1.5), "Scale Up 1.5x");
	RunTest(orb, CreateScene(pattern, sw, sh, 100, 100, 0.75), "Scale Down 0.75x");

	// Test with noise
	Image scene_noise = CreateScene(pattern, sw, sh, 100, 100, 1.0);
	ImageBuffer ib(scene_noise);
	for (RGBA& rgba : ib) {
		int noise = (Random(21) - 10);
		rgba.r = (byte)clamp((int)rgba.r + noise, 0, 255);
		rgba.g = (byte)clamp((int)rgba.g + noise, 0, 255);
		rgba.b = (byte)clamp((int)rgba.b + noise, 0, 255);
	}
	RunTest(orb, ib, "Identity + Noise");
}
