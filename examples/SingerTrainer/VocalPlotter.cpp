#include "VocalPlotter.h"
#include "ExerciseEngine.h"

namespace Upp {

void VocalPlotter::Paint(Draw& w) {
	Size sz = GetSize();
	w.DrawRect(sz, SWhite);
	
	if (!engine) return;
	
	const Vector<ExerciseNode>& nodes = engine->GetNodes();
	double duration = engine->GetDuration();
	if (duration <= 0) duration = 1.0;
	
	auto TimeToX = [&](double t) { return (int)(t / duration * sz.cx); };
	auto ModeToY = [&](double m) { return (int)((1.0 - m / 7.0) * sz.cy); };

	// Draw Target Path
	if (nodes.GetCount() > 1) {
		for (int i = 0; i < nodes.GetCount() - 1; i++) {
			int x1 = TimeToX(nodes[i].time);
			int y1 = ModeToY((double)nodes[i].mode);
			int x2 = TimeToX(nodes[i+1].time);
			int y2 = ModeToY((double)nodes[i+1].mode);
			w.DrawLine(x1, y1, x2, y2, 2, LtBlue());
		}
	}

	// Draw Nodes
	for (const auto& n : nodes) {
		int x = TimeToX(n.time);
		int y = ModeToY((double)n.mode);
		w.DrawEllipse(x - 5, y - 5, 10, 10, SRed, 1, SBlack);
		w.DrawText(x + 5, y + 5, Format("P:%.0f", n.pitch), StdFont(10), SGray);
	}
	
	// Draw Cursor
	int curX = TimeToX(engine->GetCurrentTime());
	w.DrawLine(curX, 0, curX, sz.cy, 2, SGreen);
	
	// Draw Y-axis labels
	const char* labels[] = { "FRY", "MODAL", "COMPRESSED", "HEAD", "FALSETTO", "SUBHARMONIC", "DISTORTION" };
	for (int i = 0; i < 7; i++) {
		w.DrawText(5, ModeToY((double)i) - 12, labels[i], StdFont(8).Italic(), SGray);
	}
}

}