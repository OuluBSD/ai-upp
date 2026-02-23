#include <ComputerVision/ComputerVision.h>

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
	if (good > 0 && corners == 4) {
		Cout() << " - OK\n";
	} else {
		Cout() << " - FAILED\n";
	}
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
	
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
