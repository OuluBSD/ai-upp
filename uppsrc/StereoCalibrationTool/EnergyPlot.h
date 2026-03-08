#ifndef _StereoCalibrationTool_EnergyPlot_h_
#define _StereoCalibrationTool_EnergyPlot_h_

#include <CtrlLib/CtrlLib.h>

NAMESPACE_UPP

/*
EnergyPlot.h
------------
Purpose:
- Visualize GA optimization progress as a monotonic "improvement" curve.
- Origin: Adapted from Overlook (2026-02-01) DrawVectorPolyline logic.

Coordinate System:
- Data Space: X = Generation (0..N), Y = Improvement (0..MaxImprovement).
- Screen Space: Standard U++ pixels, Y-axis inverted for visualization (rising curve).

Semantic:
- Values should RISE. 
- Plotted value = (initial_cost - current_best_cost).
- This ensures the curve starts at 0 and goes up as optimization succeeds.
*/

class EnergyPlot : public Ctrl {
public:
	typedef EnergyPlot CLASSNAME;
	EnergyPlot();

	void Clear();
	void AddPoint(double x, double cost);
	void SetInitialCost(double cost);
	void SetHistory(const Vector<double>& costs);
	void SetReplayIndex(int index);

	virtual void Paint(Draw& w) override;

private:
	double initial_cost = 0;
	Vector<double> history_costs; // Raw best costs per step
	int replay_index = -1;        // If >= 0, only plot up to this index
	
	Vector<Point> polyline_cache;
	
	void DrawLabels(Draw& w, Size sz, double min_v, double max_v, double last_v);
};

END_UPP_NAMESPACE

#endif
