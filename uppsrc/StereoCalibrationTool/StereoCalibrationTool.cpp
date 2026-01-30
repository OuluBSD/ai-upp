#include "StereoCalibrationTool.h"
NAMESPACE_UPP

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static bool SplitStereoImage(const Image& src, Image& left, Image& right) {
	Size sz = src.GetSize();
	if (sz.cx < 2 || sz.cy <= 0)
		return false;
	int half = sz.cx / 2;
	if (half <= 0)
		return false;
	const RGBA* src_data = ~src;
	if (!src_data)
		return false;
	ImageBuffer lb(half, sz.cy);
	ImageBuffer rb(half, sz.cy);
	for (int y = 0; y < sz.cy; y++) {
		const RGBA* row = src_data + y * sz.cx;
		memcpy(lb[y], row, half * sizeof(RGBA));
		memcpy(rb[y], row + half, half * sizeof(RGBA));
	}
	left = lb;
	right = rb;
	return true;
}

static Image CopyFrameImage(const VisualFrame& frame) {
	if (!IsNull(frame.img))
		return frame.img;
	if (frame.format != GEOM_EVENT_CAM_RGBA8 || frame.width <= 0 || frame.height <= 0 || !frame.data)
		return Image();
	ImageBuffer ib(frame.width, frame.height);
	const RGBA* src = (const RGBA*)frame.data;
	for (int y = 0; y < frame.height; y++)
		memcpy(ib[y], src + y * frame.width, frame.width * sizeof(RGBA));
	return ib;
}

static bool IsFrameNonBlack(const Image& img) {
	if (img.IsEmpty())
		return false;
	const RGBA* src = ~img;
	if (!src)
		return false;
	int count = img.GetLength();
	if (count <= 0)
		return false;
	int step = max(1, count / 2048);
	for (int i = 0; i < count; i += step) {
		const RGBA& c = src[i];
		if (c.r > 8 || c.g > 8 || c.b > 8)
			return true;
	}
	return false;
}

static Image ConvertRgb24ToImage(const byte* data, int width, int height) {
	if (!data || width <= 0 || height <= 0)
		return Image();
	ImageBuffer ib(width, height);
	const byte* src = data;
	for (int y = 0; y < height; y++) {
		RGBA* dst = ib[y];
		for (int x = 0; x < width; x++) {
			dst[x].r = *src++;
			dst[x].g = *src++;
			dst[x].b = *src++;
			dst[x].a = 255;
		}
	}
	return ib;
}

static Image ConvertYuyvToImage(const byte* data, int width, int height) {
	if (!data || width <= 0 || height <= 0)
		return Image();
	ImageBuffer ib(width, height);
	const byte* src = data;
	for (int y = 0; y < height; y++) {
		RGBA* dst = ib[y];
		for (int x = 0; x < width; x += 2) {
			int y0 = src[0];
			int u = src[1] - 128;
			int y1 = src[2];
			int v = src[3] - 128;
			src += 4;
			auto conv = [&](int yy) -> RGBA {
				int c = yy - 16;
				int d = u;
				int e = v;
				int r = (298 * c + 409 * e + 128) >> 8;
				int g = (298 * c - 100 * d - 208 * e + 128) >> 8;
				int b = (298 * c + 516 * d + 128) >> 8;
				RGBA out;
				out.r = (byte)Clamp(r, 0, 255);
				out.g = (byte)Clamp(g, 0, 255);
				out.b = (byte)Clamp(b, 0, 255);
				out.a = 255;
				return out;
			};
			dst[x] = conv(y0);
			if (x + 1 < width)
				dst[x + 1] = conv(y1);
		}
	}
	return ib;
}

static Image ConvertMjpegToImage(const byte* data, int bytes) {
	if (!data || bytes <= 0)
		return Image();
	String s((const char*)data, bytes);
	return StreamRaster::LoadStringAny(s);
}

static bool IsValidAnglePoly(const vec4& poly) {
	return fabs(poly.data[0]) > 1e-9;
}

static bool IsSamePoly(const vec4& a, const vec4& b) {
	for (int i = 0; i < 4; i++) {
		if (fabs(a.data[i] - b.data[i]) > 1e-6)
			return false;
	}
	return true;
}

static RGBA SampleBilinear(const Image& img, float x, float y) {
	Size sz = img.GetSize();
	if (sz.cx <= 0 || sz.cy <= 0)
		return RGBA{0,0,0,0};
	x = Clamp(x, 0.0f, (float)(sz.cx - 1));
	y = Clamp(y, 0.0f, (float)(sz.cy - 1));
	int x0 = (int)floor(x);
	int y0 = (int)floor(y);
	int x1 = min(x0 + 1, sz.cx - 1);
	int y1 = min(y0 + 1, sz.cy - 1);
	float fx = x - x0;
	float fy = y - y0;
	const RGBA* row0 = img[y0];
	const RGBA* row1 = img[y1];
	auto lerp = [](float a, float b, float t) { return a + (b - a) * t; };
	RGBA c00 = row0[x0];
	RGBA c10 = row0[x1];
	RGBA c01 = row1[x0];
	RGBA c11 = row1[x1];
	RGBA out;
	out.r = (byte)Clamp((int)lerp(lerp(c00.r, c10.r, fx), lerp(c01.r, c11.r, fx), fy), 0, 255);
	out.g = (byte)Clamp((int)lerp(lerp(c00.g, c10.g, fx), lerp(c01.g, c11.g, fx), fy), 0, 255);
	out.b = (byte)Clamp((int)lerp(lerp(c00.b, c10.b, fx), lerp(c01.b, c11.b, fx), fy), 0, 255);
	out.a = (byte)Clamp((int)lerp(lerp(c00.a, c10.a, fx), lerp(c01.a, c11.a, fx), fy), 0, 255);
	return out;
}

static Image UndistortImage(const Image& src, const LensPoly& lens, float linear_scale) {
	if (src.IsEmpty() || linear_scale <= 0)
		return Image();
	Size sz = src.GetSize();
	ImageBuffer out(sz);
	vec2 pp = lens.GetPrincipalPoint();
	float cx = pp[0];
	float cy = pp[1];
	if (cx <= 0 || cy <= 0) {
		cx = sz.cx * 0.5f;
		cy = sz.cy * 0.5f;
	}
	for (int y = 0; y < sz.cy; y++) {
		RGBA* dst = out[y];
		float dy = y - cy;
		for (int x = 0; x < sz.cx; x++) {
			float dx = x - cx;
			float r = sqrtf(dx * dx + dy * dy);
			if (r < 1e-6f) {
				dst[x] = SampleBilinear(src, cx, cy);
				continue;
			}
			float angle = r / linear_scale;
			float rd = lens.AngleToPixel(angle);
			float scale = rd / r;
			float sx = cx + dx * scale;
			float sy = cy + dy * scale;
			dst[x] = SampleBilinear(src, sx, sy);
		}
	}
	return out;
}

bool StereoCalibrationTool::HmdStereoSource::Start() {
	if (running)
		return true;
	if (!sys.Initialise())
		return false;
	cam.Create();
	if (cam.IsEmpty() || !cam->Open()) {
		cam.Clear();
		sys.Uninitialise();
		return false;
	}
	cam->SetVerbose(verbose);
	running = true;
	return true;
}

void StereoCalibrationTool::HmdStereoSource::Stop() {
	if (cam)
		cam->Close();
	cam.Clear();
	if (running)
		sys.Uninitialise();
	running = false;
	last_left = Image();
	last_right = Image();
}

bool StereoCalibrationTool::HmdStereoSource::ReadFrame(VisualFrame& left, VisualFrame& right, bool prefer_bright) {
	if (!cam || !cam->IsOpen())
		return false;
	Vector<HMD::CameraFrame> frames;
	cam->PopFrames(frames);
	if (frames.IsEmpty())
		return false;
	
	int best_idx = -1;
	if (prefer_bright) {
		for(int i = 0; i < frames.GetCount(); i++) {
			if(frames[i].is_bright) {
				best_idx = i;
			}
		}
	}
	
	if (best_idx < 0) best_idx = frames.GetCount() - 1;
	
	const HMD::CameraFrame& f = frames[best_idx];
	if (!SplitStereoImage(f.img, last_left, last_right))
		return false;
	last_is_bright = f.is_bright;

	left.timestamp_us = usecs();
	left.format = GEOM_EVENT_CAM_RGBA8;
	left.width = last_left.GetWidth();
	left.height = last_left.GetHeight();
	left.stride = left.width * (int)sizeof(RGBA);
	left.eye = 0;
	left.data = (const byte*)~last_left;
	left.data_bytes = last_left.GetLength() * (int)sizeof(RGBA);
	left.img = last_left;
	left.flags = last_is_bright ? VIS_FRAME_BRIGHT : VIS_FRAME_DARK;
	left.serial = f.serial;

	right.timestamp_us = left.timestamp_us;
	right.format = GEOM_EVENT_CAM_RGBA8;
	right.width = last_right.GetWidth();
	right.height = last_right.GetHeight();
	right.stride = right.width * (int)sizeof(RGBA);
	right.eye = 1;
	right.data = (const byte*)~last_right;
	right.data_bytes = last_right.GetLength() * (int)sizeof(RGBA);
	right.img = last_right;
	right.flags = left.flags;
	right.serial = left.serial;

	return true;
}

bool StereoCalibrationTool::HmdStereoSource::PeakFrame(VisualFrame& left, VisualFrame& right, bool prefer_bright) {
	if (!cam || !cam->IsOpen())
		return false;
	Vector<HMD::CameraFrame> frames;
	cam->PeakFrames(frames);
	if (frames.IsEmpty())
		return false;
	
	int best_idx = -1;
	if (prefer_bright) {
		for(int i = 0; i < frames.GetCount(); i++) {
			if(frames[i].is_bright) {
				best_idx = i;
			}
		}
	}
	
	if (best_idx < 0) best_idx = frames.GetCount() - 1;
	
	const HMD::CameraFrame& f = frames[best_idx];
	if (!SplitStereoImage(f.img, last_left, last_right))
		return false;
	last_is_bright = f.is_bright;

	left.timestamp_us = usecs();
	left.format = GEOM_EVENT_CAM_RGBA8;
	left.width = last_left.GetWidth();
	left.height = last_left.GetHeight();
	left.stride = left.width * (int)sizeof(RGBA);
	left.eye = 0;
	left.data = (const byte*)~last_left;
	left.data_bytes = last_left.GetLength() * (int)sizeof(RGBA);
	left.img = last_left;
	left.flags = last_is_bright ? VIS_FRAME_BRIGHT : VIS_FRAME_DARK;
	left.serial = f.serial;

	right.timestamp_us = left.timestamp_us;
	right.format = GEOM_EVENT_CAM_RGBA8;
	right.width = last_right.GetWidth();
	right.height = last_right.GetHeight();
	right.stride = right.width * (int)sizeof(RGBA);
	right.eye = 1;
	right.data = (const byte*)~last_right;
	right.data_bytes = last_right.GetLength() * (int)sizeof(RGBA);
	right.img = last_right;
	right.flags = left.flags;
	right.serial = left.serial;

	return true;
}

#ifdef flagLINUX
bool StereoCalibrationTool::UsbStereoSource::Start() {
	if (running)
		return true;
	if (capture)
		capture.Clear();
	if (device_path.IsEmpty())
		device_path = "/dev/video0";
	std::list<unsigned int> formats;
	formats.push_back(V4L2_PIX_FMT_RGB24);
	formats.push_back(V4L2_PIX_FMT_YUYV);
	formats.push_back(V4L2_PIX_FMT_MJPEG);
	V4L2DeviceParameters params(device_path.Begin(), formats, 2560, 720, 30, 0);
	capture = V4l2Capture::create(params, V4l2Access::IOTYPE_MMAP);
	if (!capture) {
		V4L2DeviceParameters fallback(device_path.Begin(), formats, 1280, 720, 30, 0);
		capture = V4l2Capture::create(fallback, V4l2Access::IOTYPE_MMAP);
	}
	if (!capture)
		return false;
	if (!capture->start()) {
		capture.Clear();
		return false;
	}

	width = (int)capture->getWidth();
	height = (int)capture->getHeight();
	pixfmt = (int)capture->getFormat();
	raw.SetCount((int)capture->getBufferSize());
	running = true;
	return true;
}

void StereoCalibrationTool::UsbStereoSource::Stop() {
	if (capture) {
		capture->stop();
		capture.Clear();
	}
	raw.Clear();
	running = false;
	last_left = Image();
	last_right = Image();
}

bool StereoCalibrationTool::UsbStereoSource::ReadFrame(VisualFrame& left, VisualFrame& right, bool prefer_bright) {
	if (!capture)
		return false;

	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	
	bool found = false;
	bool found_bright = false;
	
	while(capture->isReadable(&tv) > 0) {
		size_t read_bytes = capture->read((char*)raw.Begin(), raw.GetCount());
		if (read_bytes == 0) break;
		
		Image frame;
		if (pixfmt == V4L2_PIX_FMT_RGB24)
			frame = ConvertRgb24ToImage(raw.Begin(), width, height);
		else if (pixfmt == V4L2_PIX_FMT_YUYV)
			frame = ConvertYuyvToImage(raw.Begin(), width, height);
		else if (pixfmt == V4L2_PIX_FMT_MJPEG)
			frame = ConvertMjpegToImage(raw.Begin(), (int)read_bytes);

		if (frame.IsEmpty()) continue;
		
		int64 sum = 0;
		int samples = 0;
		for(int y = 0; y < frame.GetHeight(); y += 20) {
			for(int x = 0; x < frame.GetWidth(); x += 20) {
				sum += frame[y][x].g;
				samples++;
			}
		}
		int avg = (samples > 0) ? (int)(sum / samples) : 0;
		bool is_bright = (avg > 10);
		
		if (!found || (prefer_bright && is_bright) || !found_bright) {
			if (SplitStereoImage(frame, last_left, last_right)) {
				found = true;
				last_is_bright = is_bright;
				if (is_bright) found_bright = true;
			}
		}
	}
	
	if (!found) return false;

	left.timestamp_us = usecs();
	left.format = GEOM_EVENT_CAM_RGBA8;
	left.width = last_left.GetWidth();
	left.height = last_left.GetHeight();
	left.stride = left.width * (int)sizeof(RGBA);
	left.eye = 0;
	left.data = (const byte*)~last_left;
	left.data_bytes = last_left.GetLength() * (int)sizeof(RGBA);
	left.img = last_left;
	left.flags = last_is_bright ? VIS_FRAME_BRIGHT : VIS_FRAME_DARK;
	left.serial = 0;

	right.timestamp_us = left.timestamp_us;
	right.format = GEOM_EVENT_CAM_RGBA8;
	right.width = last_right.GetWidth();
	right.height = last_right.GetHeight();
	right.stride = right.width * (int)sizeof(RGBA);
	right.eye = 1;
	right.data = (const byte*)~last_right;
	right.data_bytes = last_right.GetLength() * (int)sizeof(RGBA);
	right.img = last_right;
	right.flags = left.flags;
	right.serial = left.serial;

	return true;
}

bool StereoCalibrationTool::UsbStereoSource::PeakFrame(VisualFrame& left, VisualFrame& right, bool prefer_bright) {
	// PeakFrame not supported for USB, fallback to ReadFrame
	return ReadFrame(left, right, prefer_bright);
}
#else
bool StereoCalibrationTool::UsbStereoSource::Start() { return false; }
void StereoCalibrationTool::UsbStereoSource::Stop() { running = false; }
bool StereoCalibrationTool::UsbStereoSource::ReadFrame(VisualFrame&, VisualFrame&, bool) { return false; }
bool StereoCalibrationTool::UsbStereoSource::PeakFrame(VisualFrame&, VisualFrame&, bool) { return false; }
#endif

void StereoCalibrationTool::PreviewCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	w.DrawRect(sz, Black());
	if (has_images) {
		int half = max(1, sz.cx / 2);
		if (!IsNull(left_img))
			w.DrawImage(0, 0, half, sz.cy, left_img);
		if (!IsNull(right_img))
			w.DrawImage(half, 0, sz.cx - half, sz.cy, right_img);

		if (show_epipolar) {
			int step = max(8, sz.cy / 24);
			for (int y = 0; y < sz.cy; y += step) {
				w.DrawLine(0, y, sz.cx, y, 1, LtRed());
			}
		}
		
		// Draw matches
		for (int i = 0; i < matches.GetCount(); i++) {
			const MatchPair& m = matches[i];
			if (!IsNull(m.left)) {
				Point p(int(m.left.x * half), int(m.left.y * sz.cy));
				w.DrawEllipse(p.x - 3, p.y - 3, 6, 6, Green());
				w.DrawText(p.x + 5, p.y + 5, AsString(i), Arial(12), Green());
			}
			if (!IsNull(m.right)) {
				Point p(int(half + m.right.x * (sz.cx - half)), int(m.right.y * sz.cy));
				w.DrawEllipse(p.x - 3, p.y - 3, 6, 6, Green());
				w.DrawText(p.x + 5, p.y + 5, AsString(i), Arial(12), Green());
			}
		}

		if (show_residuals) {
			for (const auto& r : residuals) {
				Color c = Green();
				if (r.err_px >= 3.0)
					c = Red();
				else if (r.err_px >= 1.0)
					c = Yellow();
				Point p0, p1;
				if (r.eye == 0) {
					p0 = Point(int(r.measured.x * half), int(r.measured.y * sz.cy));
					p1 = Point(int(r.reproj.x * half), int(r.reproj.y * sz.cy));
				}
				else {
					p0 = Point(int(half + r.measured.x * (sz.cx - half)), int(r.measured.y * sz.cy));
					p1 = Point(int(half + r.reproj.x * (sz.cx - half)), int(r.reproj.y * sz.cy));
				}
				w.DrawLine(p0.x, p0.y, p1.x, p1.y, 1, c);
				w.DrawEllipse(p1.x - 2, p1.y - 2, 4, 4, c);
			}
		}
		
		// Draw pending point
		if (!IsNull(pending_left)) {
			Point p(int(pending_left.x * half), int(pending_left.y * sz.cy));
			w.DrawEllipse(p.x - 3, p.y - 3, 6, 6, Yellow());
		}
	}
	String title = live ? "Live Preview" : "Captured Snapshot";
	w.DrawText(10, 10, title, Arial(18).Bold(), White());
	if (!overlay.IsEmpty())
		w.DrawText(10, 34, overlay, Arial(12), White());
}

void StereoCalibrationTool::PreviewCtrl::LeftDown(Point p, dword flags) {
	Size sz = GetSize();
	if (sz.cx <= 0 || sz.cy <= 0) return;
	int half = sz.cx / 2;
	if (p.x < half) {
		Pointf img_p(float(p.x) / half, float(p.y) / sz.cy);
		WhenClick(img_p, 0);
	}
	else {
		Pointf img_p(float(p.x - half) / (sz.cx - half), float(p.y) / sz.cy);
		WhenClick(img_p, 1);
	}
}

StereoCalibrationTool::StereoCalibrationTool() {
	Title("Stereo Calibration Tool");
	Sizeable().Zoomable();

	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));
	AddFrame(status);

	source_info.SetLabel("Source setup goes here (live HMD, USB stereo, or video file).");
	calibration_info.SetLabel("Calibration workflow goes here (checkerboard/aruco capture).");
	calibration_schema.SetLabel("Output schema (.stcal):\n" 
		"  enabled=0|1\n" 
		"  eye_dist=<float>\n" 
		"  outward_angle=<float>\n" 
		"  right_pitch=<float>\n"
		"  right_roll=<float>\n"
		"  principal_point=<cx,cy>\n"
		"  angle_poly=a,b,c,d\n");
	calibration_preview.SetLabel("Preview: (no calibration loaded)");

	preview.WhenClick = [=](Pointf p, int eye) {
		if (preview.live) return;
		int row = captures_list.GetCursor();
		if (row < 0 || row >= captured_frames.GetCount()) return;
		CapturedFrame& frame = captured_frames[row];
		
		if (eye == 0) {
			preview.SetPendingLeft(p);
			status.Set(Format("Left point selected at %.3f, %.3f. Select matching right point.", p.x, p.y));
		}
		else if (eye == 1) {
			if (IsNull(preview.pending_left)) {
				status.Set("Select left point first.");
				return;
			}
			MatchPair& m = frame.matches.Add();
			m.left = preview.pending_left;
			m.right = p;
			m.left_text = Format("%.3f, %.3f", m.left.x, m.left.y);
			m.right_text = Format("%.3f, %.3f", m.right.x, m.right.y);
			preview.SetPendingLeft(Null);
			preview.SetMatches(frame.matches);
			DataCapturedFrame();
			status.Set("Match pair added.");
		}
	};

	BuildLayout();
	LoadLastCalibration();
	LoadState();
	SyncEditsFromCalibration();
	Data();
	
	tc.Set(-1000/60, [=] { Sync(); });
}

void StereoCalibrationTool::EnableLiveTest(int timeout_ms) {
	if (timeout_ms > 0) live_test_timeout_ms = timeout_ms;
	PostCallback(THISBACK(StartLiveTest));
}

void StereoCalibrationTool::StartLiveTest() {
	if (live_test_active) return;
	live_test_active = true;
	live_test_start_us = usecs();
	live_test_cb.Set(-100, THISBACK(RunLiveTest));
	Cout() << "Live Test: Starting. Will check for dark frame flickering and try capture.\n";
	
	// Ensure source is running
	if (!sources[source_list.GetIndex()]->IsRunning())
		StartSource();
	
	LiveView();
}

void StereoCalibrationTool::RunLiveTest() {
	if (!live_test_active) return;
	
	int64 elapsed_ms = (usecs() - live_test_start_us) / 1000;
	if (elapsed_ms > live_test_timeout_ms) {
		Cout() << "Live Test: FAILED (Timeout)\n";
		Exit(1);
	}
	
	int idx = source_list.GetIndex();
	VisualFrame lf, rf;
	
	bool read_ok = false;
	{
		Mutex::Lock __(source_mutex);
		read_ok = sources[idx]->PeakFrame(lf, rf, false); // Peak for test too
	}
	
	if (read_ok) {
		if (lf.flags & VIS_FRAME_DARK) {
			Cout() << "Live Test: DARK frame received. SYNC SHOULD SKIP THIS.\n";
		}
		
		if (lf.flags & VIS_FRAME_BRIGHT) {
			Cout() << "Live Test: Bright frame detected. Attempting capture...\n";
			CaptureFrame();
			if (captured_frames.GetCount() > 0) {
				Cout() << "Live Test: SUCCESS (Captured bright frame)\n";
				Exit(0);
			}
		}
	}
}

void StereoCalibrationTool::SetVerbose(bool v) {
	verbose = v;
	for(int i = 0; i < sources.GetCount(); i++)
		if(sources[i])
			sources[i]->SetVerbose(v);
}

void StereoCalibrationTool::Sync() {
	if (!preview.live) return;
	
	int idx = source_list.GetIndex();
	if (idx >= 0 && idx < sources.GetCount()) {
		if (sources[idx]->IsRunning()) {
			VisualFrame lf, rf;
			
			// Use PeakFrame for Sync to avoid clear backlog for CaptureFrame
			if (!source_mutex.TryEnter()) return;
			bool success = sources[idx]->PeakFrame(lf, rf, false);
			source_mutex.Leave();
			
			if (success) {
				if (verbose) Cout() << Format("Sync: Frame serial=%d, flags=%d (Bright=%d, Dark=%d)\n", 
					(int)lf.serial, lf.flags, (int)(lf.flags & VIS_FRAME_BRIGHT), (int)(lf.flags & VIS_FRAME_DARK));

				if (!(lf.flags & VIS_FRAME_BRIGHT))
					return;
				
				if (lf.serial > 0 && lf.serial == last_serial)
					return;
				last_serial = lf.serial;
				
				Image left_img = CopyFrameImage(lf);
				Image right_img = CopyFrameImage(rf);
				if (!IsNull(left_img) || !IsNull(right_img)) {
					if (undistort_view && BuildLiveUndistortCache(left_img, right_img, lf.serial))
						preview.SetImages(live_undist_left, live_undist_right);
					else
						preview.SetImages(left_img, right_img);
					preview.SetOverlay("Live view");
					UpdateReviewOverlay();
				}
			}
		}
	}
}

void StereoCalibrationTool::ClearMatches() {
	int row = captures_list.GetCursor();
	if (row >= 0 && row < captured_frames.GetCount()) {
		captured_frames[row].matches.Clear();
		DataCapturedFrame();
		status.Set("Matches cleared.");
	}
}

void StereoCalibrationTool::SetProjectDir(const String& dir) {
	project_dir = dir;
	if (project_dir.IsEmpty()) return;
	RealizeDirectory(project_dir);
	LoadLastCalibration();
	LoadState();
	if (FileExists(GetReportPath())) {
		report_text <<= LoadFile(GetReportPath());
	}
	SyncEditsFromCalibration();
	Data();
}

void StereoCalibrationTool::RemoveSnapshot() {
	int row = captures_list.GetCursor();
	if (row >= 0 && row < captured_frames.GetCount()) {
		if (PromptYesNo("Remove selected snapshot and all its matches?")) {
			captured_frames.Remove(row);
			SaveState();
			LoadState(); // Refresh list and images
			status.Set("Snapshot removed.");
		}
	}
}

void StereoCalibrationTool::RemoveMatchPair() {
	int row = captures_list.GetCursor();
	int mrow = matches_list.GetCursor();
	if (row >= 0 && row < captured_frames.GetCount() && mrow >= 0) {
		CapturedFrame& f = captured_frames[row];
		if (mrow < f.matches.GetCount()) {
			f.matches.Remove(mrow);
			DataCapturedFrame();
			SaveState();
			status.Set("Match pair removed.");
		}
	}
}

void StereoCalibrationTool::SolveCalibration() {
	String math_log;
	
	math_log << "Stereo Calibration Math Report\n";
	math_log << "==============================\n\n";
	
	math_log << "Input Data Analysis:\n";
	math_log << "--------------------\n";
	for(int i = 0; i < captured_frames.GetCount(); i++) {
		const auto& f = captured_frames[i];
		Size lsz = f.left_img.GetSize();
		Size rsz = f.right_img.GetSize();
		math_log << Format("Frame %d (Source: %s):\n", i + 1, f.source);
		math_log << Format("  Left Image Resolution:  %d x %d\n", lsz.cx, lsz.cy);
		math_log << Format("  Right Image Resolution: %d x %d\n", rsz.cx, rsz.cy);
		math_log << Format("  Split Position (x):     %d (assuming side-by-side)\n", lsz.cx);
		math_log << Format("  Active Matches:         %d\n", f.matches.GetCount());
		if (lsz != rsz) math_log << "  WARNING: Left/Right size mismatch!\n";
		math_log << "\n";
	}
	math_log << "Ray Mapping:\n";
	math_log << "  dx = u - cx, dy = v - cy (v grows downward)\n";
	math_log << "  roll = atan2(dy, dx)\n";
	math_log << "  Right pixels are interpreted in right sub-image coordinates (0..width-1)\n\n";

	StereoCalibrationSolver solver;
	solver.log = &math_log;
	solver.eye_dist = (double)calib_eye_dist;
	
	int out_of_range_right = 0;
	for (const auto& f : captured_frames) {
		Size sz = f.left_img.GetSize();
		if (sz.cx <= 0 || sz.cy <= 0)
			continue;
		for (const auto& m : f.matches) {
			if (IsNull(m.left) || IsNull(m.right))
				continue;
			auto& p = solver.matches.Add();
			p.left_px = vec2((float)(m.left.x * sz.cx), (float)(m.left.y * sz.cy));
			p.right_px = vec2((float)(m.right.x * sz.cx), (float)(m.right.y * sz.cy));
			p.image_size = sz;
			p.dist_l = m.dist_l;
			p.dist_r = m.dist_r;
			if (m.right.x < 0 || m.right.x > 1 || m.right.y < 0 || m.right.y > 1)
				out_of_range_right++;
		}
	}
	
	if (solver.matches.GetCount() < 5) {
		PromptOK("At least 5 match pairs across all frames are required to solve for calibration.");
		return;
	}
	
	if (out_of_range_right > 0) {
		math_log << "WARNING: " << out_of_range_right << " right-eye coordinates are outside [0..1].\n";
		math_log << "         Ensure right points are in the right sub-image coordinates (not combined).\n\n";
	}
	
	if (fabs(solver.eye_dist) < 1e-6) {
		if(!PromptYesNo("Eye distance is close to zero. This may cause the solver to fail or produce invalid results (points at infinity). Continue?"))
			return;
	}
	
	// Always start with fresh heuristics, ignoring previous UI values (which are outputs)
	Size init_sz = solver.matches[0].image_size;
	StereoCalibrationParams params;
	params.a = 2.0 * init_sz.cx / M_PI;
	params.b = 0;
	params.c = 0;
	params.d = 0;
	params.cx = init_sz.cx * 0.5;
	params.cy = init_sz.cy * 0.5;
	params.yaw = 0;
	params.pitch = 0;
	params.roll = 0;
	
	status.Set("Solving calibration (Stage 1/2)...");
	solver.Solve(params, true);
	
	status.Set("Solving calibration (Stage 2/2)...");
	if (solver.Solve(params, false)) {
		calib_poly_a <<= params.a;
		calib_poly_b <<= params.b;
		calib_poly_c <<= params.c;
		calib_poly_d <<= params.d;
		calib_outward_angle <<= params.yaw;

		last_calibration.right_pitch = (float)params.pitch;
		last_calibration.right_roll = (float)params.roll;
		last_calibration.principal_point = vec2((float)params.cx, (float)params.cy);
		SyncCalibrationFromEdits();
		
		StereoCalibrationDiagnostics diag;
		solver.ComputeDiagnostics(params, diag);
		
		String report;
		report << "Solver converged successfully.\n";
		report << "Matches: " << solver.matches.GetCount() << "\n";
		report << "Final parameters:\n";
		report << "  a: " << params.a << "\n";
		report << "  b: " << params.b << "\n";
		report << "  c: " << params.c << "\n";
		report << "  d: " << params.d << "\n";
		report << "  cx: " << params.cx << "\n";
		report << "  cy: " << params.cy << "\n";
		report << "  yaw (outward): " << params.yaw << " rad (" << params.yaw * 180 / M_PI << " deg)\n";
		report << "  pitch: " << params.pitch << " rad (" << params.pitch * 180 / M_PI << " deg)\n";
		report << "  roll: " << params.roll << " rad (" << params.roll * 180 / M_PI << " deg)\n";

		report << "Diagnostics:\n";
		report << Format("  Reprojection RMS (L/R): %.3f / %.3f px\n", diag.reproj_rms_l, diag.reproj_rms_r);
		report << Format("  Distance RMS (L/R):     %.3f / %.3f mm\n", diag.dist_rms_l, diag.dist_rms_r);
		report << Format("  Points behind camera (L/R): %d / %d (%.1f%% / %.1f%%)\n",
			diag.behind_left, diag.behind_right,
			diag.reproj_count_l ? 100.0 * diag.behind_left / diag.reproj_count_l : 0.0,
			diag.reproj_count_r ? 100.0 * diag.behind_right / diag.reproj_count_r : 0.0);
		report << Format("  Baseline (fixed): %.3f mm\n", solver.eye_dist);

		double disp_rel_sum = 0;
		int disp_rel_count = 0;
		for (const auto& r : diag.residuals) {
			double disp = fabs(r.disparity_px);
			if (disp > 1e-3 && r.z_l > 0) {
				double depth_disp = (params.a * solver.eye_dist) / disp;
				double rel = (depth_disp - r.z_l) / r.z_l;
				disp_rel_sum += rel * rel;
				disp_rel_count++;
			}
		}
		if (disp_rel_count > 0) {
			double disp_rel_rms = sqrt(disp_rel_sum / disp_rel_count) * 100.0;
			report << Format("  Disparity-depth RMS: %.2f%% (%d samples)\n", disp_rel_rms, disp_rel_count);
		}

		double dist_pct_sum = 0;
		int dist_pct_cnt = 0;
		for (int i = 0; i < diag.residuals.GetCount(); i++) {
			const auto& m = solver.matches[i];
			const auto& r = diag.residuals[i];
			if (m.dist_l > 0) {
				dist_pct_sum += fabs(r.dist_l_err) / m.dist_l;
				dist_pct_cnt++;
			}
			if (m.dist_r > 0) {
				dist_pct_sum += fabs(r.dist_r_err) / m.dist_r;
				dist_pct_cnt++;
			}
		}
		double dist_pct_avg = dist_pct_cnt ? (dist_pct_sum / dist_pct_cnt) : 0;
		if (dist_pct_cnt > 0)
			report << Format("  Avg distance error: %.2f%% (%d samples)\n", dist_pct_avg * 100.0, dist_pct_cnt);

		if (dist_pct_cnt > 0 && dist_pct_avg > 0.5 && diag.reproj_rms_l < 1.5 && diag.reproj_rms_r < 1.5) {
			report << "WARNING: Distance errors remain >50% while reprojection is small.\n";
			report << "         Coordinate mapping/sign conventions are likely wrong.\n";
			math_log << "\nWARNING: Distance errors remain >50% while reprojection is small.\n";
			math_log << "         Coordinate mapping/sign conventions are likely wrong.\n";
		}

		math_log << "\nDiagnostics Summary:\n";
		math_log << Format("  Reprojection RMS (L/R): %.3f / %.3f px\n", diag.reproj_rms_l, diag.reproj_rms_r);
		math_log << Format("  Distance RMS (L/R):     %.3f / %.3f mm\n", diag.dist_rms_l, diag.dist_rms_r);
		math_log << Format("  Behind camera (L/R):    %d / %d\n", diag.behind_left, diag.behind_right);
		math_log << Format("  Baseline (fixed):       %.3f mm\n", solver.eye_dist);

		Vector<int> order;
		order.SetCount(diag.residuals.GetCount());
		for (int i = 0; i < order.GetCount(); i++)
			order[i] = i;
		Sort(order, [&](int a, int b) {
			const auto& ra = diag.residuals[a];
			const auto& rb = diag.residuals[b];
			double ea = max(ra.err_l_px, ra.err_r_px) + fabs(ra.dist_l_err) * solver.dist_weight + fabs(ra.dist_r_err) * solver.dist_weight;
			double eb = max(rb.err_l_px, rb.err_r_px) + fabs(rb.dist_l_err) * solver.dist_weight + fabs(rb.dist_r_err) * solver.dist_weight;
			return ea > eb;
		});

		math_log << "\nTop Residual Offenders:\n";
		int topn = min(10, order.GetCount());
		for (int i = 0; i < topn; i++) {
			const auto& r = diag.residuals[order[i]];
			const auto& m = solver.matches[order[i]];
			math_log << Format("  #%d: reproj L/R=%.2f/%.2f px, dist L/R=%.2f/%.2f mm, disp=%.2f px\n",
				order[i] + 1, r.err_l_px, r.err_r_px, r.dist_l_err, r.dist_r_err, r.disparity_px);
			math_log << Format("      measured L=(%.2f, %.2f), R=(%.2f, %.2f)\n",
				m.left_px[0], m.left_px[1], m.right_px[0], m.right_px[1]);
			math_log << Format("      reproj  L=(%.2f, %.2f), R=(%.2f, %.2f)\n",
				r.reproj_l[0], r.reproj_l[1], r.reproj_r[0], r.reproj_r[1]);
		}
		
		report_text <<= report;
		math_text <<= math_log;
		SaveFile(GetReportPath(), report);
		SaveLastCalibration();
		
		bottom_tabs.Set(1);
		status.Set("Calibration solved and saved.");
	}
	else {
		status.Set("Solver failed to converge.");
		PromptOK("Calibration solver failed. Check your matches.");
	}
}

void StereoCalibrationTool::CaptureFrame() {
	int idx = source_list.GetIndex();
	if (idx < 0 || idx >= sources.GetCount()) return;
	
	if (verbose) Cout() << "CaptureFrame: Waiting for bright frame...\n";
	
	VisualFrame lf, rf;
	bool found = false;
	TimeStop ts;
	
	// Step 1: Poll using PeakFrame (non-destructive)
	while(ts.Elapsed() < 2000) {
		bool read_ok = false;
		{
			if (source_mutex.TryEnter()) {
				read_ok = sources[idx]->PeakFrame(lf, rf, true);
				source_mutex.Leave();
			}
		}
		
		if (read_ok && (lf.flags & VIS_FRAME_BRIGHT)) {
			found = true;
			break;
		}
		Sleep(5);
	}
	
	if (!found) {
		String msg = "Failed to capture bright frame after 2000ms.";
		if (verbose) Cout() << "CaptureFrame: " << msg << "\n";
		status.Set(msg);
		return;
	}

	// Step 2: Now destructively pop the frame
	{
		Mutex::Lock __(source_mutex);
		sources[idx]->ReadFrame(lf, rf, true);
	}

	if (verbose) Cout() << "CaptureFrame: Bright frame found! Capturing.\n";

	preview.SetLive(false);
	String name = AsString(source_list.GetValue());
	Time now = GetSysTime();
	
	CapturedFrame frame;
	frame.time = now;
	frame.source = name;
	frame.left_img = CopyFrameImage(lf);
	frame.right_img = CopyFrameImage(rf);
	
	captured_frames.Add(pick(frame));
	SaveState(); // Auto-save on capture
	LoadState(); // Refresh list
	
	captures_list.SetCursor(captures_list.GetCount() - 1);
	
	bottom_tabs.Set(0);
	DataCapturedFrame();
	status.Set("Captured and saved bright snapshot.");
}

StereoCalibrationTool::~StereoCalibrationTool() {
	usb_test_cb.Kill();
	hmd_test_cb.Kill();
	live_test_cb.Kill();
	tc.Kill();
	StopSource();
	if (!project_dir.IsEmpty()) {
		SaveLastCalibration();
		SaveState();
	}
}

void StereoCalibrationTool::BuildLayout() {
	hsplitter.Horz(left, right);
	hsplitter.SetPos(2000);
	vsplitter.Vert(hsplitter, bottom_tabs);
	vsplitter.SetPos(7000);
	Add(vsplitter.SizePos());

	BuildLeftPanel();
	BuildBottomTabs();
	right.Add(preview.SizePos());
	status.Set("Status: idle");
}

void StereoCalibrationTool::BuildLeftPanel() {
	source_list.Add(0, "HMD Stereo Camera");
	source_list.Add(1, "USB Stereo (Side-by-side)");
	source_list.Add(2, "Stereo Video File");
	source_list.SetIndex(0);
	source_list.WhenAction = THISBACK(OnSourceChanged);

	start_source.SetLabel("Start");
	stop_source.SetLabel("Stop");
	live_view.SetLabel("Live view");
	capture_frame.SetLabel("Capture");
	clear_matches.SetLabel("Clear matches");
	start_source <<= THISBACK(StartSource);
	stop_source <<= THISBACK(StopSource);
	live_view <<= THISBACK(LiveView);
	capture_frame <<= THISBACK(CaptureFrame);
	solve_calibration.SetLabel("Solve");
	solve_calibration <<= THISBACK(SolveCalibration);
	clear_matches.WhenAction = THISBACK(ClearMatches);

	source_status.SetLabel("Status: idle");
	sep_source.SetLabel("Source");
	sep_mode.SetLabel("Mode");
	sep_calib.SetLabel("Calibration");
	sep_review.SetLabel("Review");
	sep_diag.SetLabel("Diagnostics");
	mode_lbl.SetLabel("Match mode");
	mode_list.Add(0, "Point pairs");
	mode_list.Add(1, "Line pairs");
	mode_list.SetIndex(0);

	export_calibration.SetLabel("Export .stcal");
	export_calibration <<= THISBACK(ExportCalibration);
	deploy_calibration.SetLabel("Deploy Calibration");
	deploy_calibration <<= THISBACK(DeployCalibration);
	load_calibration.SetLabel("Load .stcal");
	load_calibration <<= THISBACK(LoadCalibration);
	calib_enabled_lbl.SetLabel("Enabled");
	calib_enabled.WhenAction = THISBACK(SyncCalibrationFromEdits);
	calib_eye_lbl.SetLabel("Eye dist (mm)");
	calib_eye_dist.WhenAction = THISBACK(SyncCalibrationFromEdits);
	calib_outward_lbl.SetLabel("Outward angle");
	calib_outward_angle.SetReadOnly();
	calib_outward_angle.WhenAction = THISBACK(SyncCalibrationFromEdits);
	calib_poly_lbl.SetLabel("Angle poly");
	calib_poly_a.SetReadOnly();
	calib_poly_a.WhenAction = THISBACK(SyncCalibrationFromEdits);
	calib_poly_b.SetReadOnly();
	calib_poly_b.WhenAction = THISBACK(SyncCalibrationFromEdits);
	calib_poly_c.SetReadOnly();
	calib_poly_c.WhenAction = THISBACK(SyncCalibrationFromEdits);
	calib_poly_d.SetReadOnly();
	calib_poly_d.WhenAction = THISBACK(SyncCalibrationFromEdits);
	show_epipolar.SetLabel("Show Epipolar Lines");
	show_epipolar.WhenAction = THISBACK(OnReviewChanged);
	undistort_view.SetLabel("Undistort View");
	undistort_view.WhenAction = THISBACK(OnReviewChanged);

	int y = 8;
	left.Add(sep_source.TopPos(y, 18).HSizePos(8, 8));
	y += 24;
	left.Add(source_list.TopPos(y, 24).HSizePos(8, 8));
	y += 32;
	left.Add(start_source.TopPos(y, 24).LeftPos(8, 80));
	left.Add(stop_source.TopPos(y, 24).LeftPos(96, 80));
	left.Add(live_view.TopPos(y, 24).LeftPos(184, 80));
	left.Add(capture_frame.TopPos(y, 24).LeftPos(272, 80));
	y += 28;
	left.Add(solve_calibration.TopPos(y, 24).LeftPos(184, 80));
	left.Add(clear_matches.TopPos(y, 24).LeftPos(272, 100));
	y += 32;
	left.Add(source_status.TopPos(y, 20).HSizePos(8, 8));
	y += 28;

	left.Add(sep_mode.TopPos(y, 18).HSizePos(8, 8));
	y += 24;
	left.Add(mode_lbl.TopPos(y, 20).LeftPos(8, 80));
	left.Add(mode_list.TopPos(y, 20).LeftPos(96, 160));
	y += 28;

	left.Add(sep_calib.TopPos(y, 18).HSizePos(8, 8));
	y += 24;
	left.Add(calib_enabled_lbl.TopPos(y, 20).LeftPos(8, 80));
	left.Add(calib_enabled.TopPos(y, 20).LeftPos(96, 20));
	y += 24;
	left.Add(calib_eye_lbl.TopPos(y, 20).LeftPos(8, 80));
	left.Add(calib_eye_dist.TopPos(y, 20).LeftPos(96, 120));
	y += 24;
	left.Add(calib_outward_lbl.TopPos(y, 20).LeftPos(8, 80));
	left.Add(calib_outward_angle.TopPos(y, 20).LeftPos(96, 120));
	y += 24;
	left.Add(calib_poly_lbl.TopPos(y, 20).LeftPos(8, 80));
	left.Add(calib_poly_a.TopPos(y, 20).LeftPos(96, 70));
	left.Add(calib_poly_b.TopPos(y, 20).LeftPos(170, 70));
	left.Add(calib_poly_c.TopPos(y, 20).LeftPos(244, 70));
	left.Add(calib_poly_d.TopPos(y, 20).LeftPos(318, 70));
	y += 28;
	left.Add(load_calibration.TopPos(y, 24).LeftPos(8, 120));
	left.Add(export_calibration.TopPos(y, 24).LeftPos(136, 120));
	y += 28;
	left.Add(deploy_calibration.TopPos(y, 24).LeftPos(8, 248));
	y += 32;

	left.Add(sep_review.TopPos(y, 18).HSizePos(8, 8));
	y += 24;
	left.Add(show_epipolar.TopPos(y, 20).LeftPos(8, 180));
	y += 24;
	left.Add(undistort_view.TopPos(y, 20).LeftPos(8, 180));
	y += 28;

	left.Add(sep_diag.TopPos(y, 18).HSizePos(8, 8));
	y += 24;
	left.Add(calibration_schema.TopPos(y, 110).HSizePos(8, 8));
	y += 118;
	left.Add(calibration_preview.TopPos(y, 60).HSizePos(8, 8));

	sources.Clear();
	sources.Add(MakeOne<HmdStereoSource>());
	sources.Add(MakeOne<UsbStereoSource>());
	sources.Add(MakeOne<VideoStereoSource>());
}

void StereoCalibrationTool::BuildBottomTabs() {
	captures_list.AddColumn("Time");
	captures_list.AddColumn("Source");
	captures_list.AddColumn("Samples");
	captures_list.WhenCursor = THISBACK(DataCapturedFrame);
	
	matches_list.AddColumn("Left");
	matches_list.AddColumn("Right");
	matches_list.AddColumn("Dist L (mm)").Edit(dist_l_editor);
	matches_list.AddColumn("Dist R (mm)").Edit(dist_r_editor);
	matches_list.WhenAcceptRow = [=] {
		int row = captures_list.GetCursor();
		int mrow = matches_list.GetCursor();
		if (row >= 0 && row < captured_frames.GetCount() && mrow >= 0) {
			CapturedFrame& f = captured_frames[row];
			if (mrow < f.matches.GetCount()) {
				f.matches[mrow].dist_l = matches_list.Get(mrow, 2);
				f.matches[mrow].dist_r = matches_list.Get(mrow, 3);
				SaveState();
			}
		}
		return true;
	};
	
	report_text.SetReadOnly();
	report_text <<= "Solve report and .stcal preview will appear here.";
	
	math_text.SetReadOnly();
	math_text.SetFont(Courier(12));
	math_text <<= "Detailed math calculation log will appear here after solving.";

	captures_split.Horz(captures_list, matches_list);
	captures_split.SetPos(4000);

	bottom_tabs.Add(captures_split.SizePos(), "Captured Frames");
	bottom_tabs.Add(report_text.SizePos(), "Report");
	bottom_tabs.Add(math_text.SizePos(), "Math");
}

void StereoCalibrationTool::Data() {
	UpdatePreview();
	DataCapturedFrame();
}

void StereoCalibrationTool::DataCapturedFrame() {
	if (pending_capture_row >= 0 && captures_list.GetCount() > pending_capture_row) {
		int set_row = pending_capture_row;
		pending_capture_row = -1;
		captures_list.SetCursor(set_row);
		return;
	}
	int row = captures_list.GetCursor();
	if (row < 0) {
		if (!preview.live)
			preview.SetOverlay("No capture selected");
		return;
	}
	if (row >= captured_frames.GetCount()) {
		if (!preview.live)
			preview.SetOverlay("Capture data unavailable");
		return;
	}
	
	// If we have a selection, we definitely want to see it, so switch off live view.
	preview.SetLive(false);
	
	CapturedFrame& frame = captured_frames[row];
	matches_list.Clear();
	for (const MatchPair& pair : frame.matches)
		matches_list.Add(pair.left_text, pair.right_text, pair.dist_l, pair.dist_r);
	ApplyPreviewImages(frame);
	preview.SetMatches(frame.matches);
	String time = AsString(captures_list.Get(row, 0));
	String source = AsString(captures_list.Get(row, 1));
	int samples = frame.matches.GetCount();
	captures_list.Set(row, 2, samples);
	preview.SetOverlay(Format("Capture %s (%s), samples %d", time, source, samples));
	UpdateReviewOverlay();
	if (preview.show_residuals && preview.residual_rms > 0.0)
		status.Set(Format("Selected capture %s from %s | Residual RMS: %.2f px", time, source, preview.residual_rms));
	else
		status.Set(Format("Selected capture %s from %s", time, source));
}

void StereoCalibrationTool::EnableUsbTest(const String& dev, int timeout_ms) {
	usb_test_enabled = true;
	usb_test_device = dev;
	if (timeout_ms > 0)
		usb_test_timeout_ms = timeout_ms;
	PostCallback(THISBACK(StartUsbTest));
}

void StereoCalibrationTool::StartUsbTest() {
	if (!usb_test_enabled || usb_test_active)
		return;
	usb_test_active = true;
	usb_test_start_us = usecs();
	usb_test_last_start_us = 0;
	usb_test_attempts = 0;
	usb_test_cb.Set(-100, THISBACK(RunUsbTest));
	status.Set("USB test: starting...");
}

void StereoCalibrationTool::RunUsbTest() {
	if (!usb_test_active)
		return;
	int64 elapsed_ms = (usecs() - usb_test_start_us) / 1000;
	if (elapsed_ms > usb_test_timeout_ms) {
		usb_test_active = false;
		usb_test_cb.Kill();
		status.Set("USB test: timeout waiting for frame.");
		StopSource();
		Exit(1);
		return;
	}

	if (source_list.GetIndex() != 1) {
		source_list.SetIndex(1);
		OnSourceChanged();
	}
	if (sources.GetCount() <= 1)
		return;
	UsbStereoSource* usb = dynamic_cast<UsbStereoSource*>(~sources[1]);
	if (!usb)
		return;
	if (usb && !usb_test_device.IsEmpty())
		usb->device_path = usb_test_device;
	if (!usb->IsRunning()) {
		int64 now_us = usecs();
		if (usb_test_last_start_us == 0 || now_us - usb_test_last_start_us > 500000) {
			usb_test_last_start_us = now_us;
			StartSource();
			status.Set(Format("USB test: opening (%d)...", ++usb_test_attempts));
		}
		return;
	}

	VisualFrame lf, rf;
	if (!usb->ReadFrame(lf, rf, true)) {
		status.Set(Format("USB test: waiting for frame (%d)...", ++usb_test_attempts));
		return;
	}

	Image left_img = CopyFrameImage(lf);
	Image right_img = CopyFrameImage(rf);
	if (!IsFrameNonBlack(left_img) && !IsFrameNonBlack(right_img)) {
		status.Set(Format("USB test: black frame (%d), retrying...", ++usb_test_attempts));
		return;
	}

	Time now = GetSysTime();
	captures_list.Add(Format("%02d:%02d:%02d", now.hour, now.minute, now.second), usb->GetName(), 1);
	captures_list.SetCursor(captures_list.GetCount() - 1);
	CapturedFrame frame;
	frame.time = now;
	frame.source = usb->GetName();
	frame.samples = 1;
	frame.left_img = left_img;
	frame.right_img = right_img;
	captured_frames.Add(pick(frame));
	preview.SetLive(false);
	bottom_tabs.Set(0);
	DataCapturedFrame();
	status.Set("USB test: captured frame OK.");
	usb_test_active = false;
	usb_test_cb.Kill();
	StopSource();
	Exit(0);
}

void StereoCalibrationTool::EnableHmdTest(int timeout_ms) {
	hmd_test_enabled = true;
	if (timeout_ms > 0)
		hmd_test_timeout_ms = timeout_ms;
	PostCallback(THISBACK(StartHmdTest));
}

void StereoCalibrationTool::StartHmdTest() {
	if (!hmd_test_enabled || hmd_test_active)
		return;
	hmd_test_active = true;
	hmd_test_start_us = usecs();
	hmd_test_last_start_us = 0;
	hmd_test_attempts = 0;
	hmd_test_cb.Set(-100, THISBACK(RunHmdTest));
	status.Set("HMD test: starting...");
}

void StereoCalibrationTool::RunHmdTest() {
	if (!hmd_test_active)
		return;
	int64 elapsed_ms = (usecs() - hmd_test_start_us) / 1000;
	if (elapsed_ms > hmd_test_timeout_ms) {
		hmd_test_active = false;
		hmd_test_cb.Kill();
		status.Set("HMD test: timeout waiting for frame.");
		StopSource();
		Exit(1);
		return;
	}

	if (source_list.GetIndex() != 0) {
		source_list.SetIndex(0);
		OnSourceChanged();
	}
	if (sources.GetCount() <= 0)
		return;
	HmdStereoSource* hmd = dynamic_cast<HmdStereoSource*>(~sources[0]);
	if (!hmd)
		return;
	if (!hmd->IsRunning()) {
		int64 now_us = usecs();
		if (hmd_test_last_start_us == 0 || now_us - hmd_test_last_start_us > 500000) {
			hmd_test_last_start_us = now_us;
			StartSource();
			status.Set(Format("HMD test: opening (%d)...", ++hmd_test_attempts));
		}
		return;
	}

	VisualFrame lf, rf;
	if (!hmd->ReadFrame(lf, rf, true)) {
		status.Set(Format("HMD test: waiting for frame (%d)...", ++hmd_test_attempts));
		return;
	}

	Image left_img = CopyFrameImage(lf);
	Image right_img = CopyFrameImage(rf);
	if (!IsFrameNonBlack(left_img) && !IsFrameNonBlack(right_img)) {
		status.Set(Format("HMD test: black frame (%d), retrying...", ++hmd_test_attempts));
		return;
	}

	Time now = GetSysTime();
	captures_list.Add(Format("%02d:%02d:%02d", now.hour, now.minute, now.second), hmd->GetName(), 1);
	captures_list.SetCursor(captures_list.GetCount() - 1);
	CapturedFrame frame;
	frame.time = now;
	frame.source = hmd->GetName();
	frame.samples = 1;
	frame.left_img = left_img;
	frame.right_img = right_img;
	captured_frames.Add(pick(frame));
	preview.SetLive(false);
	bottom_tabs.Set(0);
	DataCapturedFrame();
	status.Set("HMD test: captured frame OK.");
	hmd_test_active = false;
	hmd_test_cb.Kill();
	StopSource();
	Exit(0);
}

void StereoCalibrationTool::OnSourceChanged() {
	if (verbose) Cout() << "OnSourceChanged: Source index=" << source_list.GetIndex() << "\n";
	StopSource();
	String name = AsString(source_list.GetValue());
	source_status.SetLabel(Format("Status: ready (%s)", name));
	status.Set(Format("Source ready: %s", name));
}

void StereoCalibrationTool::StartSource() {
	int idx = source_list.GetIndex();
	if (idx < 0 || idx >= sources.GetCount() || !sources[idx]) return;
	String name = AsString(source_list.GetValue());
	if (verbose) Cout() << "StartSource: Starting " << name << "\n";
	sources[idx]->SetVerbose(verbose);
	if (sources[idx]->Start()) {
		source_status.SetLabel(Format("Status: running (%s)", name));
		status.Set(Format("Source running: %s", name));
	} else {
		source_status.SetLabel(Format("Status: failed (%s)", name));
		status.Set(Format("Source failed: %s", name));
		if (verbose) Cout() << "StartSource: FAILED\n";
	}
}

void StereoCalibrationTool::StopSource() {
	int idx = source_list.GetIndex();
	if (idx < 0 || idx >= sources.GetCount() || !sources[idx]) return;
	if (verbose) Cout() << "StopSource: Stopping " << AsString(source_list.GetValue()) << "\n";
	sources[idx]->Stop();
	String name = AsString(source_list.GetValue());
	source_status.SetLabel(Format("Status: stopped (%s)", name));
	status.Set(Format("Source stopped: %s", name));
}

void StereoCalibrationTool::LiveView() {
	if (verbose) Cout() << "LiveView: Enabling live view\n";
	preview.SetLive(true);
	preview.SetMatches(Vector<MatchPair>());
	preview.SetImages(Image(), Image());
	preview.SetOverlay("Live view");
	live_undist_serial = -1;
	live_undist_valid = false;
	UpdateReviewOverlay();
	status.Set("Live view enabled.");
}

void StereoCalibrationTool::ExportCalibration() {
	SyncCalibrationFromEdits();
	Data();
	FileSel fs;
	fs.Type("Stereo Calibration", "*.stcal");
	fs.AllFilesType();
	if (!fs.ExecuteSaveAs("Export Stereo Calibration"))
		return;
	StereoCalibrationData data = last_calibration;
	if (!data.is_enabled) {
		data.is_enabled = false;
		data.eye_dist = 0;
		data.outward_angle = 0;
		data.angle_to_pixel = vec4(0,0,0,0);
	}
	if (!HMD::StereoTracker::SaveCalibrationFile(fs, data)) {
		PromptOK("Failed to export calibration.");
		return;
	}
	PromptOK("Calibration exported.");
}

void StereoCalibrationTool::DeployCalibration() {
	SyncCalibrationFromEdits();
	Data();
	if (!PromptYesNo("Deploy current calibration to share/calibration/hp_vr1000/calibration.stcal?"))
		return;
	String path = "share/calibration/hp_vr1000/calibration.stcal";
	RealizeDirectory(GetFileFolder(path));
	StereoCalibrationData data = last_calibration;
	if (!data.is_enabled && !PromptYesNo("Calibration is disabled. Deploy anyway?"))
		return;
	if (!HMD::StereoTracker::SaveCalibrationFile(path, data)) {
		PromptOK("Failed to deploy calibration.");
		return;
	}
	PromptOK("Calibration deployed.");
}

void StereoCalibrationTool::LoadCalibration() {
	FileSel fs;
	fs.Type("Stereo Calibration", "*.stcal");
	fs.AllFilesType();
	if (!fs.ExecuteOpen("Load Stereo Calibration"))
		return;
	StereoCalibrationData data;
	if (!HMD::StereoTracker::LoadCalibrationFile(fs, data)) {
		PromptOK("Failed to load calibration.");
		return;
	}
	last_calibration = data;
	preview_lens_size = Size(0,0);
	preview_lens_poly = vec4(0,0,0,0);
	live_undist_serial = -1;
	live_undist_valid = false;
	for (auto& frame : captured_frames)
		frame.undist_valid = false;
	SyncEditsFromCalibration();
	Data();
	PromptOK("Calibration loaded.");
}

void StereoCalibrationTool::SyncCalibrationFromEdits() {
	last_calibration.is_enabled = calib_enabled;
	last_calibration.eye_dist = (float)~calib_eye_dist;
	last_calibration.outward_angle = (float)~calib_outward_angle;
	last_calibration.angle_to_pixel[0] = (float)~calib_poly_a;
	last_calibration.angle_to_pixel[1] = (float)~calib_poly_b;
	last_calibration.angle_to_pixel[2] = (float)~calib_poly_c;
	last_calibration.angle_to_pixel[3] = (float)~calib_poly_d;
	preview_lens_size = Size(0,0);
	preview_lens_poly = vec4(0,0,0,0);
	live_undist_serial = -1;
	live_undist_valid = false;
	for (auto& frame : captured_frames)
		frame.undist_valid = false;
	Data();
}

void StereoCalibrationTool::SyncEditsFromCalibration() {
	calib_enabled = last_calibration.is_enabled;
	calib_eye_dist <<= (double)last_calibration.eye_dist;
	calib_outward_angle <<= (double)last_calibration.outward_angle;
	calib_poly_a <<= (double)last_calibration.angle_to_pixel[0];
	calib_poly_b <<= (double)last_calibration.angle_to_pixel[1];
	calib_poly_c <<= (double)last_calibration.angle_to_pixel[2];
	calib_poly_d <<= (double)last_calibration.angle_to_pixel[3];
}

void StereoCalibrationTool::UpdatePreview() {
	String s;
	s << "Preview:\n";
	s << "  enabled=" << (last_calibration.is_enabled ? "1" : "0") << "\n";
	s << "  eye_dist=" << last_calibration.eye_dist << "\n";
	s << "  outward_angle=" << last_calibration.outward_angle << "\n";
	s << "  right_pitch=" << last_calibration.right_pitch << "\n";
	s << "  right_roll=" << last_calibration.right_roll << "\n";
	s << "  principal_point=" << last_calibration.principal_point[0] << ", "
	  <<  last_calibration.principal_point[1] << "\n";
	s << "  angle_poly=" << last_calibration.angle_to_pixel[0] << ", "
	  << 	last_calibration.angle_to_pixel[1] << ", "
	  << 	last_calibration.angle_to_pixel[2] << ", "
	  << 	last_calibration.angle_to_pixel[3] << "\n";
	if (!last_calibration.is_enabled)
		s << "  WARNING: calibration disabled\n";
	if (!IsValidAnglePoly(last_calibration.angle_to_pixel))
		s << "  WARNING: angle_poly not set\n";
	calibration_preview.SetLabel(s);
	UpdateReviewEnablement();
	UpdateReviewOverlay();
}

void StereoCalibrationTool::UpdateReviewEnablement() {
	bool has_poly = IsValidAnglePoly(last_calibration.angle_to_pixel);
	bool can_review = last_calibration.is_enabled && has_poly;
	show_epipolar.Enable(can_review);
	undistort_view.Enable(can_review);
	if (!can_review) {
		show_epipolar = false;
		undistort_view = false;
		preview.SetEpipolar(false);
		preview.SetResiduals(Vector<PreviewCtrl::ResidualSample>(), 0, false);
	}
}

bool StereoCalibrationTool::PreparePreviewLens(const Size& sz) {
	if (sz.cx <= 0 || sz.cy <= 0)
		return false;
	vec4 poly = last_calibration.angle_to_pixel;
	if (!IsValidAnglePoly(poly))
		return false;
	vec2 pp = last_calibration.principal_point;
	vec2 tilt = vec2(last_calibration.right_pitch, last_calibration.right_roll);
	bool needs = (preview_lens_size != sz) || !IsSamePoly(preview_lens_poly, poly) ||
		fabs(preview_lens_outward - last_calibration.outward_angle) > 1e-6 ||
		fabs(preview_lens_pp[0] - pp[0]) > 1e-3 ||
		fabs(preview_lens_pp[1] - pp[1]) > 1e-3 ||
		fabs(preview_lens_tilt[0] - tilt[0]) > 1e-6 ||
		fabs(preview_lens_tilt[1] - tilt[1]) > 1e-6;
	if (needs) {
		preview_lens.SetAnglePixel(poly.data[0], poly.data[1], poly.data[2], poly.data[3]);
		preview_lens.SetEyeOutwardAngle(last_calibration.outward_angle);
		preview_lens.SetRightTilt(last_calibration.right_pitch, last_calibration.right_roll);
		if (pp[0] > 0 && pp[1] > 0)
			preview_lens.SetPrincipalPoint(pp[0], pp[1]);
		else
			preview_lens.ClearPrincipalPoint();
		preview_lens.SetSize(sz);
		preview_lens_size = sz;
		preview_lens_poly = poly;
		preview_lens_outward = last_calibration.outward_angle;
		preview_lens_pp = pp;
		preview_lens_tilt = tilt;
	}
	return true;
}

bool StereoCalibrationTool::BuildUndistortCache(CapturedFrame& frame) {
	Size sz = !frame.left_img.IsEmpty() ? frame.left_img.GetSize() : frame.right_img.GetSize();
	if (sz.cx <= 0 || sz.cy <= 0)
		return false;
	if (!PreparePreviewLens(sz))
		return false;
	if (frame.undist_valid && frame.undist_size == sz && IsSamePoly(frame.undist_poly, preview_lens_poly))
		return true;
	float max_radius = (float)sqrt(sz.cx * sz.cx * 0.25f + sz.cy * sz.cy * 0.25f);
	float max_angle = preview_lens.PixelToAngle(max_radius);
	if (max_angle <= 1e-6f)
		return false;
	float linear_scale = max_radius / max_angle;
	if (!frame.left_img.IsEmpty())
		frame.undist_left = UndistortImage(frame.left_img, preview_lens, linear_scale);
	else
		frame.undist_left = Image();
	if (!frame.right_img.IsEmpty())
		frame.undist_right = UndistortImage(frame.right_img, preview_lens, linear_scale);
	else
		frame.undist_right = Image();
	frame.undist_poly = preview_lens_poly;
	frame.undist_size = sz;
	frame.undist_valid = true;
	return true;
}

bool StereoCalibrationTool::BuildLiveUndistortCache(const Image& left, const Image& right, int64 serial) {
	Size sz = !left.IsEmpty() ? left.GetSize() : right.GetSize();
	if (sz.cx <= 0 || sz.cy <= 0)
		return false;
	if (!PreparePreviewLens(sz))
		return false;
	if (serial > 0 && live_undist_valid && live_undist_serial == serial &&
		live_undist_size == sz && IsSamePoly(live_undist_poly, preview_lens_poly))
		return true;
	float max_radius = (float)sqrt(sz.cx * sz.cx * 0.25f + sz.cy * sz.cy * 0.25f);
	float max_angle = preview_lens.PixelToAngle(max_radius);
	if (max_angle <= 1e-6f)
		return false;
	float linear_scale = max_radius / max_angle;
	if (!left.IsEmpty())
		live_undist_left = UndistortImage(left, preview_lens, linear_scale);
	else
		live_undist_left = Image();
	if (!right.IsEmpty())
		live_undist_right = UndistortImage(right, preview_lens, linear_scale);
	else
		live_undist_right = Image();
	live_undist_size = sz;
	live_undist_poly = preview_lens_poly;
	live_undist_valid = true;
	live_undist_serial = serial;
	return true;
}

void StereoCalibrationTool::ApplyPreviewImages(CapturedFrame& frame) {
	if (undistort_view && BuildUndistortCache(frame))
		preview.SetImages(frame.undist_left, frame.undist_right);
	else
		preview.SetImages(frame.left_img, frame.right_img);
}

void StereoCalibrationTool::UpdateReviewOverlay() {
	preview.SetEpipolar(show_epipolar);
	if (preview.live) {
		preview.SetResiduals(Vector<PreviewCtrl::ResidualSample>(), 0, false);
		return;
	}
	int row = captures_list.GetCursor();
	if (row < 0 || row >= captured_frames.GetCount()) {
		preview.SetResiduals(Vector<PreviewCtrl::ResidualSample>(), 0, false);
		return;
	}
	CapturedFrame& frame = captured_frames[row];
	Size sz = !frame.left_img.IsEmpty() ? frame.left_img.GetSize() : frame.right_img.GetSize();
	if (sz.cx <= 0 || sz.cy <= 0 || frame.matches.IsEmpty() || !PreparePreviewLens(sz)) {
		if (!IsValidAnglePoly(last_calibration.angle_to_pixel)) {
			String base = preview.overlay;
			int cut = base.Find("\nCalibration invalid");
			if (cut >= 0)
				base = base.Left(cut);
			if (!base.IsEmpty())
				preview.SetOverlay(base + "\nCalibration invalid (angle_poly missing)");
		}
		preview.SetResiduals(Vector<PreviewCtrl::ResidualSample>(), 0, false);
		return;
	}

	Vector<PreviewCtrl::ResidualSample> residuals;
	double sum_sq = 0;
	int count = 0;
	float eye_dist = (float)last_calibration.eye_dist;
	for (const auto& m : frame.matches) {
		if (IsNull(m.left) || IsNull(m.right))
			continue;
		vec2 lp(m.left.x * sz.cx, m.left.y * sz.cy);
		vec2 rp(m.right.x * sz.cx, m.right.y * sz.cy);
		axes2 axesL = preview_lens.Unproject(0, lp);
		axes2 axesR = preview_lens.Unproject(1, rp);
		vec3 dL = GetAxesDir(axesL);
		vec3 dR = GetAxesDir(axesR);
		vec3 pL = vec3(-eye_dist / 2.0f, 0, 0);
		vec3 pR = vec3(eye_dist / 2.0f, 0, 0);
		vec3 w0 = pL - pR;
		double a = Dot(dL, dL);
		double b = Dot(dL, dR);
		double c = Dot(dR, dR);
		double d = Dot(dL, w0);
		double e = Dot(dR, w0);
		double denom = a * c - b * b;
		vec3 pt;
		if (fabs(denom) > 1e-9) {
			double sc = (b * e - c * d) / denom;
			double tc = (a * e - b * d) / denom;
			pt = (pL + dL * (float)sc + pR + dR * (float)tc) * 0.5f;
		} else {
			pt = (pL + pR) * 0.5f + dL * 1000.0f;
		}

		vec3 dirL = (pt - pL).GetNormalized();
		vec3 dirR = (pt - pR).GetNormalized();
		vec2 projL = preview_lens.Project(0, GetDirAxes(dirL).Splice());
		vec2 projR = preview_lens.Project(1, GetDirAxes(dirR).Splice());
		Pointf uvL(projL[0] / sz.cx, projL[1] / sz.cy);
		Pointf uvR(projR[0] / sz.cx, projR[1] / sz.cy);

		double dxL = (uvL.x - m.left.x) * sz.cx;
		double dyL = (uvL.y - m.left.y) * sz.cy;
		double errL = sqrt(dxL * dxL + dyL * dyL);
		double dxR = (uvR.x - m.right.x) * sz.cx;
		double dyR = (uvR.y - m.right.y) * sz.cy;
		double errR = sqrt(dxR * dxR + dyR * dyR);

		PreviewCtrl::ResidualSample rl;
		rl.measured = m.left;
		rl.reproj = uvL;
		rl.eye = 0;
		rl.err_px = errL;
		residuals.Add(rl);
		PreviewCtrl::ResidualSample rr;
		rr.measured = m.right;
		rr.reproj = uvR;
		rr.eye = 1;
		rr.err_px = errR;
		residuals.Add(rr);

		sum_sq += errL * errL + errR * errR;
		count += 2;
	}
	double rms = (count > 0) ? sqrt(sum_sq / count) : 0;
	preview.SetResiduals(residuals, rms, true);
	if (count > 0) {
		String base = preview.live ? "Live view" : preview.overlay;
		int cut = base.Find("\nResidual RMS:");
		if (cut >= 0)
			base = base.Left(cut);
		if (!base.IsEmpty())
			preview.SetOverlay(base + Format("\nResidual RMS: %.2f px", rms));
	}
}

void StereoCalibrationTool::OnReviewChanged() {
	live_undist_serial = -1;
	live_undist_valid = false;
	for (auto& frame : captured_frames)
		frame.undist_valid = false;
	preview.SetEpipolar(show_epipolar);
	DataCapturedFrame();
	UpdateReviewOverlay();
}

String StereoCalibrationTool::GetPersistPath() const {
	return AppendFileName(project_dir, "calibration.stcal");
}

String StereoCalibrationTool::GetStatePath() const {
	return AppendFileName(project_dir, "project.json");
}

String StereoCalibrationTool::GetReportPath() const {
	return AppendFileName(project_dir, "report.txt");
}

void StereoCalibrationTool::LoadLastCalibration() {
	StereoCalibrationData data;
	String path = GetPersistPath();
	if (!project_dir.IsEmpty() && FileExists(path) && HMD::StereoTracker::LoadCalibrationFile(path, data))
		last_calibration = data;
}

void StereoCalibrationTool::SaveLastCalibration() {
	if (project_dir.IsEmpty()) return;
	SyncCalibrationFromEdits();
	HMD::StereoTracker::SaveCalibrationFile(GetPersistPath(), last_calibration);
}

void StereoCalibrationTool::LoadState() {
	if (project_dir.IsEmpty()) return;
	String path = GetStatePath();
	if (FileExists(path)) {
		String json = LoadFile(path);
		if (!json.IsEmpty()) {
			LoadFromJson(captured_frames, json);
			
			String dir = AppendFileName(project_dir, "captures");
			captures_list.Clear();
			for(int i = 0; i < captured_frames.GetCount(); i++) {
				auto& f = captured_frames[i];
				f.left_img = StreamRaster::LoadFileAny(AppendFileName(dir, Format("frame_%d_l.png", i)));
				f.right_img = StreamRaster::LoadFileAny(AppendFileName(dir, Format("frame_%d_r.png", i)));
				captures_list.Add(Format("%02d:%02d:%02d", f.time.hour, f.time.minute, f.time.second), f.source, f.matches.GetCount());
			}
		}
	}
}

void StereoCalibrationTool::SaveState() {
	if (project_dir.IsEmpty()) return;
	String path = GetStatePath();
	SaveFile(path, StoreAsJson(captured_frames));
	
	String dir = AppendFileName(project_dir, "captures");
	RealizeDirectory(dir);
	for(int i = 0; i < captured_frames.GetCount(); i++) {
		auto& f = captured_frames[i];
		PNGEncoder().SaveFile(AppendFileName(dir, Format("frame_%d_l.png", i)), f.left_img);
		PNGEncoder().SaveFile(AppendFileName(dir, Format("frame_%d_r.png", i)), f.right_img);
	}
}

void StereoCalibrationTool::MainMenu(Bar& bar) {
	bar.Sub("App", THISBACK(AppMenu));
	bar.Sub("Edit", THISBACK(EditMenu));
	bar.Sub("View", THISBACK(ViewMenu));
	bar.Sub("Help", THISBACK(HelpMenu));
}

void StereoCalibrationTool::AppMenu(Bar& bar) {
	bar.Add("Open Project...", [=] {
		FileSel fs;
		if (fs.ExecuteSelectDir("Select Project Directory")) {
			SetProjectDir(fs.Get());
		}
	});
	bar.Separator();
	bar.Add("Exit", [=] { Close(); });
}

void StereoCalibrationTool::EditMenu(Bar& bar) {
	bar.Add("Remove snapshot", THISBACK(RemoveSnapshot))
	   .Key(K_CTRL_DELETE)
	   .Enable(captures_list.IsCursor());
	bar.Add("Remove match pair", THISBACK(RemoveMatchPair))
	   .Key(K_DELETE)
	   .Enable(matches_list.IsCursor());
}

void StereoCalibrationTool::ViewMenu(Bar& bar) {
	bar.Add("Captured Frames", [=] { bottom_tabs.Set(0); });
	bar.Add("Report", [=] { bottom_tabs.Set(1); });
}

void StereoCalibrationTool::HelpMenu(Bar& bar) {
	bar.Add("About", [=] {
		PromptOK("Stereo Calibration Tool\n\nWork-in-progress.");
	});
}

END_UPP_NAMESPACE
