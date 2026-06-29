#ifndef _VisualStateWorkbench_FrameCanvas_h_
#define _VisualStateWorkbench_FrameCanvas_h_

// FrameCanvas — shows frame placeholder with changed-region overlays.
// Fires WhenRegionSelected(region_id) when user clicks a region.

class FrameCanvas : public Ctrl {
public:
	typedef FrameCanvas CLASSNAME;

	FrameCanvas();

	// Replace current changed regions (clears previous)
	void SetChangedRegions(const Vector<VsmChangedRect>& regions);

	// Set the session metadata for display
	void SetSession(const VsmSession* session) { session_ = session; }

	Event<String> WhenRegionSelected; // fires with region_id (may be empty)

	virtual void Paint(Draw& w) override;
	virtual void LeftDown(Point p, dword keyflags) override;

private:
	const VsmSession* session_ = nullptr;
	Vector<VsmChangedRect> regions_;
	int selected_ = -1;

	int HitTest(Point p) const;
};

#endif
