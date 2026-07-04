#ifndef _VisualStateModel_Types_h_
#define _VisualStateModel_Types_h_

namespace Upp {

// ---------------------------------------------------------------------------
// Primitive aliases

typedef String VsmRegionId;

// ---------------------------------------------------------------------------
// Region fingerprint

struct VsmRegionFingerprint : Moveable<VsmRegionFingerprint> {
	String hash; // e.g. "sha1:a3f8c..."
	String file; // optional path to 32x32 grayscale .fp.bin

	void Jsonize(JsonIO& json);
};

// ---------------------------------------------------------------------------
// Changed rectangle inside a change event

struct VsmChangedRect : Moveable<VsmChangedRect> {
	int    x = 0, y = 0, w = 0, h = 0;
	double score = 0.0; // fraction of pixels changed (0..1)

	Rect GetRect() const { return Rect(x, y, x + w, y + h); }
	void Jsonize(JsonIO& json);
};

// ---------------------------------------------------------------------------
// Change event — pixel-level changes between frame N-1 and N

struct VsmChangeEvent : Moveable<VsmChangeEvent> {
	int    frame = -1;
	String ts;
	Vector<VsmChangedRect> regions;

	void Jsonize(JsonIO& json);
};

// ---------------------------------------------------------------------------
// Region node — a stable visual region across frames

struct VsmRegionNode : Moveable<VsmRegionNode> {
	VsmRegionId         id;
	VsmRegionId         parent_id;
	int                 x = 0, y = 0, w = 0, h = 0;
	String              action;      // created/moved/resized/merged/split/closed
	VsmRegionFingerprint fingerprint;
	String              label;       // from annotation (may be empty)
	int                 frame = -1;
	String              ts;
	int                 expected_child_count = -1;  // -1 = not applicable (fixed single region)

	Rect GetRect() const { return Rect(x, y, x + w, y + h); }
	void Jsonize(JsonIO& json);
};

// ---------------------------------------------------------------------------
// Frame reference

struct VsmFrameRef : Moveable<VsmFrameRef> {
	int    frame = -1;
	String ts;
	String image_file; // optional, relative path

	void Jsonize(JsonIO& json);
};

// ---------------------------------------------------------------------------
// OCR observation

struct VsmOcrObservation : Moveable<VsmOcrObservation> {
	int    frame         = -1;
	String ts;
	VsmRegionId region_id;
	int    trigger_frame = -1;
	String text;
	double confidence    = 0.0;
	String engine;
	String crop_file;

	void Jsonize(JsonIO& json);
};

// ---------------------------------------------------------------------------
// Template match observation

struct VsmTemplateObservation : Moveable<VsmTemplateObservation> {
	int    frame = -1;
	String ts;
	VsmRegionId region_id;
	String template_name;
	double score = 0.0;
	int    mx = 0, my = 0, mw = 0, mh = 0;
	String crop_file;

	Rect GetMatchRect() const { return Rect(mx, my, mx + mw, my + mh); }
	void Jsonize(JsonIO& json);
};

// ---------------------------------------------------------------------------
// Model state snapshot — key-value string map stored as raw JSON

struct VsmModelStateRef : Moveable<VsmModelStateRef> {
	int    frame      = -1;
	String ts;
	String state_json; // serialized JSON object { "key": "value", ... }

	void Jsonize(JsonIO& json);
};

// ---------------------------------------------------------------------------
// Divergence record — expected vs observed model state

struct VsmDivergence : Moveable<VsmDivergence> {
	int    frame = -1;
	String ts;
	String severity;      // warning / error / fatal
	String message;
	VsmRegionId region_id;
	String expected_json;
	String observed_json;

	void Jsonize(JsonIO& json);
};

} // namespace Upp

#endif
