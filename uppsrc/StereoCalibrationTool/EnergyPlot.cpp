#include "EnergyPlot.h"

NAMESPACE_UPP

EnergyPlot::EnergyPlot() {
	Transparent();
}

void EnergyPlot::Clear() {
	history_costs.Clear();
	initial_cost = 0;
	replay_index = -1;
	Refresh();
}

void EnergyPlot::SetInitialCost(double cost) {
	initial_cost = cost;
}

void EnergyPlot::AddPoint(double x, double cost) {
	// Note: x is currently ignored as we assume generations are sequential
	history_costs.Add(cost);
	Refresh();
}

void EnergyPlot::SetHistory(const Vector<double>& costs) {
	history_costs <<= costs;
	Refresh();
}

void EnergyPlot::SetReplayIndex(int index) {
	replay_index = index;
	Refresh();
}

void EnergyPlot::Paint(Draw& w) {
	Size sz = GetSize();
	w.DrawRect(sz, White());
	
	int count = history_costs.GetCount();
	if (replay_index >= 0)
		count = min(count, replay_index + 1);

	if (count == 0) {
		w.DrawText(sz.cx / 2 - 20, sz.cy / 2 - 10, "No Data", Arial(12), Gray());
		return;
	}

	// Calculate improvement scores (initial - current)
	// We want this to be monotonic rising, even if raw cost fluctuates 
	// (though GA best cost is usually monotonic decreasing anyway).
	Vector<double> scores;
	double max_score = 0;
	double min_score = 0;
	
	for (int i = 0; i < count; i++) {
		double score = initial_cost - history_costs[i];
		scores.Add(score);
		if (i == 0) {
			min_score = max_score = score;
		} else {
			min_score = min(min_score, score);
			max_score = max(max_score, score);
		}
	}

	// Stability: ensure we have some range
	if (abs(max_score - min_score) < 1e-9)
		max_score = min_score + 1.0;

	// Draw Grid (optional but helpful)
	w.DrawLine(0, sz.cy / 2, sz.cx, sz.cy / 2, 1, Gray(230));

	// Map to pixels
	polyline_cache.SetCount(0);
	double x_step = (double)sz.cx / max(1, count - 1);
	
	for (int i = 0; i < count; i++) {
		int x = (int)(i * x_step);
		// Y = (1.0 - normalized) * height
		// We want 0 improvement at bottom, max improvement at top.
		double norm = (scores[i] - min_score) / (max_score - min_score);
		int y = (int)((1.0 - norm) * (sz.cy - 20) + 10);
		polyline_cache.Add(Point(x, y));
	}

	if (polyline_cache.GetCount() >= 2) {
		w.DrawPolyline(polyline_cache, 2, Blue());
	} else if (polyline_cache.GetCount() == 1) {
		w.DrawRect(polyline_cache[0].x - 2, polyline_cache[0].y - 2, 4, 4, Blue());
	}

	DrawLabels(w, sz, min_score, max_score, scores.Top());
}

void EnergyPlot::DrawLabels(Draw& w, Size sz, double min_v, double max_v, double last_v) {
	Font fnt = Arial(10);
	String top_lbl = Format("Max Improvement: %.2f", max_v);
	String last_lbl = Format("Current: %.2f", last_v);
	
	w.DrawText(5, 2, top_lbl, fnt, Gray());
	w.DrawText(sz.cx - GetTextSize(last_lbl, fnt).cx - 5, sz.cy - 15, last_lbl, fnt, Gray());
}

END_UPP_NAMESPACE
