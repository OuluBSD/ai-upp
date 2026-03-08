#ifndef _Ctrl_Camera_CameraView_h_
#define _Ctrl_Camera_CameraView_h_

class CameraView : public Ctrl {
	Image img;

public:
	Callback3<Draw&, const Rect&, const Image&> WhenOverlay;
	void SetImage(const Image& m);
	void Paint(Draw& d) override;
};

#endif
