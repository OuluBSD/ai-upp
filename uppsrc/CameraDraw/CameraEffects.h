#ifndef _CameraDraw_CameraEffects_h_
#define _CameraDraw_CameraEffects_h_

class CameraEffect {
public:
	virtual ~CameraEffect() = default;
	virtual void Process(CameraFrame& frame) = 0;
};

#endif
