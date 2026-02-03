#ifndef _Draw_Camera_CameraEffects_h_
#define _Draw_Camera_CameraEffects_h_

class CameraEffect : public VideoEffect {
public:
	virtual ~CameraEffect() = default;
	virtual void Process(CameraFrame& frame) = 0;
};

#endif
