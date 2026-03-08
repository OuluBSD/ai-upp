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
	tree.AddColumn("Item", 300);
	tree.AddColumn("Status", 100);
	tree.AddColumn("Progress", 80);
	
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
	int root_id = tree.Add(0, CtrlImg::Dir(), "Plans", "active");
	tree.SetRowValue(root_id, 1, "Active");
	
	for(const auto& t : tracks) {
		ValueMap vmTrack;
		vmTrack.Add("type", "track");
		vmTrack.Add("track", t.id);
		
		int tid = tree.Add(root_id, CtrlImg::Dir(), vmTrack, t.name);
		tree.SetRowValue(tid, 1, t.status);
		tree.SetRowValue(tid, 2, Format("%d%%", t.completion));
		
		for(const auto& p : t.phases) {
			ValueMap vmPhase;
			vmPhase.Add("type", "phase");
			vmPhase.Add("track", t.id);
			vmPhase.Add("phase", p.id);
			
			int pid = tree.Add(tid, CtrlImg::Dir(), vmPhase, p.name);
			tree.SetRowValue(pid, 1, p.status);
			tree.SetRowValue(pid, 2, Format("%d%%", p.completion));
			
			for(const auto& tk : p.tasks) {
				ValueMap vmTask;
				vmTask.Add("type", "task");
				vmTask.Add("track", t.id);
				vmTask.Add("phase", p.id);
				vmTask.Add("task", tk.id);
				
				int tkid = tree.Add(pid, CtrlImg::File(), vmTask, tk.name);
				tree.SetRowValue(tkid, 1, StatusToString(tk.status));
			}
		}
	}
	tree.Open(root_id); // Just open the root by default
}

END_UPP_NAMESPACE

#endif