#ifndef _Draw_Video_DeviceManager_h_
#define _Draw_Video_DeviceManager_h_

struct VideoDeviceInfo {
	String id;
	String name;
	String path;
};

class VideoDeviceManager {
public:
	virtual ~VideoDeviceManager() = default;
	virtual void Enumerate(Vector<VideoDeviceInfo>& out) = 0;
};

class V4L2DeviceManager : public VideoDeviceManager {
public:
	void Enumerate(Vector<VideoDeviceInfo>& out) override;
};

#endif
