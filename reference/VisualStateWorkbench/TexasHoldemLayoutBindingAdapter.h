#ifndef _VisualStateWorkbench_TexasHoldemLayoutBindingAdapter_h_
#define _VisualStateWorkbench_TexasHoldemLayoutBindingAdapter_h_

// ---------------------------------------------------------------------------
// TexasHoldem layout-binding adapter (task 0132 / M06-02).
//
// GUI-independent (NO CtrlLib / Ctrl dependency): computes, for one frame of a
// loaded TexasHoldem M01/M02 session (task 0131's VsmTexasHoldemSession), the
// SAME per-region layout binding data reference/VisualStateLayoutAssign's CLI
// computes headlessly — which detected changed region got matched to which
// `.form` layout element / sub-slot, with what role / seat_index / card_index
// / overlap score.
//
// This does NOT re-implement any matching logic: it loads the `.form` file and
// calls the exact same shared library functions the CLI calls —
//   VsmParseFormFile        (uppsrc/VisualStateModel/FormLayout.h)
//   VsmBuildLayoutProfile   (uppsrc/VisualStateModel/LayoutProfile.h)
//   VsmBuildCandidates      (uppsrc/VisualStateModel/RegionAssign.h)
//   VsmMatchRegion          (uppsrc/VisualStateModel/RegionAssign.h)
// — with the same design-space-to-frame scaling convention (VsmScaleRect via
// VsmBuildCandidates, using sx/sy derived from the actual decoded frame size,
// exactly as reference/VisualStateLayoutAssign/main.cpp does). Because it also
// reuses task 0131's already-detected region nodes (VsmTexasHoldemSession::
// regions — same VsmChangeDetectParams / VsmRegionMemory replay the CLI uses),
// the region_id / rect / detection-order it emits are identical to the CLI's
// for the same session/frame, so a headless cross-check can agree with the CLI
// byte-for-byte on role / seat_index / card_index / rect / overlap (see this
// task's evidence section).
//
// Being CtrlLib-free, this whole adapter can be exercised from a
// CONSOLE/GUI_APP_MAIN harness (the project's "Headless/GUI Dual-Purpose
// Rule"), which is how it is verified — the GUI panel that consumes it cannot
// be driven by a background agent.
// ---------------------------------------------------------------------------

#include "TexasHoldemSessionAdapter.h"

namespace Upp {

// Frame-independent layout model for one session: the parsed/classified
// `.form` profile plus its match candidates already scaled into this session's
// actual frame-pixel space (elements + resolved sub-slots, one flat list, the
// exact output of VsmBuildCandidates). Built once per opened session; the
// scale factor is derived from the session's actual decoded frame-0 size, the
// same source reference/VisualStateLayoutAssign uses (NOT metadata.json's
// declared table size — see that CLI's scaling note).
struct VsmSessionLayoutModel {
	bool   loaded = false;
	String error;
	String form_path;

	VsmLayoutProfile           profile;      // design-space profile (elements + subslots, roles)
	Vector<VsmLayoutCandidate> candidates;   // frame-space-scaled candidates (elements + subslots)

	int    frame_width  = 0;   // actual decoded frame size used for scaling
	int    frame_height = 0;
	double sx = 1.0, sy = 1.0; // frame / profile design-space scale (per axis)

	int    element_count = 0;  // profile.elements.GetCount()
	int    subslot_count = 0;  // profile.subslots.GetCount()

	bool IsEmpty() const { return !loaded; }
};

// One region-to-layout binding for a single changed region of one frame. The
// region side (region_index / region_id / x / y / w / h) is copied straight
// from the session's already-detected region node so it lines up 1:1 with the
// FrameCanvas changed-region overlay and the Regions list (region_index is the
// same "region-N" index the canvas fires via WhenRegionSelected). The layout
// side is the VsmMatchRegion result against the session layout model's
// candidates — identical fields to the CLI's VsmLayoutObservationOut.
struct VsmLayoutBinding : Moveable<VsmLayoutBinding> {
	int    region_index = -1;   // index into VsmTexasHoldemSession::RegionsForFrame(frame)
	String region_id;           // stable rgn-%04d id (may repeat within a frame, as in the CLI)
	int    x = 0, y = 0, w = 0, h = 0; // region rect in frame pixels

	bool   matched = false;     // true if a candidate cleared kOverlapThreshold
	String assigned = "unassigned"; // matched candidate label, or "unassigned"
	String kind     = "unassigned"; // "element" / "subslot" / "unassigned"
	String role     = "unassigned"; // matched candidate role, or "unassigned"
	int    seat_index = -1;
	int    card_index = -1;
	double overlap    = 0.0;

	Rect   candidate_rect = Rect(0, 0, 0, 0); // matched candidate's frame-space rect (valid iff matched)

	// Same field set (and names) the CLI's VsmLayoutObservationOut emits, minus
	// the change-detection `score` (a region-detection artifact this adapter
	// does not carry) and the frame_prev/frame pair (supplied by the caller,
	// which knows the frame id). Lets a headless harness serialize a binding
	// and diff it against the CLI's JSONL on the layout-relevant fields.
	void Jsonize(JsonIO& json)
	{
		json
			("region_id", region_id)
			("x", x)("y", y)("w", w)("h", h)
			("assigned", assigned)("kind", kind)("role", role)
			("seat_index", seat_index)("card_index", card_index)
			("overlap", overlap)
		;
	}
};

// Build the frame-independent layout model for `session` from `form_path`.
// Reuses VsmParseFormFile / VsmBuildLayoutProfile / VsmBuildCandidates and
// derives the frame/profile scale from the session's actual decoded frame-0
// size (VsmLoadTexasHoldemFrameImage). On failure sets `.loaded=false` and
// `.error`. Purely headless.
VsmSessionLayoutModel VsmBuildSessionLayoutModel(const VsmTexasHoldemSession& session,
                                                 const String& form_path);

// Best-effort locate the provider's catalog-defined `.form` layout file,
// searching `search_dirs` in order and returning the first candidate that
// exists on disk (empty String if none). GUI-independent so the workbench's
// form-path resolution can be verified headlessly; the workbench supplies
// exe-relative + cwd-relative search roots (the `.form` files live under
// game/TexasHoldem/ in the repo).
String VsmDefaultFormPathForProvider(const String& provider,
                                     const Vector<String>& search_dirs);

// Compute the layout bindings for one frame: one VsmLayoutBinding per changed
// region of `frame_id` (in the session's stable detection order, i.e. the same
// order VsmTexasHoldemSession::RegionsForFrame returns and the FrameCanvas
// overlay draws), each matched against `model.candidates` via VsmMatchRegion.
// Returns an empty vector for frame 0 (no predecessor -> no changed regions)
// or if the model is not loaded.
Vector<VsmLayoutBinding> VsmComputeFrameBindings(const VsmTexasHoldemSession& session,
                                                 const VsmSessionLayoutModel& model,
                                                 int frame_id);

} // namespace Upp

#endif
