#ifndef _StereoCalibrationTool_PreviewCtrl_h_
#define _StereoCalibrationTool_PreviewCtrl_h_

NAMESPACE_UPP

/*
PreviewCtrl.h
=============
Purpose:
- Specialized image viewer for stereo calibration Stage A.
- Handles scaling-to-fit, coordinate mapping (screen <-> image), and overlays.

Coordinate Spaces:
- Image Pixels: (0,0) to (width, height) of the source image.
- Screen Pixels: (0,0) to (cx, cy) of the control widget.

Responsibilities:
- Correct aspect-ratio-preserved scaling (Contain).
- Rendering matching points (circles + index labels).
- Optional line annotation mode (point chain).
- Mouse input events in Image Pixel coordinates.

Non-Responsibilities:
- Does NOT load images from disk.
- Does NOT own project state or match lists.
- Does NOT perform camera capture or GA solving.
*/

enum class ToolMode {
	None = 0,
	PickMatch = 1,
	LineAnnotate = 2
};

class PreviewCtrl : public Ctrl {
public:
	typedef PreviewCtrl CLASSNAME;
	PreviewCtrl();

	// Data API
	void SetImage(const Image& img);
	void SetTitle(const String& t) { title = t; Refresh(); }
	void SetEye(int e) { eye = e; } // 0=Left, 1=Right
	void SetMatchingPoints(const Vector<Pointf>& pts_img_space);
	void SetHighlightIndex(int idx_or_minus1) { highlight_idx = idx_or_minus1; Refresh(); }
	void SetToolMode(ToolMode m) { mode = m; Refresh(); }
	
	// Line Annotation API (Forward clicks if mode == LineAnnotate)
	void SetLinePoints(const Vector<Pointf>& pts_img_space) { line_points <<= pts_img_space; Refresh(); }
	const Vector<Pointf>& GetLinePoints() const { return line_points; }
	void ClearLinePoints() { line_points.Clear(); Refresh(); }

	void Clear();

	// Coordinate Mapping
	Pointf ScreenToImage(Point p) const;
	Point  ImageToScreen(Pointf p) const;

	// Callbacks
	Event<int /*eye*/, Pointf /*img_px*/> WhenClickPoint;

private:
	Image img;
	String title;
	int eye = 0; // 0=Left, 1=Right
	ToolMode mode = ToolMode::None;
	
	Vector<Pointf> match_points; // in image pixels
	Vector<Pointf> line_points;  // in image pixels
	int highlight_idx = -1;

	// Computed fitting parameters (internal cache for mapping)
	double img_scale = 1.0;
	Pointf img_offset;

	void UpdateLayout();
	
	virtual void Paint(Draw& w) override;
	virtual void LeftDown(Point p, dword flags) override;
	virtual void MouseMove(Point p, dword flags) override;
};

END_UPP_NAMESPACE

#endif