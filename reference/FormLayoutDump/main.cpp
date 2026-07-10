#include <VisualStateModel/VisualStateModel.h>

using namespace Upp;

// Minimal CLI for task 0112: dumps the element list a .form file parses
// into via uppsrc/VisualStateModel/FormLayout.h, as plain text. This is only
// a demonstration/test harness for that parser (the reusable model code
// lives in uppsrc/VisualStateModel/FormLayout.h/.cpp) — it does not do any
// region-to-element assignment (out of scope for task 0112).
//
// Usage: FormLayoutDump.exe <path-to-.form>

CONSOLE_APP_MAIN
{
	const Vector<String>& args = CommandLine();
	if(args.GetCount() < 1) {
		Cout() << "Usage: FormLayoutDump <path-to-.form>\n";
		SetExitCode(1);
		return;
	}

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
		}
	}

	SetExitCode(0);
}
