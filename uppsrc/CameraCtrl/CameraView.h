#ifndef _CameraCtrl_CameraView_h_
#define _CameraCtrl_CameraView_h_

class CameraView : public Ctrl {
	Image img;

public:
	void SetImage(const Image& m);
	void Paint(Draw& d) override;
};

#endif
