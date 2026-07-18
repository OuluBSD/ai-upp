#ifdef _WIN32
#define _WINSOCKAPI_
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#endif

#ifdef _WIN32
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#endif

#include <Core/Core.h>
#include <Draw/Draw.h>
#include <plugin/jpg/jpg.h>

#ifdef PLATFORM_LINUX
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

using namespace Upp;

static String g_latest_jpeg;
static uint32 g_latest_id = 0;
static Mutex  g_data_mutex;
static std::atomic<bool> g_running(false);
static std::atomic<int64> g_stat_frames(0);
static std::atomic<int64> g_stat_bytes(0);

struct LocalCommandLineArguments {
	int port = 8082;
	int fps = 10;
	int device_idx = 0;
	String source = "camera";
	String image_path;
	String image_dir;
	String video_path;
	String video_work_dir;
	String ffmpeg = "ffmpeg";
	String screen_rect;
	bool list_devices = false;
	bool show_help = false;
	bool self_test = false;
	bool test_capture = false;
	bool capture_after = false;
	int capture_after_seconds = 0;
	bool red_test = false;
	bool no_touch = false;
	bool stats = false;
	String wire_format = "mjpeg";

	void Parse(const Vector<String>& args) {
		for(int i = 0; i < args.GetCount(); i++) {
			if(args[i] == "--port" && i + 1 < args.GetCount()) { port = StrInt(args[i+1]); i++; }
			else if(args[i] == "--fps" && i + 1 < args.GetCount()) { fps = StrInt(args[i+1]); i++; }
			else if(args[i] == "--device" && i + 1 < args.GetCount()) { device_idx = StrInt(args[i+1]); i++; }
			else if(args[i] == "--source" && i + 1 < args.GetCount()) { source = ToLower(args[i+1]); i++; }
			else if(args[i] == "--image" && i + 1 < args.GetCount()) { image_path = args[i+1]; i++; }
			else if(args[i] == "--image-dir" && i + 1 < args.GetCount()) { image_dir = args[i+1]; i++; }
			else if(args[i] == "--video" && i + 1 < args.GetCount()) { video_path = args[i+1]; i++; }
			else if(args[i] == "--video-work-dir" && i + 1 < args.GetCount()) { video_work_dir = args[i+1]; i++; }
			else if(args[i] == "--ffmpeg" && i + 1 < args.GetCount()) { ffmpeg = args[i+1]; i++; }
			else if(args[i] == "--screen-rect" && i + 1 < args.GetCount()) { screen_rect = args[i+1]; i++; }
			else if(args[i] == "-l") list_devices = true;
			else if(args[i] == "--self-test") self_test = true;
			else if(args[i] == "--test-capture") test_capture = true;
			else if(args[i] == "--capture-after" && i + 1 < args.GetCount()) { capture_after = true; capture_after_seconds = max(0, StrInt(args[i+1])); i++; }
			else if(args[i] == "--red-test") { red_test = true; source = "red"; }
			else if(args[i] == "--no-touch") no_touch = true;
			else if(args[i] == "--stats") stats = true;
			else if(args[i] == "--wire-format" && i + 1 < args.GetCount()) { wire_format = ToLower(args[i+1]); i++; }
			else if(args[i] == "--help" || args[i] == "-h") show_help = true;
		}
		if(source != "camera" && source != "red" && source != "image" && source != "image-dir" && source != "screen" && source != "video")
			source = "camera";
		if (wire_format != "mjpeg" && wire_format != "yuv")
			wire_format = "mjpeg";
	}

	void PrintHelp() {
		Cout() << "VideoServer - Remote Video Capture Server\n\n"
		       << "Usage: VideoServer [options]\n\n"
		       << "Options:\n"
		       << "  -l               List available video capture devices\n"
		       << "  --source <camera|red|image|image-dir|screen|video>\n"
		       << "                   Set frame source (default: camera)\n"
		       << "  --device <idx>   Set the capture device index (default: 0)\n"
		       << "  --image <path>   Source image path (required when --source image)\n"
		       << "  --image-dir <path>  Directory source for --source image-dir\n"
		       << "  --video <path>   Video file source for --source video (decoded in-process via libavcodec, loops forever)\n"
		       << "  --video-work-dir <dir>  Deprecated/ignored (no longer extracts frames to disk)\n"
		       << "  --ffmpeg <exe>   Deprecated/ignored (no external ffmpeg subprocess is used)\n"
		       << "  --screen-rect <x,y,w,h>  Screen region (default: full screen)\n"
		       << "  --port <port>    Set the server port (default: 8082)\n"
		       << "  --fps <fps>      Set the target FPS (default: 10)\n"
		       << "  --self-test      Capture one frame from selected source and exit\n"
		       << "  --test-capture   Capture one frame, save to bin/test_capture.jpg, and exit\n"
		       << "  --capture-after <seconds>\n"
		       << "                   Wait, capture one camera frame, save to bin/delayed_capture.jpg, and exit\n"
		       << "  --red-test       Alias for --source red\n"
		       << "  --wire-format <mjpeg|yuv>  Payload format for streaming (default: mjpeg)\n"
		       << "  --no-touch       Skip rescale/extra transforms when possible\n"
		       << "  --stats          Print capture payload stats once per second\n"
		       << "  --help, -h       Show this help message\n";
	}
};

#ifdef _WIN32
String GetGuidName(const GUID& guid) {
	if (guid == MFVideoFormat_RGB32) return "RGB32";
	if (guid == MFVideoFormat_RGB24) return "RGB24";
	if (guid == MFVideoFormat_YUY2)  return "YUY2";
	if (guid == MFVideoFormat_MJPG)  return "MJPEG";
	if (guid == MFVideoFormat_NV12)  return "NV12";
	if (guid == MFMediaType_Video)   return "Video";
	
	const RPC_WSTR *pStr = nullptr;
	String s = "Unknown GUID";
	if (UuidToStringW(&guid, (RPC_WSTR*)&pStr) == RPC_S_OK) {
		s = WString((const wchar_t*)pStr).ToString();
		RpcStringFreeW((RPC_WSTR*)&pStr);
	}
	return s;
}

void DumpMediaType(IMFMediaType* pType) {
	GUID major, sub;
	pType->GetMajorType(&major);
	pType->GetGUID(MF_MT_SUBTYPE, &sub);
	UINT32 w, h;
	MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &w, &h);
	UINT32 num, den;
	MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, &num, &den);
	
	Cout() << "  Type: " << GetGuidName(major) << "/" << GetGuidName(sub) << "\n";
	Cout() << "  Size: " << w << "x" << h << "\n";
	if (den > 0) Cout() << "  FPS:  " << (double)num / den << "\n";
}

class WinMFCapture {
	enum PixelFormat {
		PIXEL_RGB32,
		PIXEL_RGB24,
		PIXEL_NV12
	};

	IMFSourceReader* reader = nullptr;
	Size size;
	int  stride = 0;
	int  bpp = 4;
	PixelFormat pixel_format = PIXEL_RGB32;
	bool opened = false;

public:
	WinMFCapture() {}
	virtual ~WinMFCapture() { Close(); }

	Size GetSize() const { return size; }

	static Vector<String> GetDeviceList() {
		Vector<String> list;
		CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		MFStartup(MF_VERSION);
		IMFAttributes* attrs = nullptr;
		if (SUCCEEDED(MFCreateAttributes(&attrs, 1))) {
			if (SUCCEEDED(attrs->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID))) {
				IMFActivate** devices = nullptr;
				UINT32 count = 0;
				if (SUCCEEDED(MFEnumDeviceSources(attrs, &devices, &count))) {
					for (UINT32 i = 0; i < count; i++) {
						WCHAR* name = nullptr;
						UINT32 nameLen = 0;
						if (SUCCEEDED(devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &name, &nameLen))) {
							list.Add(String(name));
							CoTaskMemFree(name);
						} else list.Add("Unknown Device " + AsString(i));
						devices[i]->Release();
					}
					CoTaskMemFree(devices);
				}
			}
			attrs->Release();
		}
		MFShutdown();
		CoUninitialize();
		return list;
	}

	bool Open(int deviceIndex = 0) {
		if (opened) return true;
		CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		MFStartup(MF_VERSION);
		IMFAttributes* attrs = nullptr;
		MFCreateAttributes(&attrs, 1);
		attrs->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
		IMFActivate** devices = nullptr;
		UINT32 count = 0;
		if (FAILED(MFEnumDeviceSources(attrs, &devices, &count)) || count == 0) { 
			Cerr() << "No video devices found.\n";
			attrs->Release(); return false; 
		}
		attrs->Release();
		if (deviceIndex >= (int)count) deviceIndex = 0;
		IMFMediaSource* source = nullptr;
		devices[deviceIndex]->ActivateObject(__uuidof(IMFMediaSource), reinterpret_cast<void**>(&source));
		for (UINT32 i = 0; i < count; ++i) devices[i]->Release();
		CoTaskMemFree(devices);
		IMFAttributes* readerAttrs = nullptr;
		MFCreateAttributes(&readerAttrs, 1);
		readerAttrs->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
		HRESULT hr = MFCreateSourceReaderFromMediaSource(source, readerAttrs, &reader);
		if (readerAttrs) readerAttrs->Release();
		source->Release();
		if (FAILED(hr)) { Cerr() << "Failed to create SourceReader.\n"; return false; }

		Cout() << "Negotiating media type...\n";
		IMFMediaType* pTargetType = nullptr;
		MFCreateMediaType(&pTargetType);
		pTargetType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
		pTargetType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
		MFSetAttributeSize(pTargetType, MF_MT_FRAME_SIZE, 1920, 1080);
		
		hr = reader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, pTargetType);
		pTargetType->Release();
		if (FAILED(hr)) Cout() << "Warning: Could not force RGB32, using device default.\n";

		IMFMediaType* pActualType = nullptr;
		if (SUCCEEDED(reader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pActualType))) {
			DumpMediaType(pActualType);
			UINT32 w, h;
			MFGetAttributeSize(pActualType, MF_MT_FRAME_SIZE, &w, &h);
			size = Size(w, h);
			GUID sub;
			pActualType->GetGUID(MF_MT_SUBTYPE, &sub);
			if (sub == MFVideoFormat_RGB24) {
				pixel_format = PIXEL_RGB24;
				bpp = 3;
			}
			else if (sub == MFVideoFormat_NV12) {
				pixel_format = PIXEL_NV12;
				bpp = 1;
			}
			else {
				pixel_format = PIXEL_RGB32;
				bpp = 4;
			}
			
			UINT32 s = 0;
			if (FAILED(pActualType->GetUINT32(MF_MT_DEFAULT_STRIDE, &s)))
				stride = pixel_format == PIXEL_NV12 ? size.cx : size.cx * bpp;
			else stride = (int)s;
			pActualType->Release();
		}

		Cout() << "Final format: " << size.cx << "x" << size.cy << " Stride: " << stride << " BPP: " << bpp << "\n";
		opened = true;
		return true;
	}

	void Close() {
		if (reader) { reader->Release(); reader = nullptr; }
		if (opened) { MFShutdown(); CoUninitialize(); }
		opened = false;
	}

	Image GrabFrame() {
		if (!opened || !reader) return Null;
		DWORD streamIndex = 0, flags = 0;
		LONGLONG timestamp = 0;
		IMFSample* sample = nullptr;
		
		// Try a few times because first samples might be null/ticks
		for (int i = 0; i < 10; i++) {
			HRESULT hr = reader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &streamIndex, &flags, &timestamp, &sample);
			if (FAILED(hr)) return Null;
			if (sample) break;
			Sleep(10);
		}
		
		if (!sample) return Null;
		
		IMFMediaBuffer* buffer = nullptr;
		sample->ConvertToContiguousBuffer(&buffer);
		if (!buffer) { sample->Release(); return Null; }
		BYTE* data = nullptr;
		DWORD curLen = 0;
		buffer->Lock(&data, nullptr, &curLen);

		ImageBuffer ib(size);
		int abs_stride = abs(stride);
		bool bottom_up = stride < 0;

		if (pixel_format == PIXEL_NV12) {
			const byte* y_plane = data;
			const byte* uv_plane = data + abs_stride * size.cy;
			for (int y = 0; y < size.cy; y++) {
				const byte* y_row = y_plane + y * abs_stride;
				const byte* uv_row = uv_plane + (y >> 1) * abs_stride;
				RGBA* dst_row = ib[y];
				for (int x = 0; x < size.cx; x++) {
					int yv = (int)y_row[x] - 16;
					int u = (int)uv_row[(x & ~1) + 0] - 128;
					int v = (int)uv_row[(x & ~1) + 1] - 128;
					int c = max(yv, 0);
					dst_row[x].r = (byte)minmax((298 * c + 409 * v + 128) >> 8, 0, 255);
					dst_row[x].g = (byte)minmax((298 * c - 100 * u - 208 * v + 128) >> 8, 0, 255);
					dst_row[x].b = (byte)minmax((298 * c + 516 * u + 128) >> 8, 0, 255);
					dst_row[x].a = 255;
				}
			}
		}
		else {
			for (int y = 0; y < size.cy; y++) {
				const byte* src_row = data + (bottom_up ? (size.cy - 1 - y) : y) * abs_stride;
				RGBA* dst_row = ib[y];
				if (pixel_format == PIXEL_RGB32) {
					for (int x = 0; x < size.cx; x++) {
						dst_row[x].r = src_row[x * 4 + 2];
						dst_row[x].g = src_row[x * 4 + 1];
						dst_row[x].b = src_row[x * 4 + 0];
						dst_row[x].a = 255;
					}
				} else {
					for (int x = 0; x < size.cx; x++) {
						dst_row[x].r = src_row[x * 3 + 2];
						dst_row[x].g = src_row[x * 3 + 1];
						dst_row[x].b = src_row[x * 3 + 0];
						dst_row[x].a = 255;
					}
				}
			}
		}
		buffer->Unlock();
		buffer->Release();
		sample->Release();
		return ib;
	}
};

#ifdef PLATFORM_LINUX
struct V4L2MMapBuffer {
	void*  start = nullptr;
	size_t length = 0;
};

class LinuxMJPEGCapture {
	int fd = -1;
	Size size;
	Vector<V4L2MMapBuffer> buffers;
	bool streaming = false;

public:
	~LinuxMJPEGCapture() { Close(); }

	Size GetSize() const { return size; }

	bool Open(int deviceIndex = 0) {
		if(fd != -1)
			return true;

		String device = "/dev/video" + AsString(deviceIndex);
		fd = open(device, O_RDWR);
		if(fd == -1) {
			Cerr() << "Failed to open " << device << "\n";
			return false;
		}

		v4l2_capability cap;
		memset(&cap, 0, sizeof(cap));
		if(ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
			Cerr() << "Failed to query V4L2 capability for " << device << "\n";
			Close();
			return false;
		}

		if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) || !(cap.capabilities & V4L2_CAP_STREAMING)) {
			Cerr() << device << " does not support V4L2 capture streaming.\n";
			Close();
			return false;
		}

		v4l2_format fmt;
		memset(&fmt, 0, sizeof(fmt));
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt.fmt.pix.width = 1920;
		fmt.fmt.pix.height = 1080;
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
		fmt.fmt.pix.field = V4L2_FIELD_ANY;
		fmt.fmt.pix.bytesperline = 0;
		fmt.fmt.pix.sizeimage = 1920 * 1080 * 2;
		if(ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
			Cerr() << "Failed to request MJPEG 1920x1080 on " << device << "\n";
			Close();
			return false;
		}

		if(fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_MJPEG) {
			Cerr() << device << " did not accept MJPEG capture.\n";
			Close();
			return false;
		}

		size = Size((int)fmt.fmt.pix.width, (int)fmt.fmt.pix.height);
		Cout() << "Camera format: MJPEG " << size.cx << "x" << size.cy << "\n";

		v4l2_requestbuffers req;
		memset(&req, 0, sizeof(req));
		req.count = 4;
		req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		req.memory = V4L2_MEMORY_MMAP;
		if(ioctl(fd, VIDIOC_REQBUFS, &req) == -1 || req.count < 2) {
			Cerr() << "Failed to allocate V4L2 buffers for " << device << "\n";
			Close();
			return false;
		}

		buffers.SetCount((int)req.count);
		for(unsigned int i = 0; i < req.count; ++i) {
			v4l2_buffer buf;
			memset(&buf, 0, sizeof(buf));
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;
			if(ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
				Cerr() << "Failed to query V4L2 buffer " << i << "\n";
				Close();
				return false;
			}

			void* start = mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
			if(start == MAP_FAILED) {
				Cerr() << "Failed to mmap V4L2 buffer " << i << "\n";
				Close();
				return false;
			}

			buffers[(int)i].start = start;
			buffers[(int)i].length = buf.length;
		}

			for(unsigned int i = 0; i < req.count; ++i) {
				v4l2_buffer buf;
				memset(&buf, 0, sizeof(buf));
				buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory = V4L2_MEMORY_MMAP;
				buf.index = i;
				buf.length = buffers[(int)i].length;
				if(ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
					Cerr() << "Failed to queue V4L2 buffer " << i << "\n";
					Close();
					return false;
			}
		}

		enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if(ioctl(fd, VIDIOC_STREAMON, &type) == -1) {
			Cerr() << "Failed to start V4L2 stream for " << device << "\n";
			Close();
			return false;
		}

		streaming = true;
		return true;
	}

	bool GrabJpeg(String& payload) {
		payload.Clear();
		if(fd == -1 || !streaming)
			return false;

		v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;

			if(ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
				Cerr() << "Failed to dequeue V4L2 buffer: " << strerror(errno) << "\n";
				return false;
			}

			if(buf.index >= (unsigned int)buffers.GetCount() || buf.bytesused == 0) {
				Cerr() << "Received empty V4L2 buffer.\n";
				buf.length = buffers[(int)buf.index].length;
				ioctl(fd, VIDIOC_QBUF, &buf);
				return false;
			}

			payload.Cat((const char*)buffers[(int)buf.index].start, (int)buf.bytesused);

			buf.length = buffers[(int)buf.index].length;
			if(ioctl(fd, VIDIOC_QBUF, &buf) == -1)
				Cerr() << "Failed to requeue V4L2 buffer: " << strerror(errno) << "\n";

		return !payload.IsEmpty();
	}

	void Close() {
		if(fd == -1)
			return;

		if(streaming) {
			enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			ioctl(fd, VIDIOC_STREAMOFF, &type);
			streaming = false;
		}

		for(int i = 0; i < buffers.GetCount(); ++i) {
			if(buffers[i].start && buffers[i].length)
				munmap(buffers[i].start, buffers[i].length);
		}
		buffers.Clear();

		close(fd);
		fd = -1;
		size = Size();
	}
};
#endif

static Rect GetWinVirtualScreenRect()
{
	int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
	int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	return RectC(x, y, w, h);
}

static Image CaptureWinScreenFrame(const Rect& rect)
{
	if(rect.IsEmpty())
		return Null;

	const int w = rect.GetWidth();
	const int h = rect.GetHeight();

	HDC screen_dc = GetDC(NULL);
	if(!screen_dc)
		return Null;

	HDC mem_dc = CreateCompatibleDC(screen_dc);
	if(!mem_dc) {
		ReleaseDC(NULL, screen_dc);
		return Null;
	}

	BITMAPINFO bi;
	memset(&bi, 0, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = w;
	bi.bmiHeader.biHeight = -h; // top-down
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	void* pixels = nullptr;
	HBITMAP dib = CreateDIBSection(screen_dc, &bi, DIB_RGB_COLORS, &pixels, NULL, 0);
	if(!dib || !pixels) {
		if(dib) DeleteObject(dib);
		DeleteDC(mem_dc);
		ReleaseDC(NULL, screen_dc);
		return Null;
	}

	HGDIOBJ old = SelectObject(mem_dc, dib);
	BOOL ok = BitBlt(mem_dc, 0, 0, w, h, screen_dc, rect.left, rect.top, SRCCOPY | CAPTUREBLT);

	Image out = Null;
	if(ok) {
		ImageBuffer ib(w, h);
		const byte* src = (const byte*)pixels;
		for(int y = 0; y < h; y++) {
			const byte* s = src + y * w * 4;
			RGBA* d = ib[y];
			for(int x = 0; x < w; x++) {
				d[x].b = s[x * 4 + 0];
				d[x].g = s[x * 4 + 1];
				d[x].r = s[x * 4 + 2];
				d[x].a = 255;
			}
		}
		ib.SetKind(IMAGE_OPAQUE);
		out = ib;
	}

	SelectObject(mem_dc, old);
	DeleteObject(dib);
	DeleteDC(mem_dc);
	ReleaseDC(NULL, screen_dc);
	return out;
}
#endif

static void YUYVToImage(const unsigned char* src, int w, int h, ImageBuffer& ib) {
	const unsigned char* s = src;
	for(int y = 0; y < h; y++) {
		RGBA* t = ib[y];
		for(int x = 0; x < w / 2; x++) {
			int y0 = s[0]; int u0 = s[1]; int y1 = s[2]; int v0 = s[3];
			s += 4;
			auto YUV2RGB = [](int y, int u, int v, RGBA& p) {
				int c = y - 16; int d = u - 128; int e = v - 128;
				p.r = (byte)clamp((298 * c + 409 * e + 128) >> 8, 0, 255);
				p.g = (byte)clamp((298 * c - 100 * d - 208 * e + 128) >> 8, 0, 255);
				p.b = (byte)clamp((298 * c + 516 * d + 128) >> 8, 0, 255);
				p.a = 255;
			};
			YUV2RGB(y0, u0, v0, t[0]);
			YUV2RGB(y1, u0, v0, t[1]);
			t += 2;
		}
	}
}

static String BuildYuvPacket(int w, int h, const unsigned char* data, int bytes) {
	String pkt;
	pkt.Cat("YUV0", 4);
	uint32 ww = (uint32)w, hh = (uint32)h, sz = (uint32)bytes;
	pkt.Cat((const char*)&ww, 4);
	pkt.Cat((const char*)&hh, 4);
	pkt.Cat((const char*)&sz, 4);
	pkt.Cat((const char*)data, bytes);
	return pkt;
}

static bool ParseScreenRect(const String& text, Rect& out)
{
	Vector<String> p = Split(text, ',');
	if(p.GetCount() != 4)
		return false;
	int x = ScanInt(TrimBoth(p[0]));
	int y = ScanInt(TrimBoth(p[1]));
	int w = ScanInt(TrimBoth(p[2]));
	int h = ScanInt(TrimBoth(p[3]));
	if(w <= 0 || h <= 0)
		return false;
	out = RectC(x, y, w, h);
	return true;
}

static bool IsImageFileExt(const String& path)
{
	String e = ToLower(GetFileExt(path));
	return e == ".png" || e == ".jpg" || e == ".jpeg" || e == ".bmp" || e == ".gif" || e == ".tif" || e == ".tiff";
}

static bool CollectImageFiles(const String& dir, Vector<String>& files)
{
	files.Clear();
	FindFile ff(AppendFileName(dir, "*"));
	while(ff) {
		if(ff.IsFile()) {
			String p = AppendFileName(dir, ff.GetName());
			if(IsImageFileExt(p))
				files.Add(p);
		}
		ff.Next();
	}
	Sort(files);
	return !files.IsEmpty();
}

#ifdef _WIN32
// --- libavcodec/libavformat looping video decoder ------------------------
//
// Replaces the old ffmpeg.exe subprocess frame-extraction. Opens the input
// file once, decodes frames on demand, and loops back to the start of the
// file when it reaches EOF (genuine infinite looping, no disk extraction,
// no subprocess). Mirrors the RAII/error-string-out-param style of
// VideoRecorder's DirectMp4Writer (the encode direction of the same API).

static String AvError(int code)
{
	char buffer[AV_ERROR_MAX_STRING_SIZE] = {};
	av_strerror(code, buffer, sizeof(buffer));
	return buffer;
}

static String GetDefaultFfmpegDllDirectory()
{
	return AppendFileName(AppendFileName(AppendFileName(AppendFileName(GetHomeDirectory(), "vcpkg"),
	                                                   "installed"),
	                                    "x64-windows"),
	                      "bin");
}

// Points the loader at vcpkg's bin dir so the delay-loaded FFmpeg DLLs
// (avcodec-62.dll etc.) actually resolve at runtime. Same pattern as
// VideoRecorder::PrepareFfmpegRuntime().
static bool PrepareFfmpegRuntime(const String& override_dir, String& error)
{
	String dir = override_dir.IsEmpty() ? GetDefaultFfmpegDllDirectory() : override_dir;
	if(!DirectoryExists(dir)) {
		error = "FFmpeg DLL directory does not exist: " + dir;
		return false;
	}
	if(!SetDllDirectoryA(~dir)) {
		error = "SetDllDirectoryA failed for: " + dir;
		return false;
	}
	return true;
}

struct LoopingVideoDecoder {
	AVFormatContext *fmt = nullptr;
	AVCodecContext  *codec = nullptr;
	SwsContext      *sws = nullptr;
	AVFrame         *frame = nullptr;
	AVPacket        *packet = nullptr;
	int              video_stream_index = -1;
	Size             size;
	int              sws_w = 0;
	int              sws_h = 0;
	int              sws_in_fmt = AV_PIX_FMT_NONE;
	int64            loops = 0;
	bool             opened = false;

	~LoopingVideoDecoder() { Close(); }

	bool Open(const String& path, String& error)
	{
		int rc = avformat_open_input(&fmt, ~path, nullptr, nullptr);
		if(rc < 0 || !fmt) {
			error = "avformat_open_input failed for " + path + ": " + AvError(rc);
			return false;
		}
		rc = avformat_find_stream_info(fmt, nullptr);
		if(rc < 0) {
			error = "avformat_find_stream_info failed: " + AvError(rc);
			return false;
		}
		const AVCodec *decoder = nullptr;
		video_stream_index = av_find_best_stream(fmt, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
		if(video_stream_index < 0 || !decoder) {
			error = "no decodable video stream found in " + path;
			return false;
		}
		codec = avcodec_alloc_context3(decoder);
		if(!codec) {
			error = "avcodec_alloc_context3 failed";
			return false;
		}
		rc = avcodec_parameters_to_context(codec, fmt->streams[video_stream_index]->codecpar);
		if(rc < 0) {
			error = "avcodec_parameters_to_context failed: " + AvError(rc);
			return false;
		}
		rc = avcodec_open2(codec, decoder, nullptr);
		if(rc < 0) {
			error = "avcodec_open2 failed: " + AvError(rc);
			return false;
		}
		frame = av_frame_alloc();
		packet = av_packet_alloc();
		if(!frame || !packet) {
			error = "av_frame_alloc/av_packet_alloc failed";
			return false;
		}
		opened = true;
		return true;
	}

	bool SeekStart(String& error)
	{
		int rc = av_seek_frame(fmt, video_stream_index, 0, AVSEEK_FLAG_BACKWARD);
		if(rc < 0)
			rc = avformat_seek_file(fmt, video_stream_index, INT64_MIN, 0, INT64_MAX, 0);
		if(rc < 0) {
			error = "seek-to-start failed: " + AvError(rc);
			return false;
		}
		avcodec_flush_buffers(codec);
		loops++;
		return true;
	}

	bool Convert(Image& out, String& error)
	{
		int w = codec->width;
		int h = codec->height;
		if(w <= 0 || h <= 0) {
			error = "invalid decoded frame size";
			return false;
		}
		int in_fmt = frame->format;
		if(!sws || sws_w != w || sws_h != h || sws_in_fmt != in_fmt) {
			if(sws) {
				sws_freeContext(sws);
				sws = nullptr;
			}
			sws = sws_getContext(w, h, (AVPixelFormat)in_fmt,
			                     w, h, AV_PIX_FMT_BGRA,
			                     SWS_BILINEAR, nullptr, nullptr, nullptr);
			if(!sws) {
				error = "sws_getContext failed";
				return false;
			}
			sws_w = w;
			sws_h = h;
			sws_in_fmt = in_fmt;
		}
		ImageBuffer ib(w, h);
		byte *dst_data[4] = {(byte*)ib[0], nullptr, nullptr, nullptr};
		int dst_linesize[4] = {w * (int)sizeof(RGBA), 0, 0, 0};
		sws_scale(sws, frame->data, frame->linesize, 0, h, dst_data, dst_linesize);
		ib.SetKind(IMAGE_OPAQUE);
		out = ib;
		size = Size(w, h);
		return true;
	}

	// Decode and return the next frame, looping back to the file start on EOF.
	bool NextFrame(Image& out, String& error)
	{
		if(!opened) {
			error = "decoder is not open";
			return false;
		}
		for(;;) {
			int rc = avcodec_receive_frame(codec, frame);
			if(rc == 0)
				return Convert(out, error);
			if(rc != AVERROR(EAGAIN) && rc != AVERROR_EOF) {
				error = "avcodec_receive_frame failed: " + AvError(rc);
				return false;
			}
			av_packet_unref(packet);
			rc = av_read_frame(fmt, packet);
			if(rc == AVERROR_EOF) {
				if(!SeekStart(error))
					return false;
				continue;
			}
			if(rc < 0) {
				error = "av_read_frame failed: " + AvError(rc);
				return false;
			}
			if(packet->stream_index != video_stream_index) {
				av_packet_unref(packet);
				continue;
			}
			rc = avcodec_send_packet(codec, packet);
			av_packet_unref(packet);
			if(rc < 0 && rc != AVERROR(EAGAIN)) {
				error = "avcodec_send_packet failed: " + AvError(rc);
				return false;
			}
		}
	}

	void Close()
	{
		if(sws) {
			sws_freeContext(sws);
			sws = nullptr;
		}
		if(frame)
			av_frame_free(&frame);
		if(packet)
			av_packet_free(&packet);
		if(codec)
			avcodec_free_context(&codec);
		if(fmt)
			avformat_close_input(&fmt);
		opened = false;
	}
};
#endif // _WIN32

#ifdef PLATFORM_LINUX
static byte ChannelFromMask(unsigned long pixel, unsigned long mask)
{
	if(mask == 0)
		return 0;
	unsigned long v = pixel & mask;
	int shift = 0;
	while(((mask >> shift) & 1) == 0 && shift < 63)
		shift++;
	v >>= shift;
	unsigned long maxv = mask >> shift;
	if(maxv == 0)
		return 0;
	return (byte)((v * 255 + maxv / 2) / maxv);
}

static Image CaptureX11Frame(::Display* dpy, Window root, const Rect& rect)
{
	if(!dpy || rect.IsEmpty())
		return Null;

	XImage* ximg = XGetImage(dpy, root, rect.left, rect.top, rect.GetWidth(), rect.GetHeight(), AllPlanes, ZPixmap);
	if(!ximg)
		return Null;

	ImageBuffer ib(rect.GetWidth(), rect.GetHeight());
	for(int y = 0; y < rect.GetHeight(); y++) {
		RGBA* row = ib[y];
		for(int x = 0; x < rect.GetWidth(); x++) {
			unsigned long px = XGetPixel(ximg, x, y);
			row[x].r = ChannelFromMask(px, ximg->red_mask);
			row[x].g = ChannelFromMask(px, ximg->green_mask);
			row[x].b = ChannelFromMask(px, ximg->blue_mask);
			row[x].a = 255;
		}
	}
	ib.SetKind(IMAGE_OPAQUE);
	XDestroyImage(ximg);
	return Image(ib);
}
#endif

void ClientThread(TcpSocket* s_ptr) {
	One<TcpSocket> client(s_ptr);
	Cout() << "Client connected from " << client->GetPeerAddr() << "\n";
	while(g_running && client->IsOpen() && !client->IsError() && !client->IsEof()) {
		uint32 last_id = 0;
		if (client->Timeout(5000).GetAll(&last_id, 4)) {
			String data; uint32 id;
			{
				Mutex::Lock __(g_data_mutex);
				id = g_latest_id;
				if (id > last_id) data = g_latest_jpeg;
			}
			uint32 sz = (uint32)data.GetCount();
			if (client->Timeout(5000).Put(&id, 4) && client->Timeout(5000).Put(&sz, 4)) {
				if (sz > 0) client->Put(data);
			}
		} else if (client->IsError() || client->IsEof()) break;
	}
	Cout() << "Client disconnected.\n";
}

CONSOLE_APP_MAIN {
	LocalCommandLineArguments cla;
	cla.Parse(CommandLine());
	if (cla.show_help) { cla.PrintHelp(); return; }
	if (cla.list_devices) {
		#ifdef _WIN32
		Vector<String> devices = WinMFCapture::GetDeviceList();
		Cout() << "Available devices:\n";
		for (int i = 0; i < devices.GetCount(); i++) Cout() << i << ": " << devices[i] << "\n";
		#endif
		return;
	}

	if(cla.source == "image" && cla.image_path.IsEmpty()) {
		Cerr() << "--image is required when --source image\n";
		return;
	}
	if(cla.source == "image-dir" && cla.image_dir.IsEmpty()) {
		Cerr() << "--image-dir is required when --source image-dir\n";
		return;
	}
	if(cla.source == "video" && cla.video_path.IsEmpty()) {
		Cerr() << "--video is required when --source video\n";
		return;
	}
#if !defined(PLATFORM_LINUX) && !defined(_WIN32)
	if(cla.source == "screen") {
		Cerr() << "--source screen is supported on Linux and Windows only\n";
		return;
	}
#endif
	Rect screen_rect_req;
	bool has_screen_rect_req = false;
	if(cla.source == "screen" && !cla.screen_rect.IsEmpty()) {
		if(!ParseScreenRect(cla.screen_rect, screen_rect_req)) {
			Cerr() << "Invalid --screen-rect: " << cla.screen_rect << "\n";
			return;
		}
		has_screen_rect_req = true;
	}
	Vector<String> image_dir_files;
	if(cla.source == "image-dir") {
		if(!CollectImageFiles(cla.image_dir, image_dir_files)) {
			Cerr() << "No image files found in --image-dir: " << cla.image_dir << "\n";
			return;
		}
	}
	else if(cla.source == "video") {
#ifdef _WIN32
		if(!FileExists(cla.video_path)) {
			Cerr() << "Video file does not exist: " << cla.video_path << "\n";
			return;
		}
		String ffrt_error;
		if(!PrepareFfmpegRuntime(String(), ffrt_error)) {
			Cerr() << "Failed to prepare FFmpeg runtime: " << ffrt_error << "\n";
			return;
		}
#else
		Cerr() << "--source video (libavcodec decoding) is only supported on Windows in this build\n";
		return;
#endif
	}

#ifdef PLATFORM_LINUX
	struct LocalLinuxMJPEGCapture {
		struct Buffer {
			void* start = nullptr;
			size_t length = 0;
		};

		int fd = -1;
		Size size;
		Vector<Buffer> buffers;
		bool streaming = false;

		~LocalLinuxMJPEGCapture() { Close(); }

		bool Open(int deviceIndex = 0) {
			if(fd != -1)
				return true;

			String device = "/dev/video" + AsString(deviceIndex);
			fd = open(device, O_RDWR);
			if(fd == -1) {
				Cerr() << "Failed to open " << device << "\n";
				return false;
			}

			v4l2_capability cap;
			memset(&cap, 0, sizeof(cap));
			if(ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
				Cerr() << "Failed to query V4L2 capability for " << device << "\n";
				Close();
				return false;
			}

			if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) || !(cap.capabilities & V4L2_CAP_STREAMING)) {
				Cerr() << device << " does not support V4L2 capture streaming.\n";
				Close();
				return false;
			}

			v4l2_format fmt;
			memset(&fmt, 0, sizeof(fmt));
			fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			fmt.fmt.pix.width = 1920;
			fmt.fmt.pix.height = 1080;
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
			fmt.fmt.pix.field = V4L2_FIELD_ANY;
			fmt.fmt.pix.bytesperline = 0;
			fmt.fmt.pix.sizeimage = 1920 * 1080 * 2;
			if(ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
				Cerr() << "Failed to request MJPEG 1920x1080 on " << device << "\n";
				Close();
				return false;
			}

			if(fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_MJPEG) {
				Cerr() << device << " did not accept MJPEG capture.\n";
				Close();
				return false;
			}

			size = Size((int)fmt.fmt.pix.width, (int)fmt.fmt.pix.height);
			Cout() << "Camera format: MJPEG " << size.cx << "x" << size.cy << "\n";

			v4l2_requestbuffers req;
			memset(&req, 0, sizeof(req));
			req.count = 4;
			req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			req.memory = V4L2_MEMORY_MMAP;
			if(ioctl(fd, VIDIOC_REQBUFS, &req) == -1 || req.count < 2) {
				Cerr() << "Failed to allocate V4L2 buffers for " << device << "\n";
				Close();
				return false;
			}

			buffers.SetCount((int)req.count);
			for(unsigned int i = 0; i < req.count; ++i) {
				v4l2_buffer buf;
				memset(&buf, 0, sizeof(buf));
				buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory = V4L2_MEMORY_MMAP;
				buf.index = i;
				if(ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
					Cerr() << "Failed to query V4L2 buffer " << i << "\n";
					Close();
					return false;
				}

				void* start = mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
				if(start == MAP_FAILED) {
					Cerr() << "Failed to mmap V4L2 buffer " << i << "\n";
					Close();
					return false;
				}

				buffers[(int)i].start = start;
				buffers[(int)i].length = buf.length;
			}

			for(unsigned int i = 0; i < req.count; ++i) {
				v4l2_buffer buf;
				memset(&buf, 0, sizeof(buf));
				buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory = V4L2_MEMORY_MMAP;
				buf.index = i;
				buf.length = buffers[(int)i].length;
				if(ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
					Cerr() << "Failed to queue V4L2 buffer " << i << "\n";
					Close();
					return false;
				}
			}

			enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if(ioctl(fd, VIDIOC_STREAMON, &type) == -1) {
				Cerr() << "Failed to start V4L2 stream for " << device << "\n";
				Close();
				return false;
			}

			streaming = true;
			return true;
		}

		bool GrabJpeg(String& payload) {
			payload.Clear();
			if(fd == -1 || !streaming)
				return false;

			for(int attempt = 0; attempt < 8; ++attempt) {
				v4l2_buffer buf;
				memset(&buf, 0, sizeof(buf));
				buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory = V4L2_MEMORY_MMAP;

				if(ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
					Cerr() << "Failed to dequeue V4L2 buffer: " << strerror(errno) << "\n";
					return false;
				}

				if(buf.index >= (unsigned int)buffers.GetCount() || buf.bytesused < 1024) {
					buf.length = buffers[(int)buf.index].length;
					if(ioctl(fd, VIDIOC_QBUF, &buf) == -1)
						Cerr() << "Failed to requeue V4L2 buffer: " << strerror(errno) << "\n";
					continue;
				}

				payload.Cat((const char*)buffers[(int)buf.index].start, (int)buf.bytesused);

				buf.length = buffers[(int)buf.index].length;
				if(ioctl(fd, VIDIOC_QBUF, &buf) == -1)
					Cerr() << "Failed to requeue V4L2 buffer: " << strerror(errno) << "\n";

				return !payload.IsEmpty();
			}

			return false;
		}

		void Close() {
			if(fd == -1)
				return;

			if(streaming) {
				enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				ioctl(fd, VIDIOC_STREAMOFF, &type);
				streaming = false;
			}

			for(int i = 0; i < buffers.GetCount(); ++i) {
				if(buffers[i].start && buffers[i].length)
					munmap(buffers[i].start, buffers[i].length);
			}
			buffers.Clear();

			close(fd);
			fd = -1;
			size = Size();
		}
	};
#endif

	if (cla.self_test) {
		Image img;
		if(cla.source == "red") {
			ImageBuffer ib(1920, 1080);
			for(int y = 0; y < 1080; y++) {
				RGBA* row = ib[y];
				for(int x = 0; x < 1920; x++) {
					row[x].r = 255;
					row[x].g = 0;
					row[x].b = 0;
					row[x].a = 255;
				}
			}
			ib.SetKind(IMAGE_OPAQUE);
			img = ib;
		}
		else if(cla.source == "image") {
			img = StreamRaster::LoadFileAny(cla.image_path);
		}
		else if(cla.source == "image-dir") {
			img = StreamRaster::LoadFileAny(image_dir_files[0]);
		}
		else if(cla.source == "video") {
#ifdef _WIN32
			LoopingVideoDecoder decoder;
			String derr;
			if(!decoder.Open(cla.video_path, derr) || !decoder.NextFrame(img, derr)) {
				Cerr() << "Self-test failed: " << derr << "\n";
				Exit(1);
			}
#endif
		}
		else if(cla.source == "screen") {
#ifdef PLATFORM_LINUX
			::Display* dpy = XOpenDisplay(nullptr);
			if(!dpy) {
				Cerr() << "Self-test failed: cannot open X11 display\n";
				Exit(1);
			}
			int sn = DefaultScreen(dpy);
			Window root = RootWindow(dpy, sn);
			Rect rect = RectC(0, 0, DisplayWidth(dpy, sn), DisplayHeight(dpy, sn));
			if(has_screen_rect_req)
				rect &= screen_rect_req;
			img = CaptureX11Frame(dpy, root, rect);
			XCloseDisplay(dpy);
#elif defined(_WIN32)
			Rect rect = GetWinVirtualScreenRect();
			if(has_screen_rect_req)
				rect &= screen_rect_req;
			if(rect.IsEmpty()) {
				Cerr() << "Self-test failed: screen capture rect is empty\n";
				Exit(1);
			}
			img = CaptureWinScreenFrame(rect);
#else
			Cerr() << "Self-test failed: --source screen is not supported on this platform\n";
			Exit(1);
#endif
		}
		else if(cla.source == "camera") {
#ifdef _WIN32
			WinMFCapture win_cap;
			if(win_cap.Open(cla.device_idx)) {
				img = win_cap.GrabFrame();
				win_cap.Close();
			}
#elif defined(PLATFORM_LINUX)
			LocalLinuxMJPEGCapture cam;
			String payload;
			if(cam.Open(cla.device_idx) && cam.GrabJpeg(payload)) {
				Image jpg = JPGRaster().LoadString(payload);
				if(!jpg.IsEmpty())
					img = jpg;
			}
#endif
		}

		if(img.IsEmpty()) {
			Cerr() << "Self-test failed: could not capture frame from source '" << cla.source << "'\n";
			Exit(1);
		}

		String path = AppendFileName(GetFileDirectory(GetExeFilePath()), "self_test_capture.jpg");
		if(!JPGEncoder().Quality(95).SaveFile(path, img)) {
			Cerr() << "Self-test failed: captured frame but failed to save " << path << "\n";
			Exit(1);
		}
		Cout() << "Self-test PASS source=" << cla.source
		       << " size=" << img.GetWidth() << "x" << img.GetHeight()
		       << " saved=" << path << "\n";
		return;
	}

	if (cla.test_capture) {
		#ifdef _WIN32
		WinMFCapture win_cap;
		if (win_cap.Open(cla.device_idx)) {
			Image img = win_cap.GrabFrame();
			if (!img.IsEmpty()) {
				String path = AppendFileName(GetFileDirectory(GetExeFilePath()), "test_capture.jpg");
				if (JPGEncoder().Quality(100).SaveFile(path, img)) {
					Cout() << "Frame captured and saved to " << path << "\n";
				} else {
					Cerr() << "Failed to save JPEG to " << path << "\n";
				}
			} else {
				Cerr() << "Failed to grab frame.\n";
			}
			win_cap.Close();
		} else {
			Cerr() << "Failed to open device.\n";
		}
		#else
		Cout() << "Test capture not implemented for this platform.\n";
		#endif
		return;
	}

	if (cla.capture_after) {
		#ifdef _WIN32
		if(cla.source != "camera") {
			Cerr() << "--capture-after currently supports --source camera on Windows\n";
			Exit(1);
		}
		WinMFCapture win_cap;
		if(!win_cap.Open(cla.device_idx)) {
			Cerr() << "Failed to open device.\n";
			Exit(1);
		}
		Cout() << "Waiting " << cla.capture_after_seconds << " seconds before capture...\n";
		Sleep(cla.capture_after_seconds * 1000);
		Image img = win_cap.GrabFrame();
		win_cap.Close();
		if(img.IsEmpty()) {
			Cerr() << "Failed to grab delayed frame.\n";
			Exit(1);
		}
		String path = AppendFileName(GetFileDirectory(GetExeFilePath()), "delayed_capture.jpg");
		if(!JPGEncoder().Quality(100).SaveFile(path, img)) {
			Cerr() << "Failed to save JPEG to " << path << "\n";
			Exit(1);
		}
		Cout() << "Delayed frame captured after " << cla.capture_after_seconds
		       << " seconds size=" << img.GetWidth() << "x" << img.GetHeight()
		       << " saved=" << path << "\n";
		#else
		Cout() << "Delayed capture not implemented for this platform.\n";
		#endif
		return;
	}

	g_running = true;
	int interval = 1000 / (cla.fps > 0 ? cla.fps : 10);
	
	TcpSocket listener;
	if (!listener.Listen(cla.port, 5)) { Cerr() << "Failed to listen on port " << cla.port << "\n"; return; }
	listener.Timeout(200);
	Cout() << "Video Server listening on port " << cla.port << "\n";
	Cout() << "Source: " << cla.source << ", wire format: " << cla.wire_format
	       << ", no-touch: " << (cla.no_touch ? "on" : "off") << "\n";

	Thread().Run([&] {
		#ifdef _WIN32
		WinMFCapture win_cap;
		Rect win_screen_rect;
		if (cla.source == "camera") {
			if (!win_cap.Open(cla.device_idx)) { Cerr() << "Failed to open device.\n"; g_running = false; return; }
		}
		else if(cla.source == "screen") {
			win_screen_rect = GetWinVirtualScreenRect();
			if(has_screen_rect_req)
				win_screen_rect &= screen_rect_req;
			if(win_screen_rect.IsEmpty()) {
				Cerr() << "Screen capture rect is empty\n";
				g_running = false;
				return;
			}
			Cout() << "Screen capture rect: " << win_screen_rect.left << "," << win_screen_rect.top
			       << " " << win_screen_rect.GetWidth() << "x" << win_screen_rect.GetHeight() << "\n";
		}
		#endif
		#ifdef PLATFORM_LINUX
		LocalLinuxMJPEGCapture cam;
		::Display* xdisplay = nullptr;
		Window xroot = 0;
		Rect xrect;
		if (cla.source == "camera") {
			if(!cam.Open(cla.device_idx)) { g_running = false; return; }
			if(cla.wire_format == "yuv")
				Cout() << "Warning: Linux camera source sends MJPEG payloads; ignoring --wire-format yuv.\n";
		}
		else if(cla.source == "screen") {
			xdisplay = XOpenDisplay(nullptr);
			if(!xdisplay) {
				Cerr() << "Failed to open X11 display for --source screen\n";
				g_running = false;
				return;
			}
			int sn = DefaultScreen(xdisplay);
			xroot = RootWindow(xdisplay, sn);
			xrect = RectC(0, 0, DisplayWidth(xdisplay, sn), DisplayHeight(xdisplay, sn));
			if(has_screen_rect_req)
				xrect &= screen_rect_req;
			if(xrect.IsEmpty()) {
				Cerr() << "Screen capture rect is empty\n";
				g_running = false;
				XCloseDisplay(xdisplay);
				return;
			}
			Cout() << "Screen capture rect: " << xrect.left << "," << xrect.top
			       << " " << xrect.GetWidth() << "x" << xrect.GetHeight() << "\n";
		}
		#endif

		#ifdef _WIN32
		LoopingVideoDecoder video_decoder;
		if(cla.source == "video") {
			String derr;
			if(!video_decoder.Open(cla.video_path, derr)) {
				Cerr() << "Failed to open video source: " << derr << "\n";
				g_running = false;
				return;
			}
			Cout() << "video_source_opened backend=libavcodec file=" << cla.video_path << "\n";
		}
		#endif

		TimeStop ts;
		TimeStop stats_ts;
		int image_dir_index = 0;
		while (g_running) {
			Image img;
			String payload;
			if (cla.source == "red") {
				if (cla.wire_format == "yuv") {
					const int w = 1920, h = 1080;
					Buffer<byte> yuyv(w * h * 2);
					byte* d = ~yuyv;
					for (int i = 0; i < w * h; i += 2) {
						*d++ = 82;  // Y for red
						*d++ = 90;  // U
						*d++ = 82;  // Y
						*d++ = 240; // V
					}
					payload = BuildYuvPacket(w, h, ~yuyv, w * h * 2);
				} else {
					ImageBuffer ib(1920, 1080);
					for (int y = 0; y < 1080; y++) {
						RGBA* row = ib[y];
						for (int x = 0; x < 1920; x++) {
							row[x].r = 255;
							row[x].g = 0;
							row[x].b = 0;
							row[x].a = 255;
						}
					}
					img = ib;
				}
			} else if(cla.source == "image") {
				img = StreamRaster::LoadFileAny(cla.image_path);
				if(img.IsEmpty()) {
					Cerr() << "Failed to load image source: " << cla.image_path << "\n";
					g_running = false;
					break;
				}
			} else if(cla.source == "image-dir") {
				if(image_dir_files.IsEmpty()) {
					Cerr() << "No image files available in source frame list\n";
					g_running = false;
					break;
				}
				const String& p = image_dir_files[image_dir_index % image_dir_files.GetCount()];
				image_dir_index++;
				img = StreamRaster::LoadFileAny(p);
				if(img.IsEmpty()) {
					Cerr() << "Failed to load image source from --image-dir: " << p << "\n";
					g_running = false;
					break;
				}
			} else if(cla.source == "video") {
			#ifdef _WIN32
				String derr;
				if(!video_decoder.NextFrame(img, derr)) {
					Cerr() << "Failed to decode video frame: " << derr << "\n";
					g_running = false;
					break;
				}
			#else
				Cerr() << "--source video is not supported on this platform\n";
				g_running = false;
				break;
			#endif
	} else if(cla.source == "screen") {
			#ifdef PLATFORM_LINUX
				img = CaptureX11Frame(xdisplay, xroot, xrect);
				if(img.IsEmpty()) {
					Cerr() << "Failed to capture X11 screen frame\n";
					g_running = false;
					break;
				}
			#elif defined(_WIN32)
				img = CaptureWinScreenFrame(win_screen_rect);
				if(img.IsEmpty()) {
					Cerr() << "Failed to capture Windows screen frame\n";
					g_running = false;
					break;
				}
			#else
				Cerr() << "--source screen is not supported on this platform\n";
				g_running = false;
				break;
			#endif
			} else {
			#ifdef _WIN32
			img = win_cap.GrabFrame();
			#endif
			#ifdef PLATFORM_LINUX
			if (cla.source == "camera") {
				if(!cam.GrabJpeg(payload)) {
					Cerr() << "Failed to capture MJPEG frame from camera\n";
					g_running = false;
					break;
				}
			}
			#endif
			}

			if (payload.IsEmpty() && !img.IsEmpty()) {
				if (!cla.no_touch && img.GetSize() != Size(1920, 1080))
					img = Rescale(img, 1920, 1080);
				payload = JPGEncoder().Quality(100).SaveString(img);
			}
			if (!payload.IsEmpty()) {
				{
					Mutex::Lock __(g_data_mutex);
					g_latest_jpeg = payload;
					g_latest_id++;
				}
				g_stat_frames.fetch_add(1, std::memory_order_relaxed);
				g_stat_bytes.fetch_add(payload.GetCount(), std::memory_order_relaxed);
			}
			if (cla.stats && stats_ts.Elapsed() >= 1000000) {
				static int64 last_frames = 0;
				static int64 last_bytes = 0;
				int64 frames = g_stat_frames.load(std::memory_order_relaxed);
				int64 bytes = g_stat_bytes.load(std::memory_order_relaxed);
				int64 df = frames - last_frames;
				int64 db = bytes - last_bytes;
				double avg = df > 0 ? (double)db / (double)df : 0.0;
				Cout() << "stats fps=" << df << " payload_avg=" << (int)avg << "B total_frames=" << frames << "\n";
				last_frames = frames;
				last_bytes = bytes;
				stats_ts.Reset();
			}
			int elapsed = (int)ts.Elapsed();
			if(elapsed < interval) Sleep(interval - elapsed);
			else Sleep(1);
			ts.Reset();
		}
		#ifdef PLATFORM_LINUX
		cam.Close();
		if (xdisplay) XCloseDisplay(xdisplay);
		#endif
	});

	while (g_running) {
		TcpSocket *ns = new TcpSocket;
		ns->Timeout(200);
		if(ns->Accept(listener)) Thread().Run([=] { ClientThread(ns); });
		else {
			delete ns;
			if(!g_running)
				break;
			if (listener.IsError()) { Cerr() << "Listener error: " << listener.GetErrorDesc() << "\n"; break; }
		}
		Sleep(10);
	}
	g_running = false;
	Cout() << "Video Server exiting.\n";
}
