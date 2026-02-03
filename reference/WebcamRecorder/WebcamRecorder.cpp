#include <CtrlLib/CtrlLib.h>
#include <Ctrl/Camera/Camera.h>
#include <plugin/libv4l2/libv4l2.h>
#include <plugin/jpg/jpg.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

using namespace Upp;

class WebcamRecorder : public TopWindow {
	Splitter splitter;
	ParentCtrl left_pane;
	CameraView camera_view;
	
	DropList webcams;
	DropList formats;
	DropList resolutions;
	DropList fps_list;
	Button start, stop;
#ifdef flagLEGACY
#ifdef flagLEGACY
	Option legacy_callbacks;
#endif
#endif
	Option show_stats;
	Option overlay_stats;
#ifdef flagLEGACY
	Option backend_draw_video;
#endif
	Label status;
	TimeCallback tc;
	TimeCallback timeout_tc;
	
	Thread work;
	Atomic exit_flag;
	bool is_recording;
	
	String current_dev;
	
	struct ResInfo : Moveable<ResInfo> {
		int w, h;
		Vector<double> fps;
		String ToString() const { return AsString(w) + "x" + AsString(h); }
	};
	
	struct FormatInfo : Moveable<FormatInfo> {
		String description;
		unsigned int pixelformat;
		VideoPixelFormat video_format = VID_PIX_UNKNOWN;
		Vector<ResInfo> resolutions;
	};
	
	ArrayMap<String, FormatInfo> format_map;
	Mutex background_mutex;
	Image background_img;
	String background_status;
	int background_frames = 0;
	int background_drops = 0;
	int background_decode_usecs = 0;
	int64 background_start_us = 0;
	int64 background_total_decode_us = 0;
#ifdef flagLEGACY
	int legacy_frames = 0;
	int64 legacy_start_us = 0;
	int64 legacy_total_decode_us = 0;
#endif
	bool use_draw_video = true;
	int timeout_ms = 0;

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

	VideoPixelFormat MapVideoFormat(const String& fourcc) const {
		if (fourcc == "MJPG") return VID_PIX_MJPEG;
		if (fourcc == "YUYV") return VID_PIX_YUYV;
		if (fourcc == "RGB3") return VID_PIX_RGB8;
		if (fourcc == "BGR3") return VID_PIX_BGR8;
		return VID_PIX_UNKNOWN;
	}

	void StopForTimeout() {
		if (is_recording)
			OnStop();
		Close();
	}

#ifdef flagLEGACY
	void CaptureLoopLegacy() {
		legacy_start_us = usecs();
		legacy_frames = 0;
		legacy_total_decode_us = 0;
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

		int fps = GetSelectedFps();
		V4L2DeviceParameters param(dev.Begin(), fmt, width, height, fps, 1);
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
					
					int decode_usecs = 0;
					if(fcc == "MJPG") {
						TimeStop ts;
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
						decode_usecs = ts.Elapsed();
					}
					else if(fcc == "YUYV") {
						TimeStop ts;
						YUYVToImage((const unsigned char*)~buffer, actualW, actualH, m);
						decode_usecs = ts.Elapsed();
					}
					legacy_frames++;
					legacy_total_decode_us += decode_usecs;
					
					if(!m.IsEmpty()) {
						String statusText = "Active: " + AsString(actualW) + "x" + AsString(actualH);
						if(m.GetSize() != Size(actualW, actualH)) {
							statusText << " (Img: " << m.GetSize().cx << "x" << m.GetSize().cy << ")";
						}
						statusText << " | decode=" << decode_usecs << "us";
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
#endif

	void CaptureLoopThreaded() {
		if (use_draw_video) {
			VideoV4L2Backend backend;
			backend.SetDevice(current_dev);
			VideoPixelFormat vpix = VID_PIX_MJPEG;
			int fmtIdx = formats.GetIndex();
			if (fmtIdx >= 0) {
				String fmtKey = formats.GetKey(fmtIdx);
				const FormatInfo& fi = format_map.Get(fmtKey);
				if (fi.video_format != VID_PIX_UNKNOWN)
					vpix = fi.video_format;
			}
			int resIdx = resolutions.GetIndex();
			Size sz(0,0);
			if (resIdx >= 0) {
				String resStr = resolutions.GetValue();
				Vector<String> parts = Split(resStr, 'x');
				if (parts.GetCount() == 2) {
					sz.cx = StrInt(parts[0]);
					sz.cy = StrInt(parts[1]);
				}
			}
			if (sz.cx <= 0 || sz.cy <= 0)
				sz = Size(640, 480);
			int fps = GetSelectedFps();
			backend.SetFormat(vpix, sz, fps);
			if (!backend.Open())
				return;
			background_start_us = usecs();
			background_total_decode_us = 0;
			while (!exit_flag) {
				Vector<VideoFrame> frames;
				backend.PopFrames(frames);
				if (frames.IsEmpty()) {
					Sleep(1);
					continue;
				}
				for (const auto& vf : frames) {
					String statusText = "Active: " + AsString(vf.size.cx) + "x" + AsString(vf.size.cy);
					Mutex::Lock __(background_mutex);
					background_img = vf.img;
					background_status = statusText;
					background_frames++;
					background_decode_usecs = 0;
				}
			}
			backend.Close();
			Mutex::Lock __(background_mutex);
			background_status = "Stopped";
			return;
		}

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

		int fps = GetSelectedFps();
		V4L2DeviceParameters param(dev.Begin(), fmt, width, height, fps, 1);
		V4l2Capture* capture = V4l2Capture::create(param);
		
		if(!capture || !capture->isReady()) {
			if(capture) delete capture;
			return;
		}
		
		int actualW = capture->getWidth();
		int actualH = capture->getHeight();
		String actualRes = AsString(actualW) + "x" + AsString(actualH);
		{
			Mutex::Lock __(background_mutex);
			background_status = "Active: " + actualRes;
		}
		
		Buffer<char> buffer(capture->getBufferSize());
		
		background_start_us = usecs();
		background_total_decode_us = 0;
		while(!exit_flag) {
			timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 100000;
			
			if(capture->isReadable(&tv)) {
				TimeStop ts;
				size_t n = capture->read(buffer, capture->getBufferSize());
				if(n > 0) {
					Image m;
					String fcc = V4l2Device::fourcc(capture->getFormat()).c_str();
					
					if(fcc == "MJPG") {
						JPGRaster raster;
						m = raster.LoadString(String(~buffer, (int)n));
						if(!m.IsEmpty()) {
							ImageBuffer ib(m);
							for(RGBA& p : ib) Swap(p.r, p.b);
							m = ib;
							if(m.GetSize() != Size(actualW, actualH)) {
								m = Rescale(m, actualW, actualH);
							}
						}
					}
					else if(fcc == "YUYV") {
						YUYVToImage((const unsigned char*)~buffer, actualW, actualH, m);
					}
					
					int decode_usecs = ts.Elapsed();
					if(!m.IsEmpty()) {
						String statusText = "Active: " + actualRes;
						if(m.GetSize() != Size(actualW, actualH)) {
							statusText << " (Img: " << m.GetSize().cx << "x" << m.GetSize().cy << ")";
						}
						{
							Mutex::Lock __(background_mutex);
							background_img = m;
							background_status = statusText;
							background_frames++;
							background_decode_usecs = decode_usecs;
							background_total_decode_us += decode_usecs;
						}
					}
					else {
						Mutex::Lock __(background_mutex);
						background_drops++;
					}
				}
			}
		}
		
		delete capture;
		{
			Mutex::Lock __(background_mutex);
			background_status = "Stopped";
		}
	}

	void Data() {
#ifdef flagLEGACY
		if (legacy_callbacks)
			return;
#endif
		Image img;
		String statusText;
		int frames = 0;
		int drops = 0;
		int decode_usecs = 0;
		{
			Mutex::Lock __(background_mutex);
			img = background_img;
			statusText = background_status;
			frames = background_frames;
			drops = background_drops;
			decode_usecs = background_decode_usecs;
		}
		if (show_stats && !statusText.IsEmpty()) {
			statusText << " | frames=" << frames << " drops=" << drops
			           << " decode=" << decode_usecs << "us";
		}
		if (!img.IsEmpty())
			camera_view.SetImage(img);
		if (!statusText.IsEmpty())
			status.SetLabel(statusText);
	}

	void OnWebcamCursor() {
		OnStop();
		format_map.Clear();
		formats.Clear();
		resolutions.Clear();
		fps_list.Clear();
		
		current_dev = webcams.GetKey(webcams.GetIndex());
		if (IsNull(current_dev) || current_dev.IsEmpty())
			current_dev = webcams.GetValue();
		if (IsNull(current_dev) || current_dev.IsEmpty()) return;
		
		auto AddResolution = [&](FormatInfo& fi, int w, int h, double fps) {
			for (const auto& r : fi.resolutions) {
				if (r.w == w && r.h == h) {
					if (fps > 0) {
						bool exists = false;
						for (double f : r.fps) {
							if (fabs(f - fps) < 0.01) { exists = true; break; }
						}
						if (!exists)
							const_cast<ResInfo&>(r).fps.Add(fps);
					}
					return;
				}
			}
			ResInfo& ri = fi.resolutions.Add();
			ri.w = w;
			ri.h = h;
			if (fps > 0)
				ri.fps.Add(fps);
		};

		VideoDeviceCaps caps;
		V4L2DeviceManager mgr;
		if (mgr.EnumerateCaps(current_dev, caps)) {
			for (const auto& capfmt : caps.formats) {
				String key = capfmt.pixelformat ? FormatIntHex(capfmt.pixelformat) : capfmt.description;
				String fourcc = capfmt.pixelformat ? String(V4l2Device::fourcc(capfmt.pixelformat).c_str()) : String();
				if (!fourcc.IsEmpty())
					key = fourcc;
				FormatInfo& fi = format_map.GetAdd(key);
				fi.description = capfmt.description;
				fi.pixelformat = capfmt.pixelformat;
				fi.video_format = MapVideoFormat(key);
				for (const auto& res : capfmt.resolutions)
					AddResolution(fi, res.size.cx, res.size.cy, res.fps);
				Sort(fi.resolutions, [](const ResInfo& a, const ResInfo& b) {
					return a.w * a.h > b.w * b.h;
				});
				formats.Add(key, fi.description + " (" + key + ")");
			}
		}
		else {
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
				fi.video_format = MapVideoFormat(key);
				
				struct v4l2_frmsizeenum frmsize;
				memset(&frmsize, 0, sizeof(frmsize));
				frmsize.pixel_format = fmt.pixelformat;
				
				while(ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
					if(frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
						AddResolution(fi, frmsize.discrete.width, frmsize.discrete.height, 30.0);
					frmsize.index++;
				}
				Sort(fi.resolutions, [](const ResInfo& a, const ResInfo& b) {
					return a.w * a.h > b.w * b.h;
				});
				
				formats.Add(key, desc + " (" + key + ")");
				fmt.index++;
			}
			
			close(fd);
		}
		
		// Prefer MJPG
		int idx = formats.FindKey("MJPG");
		if(idx >= 0) formats.SetIndex(idx);
		else if(formats.GetCount() > 0) formats.SetIndex(0);
		
		OnWebcamFormat();
	}
	
	void OnWebcamFormat() {
		OnStop();
		resolutions.Clear();
		fps_list.Clear();
		int idx = formats.GetIndex();
		if(idx < 0) return;
		
		String key = formats.GetKey(idx);
		const FormatInfo& fi = format_map.Get(key);
		
		for(const auto& r : fi.resolutions) {
			resolutions.Add(r.ToString());
		}
		
		if(resolutions.GetCount() > 0) {
			resolutions.SetIndex(0);
			OnResolution();
		}
	}

	void OnResolution() {
		fps_list.Clear();
		int ridx = resolutions.GetIndex();
		int fidx = formats.GetIndex();
		if (ridx < 0 || fidx < 0)
			return;
		String key = formats.GetKey(fidx);
		const FormatInfo& fi = format_map.Get(key);
		if (ridx >= fi.resolutions.GetCount())
			return;
		const ResInfo& ri = fi.resolutions[ridx];
		if (ri.fps.IsEmpty()) {
			fps_list.Add("30");
		} else {
			for (double f : ri.fps)
				fps_list.Add(Format("%.0f", f));
		}
		if (fps_list.GetCount() > 0)
			fps_list.SetIndex(0);
	}
	
	void OnStart() {
		if(is_recording) return;
		exit_flag = 0;
		is_recording = true;
#ifdef flagLEGACY
		use_draw_video = backend_draw_video;
#else
		use_draw_video = true;
#endif
#ifdef flagLEGACY
		if (!legacy_callbacks) {
			Mutex::Lock __(background_mutex);
			background_frames = 0;
			background_drops = 0;
			background_decode_usecs = 0;
			background_start_us = 0;
			background_total_decode_us = 0;
			background_status.Clear();
		}
#else
		{
			Mutex::Lock __(background_mutex);
			background_frames = 0;
			background_drops = 0;
			background_decode_usecs = 0;
			background_start_us = 0;
			background_total_decode_us = 0;
			background_status.Clear();
		}
#endif
		
		// Disable controls
		webcams.Disable();
		formats.Disable();
		resolutions.Disable();
		fps_list.Disable();
		start.Disable();
		stop.Enable();
		
#ifdef flagLEGACY
		if (legacy_callbacks)
			work.Run(THISBACK(CaptureLoopLegacy));
		else
			work.Run(THISBACK(CaptureLoopThreaded));
#else
		work.Run(THISBACK(CaptureLoopThreaded));
#endif

		if (timeout_ms > 0)
			timeout_tc.Set(-timeout_ms, THISBACK(StopForTimeout));
	}
	
	void OnStop() {
		if(!is_recording) return;
		exit_flag = 1;
		work.Wait();
		is_recording = false;

#ifdef flagLEGACY
		if (!legacy_callbacks) {
			int frames = 0;
			int drops = 0;
			int64 start_us = 0;
			int64 total_decode_us = 0;
			Mutex::Lock __(background_mutex);
			frames = background_frames;
			drops = background_drops;
			start_us = background_start_us;
			total_decode_us = background_total_decode_us;
			background_img.Clear();
			background_status = "Stopped";
			if (start_us > 0) {
				int64 elapsed_us = usecs() - start_us;
				double fps = elapsed_us > 0 ? (double)frames * 1000000.0 / (double)elapsed_us : 0.0;
				double avg_decode = frames > 0 ? (double)total_decode_us / (double)frames : 0.0;
				Cout() << "Threaded: frames=" << frames << " drops=" << drops
				       << " fps=" << fps << " avg_decode_us=" << avg_decode << "\n";
			}
		}
#ifdef flagLEGACY
		else {
			if (legacy_start_us > 0) {
				int64 elapsed_us = usecs() - legacy_start_us;
				double fps = elapsed_us > 0 ? (double)legacy_frames * 1000000.0 / (double)elapsed_us : 0.0;
				double avg_decode = legacy_frames > 0 ? (double)legacy_total_decode_us / (double)legacy_frames : 0.0;
				Cout() << "Legacy: frames=" << legacy_frames
				       << " fps=" << fps << " avg_decode_us=" << avg_decode << "\n";
			}
		}
#endif
#else
		{
			int frames = 0;
			int drops = 0;
			int64 start_us = 0;
			int64 total_decode_us = 0;
			Mutex::Lock __(background_mutex);
			frames = background_frames;
			drops = background_drops;
			start_us = background_start_us;
			total_decode_us = background_total_decode_us;
			background_img.Clear();
			background_status = "Stopped";
			if (start_us > 0) {
				int64 elapsed_us = usecs() - start_us;
				double fps = elapsed_us > 0 ? (double)frames * 1000000.0 / (double)elapsed_us : 0.0;
				double avg_decode = frames > 0 ? (double)total_decode_us / (double)frames : 0.0;
				Cout() << "Threaded: frames=" << frames << " drops=" << drops
				       << " fps=" << fps << " avg_decode_us=" << avg_decode << "\n";
			}
		}
#endif
		Cout() << "Backend: " << (use_draw_video ? "Draw/Video" : "Direct V4L2")
		       << " | Mode: "
#ifdef flagLEGACY
		       << (legacy_callbacks ? "Legacy" : "Threaded")
#else
		       << "Threaded"
#endif
		       << "\n";
		
		webcams.Enable();
		formats.Enable();
		resolutions.Enable();
		fps_list.Enable();
		start.Enable();
		stop.Disable();
		timeout_tc.Kill();
	}

	void DrawOverlay(Draw& d, const Rect& r, const Image& img) {
		if (!overlay_stats || img.IsEmpty())
			return;
		String text;
		{
			Mutex::Lock __(background_mutex);
			text = Format("frames=%d drops=%d decode=%dus", background_frames, background_drops, background_decode_usecs);
		}
		d.DrawText(r.left + 6, r.top + 6, text, Arial(12).Bold(), White());
	}
	
	void OnChange() {
		if(is_recording) OnStop();
	}

	int GetSelectedFps() const {
		int idx = fps_list.GetIndex();
		if (idx < 0)
			return 30;
		String s = fps_list.GetValue();
		if (s.IsEmpty())
			return 30;
		return (int)ScanDouble(s);
	}

public:
	typedef WebcamRecorder CLASSNAME;

	void SetDevice(const String& path) {
		int idx = webcams.Find(path);
		if (idx >= 0) {
			webcams.SetIndex(idx);
			OnWebcamCursor();
		}
	}
#ifdef flagLEGACY
	void SetLegacy(bool b) { legacy_callbacks = b; }
#else
	void SetLegacy(bool) {}
#endif
#ifdef flagLEGACY
	void SetBackendDrawVideo(bool b) { backend_draw_video = b; }
#else
	void SetBackendDrawVideo(bool) {}
#endif
	void SetTimeoutMs(int ms) { timeout_ms = ms; }
	void AutoStartIfNeeded() {
		if (timeout_ms > 0)
			PostCallback(THISBACK(OnStart));
	}

	WebcamRecorder() {
		Title("Webcam Recorder");
		
		splitter.Horz(left_pane, camera_view);
		splitter.SetPos(2000); // 20% width
		Add(splitter.SizePos());
		
		left_pane.Add(webcams.TopPos(10, 24).HSizePos(10, 10));
		left_pane.Add(formats.TopPos(40, 24).HSizePos(10, 10));
		left_pane.Add(resolutions.TopPos(70, 24).HSizePos(10, 10));
		left_pane.Add(fps_list.TopPos(100, 24).HSizePos(10, 10));
		left_pane.Add(start.TopPos(130, 24).LeftPos(10, 80));
		left_pane.Add(stop.TopPos(130, 24).RightPos(10, 80));
		left_pane.Add(status.TopPos(160, 24).HSizePos(10, 10));
#ifdef flagLEGACY
		left_pane.Add(legacy_callbacks.TopPos(190, 24).HSizePos(10, 10));
#endif
		left_pane.Add(show_stats.TopPos(220, 24).HSizePos(10, 10));
		left_pane.Add(overlay_stats.TopPos(250, 24).HSizePos(10, 10));
#ifdef flagLEGACY
		left_pane.Add(backend_draw_video.TopPos(280, 24).HSizePos(10, 10));
#endif
		
		start.SetLabel("Start").WhenAction = THISBACK(OnStart);
		stop.SetLabel("Stop").WhenAction = THISBACK(OnStop);
		stop.Disable();

#ifdef flagLEGACY
		legacy_callbacks.SetLabel("Legacy callbacks");
		legacy_callbacks.WhenAction = THISBACK(OnChange);
#endif
		show_stats.SetLabel("Show stats");
		show_stats = true;
		overlay_stats.SetLabel("Overlay stats");
		overlay_stats = true;
#ifdef flagLEGACY
		backend_draw_video.SetLabel("Backend: Draw/Video");
		backend_draw_video.WhenAction = THISBACK(OnChange);
#endif
		
		webcams.WhenAction = THISBACK(OnWebcamCursor);
		formats.WhenAction = THISBACK(OnWebcamFormat);
		fps_list.WhenAction = THISBACK(OnChange);
		
		// Hook dropdown changes to auto-stop if running (though OnWebcamCursor rebuilds lists anyway)
		// But changing resolution should stop.
		resolutions.WhenAction = THISBACK(OnChange);
		
		// Enumerate devices via Draw/Video manager (with fallback)
		Vector<VideoDeviceInfo> devs;
		V4L2DeviceManager mgr;
		mgr.Enumerate(devs);
		for (const auto& dev : devs) {
			String label = dev.path;
			if (!dev.name.IsEmpty())
				label = dev.name + " (" + dev.path + ")";
			webcams.Add(dev.path, label);
		}
		if (webcams.GetCount() == 0) {
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
		}
		
		if(webcams.GetCount() > 0) {
			webcams.SetIndex(0);
			OnWebcamCursor();
		}
		
		camera_view.WhenOverlay = THISBACK(DrawOverlay);
		Sizeable().Zoomable();
		tc.Set(-1000/60, THISBACK(Data));
	}
	
	~WebcamRecorder() {
		timeout_tc.Kill();
		OnStop();
	}
};

GUI_APP_MAIN
{
	const Vector<String>& args = CommandLine();
	String device;
	bool legacy = false;
	bool backend_dv = false;
	int timeout_ms = 0;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--device" && i + 1 < args.GetCount())
			device = args[i + 1];
#ifdef flagLEGACY
		if(args[i] == "--legacy")
			legacy = true;
#endif
#ifdef flagLEGACY
		if(args[i] == "--backend-draw-video")
			backend_dv = true;
#endif
		if(args[i] == "--timeout" && i + 1 < args.GetCount())
			timeout_ms = atoi(args[i + 1]);
	}
	auto ListDevices = [] {
		Vector<VideoDeviceInfo> devs;
		V4L2DeviceManager mgr;
		mgr.Enumerate(devs);
		for (const auto& dev : devs) {
			Cout() << dev.path;
			if (!dev.name.IsEmpty())
				Cout() << " | " << dev.name;
			if (!dev.driver.IsEmpty())
				Cout() << " | driver=" << dev.driver;
			if (!dev.bus_info.IsEmpty())
				Cout() << " | bus=" << dev.bus_info;
			Cout() << "\n";
		}
	};

	auto ListFormats = [](const String& devpath) {
		VideoDeviceCaps caps;
		V4L2DeviceManager mgr;
		if (!mgr.EnumerateCaps(devpath, caps)) {
			Cout() << "No formats for " << devpath << "\n";
			return;
		}
		for (const auto& f : caps.formats) {
			String fourcc = f.pixelformat ? String(V4l2Device::fourcc(f.pixelformat).c_str()) : String();
			Cout() << (fourcc.IsEmpty() ? f.description : fourcc) << " | " << f.description << "\n";
		}
	};

	auto ListResolutions = [](const String& devpath, const String& fmt_key) {
		VideoDeviceCaps caps;
		V4L2DeviceManager mgr;
		if (!mgr.EnumerateCaps(devpath, caps)) {
			Cout() << "No formats for " << devpath << "\n";
			return;
		}
		for (const auto& f : caps.formats) {
			String fourcc = f.pixelformat ? String(V4l2Device::fourcc(f.pixelformat).c_str()) : String();
			if (!fmt_key.IsEmpty() && fourcc != fmt_key)
				continue;
			for (const auto& r : f.resolutions) {
				Cout() << fourcc << " " << r.size.cx << "x" << r.size.cy;
				if (r.fps > 0)
					Cout() << " @" << r.fps;
				Cout() << "\n";
			}
		}
	};

	bool list_devices = false;
	bool list_formats = false;
	bool list_resolutions = false;
	String list_format_key;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--list-devices") list_devices = true;
		if(args[i] == "--list-formats") list_formats = true;
		if(args[i] == "--list-resolutions") list_resolutions = true;
		if(args[i] == "--format" && i + 1 < args.GetCount())
			list_format_key = args[i + 1];
	}
	if (list_devices) { ListDevices(); return; }
	if (list_formats) {
		if (device.IsEmpty()) device = "/dev/video0";
		ListFormats(device);
		return;
	}
	if (list_resolutions) {
		if (device.IsEmpty()) device = "/dev/video0";
		ListResolutions(device, list_format_key);
		return;
	}

	WebcamRecorder app;
	if (!device.IsEmpty())
		app.SetDevice(device);
#ifdef flagLEGACY
	app.SetLegacy(legacy);
#else
	(void)legacy;
#endif
#ifdef flagLEGACY
	app.SetBackendDrawVideo(backend_dv);
#else
	(void)backend_dv;
#endif
	app.SetTimeoutMs(timeout_ms);
	app.AutoStartIfNeeded();
	app.Run();
}
