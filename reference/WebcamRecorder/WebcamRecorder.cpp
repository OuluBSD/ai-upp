#include <CtrlLib/CtrlLib.h>
#include <plugin/libv4l2/libv4l2.h>
#include <plugin/jpg/jpg.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

using namespace Upp;

class CameraView : public Ctrl {
	Image img;
	
public:
	void SetImage(Image m) { img = m; Refresh(); }
	
	virtual void Paint(Draw& d) {
		d.DrawRect(GetSize(), Black());
		if(!img.IsEmpty()) {
			Size isz = img.GetSize();
			Size dsz = GetSize();
			Size sz = GetFitSize(isz, dsz);
			d.DrawImage((dsz.cx - sz.cx) / 2, (dsz.cy - sz.cy) / 2, sz.cx, sz.cy, img);
		}
	}
};

class WebcamRecorder : public TopWindow {
	Splitter splitter;
	ParentCtrl left_pane;
	CameraView camera_view;
	
	DropList webcams;
	DropList formats;
	DropList resolutions;
	Button start, stop;
	Label status;
	
	Thread work;
	Atomic exit_flag;
	bool is_recording;
	
	String current_dev;
	
	struct ResInfo : Moveable<ResInfo> {
		int w, h;
		String ToString() const { return AsString(w) + "x" + AsString(h); }
	};
	
	struct FormatInfo : Moveable<FormatInfo> {
		String description;
		unsigned int pixelformat;
		Vector<ResInfo> resolutions;
	};
	
	ArrayMap<String, FormatInfo> format_map;

	void YUYVToImage(const unsigned char* src, int w, int h, Image& img) {
		ImageBuffer ib(w, h);
		const unsigned char* s = src;
		RGBA* t = ib.Begin();
		for(int i = 0; i < w * h / 2; i++) {
			int y0 = s[0]; int u0 = s[1]; int y1 = s[2]; int v0 = s[3];
			s += 4;
			
			// Standard YUYV conversion
			auto YUV2RGB = [](int y, int u, int v, RGBA& p) {
				int c = y - 16; int d = u - 128; int e = v - 128;
				int r = (298 * c + 409 * e + 128) >> 8;
				int g = (298 * c - 100 * d - 208 * e + 128) >> 8;
				int b = (298 * c + 516 * d + 128) >> 8;
				// Swap R/B here for the "fix"
				p.b = (byte)clamp(r, 0, 255); 
				p.g = (byte)clamp(g, 0, 255); 
				p.r = (byte)clamp(b, 0, 255); 
				p.a = 255;
			};
			YUV2RGB(y0, u0, v0, *t++);
			YUV2RGB(y1, u0, v0, *t++);
		}
		img = ib;
	}

	void CaptureLoop() {
		String dev = current_dev;
		int width = 0, height = 0;
		unsigned int fmt = 0;
		
		int fmtIdx = formats.GetIndex();
		if(fmtIdx < 0) return;
		
		String fmtKey = formats.GetKey(fmtIdx);
		const FormatInfo& fi = format_map.Get(fmtKey);
		fmt = fi.pixelformat;
		
		int resIdx = resolutions.GetIndex();
		if(resIdx < 0) return;
		
		String resStr = resolutions.GetValue();
		Vector<String> parts = Split(resStr, 'x');
		if(parts.GetCount() == 2) {
			width = StrInt(parts[0]);
			height = StrInt(parts[1]);
		} else return;

		V4L2DeviceParameters param(dev.Begin(), fmt, width, height, 30, 1);
		V4l2Capture* capture = V4l2Capture::create(param);
		
		if(!capture || !capture->isReady()) {
			if(capture) delete capture;
			return;
		}
		
		int actualW = capture->getWidth();
		int actualH = capture->getHeight();
		Cout() << "Negotiated Resolution: " << actualW << "x" << actualH << "\n";
		
		String actualRes = AsString(actualW) + "x" + AsString(actualH);
		PostCallback([=] { status.SetLabel("Active: " + actualRes); });
		
		Buffer<char> buffer(capture->getBufferSize());
		
		int frame_count = 0;
		while(!exit_flag) {
			timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 100000;
			
			if(capture->isReadable(&tv)) {
				size_t n = capture->read(buffer, capture->getBufferSize());
				if(n > 0) {
					Image m;
					String fcc = V4l2Device::fourcc(capture->getFormat()).c_str();
					
					if(fcc == "MJPG") {
						// Use JPGRaster directly to avoid detection errors
						JPGRaster raster;
						m = raster.LoadString(String(~buffer, (int)n));
						if(!m.IsEmpty()) {
							// Swap R/B for MJPEG
							ImageBuffer ib(m);
							for(RGBA& p : ib) Swap(p.r, p.b);
							m = ib;
							
							if(frame_count % 30 == 0) {
								Cout() << "Decoded Resolution: " << m.GetSize().cx << "x" << m.GetSize().cy << "\n";
							}
							
							// Enforce aspect ratio/resolution if mismatch
							if(m.GetSize() != Size(actualW, actualH)) {
								m = Rescale(m, actualW, actualH);
							}
						}
					}
					else if(fcc == "YUYV") {
						YUYVToImage((const unsigned char*)~buffer, actualW, actualH, m);
					}
					
					if(!m.IsEmpty()) {
						String statusText = "Active: " + AsString(actualW) + "x" + AsString(actualH);
						if(m.GetSize() != Size(actualW, actualH)) {
							statusText << " (Img: " << m.GetSize().cx << "x" << m.GetSize().cy << ")";
						}
						PostCallback([=] { 
							camera_view.SetImage(m); 
							status.SetLabel(statusText);
						});
					}
					frame_count++;
				}
			}
		}
		
		delete capture;
		PostCallback([=] { status.SetLabel("Stopped"); });
	}

	void OnWebcamCursor() {
		OnStop();
		format_map.Clear();
		formats.Clear();
		resolutions.Clear();
		
		current_dev = webcams.GetValue();
		if(IsNull(current_dev)) return;
		
		int fd = open(current_dev, O_RDWR);
		if(fd < 0) return;
		
		struct v4l2_fmtdesc fmt;
		memset(&fmt, 0, sizeof(fmt));
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		
		while(ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == 0) {
			String desc = (char*)fmt.description;
			String fourcc = V4l2Device::fourcc(fmt.pixelformat).c_str();
			String key = fourcc; // Use FourCC as key
			
			FormatInfo& fi = format_map.GetAdd(key);
			fi.description = desc;
			fi.pixelformat = fmt.pixelformat;
			
			struct v4l2_frmsizeenum frmsize;
			memset(&frmsize, 0, sizeof(frmsize));
			frmsize.pixel_format = fmt.pixelformat;
			
			while(ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
				if(frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
					ResInfo& ri = fi.resolutions.Add();
					ri.w = frmsize.discrete.width;
					ri.h = frmsize.discrete.height;
				}
				frmsize.index++;
			}
			// Sort resolutions descending
			Sort(fi.resolutions, [](const ResInfo& a, const ResInfo& b) {
				return a.w * a.h > b.w * b.h;
			});
			
			formats.Add(key, desc + " (" + key + ")");
			fmt.index++;
		}
		
		close(fd);
		
		// Prefer MJPG
		int idx = formats.FindKey("MJPG");
		if(idx >= 0) formats.SetIndex(idx);
		else if(formats.GetCount() > 0) formats.SetIndex(0);
		
		OnWebcamFormat();
	}
	
	void OnWebcamFormat() {
		OnStop();
		resolutions.Clear();
		int idx = formats.GetIndex();
		if(idx < 0) return;
		
		String key = formats.GetKey(idx);
		const FormatInfo& fi = format_map.Get(key);
		
		for(const auto& r : fi.resolutions) {
			resolutions.Add(r.ToString());
		}
		
		if(resolutions.GetCount() > 0) resolutions.SetIndex(0); // Max resolution
	}
	
	void OnStart() {
		if(is_recording) return;
		exit_flag = 0;
		is_recording = true;
		
		// Disable controls
		webcams.Disable();
		formats.Disable();
		resolutions.Disable();
		start.Disable();
		stop.Enable();
		
		work.Run(THISBACK(CaptureLoop));
	}
	
	void OnStop() {
		if(!is_recording) return;
		exit_flag = 1;
		work.Wait();
		is_recording = false;
		
		webcams.Enable();
		formats.Enable();
		resolutions.Enable();
		start.Enable();
		stop.Disable();
	}
	
	void OnChange() {
		if(is_recording) OnStop();
	}

public:
	typedef WebcamRecorder CLASSNAME;

	WebcamRecorder() {
		Title("Webcam Recorder");
		
		splitter.Horz(left_pane, camera_view);
		splitter.SetPos(2000); // 20% width
		Add(splitter.SizePos());
		
		left_pane.Add(webcams.TopPos(10, 24).HSizePos(10, 10));
		left_pane.Add(formats.TopPos(40, 24).HSizePos(10, 10));
		left_pane.Add(resolutions.TopPos(70, 24).HSizePos(10, 10));
		left_pane.Add(start.TopPos(100, 24).LeftPos(10, 80));
		left_pane.Add(stop.TopPos(100, 24).RightPos(10, 80));
		left_pane.Add(status.TopPos(130, 24).HSizePos(10, 10));
		
		start.SetLabel("Start").WhenAction = THISBACK(OnStart);
		stop.SetLabel("Stop").WhenAction = THISBACK(OnStop);
		stop.Disable();
		
		webcams.WhenAction = THISBACK(OnWebcamCursor);
		formats.WhenAction = THISBACK(OnWebcamFormat);
		
		// Hook dropdown changes to auto-stop if running (though OnWebcamCursor rebuilds lists anyway)
		// But changing resolution should stop.
		resolutions.WhenAction = THISBACK(OnChange);
		
		// Enumerate devices
		for(int i = 0; i < 64; i++) {
			String path = "/dev/video" + AsString(i);
			int fd = open(path, O_RDWR);
			if(fd >= 0) {
				struct v4l2_capability cap;
				if(ioctl(fd, VIDIOC_QUERYCAP, &cap) == 0) {
					if(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
						webcams.Add(path);
					}
				}
				close(fd);
			}
		}
		
		if(webcams.GetCount() > 0) {
			webcams.SetIndex(0);
			OnWebcamCursor();
		}
		
		Sizeable().Zoomable();
	}
	
	~WebcamRecorder() {
		OnStop();
	}
};

GUI_APP_MAIN
{
	WebcamRecorder().Run();
}