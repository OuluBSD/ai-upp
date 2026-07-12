#include "TexasHoldemLayoutBindingAdapter.h"

namespace Upp {

VsmSessionLayoutModel VsmBuildSessionLayoutModel(const VsmTexasHoldemSession& session,
                                                 const String& form_path)
{
	VsmSessionLayoutModel model;
	model.form_path = form_path;

	if(session.IsEmpty()) {
		model.error = "session not loaded";
		return model;
	}

	// --- Parse + classify the .form layout (same calls as the CLI). ---
	Vector<VsmFormLayout> layouts = VsmParseFormFile(form_path);
	if(layouts.IsEmpty()) {
		model.error = "failed to parse any <layouts><item> from: " + form_path;
		return model;
	}
	model.profile = VsmBuildLayoutProfile(layouts[0]);
	model.element_count = model.profile.elements.GetCount();
	model.subslot_count = model.profile.subslots.GetCount();

	if(model.profile.width <= 0 || model.profile.height <= 0) {
		model.error = "layout profile has zero/negative design-space size";
		return model;
	}

	// --- Derive the frame/profile scale from the ACTUAL decoded frame-0 size,
	// exactly as reference/VisualStateLayoutAssign/main.cpp does (do NOT trust
	// metadata.json's declared table size — see that CLI's scaling note). ---
	VsmFrameImage probe;
	if(!VsmLoadTexasHoldemFrameImage(session, 0, probe)) {
		model.error = "failed to decode frame 0 for scale derivation";
		return model;
	}
	model.frame_width  = probe.width;
	model.frame_height = probe.height;
	model.sx = (double)probe.width  / model.profile.width;
	model.sy = (double)probe.height / model.profile.height;

	// --- Flatten + scale candidates (shared library call, same as the CLI). ---
	model.candidates = VsmBuildCandidates(model.profile, model.sx, model.sy);

	model.loaded = true;
	return model;
}

String VsmDefaultFormPathForProvider(const String& provider,
                                     const Vector<String>& search_dirs)
{
	if(provider.IsEmpty())
		return String();
	String filename = "GameTable_" + provider + ".form";
	for(const String& dir : search_dirs) {
		if(dir.IsEmpty())
			continue;
		String cand = NormalizePath(AppendFileName(dir, filename));
		if(FileExists(cand))
			return cand;
	}
	return String();
}

Vector<VsmLayoutBinding> VsmComputeFrameBindings(const VsmTexasHoldemSession& session,
                                                 const VsmSessionLayoutModel& model,
                                                 int frame_id)
{
	Vector<VsmLayoutBinding> out;
	if(session.IsEmpty() || !model.loaded)
		return out;

	// Reuse task 0131's already-detected region nodes for this frame, in the
	// same stable detection order the FrameCanvas overlay draws them (so
	// region_index == the canvas "region-N" click index).
	Vector<const VsmRegionNode*> nodes = session.RegionsForFrame(frame_id);
	for(int i = 0; i < nodes.GetCount(); i++) {
		const VsmRegionNode* rn = nodes[i];

		VsmLayoutBinding b;
		b.region_index = i;
		b.region_id    = rn->id;
		b.x = rn->x; b.y = rn->y; b.w = rn->w; b.h = rn->h;

		Rect region_rect(rn->x, rn->y, rn->x + rn->w, rn->y + rn->h);
		VsmMatchResult m = VsmMatchRegion(region_rect, model.candidates);
		if(m.best) {
			b.matched        = true;
			b.assigned       = m.best->label;
			b.kind           = m.best->kind;
			b.role           = m.best->role;
			b.seat_index     = m.best->seat_index;
			b.card_index     = m.best->card_index;
			b.overlap        = m.overlap;
			b.candidate_rect = m.best->rect;
		}
		out.Add(pick(b));
	}
	return out;
}

} // namespace Upp
