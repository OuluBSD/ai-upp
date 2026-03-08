#ifndef _Ctrl_Camera_CameraStereoView_h_
#define _Ctrl_Camera_CameraStereoView_h_

class CameraStereoView : public Ctrl {
	Image img_a;
	Image img_b;
	bool split_view = true;
	bool show_missing_text = true;
	String label_a;
	String label_b;

	void DrawImage(Draw& d, const Rect& r, const Image& img);

public:
	Callback4<Draw&, const Rect&, const Image&, int> WhenOverlay;
	bool draw_label = false;

	void SetImages(const Image& a, const Image& b);
	void SetSplitView(bool b);
	void SetShowMissingText(bool b);
	void SetLabels(const String& a, const String& b);
	void SetDrawLabel(bool b) { draw_label = b; Refresh(); }

	void Paint(Draw& d) override;
};

#endif
