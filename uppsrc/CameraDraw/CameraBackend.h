#ifndef _CameraDraw_CameraBackend_h_
#define _CameraDraw_CameraBackend_h_

class CameraBackend {
public:
	virtual ~CameraBackend() = default;

	virtual bool Open() = 0;
	virtual void Close() = 0;
	virtual bool IsOpen() const = 0;

	virtual void PopFrames(Vector<CameraFrame>& out) = 0;
	virtual CameraStats GetStats() const = 0;
};

#endif
