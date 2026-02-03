#ifndef _Draw_Camera_CameraBackend_h_
#define _Draw_Camera_CameraBackend_h_

class CameraBackend : public VideoBackend {
public:
	virtual ~CameraBackend() = default;
	virtual void PopFrames(Vector<CameraFrame>& out) = 0;
	virtual CameraStats GetStats() const = 0;
};

#endif
