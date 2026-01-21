#include "Maestro.h"

#ifdef flagGUI

NAMESPACE_UPP

struct ColoredDisplay : public Display {
	virtual void Paint(Draw& w, const Rect& r, const Value& v, Color fg, Color bg, dword style) const {
		Color color = fg;
		String status = v;
		if(status == "done") color = Green();
		else if(status == "in_progress") color = Yellow();
		else if(status == "blocked") color = Red();
		
		w.DrawRect(r, bg);
		w.DrawText(r.left, r.top, status, StdFont(), color);
	}
};

PlanView::PlanView() {
	Add(tree.SizePos());
}

void PlanView::Set(const Array<Track>& tracks) {
	tree.Clear();
	int root_id = tree.Add(0, CtrlImg::Dir(), "Plans");
	for(const auto& t : tracks) {
		int tid = tree.Add(root_id, CtrlImg::Dir(), t.name);
		for(const auto& p : t.phases) {
			int pid = tree.Add(tid, CtrlImg::Dir(), p.name);
			for(const auto& tk : p.tasks) {
				tree.Add(pid, CtrlImg::File(), tk.name);
			}
		}
	}
	tree.OpenDeep(root_id);
}

END_UPP_NAMESPACE

#endif
