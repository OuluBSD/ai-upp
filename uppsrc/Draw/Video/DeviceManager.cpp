#include "Video.h"

#ifdef flagLINUX
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <unistd.h>
#endif

NAMESPACE_UPP

void V4L2DeviceManager::Enumerate(Vector<VideoDeviceInfo>& out) {
	out.Clear();
#ifdef flagLINUX
	for (int i = 0; i < 64; i++) {
		String path = "/dev/video" + AsString(i);
		int fd = open(path.Begin(), O_RDWR);
		if (fd < 0)
			continue;
		struct v4l2_capability cap;
		if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == 0) {
			if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
				VideoDeviceInfo& info = out.Add();
				info.id = AsString(i);
				info.name = (const char*)cap.card;
				info.path = path;
			}
		}
		close(fd);
	}
#endif
}

END_UPP_NAMESPACE
