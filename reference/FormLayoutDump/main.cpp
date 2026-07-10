#include <VisualStateModel/VisualStateModel.h>

using namespace Upp;

// Minimal CLI for task 0112: dumps the element list a .form file parses
// into via uppsrc/VisualStateModel/FormLayout.h, as plain text. This is only
// a demonstration/test harness for that parser (the reusable model code
// lives in uppsrc/VisualStateModel/FormLayout.h/.cpp) — it does not do any
// region-to-element assignment (out of scope for task 0112).
//
// Task 0114 extends this same demo CLI (rather than adding a new one, per
// its "keep it simple" guardrail) to also dump the M04-03 `VsmLayoutProfile`
// artifact (uppsrc/VisualStateModel/LayoutProfile.h) — every element/sub-slot
// tagged with its classified role. Pass `--json` as a second argument to
// print the profile as JSON (via VsmLayoutProfile::Jsonize) instead of the
// default readable-text dump.
//
// Usage: FormLayoutDump.exe <path-to-.form> [--json]

CONSOLE_APP_MAIN
{
	const Vector<String>& args = CommandLine();
	if(args.GetCount() < 1) {
		Cout() << "Usage: FormLayoutDump <path-to-.form> [--json]\n";
		SetExitCode(1);
		return;
	}
	bool json_mode = args.GetCount() > 1 && args[1] == "--json";

	String path = args[0];
	Vector<VsmFormLayout> layouts = VsmParseFormFile(path);
	if(layouts.IsEmpty()) {
		Cout() << "FAILED to parse any <layouts><item> from: " << path << "\n";
		SetExitCode(1);
		return;
	}

	Cout() << "Parsed " << path << ": " << layouts.GetCount() << " layout(s)\n";
	for(const VsmFormLayout& layout : layouts) {
		Cout() << "== layout \"" << layout.name << "\" "
		       << layout.width << "x" << layout.height
		       << " scale_mode=" << layout.scale_mode
		       << " elements=" << layout.elements.GetCount() << "\n";

		for(const VsmFormElement& el : layout.elements) {
			Rect r = el.GetRect();
			Cout() << "  " << el.name
			       << " type=" << el.type
			       << " variable=" << el.variable
			       << " parent=" << (el.parent.IsEmpty() ? String("-") : el.parent)
			       << " rect=(" << r.left << "," << r.top << "," << r.Width() << "x" << r.Height() << ")";
			if(!el.parent.IsEmpty()) {
				Rect ar = layout.GetAbsoluteRect(el);
				Cout() << " abs_rect=(" << ar.left << "," << ar.top << "," << ar.Width() << "x" << ar.Height() << ")";
			}
			Cout() << "\n";

			// M04-02 (task 0113) demonstration: if this element's Type has
			// known sub-slots (VsmFormSubSlot, VisualStateModel/FormLayout.h),
			// dump each sub-slot's resolved absolute rect too. This is only a
			// demonstration/test harness for the resolver, same spirit as the
			// rest of this CLI (task 0112).
			Vector<VsmFormSubSlot> subslots = VsmGetSubSlots(el.type);
			if(!subslots.IsEmpty()) {
				Rect owner_abs = layout.GetAbsoluteRect(el);
				for(const VsmFormSubSlot& s : subslots) {
					Rect sr = VsmResolveSubSlot(s, owner_abs);
					Cout() << "    subslot " << s.name
					       << " rect=(" << sr.left << "," << sr.top << "," << sr.Width() << "x" << sr.Height() << ")\n";
				}
			}
		}

		// M04-03 (task 0114) demonstration: the VsmLayoutProfile artifact —
		// every element/sub-slot tagged with its classified role (see
		// uppsrc/VisualStateModel/LayoutProfile.h). Same "demo harness, not
		// the reusable model" spirit as the rest of this CLI.
		VsmLayoutProfile profile = VsmBuildLayoutProfile(layout);
		if(json_mode) {
			Cout() << StoreAsJson(profile, true) << "\n";
			continue;
		}

		Cout() << "-- layout profile (roles) --\n";
		for(const VsmLayoutElementInfo& e : profile.elements) {
			Cout() << "  " << e.name << " role=" << e.role
			       << " seat_index=" << e.seat_index
			       << " rect=(" << e.x << "," << e.y << "," << e.cx << "x" << e.cy << ")\n";
		}
		for(const VsmLayoutSubSlotInfo& s : profile.subslots) {
			Cout() << "    " << s.owner_name << "." << s.slot_name
			       << " role=" << s.role
			       << " seat_index=" << s.seat_index
			       << " card_index=" << s.card_index
			       << " rect=(" << s.x << "," << s.y << "," << s.cx << "x" << s.cy << ")\n";
		}
	}

	SetExitCode(0);
}
