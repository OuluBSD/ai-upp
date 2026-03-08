#ifndef PLATFORM_WIN32

#include <Core/Core.h>
#include <EditorCommon/Capture.h>
#include <EditorCommon/EditorCommon.h>
#include <EditorCommon/Recognition.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/videodev2.h>
#include <drm/drm_fourcc.h>

namespace Upp {

static int xioctl(int fh, int request, void *arg) {
	int r;
	do {
		r = ioctl(fh, request, arg);
	} while (-1 == r && EINTR == errno);
	return r;
}

struct V4l2MappedBuffer {
	void *addr = MAP_FAILED;
	size_t length = 0;
};

static uint32 V4l2PixFmtToDrmFourcc(uint32_t pixfmt) {
	switch (pixfmt) {
	case V4L2_PIX_FMT_YUYV:  return DRM_FORMAT_YUYV;
	case V4L2_PIX_FMT_RGB24: return DRM_FORMAT_RGB888;
	case V4L2_PIX_FMT_BGR24: return DRM_FORMAT_BGR888;
	case V4L2_PIX_FMT_RGB32: return DRM_FORMAT_ARGB8888;
	case V4L2_PIX_FMT_BGR32: return DRM_FORMAT_ABGR8888;
	default: return 0;
	}
}

static GpuFrame::ExternalFormat V4l2PixFmtToExternalFormat(uint32_t pixfmt) {
	switch (pixfmt) {
	case V4L2_PIX_FMT_YUYV:  return GpuFrame::EXTERNAL_YUYV422;
	case V4L2_PIX_FMT_RGB32: return GpuFrame::EXTERNAL_RGBA8;
	case V4L2_PIX_FMT_BGR32: return GpuFrame::EXTERNAL_BGRA8;
	default: return GpuFrame::EXTERNAL_UNKNOWN;
	}
}

static bool IsV4l2CompressedFormat(uint32_t pixfmt) {
	return pixfmt == V4L2_PIX_FMT_MJPEG || pixfmt == V4L2_PIX_FMT_JPEG;
}

static int ScoreV4l2FormatCandidate(const String& policy, uint32_t pixfmt, int width, int height) {
	int score = width * height;
	bool compressed = IsV4l2CompressedFormat(pixfmt);
	if (policy == "low-copy") {
		if (compressed) score /= 10;
		if (pixfmt == V4L2_PIX_FMT_YUYV) score *= 2;
		if (pixfmt == V4L2_PIX_FMT_BGR32 || pixfmt == V4L2_PIX_FMT_RGB32) score *= 5;
	} else if (policy == "throughput") {
		if (compressed) score *= 5;
	}
	return score;
}

static bool ChooseV4l2CaptureFormat(int v4l2_fd, const String& policy, v4l2_format& out_fmt, String& selection_log) {
	v4l2_fmtdesc fmtdesc;
	memset(&fmtdesc, 0, sizeof(fmtdesc));
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int best_score = -1;
	uint32_t best_pixfmt = 0;
	int best_w = 0, best_h = 0;

	while (xioctl(v4l2_fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
		v4l2_frmsizeenum frmsize;
		memset(&frmsize, 0, sizeof(frmsize));
		frmsize.pixel_format = fmtdesc.pixelformat;
		while (xioctl(v4l2_fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
			int w = 0, h = 0;
			if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
				w = frmsize.discrete.width;
				h = frmsize.discrete.height;
			} else if (frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
				w = frmsize.stepwise.max_width;
				h = frmsize.stepwise.max_height;
			}
				if (w > 0 && h > 0) {
					int score = ScoreV4l2FormatCandidate(policy, fmtdesc.pixelformat, w, h);
					selection_log << "  candidate " << (const char*)fmtdesc.description << " (" << (char)(fmtdesc.pixelformat & 0xFF) << (char)((fmtdesc.pixelformat >> 8) & 0xFF) << (char)((fmtdesc.pixelformat >> 16) & 0xFF) << (char)((fmtdesc.pixelformat >> 24) & 0xFF) << ") " << w << "x" << h << " score=" << score << "\n";
					if (score > best_score) {
						best_score = score;
						best_pixfmt = fmtdesc.pixelformat;
						best_w = w;
						best_h = h;
					}
				}
				frmsize.index++;
			}
			fmtdesc.index++;
		}

		if (best_score >= 0) {
			memset(&out_fmt, 0, sizeof(out_fmt));
			out_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			out_fmt.fmt.pix.width = best_w;
			out_fmt.fmt.pix.height = best_h;
			out_fmt.fmt.pix.pixelformat = best_pixfmt;
			out_fmt.fmt.pix.field = V4L2_FIELD_ANY;
			selection_log << "  selected " << (char)(best_pixfmt & 0xFF) << (char)((best_pixfmt >> 8) & 0xFF) << (char)((best_pixfmt >> 16) & 0xFF) << (char)((best_pixfmt >> 24) & 0xFF) << " " << best_w << "x" << best_h << " score=" << best_score << "\n";
			return true;
		}
	return false;
}

class V4l2CaptureSource : public CaptureSource {
	String device_path;
	String format_policy;
	int fd = -1;
	bool opened = false;
	bool streaming = false;
	Size capture_size;
	uint32_t pixelformat = 0;
	int bytesperline = 0;
	Vector<V4l2MappedBuffer> buffers;
	int last_dequeued_index = -1;

public:
	V4l2CaptureSource(const String& dev, const String& policy) 
		: device_path(dev), format_policy(policy) {}

	virtual ~V4l2CaptureSource() {
		Close();
	}

	String GetName() const override { return "V4L2 (" + device_path + ")"; }
	Size GetSize() const override { return capture_size; }

	bool Open() override {
		if (opened) return true;
		
		fd = open(device_path.Begin(), O_RDWR);
		if (fd < 0) return false;

		v4l2_capability cap;
		if (xioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
			Close();
			return false;
		}
		if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) || !(cap.capabilities & V4L2_CAP_STREAMING)) {
			Close();
			return false;
		}

		v4l2_format fmt;
		String log;
		if (!ChooseV4l2CaptureFormat(fd, format_policy, fmt, log)) {
			Close();
			return false;
		}

		if (xioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
			memset(&fmt, 0, sizeof(fmt));
			fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if (xioctl(fd, VIDIOC_G_FMT, &fmt) < 0) {
				Close();
				return false;
			}
		}

		capture_size = Size(fmt.fmt.pix.width, fmt.fmt.pix.height);
		pixelformat = fmt.fmt.pix.pixelformat;
		bytesperline = fmt.fmt.pix.bytesperline;

		v4l2_requestbuffers req;
		memset(&req, 0, sizeof(req));
		req.count = 4;
		req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		req.memory = V4L2_MEMORY_MMAP;
		if (xioctl(fd, VIDIOC_REQBUFS, &req) < 0 || req.count < 2) {
			Close();
			return false;
		}

		buffers.SetCount(req.count);
		for (unsigned i = 0; i < req.count; i++) {
			v4l2_buffer buf;
			memset(&buf, 0, sizeof(buf));
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;
			if (xioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
				Close();
				return false;
			}
			buffers[i].length = buf.length;
			buffers[i].addr = mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
			if (buffers[i].addr == MAP_FAILED) {
				Close();
				return false;
			}
		}

		for (int i = 0; i < buffers.GetCount(); i++) {
			v4l2_buffer buf;
			memset(&buf, 0, sizeof(buf));
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;
			if (xioctl(fd, VIDIOC_QBUF, &buf) < 0) {
				Close();
				return false;
			}
		}

		v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (xioctl(fd, VIDIOC_STREAMON, &type) < 0) {
			Close();
			return false;
		}

		streaming = true;
		opened = true;
		return true;
	}

	void Close() override {
		if (streaming) {
			v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			xioctl(fd, VIDIOC_STREAMOFF, &type);
			streaming = false;
		}
		for (auto& b : buffers) {
			if (b.addr != MAP_FAILED)
				munmap(b.addr, b.length);
		}
		buffers.Clear();
		if (fd >= 0) {
			close(fd);
			fd = -1;
		}
		opened = false;
		last_dequeued_index = -1;
	}

	void Requeue() {
		if (last_dequeued_index >= 0) {
			v4l2_buffer buf;
			memset(&buf, 0, sizeof(buf));
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = last_dequeued_index;
			xioctl(fd, VIDIOC_QBUF, &buf);
			last_dequeued_index = -1;
		}
	}

	Image GrabFrame(bool force_fetch = false) override {
		if (!opened) return Null;
		Requeue();

		v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		if (xioctl(fd, VIDIOC_DQBUF, &buf) < 0) return Null;
		last_dequeued_index = buf.index;

		Image img;
		if (pixelformat == V4L2_PIX_FMT_YUYV) {
			ImageBuffer ib(capture_size);
			YUYVToImage((const unsigned char*)buffers[buf.index].addr, capture_size.cx, capture_size.cy, ib);
			img = ib;
		} else if (IsV4l2CompressedFormat(pixelformat)) {
			String data((const char*)buffers[buf.index].addr, (int)buf.bytesused);
			img = JPGRaster().LoadString(data);
		}
		
		return img;
	}

	bool GrabGpuFrame(GpuFrame& out) override {
		if (!opened) return false;
		Requeue();

		v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		if (xioctl(fd, VIDIOC_DQBUF, &buf) < 0) return false;
		last_dequeued_index = buf.index;

		if (IsV4l2CompressedFormat(pixelformat)) {
			// Fallback to CPU decode for compressed formats
			String data((const char*)buffers[buf.index].addr, (int)buf.bytesused);
			out.type = GpuFrame::CPU_IMAGE;
			out.cpu_image = JPGRaster().LoadString(data);
			return !out.cpu_image.IsEmpty();
		}

		v4l2_exportbuffer expbuf;
		memset(&expbuf, 0, sizeof(expbuf));
		expbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		expbuf.index = buf.index;
		expbuf.plane = 0;
		expbuf.flags = O_CLOEXEC;
		if (xioctl(fd, VIDIOC_EXPBUF, &expbuf) < 0) {
			// Fallback to CPU image if export fails
			out.type = GpuFrame::CPU_IMAGE;
			if (pixelformat == V4L2_PIX_FMT_YUYV) {
				ImageBuffer ib(capture_size);
				YUYVToImage((const unsigned char*)buffers[buf.index].addr, capture_size.cx, capture_size.cy, ib);
				out.cpu_image = ib;
			}
			return !out.cpu_image.IsEmpty();
		}

		out.type = GpuFrame::DMA_BUF;
		out.dmabuf.fd = expbuf.fd; // Note: consumer MUST close this fd
		out.dmabuf.fourcc = V4l2PixFmtToDrmFourcc(pixelformat);
		out.dmabuf.width = capture_size.cx;
		out.dmabuf.height = capture_size.cy;
		out.dmabuf.stride = bytesperline;
		out.dmabuf.modifier = 0;
		out.dmabuf.external_format = V4l2PixFmtToExternalFormat(pixelformat);
		
		return true;
	}
};

CaptureSource* CreateV4l2CaptureImpl(const String& device, const String& format_policy) {
	return new V4l2CaptureSource(device, format_policy);
}

}

#endif
