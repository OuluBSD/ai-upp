#include "VsmOcrEngineSmoke.h"

NAMESPACE_UPP

// Task 0277 Part B verification tool: loads real crop JPGs into a
// VsmFrameImage (the in-memory RGBA type VsmObservationPipeline actually
// passes to a VsmOcrEngine), runs each through a real VsmTesseractOcrEngine,
// and prints actual vs expected text -- direct proof the engine works
// end-to-end, not just that it compiles.
//
// Loading: VsmLoadPngFrame() (PngFrame.h) is implemented over
// StreamRaster::LoadFileAny() despite its name, so it decodes any raster
// format plugin/jpg (linked into this tool -- see VsmOcrEngineSmoke.upp)
// registers, including JPG -- confirmed by this tool's own real run against
// real .jpg crop files, not assumed.

struct SmokeCase : Moveable<SmokeCase> {
	String path;
	String semantic;
	String expected;
};

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN
{
	const Vector<String>& args = CommandLine();

	Vector<SmokeCase> cases;
	if(args.IsEmpty()) {
		SmokeCase& c1 = cases.Add();
		c1.path = "tmp/_task0274_refined_crops/h1_f36_balance.jpg";
		c1.semantic = "pot_label";
		c1.expected = "Pot: 25.7 BB";

		SmokeCase& c2 = cases.Add();
		c2.path = "tmp/_task0274_refined_crops/h1_f52_pot.jpg";
		c2.semantic = "pot_label";
		c2.expected = "213.8 BB";
	}
	else {
		for(int i = 0; i + 1 < args.GetCount(); i += 3) {
			SmokeCase& c = cases.Add();
			c.path = args[i];
			c.semantic = i + 1 < args.GetCount() ? args[i + 1] : String();
			c.expected = i + 2 < args.GetCount() ? args[i + 2] : String();
		}
	}

	VsmTesseractOcrEngine engine;
	VsmOcrEngineInfo info = engine.GetInfo();
	Cout() << "engine name=" << info.name << " version=" << info.version
	       << " available=" << (info.available ? "true" : "false") << "\n";
	if(!info.available) {
		Cerr() << "ERROR: VsmTesseractOcrEngine reports unavailable "
		          "(tesseract/tessdata not resolved) -- smoke test cannot proceed\n";
		SetExitCode(1);
		return;
	}

	bool all_ok = true;
	for(const SmokeCase& c : cases) {
		Cout() << "--- " << c.path << " (semantic=" << c.semantic << ") ---\n";
		if(!FileExists(c.path)) {
			Cerr() << "ERROR: file does not exist: " << c.path << "\n";
			all_ok = false;
			continue;
		}

		VsmFrameImage frame;
		bool loaded = VsmLoadPngFrame(c.path, frame);
		Cout() << "loaded=" << (loaded ? "true" : "false")
		       << " width=" << frame.width << " height=" << frame.height << "\n";
		if(!loaded || frame.IsEmpty()) {
			Cerr() << "ERROR: failed to decode crop into VsmFrameImage: " << c.path << "\n";
			all_ok = false;
			continue;
		}

		VsmOcrRequest req;
		req.rule_id = "smoke-rule";
		req.region_id = "smoke-region";
		req.semantic = c.semantic;
		req.frame = 0;

		VsmOcrResult result = engine.Execute(frame, req);
		bool match = !c.expected.IsEmpty() && result.text == c.expected;
		Cout() << "actual_text=\"" << result.text << "\"\n";
		Cout() << "confidence=" << result.confidence << " ts=" << result.ts << "\n";
		if(!c.expected.IsEmpty()) {
			Cout() << "expected_text=\"" << c.expected << "\"\n";
			Cout() << (match ? "MATCH\n" : "MISMATCH\n");
			if(!match)
				all_ok = false;
		}
	}

	Cout() << (all_ok ? "smoke: ALL OK\n" : "smoke: FAILURES\n");
	if(!all_ok)
		SetExitCode(1);
}
