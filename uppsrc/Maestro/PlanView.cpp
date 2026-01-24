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
	tree.WhenBar = [=](Bar& bar) {
		int id = tree.GetCursor();
		if(id < 0) return;
		
		Value v = tree.Get(id);
		if(v.Is<ValueMap>()) {
			ValueMap m = v;
			String type = m["type"];
			if(type == "task") {
				bar.Add("Enact Task", [=] {
					if(WhenEnact) WhenEnact(m["track"], m["phase"], m["task"]);
				});
			}
		}
	};
}

void PlanView::Set(const Array<Track>& tracks) {
	tree.Clear();
	int root_id = tree.Add(0, CtrlImg::Dir(), "Plans");
	for(const auto& t : tracks) {
		ValueMap vmTrack;
		vmTrack.Add("type", "track");
		vmTrack.Add("track", t.id);
		
		int tid = tree.Add(root_id, CtrlImg::Dir(), vmTrack, t.name);
		for(const auto& p : t.phases) {
			ValueMap vmPhase;
			vmPhase.Add("type", "phase");
			vmPhase.Add("track", t.id);
			vmPhase.Add("phase", p.id);
			
			int pid = tree.Add(tid, CtrlImg::Dir(), vmPhase, p.name);
			for(const auto& tk : p.tasks) {
				ValueMap vmTask;
				vmTask.Add("type", "task");
				vmTask.Add("track", t.id);
				vmTask.Add("phase", p.id);
				vmTask.Add("task", tk.id);
				
				tree.Add(pid, CtrlImg::File(), vmTask, tk.name);
			}
		}
	}
	tree.OpenDeep(root_id);
}

END_UPP_NAMESPACE

#endif