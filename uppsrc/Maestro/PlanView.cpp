#include "Maestro.h"

namespace Upp {

struct ColoredDisplay : public Display {
	Color color;
	virtual void Paint(Draw& w, const Rect& r, const Value& v, Color fg, Color bg, dword style) const {
		Display::Paint(w, r, v, color, bg, style);
	}
};

static ColoredDisplay& StatusDisplay(Color c) {
	static VectorMap<Color, One<ColoredDisplay>> cache;
	int q = cache.Find(c);
	if(q < 0) {
		ColoredDisplay *d = new ColoredDisplay;
		d->color = c;
		cache.Add(c, d);
		return *d;
	}
	return *cache[q];
}

PlanView::PlanView() {
	Add(tree.SizePos());
}

void PlanView::Set(const Array<Track>& tracks) {
	tree.Clear();
	int root = tree.Add(0, CtrlImg::Dir(), "Planning");
	
	for(const auto& t : tracks) {
		int tid = tree.Add(root, CtrlImg::Dir(), t.name);
		
		for(const auto& p : t.phases) {
			int pid = tree.Add(tid, CtrlImg::Dir(), p.name);
			
			for(const auto& task : p.tasks) {
				String label = task.name + " [" + StatusToString(task.status) + "]";
				Color clr = SColorText();
				
				switch(task.status) {
					case STATUS_DONE: clr = Green(); break;
					case STATUS_IN_PROGRESS: clr = LtYellow(); break;
					case STATUS_BLOCKED: clr = Red(); break;
					default: clr = SColorDisabled(); break;
				}
				
				int id = tree.Add(pid, CtrlImg::File(), label);
				tree.SetDisplay(id, StatusDisplay(clr));
			}
		}
	}
	
	tree.Open(root);
}

}