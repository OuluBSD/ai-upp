#ifndef _Draw_Video_DeviceManager_h_
#define _Draw_Video_DeviceManager_h_

struct VideoDeviceInfo : Moveable<VideoDeviceInfo> {
	String id;
	String name;
	String path;
};

class VideoDeviceManager {
public:
	virtual ~VideoDeviceManager() = default;
	virtual void Enumerate(Vector<VideoDeviceInfo>& out) = 0;
};

#endif
