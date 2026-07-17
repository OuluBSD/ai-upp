#ifndef _VideoChangedRegionEnricher_VideoChangedRegionEnricher_h_
#define _VideoChangedRegionEnricher_VideoChangedRegionEnricher_h_

#include <Core/Core.h>
#include <Draw/Draw.h>
#include <plugin/jpg/jpg.h>

NAMESPACE_UPP

struct EnricherOptions {
	String manifest;
	String out_dir;
	// Task 0277: when supplied, OcrEvidence() reads OCR results from this
	// exact path instead of guessing <tracker_dir>/ocr_probe.json. Real
	// VideoSemanticOcrProbe runs write wherever --out says, which is
	// essentially never the guessed default in practice -- see AGENTS.md.
	String ocr_probe_json;
	bool help = false;
};

END_UPP_NAMESPACE

#endif
