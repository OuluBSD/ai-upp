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

static vec3 TriangulatePoint(const vec3& pL, const vec3& dL, const vec3& pR, const vec3& dR) {
	vec3 w0 = pL - pR;
	double a = Dot(dL, dL);
	double b = Dot(dL, dR);
	double c = Dot(dR, dR);
	double d = Dot(dL, w0);
	double e = Dot(dR, w0);
	double denom = a * c - b * b;
	if (fabs(denom) > 1e-9) {
		double sc = (b * e - c * d) / denom;
		double tc = (a * e - b * d) / denom;
		return (pL + dL * (float)sc + pR + dR * (float)tc) * 0.5f;
	}
	return (pL + pR) * 0.5f + dL * 1000.0f;
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
		
		if (show_difference && !IsNull(left_img) && !IsNull(right_img)) {
			// Difference view
			Size isz = left_img.GetSize();
			if (right_img.GetSize() != isz) {
				// Fallback if sizes differ
				w.DrawText(10, 30, "Image size mismatch for difference view", Arial(12), Red());
			} else {
				ImageBuffer ib(isz);
				const RGBA* l = ~left_img;
				const RGBA* r = ~right_img;
				RGBA* t = ib;
				int n = isz.cx * isz.cy;
				for (int i = 0; i < n; i++) {
					t[i].r = (byte)abs((int)l[i].r - (int)r[i].r);
					t[i].g = (byte)abs((int)l[i].g - (int)r[i].g);
					t[i].b = (byte)abs((int)l[i].b - (int)r[i].b);
					t[i].a = 255;
				}
				w.DrawImage(0, 0, sz.cx, sz.cy, ib);
			}
		} else if (overlay_mode) {
			Image base = (overlay_base_eye == 0) ? left_img : right_img;
			Image over = (overlay_base_eye == 0) ? right_img : left_img;
			if (!IsNull(base))
				w.DrawImage(0, 0, sz.cx, sz.cy, base);
			if (!IsNull(over)) {
				ImageBuffer ib(over);
				int a = (int)(overlay_alpha * 255);
				for(RGBA* p = ib.Begin(), *e = ib.End(); p < e; p++)
					p->a = (byte)((p->a * a) >> 8);
				w.DrawImage(0, 0, sz.cx, sz.cy, Image(ib));
			}
		} else {
			if (!IsNull(left_img))
				w.DrawImage(0, 0, half, sz.cy, left_img);
			if (!IsNull(right_img))
				w.DrawImage(half, 0, sz.cx - half, sz.cy, right_img);
		}

		if (show_epipolar) {
			int step = max(8, sz.cy / 24);
			for (int y = 0; y < sz.cy; y += step) {
				w.DrawLine(0, y, sz.cx, y, 1, LtRed());
			}
		}
		
		// Draw matches
		for (int i = 0; i < matches.GetCount(); i++) {
			const MatchPair& m = matches[i];
			if (overlay_mode || show_difference) {
				if (!IsNull(m.left)) {
					Point p(int(m.left.x * sz.cx), int(m.left.y * sz.cy));
					w.DrawEllipse(p.x - 3, p.y - 3, 6, 6, Green());
				}
				if (!IsNull(m.right)) {
					Point p(int(m.right.x * sz.cx), int(m.right.y * sz.cy));
					w.DrawEllipse(p.x - 3, p.y - 3, 6, 6, Blue());
				}
			} else {
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
		}

		if (show_residuals) {
			for (const auto& r : residuals) {
				Color c = Green();
				if (r.err_px >= 3.0)
					c = Red();
				else if (r.err_px >= 1.0)
					c = Yellow();
				Point p0, p1;
				if (overlay_mode || show_difference) {
					p0 = Point(int(r.measured.x * sz.cx), int(r.measured.y * sz.cy));
					p1 = Point(int(r.reproj.x * sz.cx), int(r.reproj.y * sz.cy));
				} else {
					if (r.eye == 0) {
						p0 = Point(int(r.measured.x * half), int(r.measured.y * sz.cy));
						p1 = Point(int(r.reproj.x * half), int(r.reproj.y * sz.cy));
					}
					else {
						p0 = Point(int(half + r.measured.x * (sz.cx - half)), int(r.measured.y * sz.cy));
						p1 = Point(int(half + r.reproj.x * (sz.cx - half)), int(r.reproj.y * sz.cy));
					}
				}
				w.DrawLine(p0.x, p0.y, p1.x, p1.y, 1, c);
				w.DrawEllipse(p1.x - 2, p1.y - 2, 4, 4, c);
			}
		}
		
		// Draw pending point
		if (!IsNull(pending_left)) {
			if (overlay_mode || show_difference) {
				Point p(int(pending_left.x * sz.cx), int(pending_left.y * sz.cy));
				w.DrawEllipse(p.x - 3, p.y - 3, 6, 6, Yellow());
			} else {
				Point p(int(pending_left.x * half), int(pending_left.y * sz.cy));
				w.DrawEllipse(p.x - 3, p.y - 3, 6, 6, Yellow());
			}
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
	if (overlay_mode || show_difference) {
		Pointf img_p(float(p.x) / sz.cx, float(p.y) / sz.cy);
		// In overlay mode, we use the pending_left state to distinguish between left/right eye clicks
		WhenClick(img_p, IsNull(pending_left) ? 0 : 1);
	} else {
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
		
		int mode = mode_list.GetIndex();
		
		// Map click to raw coordinates (if viewing undistorted)
		Pointf raw_p = MapClickToRaw(p);
		
		if (eye == 0) {
			preview.SetPendingLeft(raw_p);
			status.Set(Format("Left point selected at %.3f, %.3f. Select matching right point.", raw_p.x, raw_p.y));
		}
		else if (eye == 1) {
			if (IsNull(preview.pending_left)) {
				status.Set("Select left point first.");
				return;
			}
			
			if (mode == 0) { // Point pairs
				MatchPair& m = frame.matches.Add();
				m.left = preview.pending_left;
				m.right = raw_p;
				m.left_text = Format("%.3f, %.3f", m.left.x, m.left.y);
				m.right_text = Format("%.3f, %.3f", m.right.x, m.right.y);
				preview.SetPendingLeft(Null);
				preview.SetMatches(frame.matches);
				DataCapturedFrame();
				status.Set("Match pair added.");
			}
			else if (mode == 2 || mode == 3) { // Yaw or Pitch center
				MatchPair m;
				m.left = preview.pending_left;
				m.right = raw_p;
				frame.matches.Add(m); // Temporarily add to use existing logic or just call centering
				if (mode == 2) OnYawCenter();
				else OnPitchCenter();
				frame.matches.Drop(); // Remove temporary pair
				preview.SetPendingLeft(Null);
			}
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

void StereoCalibrationTool::EnableGABootstrap(bool enable, int population, int generations) {
	use_ga_bootstrap = enable;
	ga_population = population;
	ga_generations = generations;
	if (verbose && enable) {
		LOG("GA Bootstrap enabled: population=" << population << ", generations=" << generations);
	}
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
					if (BuildLiveUndistortCache(left_img, right_img, lf.serial))
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
	math_log << "Stereo Calibration Math Report (Stage B)\n";
	math_log << "========================================\n\n";

	StereoCalibrationSolver solver;
	solver.log = &math_log;
	solver.eye_dist = (double)calib_eye_dist / 1000.0;
	solver.EnableTrace(verbose_math_log, 2, 20000);
	
	for (const auto& f : captured_frames) {
		Size sz = f.left_img.GetSize();
		if (sz.cx <= 0) continue;
		for (const auto& m : f.matches) {
			if (IsNull(m.left) || IsNull(m.right)) continue;
			auto& p = solver.matches.Add();
			p.left_px = vec2((float)(m.left.x * sz.cx), (float)(m.left.y * sz.cy));
			p.right_px = vec2((float)(m.right.x * sz.cx), (float)(m.right.y * sz.cy));
			p.image_size = sz;
			p.dist_l = m.dist_l / 1000.0;
			p.dist_r = m.dist_r / 1000.0;
		}
	}
	
	if (solver.matches.GetCount() < 5) {
		PromptOK("At least 5 match pairs across all frames are required.");
		return;
	}
	
	// Map Stage A extrinsics to solver params
	StereoCalibrationParams params;
	params.yaw = (double)yaw_r - (double)yaw_l;
	params.pitch = (double)pitch_r - (double)pitch_l;
	params.roll = (double)roll_r - (double)roll_l;
	
	// Use Stage A basic intrinsics as starting point or heuristics
	Size init_sz = solver.matches[0].image_size;
	double fov_rad = (double)fov_deg * M_PI / 180.0;
	params.a = (init_sz.cx * 0.5) / (fov_rad * 0.5);
	double s = (double)barrel_strength * 0.01;
	params.b = params.a * s;
	params.c = params.a * s;
	params.d = params.a * s;
	params.cx = init_sz.cx * 0.5;
	params.cy = init_sz.cy * 0.5;

	status.Set("Solving intrinsics (extrinsics locked from Stage A)...");
	if (solver.SolveIntrinsicsOnly(params)) {
		last_calibration.is_enabled = true;
		last_calibration.eye_dist = (float)solver.eye_dist;
		last_calibration.outward_angle = (float)params.yaw; // Solver's yaw is relative yaw
		last_calibration.right_pitch = (float)params.pitch;
		last_calibration.right_roll = (float)params.roll;
		last_calibration.principal_point = vec2((float)params.cx, (float)params.cy);
		last_calibration.angle_to_pixel = vec4((float)params.a, (float)params.b, (float)params.c, (float)params.d);
		
		// Note: We might want to store the Stage A baseline yaw_l separately if we want to support absolute orientations.
		// For now, solver's yaw IS the delta.
		
		SyncEditsFromCalibration();
		UpdatePreview();
		
		StereoCalibrationDiagnostics diag;
		solver.ComputeDiagnostics(params, diag);
		report_text <<= "Stage B Solve Success.\n" + Format("Reproj RMS (L/R): %.3f / %.3f px\n", diag.reproj_rms_l, diag.reproj_rms_r);
		math_text <<= math_log + solver.GetTraceText();
		bottom_tabs.Set(1);
		status.Set("Intrinsics solved.");
	} else {
		PromptOK("Solver failed: " + solver.last_failure_reason);
	}
}

void StereoCalibrationTool::RefineExtrinsics() {
	if (!enable_stage_c) {
		PromptOK("Enable Stage C first.");
		return;
	}
	
	String math_log;
	math_log << "Stage C Micro-Refine\n====================\n\n";
	
	StereoCalibrationSolver solver;
	solver.log = &math_log;
	solver.eye_dist = (double)calib_eye_dist / 1000.0;
	solver.EnableTrace(verbose_math_log, 2, 20000);
	
	for (const auto& f : captured_frames) {
		Size sz = f.left_img.GetSize();
		if (sz.cx <= 0) continue;
		for (const auto& m : f.matches) {
			if (IsNull(m.left) || IsNull(m.right)) continue;
			auto& p = solver.matches.Add();
			p.left_px = vec2((float)(m.left.x * sz.cx), (float)(m.left.y * sz.cy));
			p.right_px = vec2((float)(m.right.x * sz.cx), (float)(m.right.y * sz.cy));
			p.image_size = sz;
			p.dist_l = m.dist_l / 1000.0;
			p.dist_r = m.dist_r / 1000.0;
		}
	}
	
	if (solver.matches.GetCount() < 5) {
		PromptOK("Need at least 5 matches.");
		return;
	}
	
	StereoCalibrationParams params;
	// Intrinsics from solved Stage B (or current calibration)
	params.a = last_calibration.angle_to_pixel[0];
	params.b = last_calibration.angle_to_pixel[1];
	params.c = last_calibration.angle_to_pixel[2];
	params.d = last_calibration.angle_to_pixel[3];
	params.cx = last_calibration.principal_point[0];
	params.cy = last_calibration.principal_point[1];
	
	// Extrinsics from Stage A (Baseline)
	double base_yaw_l = (double)yaw_l;
	double base_pitch_l = (double)pitch_l;
	double base_roll_l = (double)roll_l;
	double base_yaw_r = (double)yaw_r;
	double base_pitch_r = (double)pitch_r;
	double base_roll_r = (double)roll_r;
	
	params.yaw_l = base_yaw_l;
	params.pitch_l = base_pitch_l;
	params.roll_l = base_roll_l;
	params.yaw = base_yaw_r - base_yaw_l; // Relative for solver if relative mode
	// But for per-eye mode, solver uses params.yaw as Right absolute (or relative? Check solver)
	// My solver update uses:
	// if per_eye: 
	//   cur.yaw_l += dy_L; 
	//   cur.yaw += dy_R; (This means params.yaw is treated as Right ABSOLUTE in the solver update logic I wrote?)
	
	// Let's re-read my solver update:
	// cur.yaw += dy_R;
	// And helper: EyeRotation(p, 1) -> AxesMat(p.yaw...);
	// So p.yaw IS the Right Eye Rotation.
	// In legacy/relative mode, Left is 0, so p.yaw is Relative.
	// In per-eye mode, Left is p.yaw_l.
	// So p.yaw MUST BE Right Eye Absolute if we use it in EyeRotation(1).
	// So I should set params.yaw = base_yaw_r;
	
	params.yaw = base_yaw_r;
	params.pitch = base_pitch_r;
	params.roll = base_roll_r;
	
	vec3 bounds((double)max_dyaw, (double)max_dpitch, (double)max_droll);
	double lambda = (double)lambda_edit;
	bool per_eye = ((int)stage_c_mode == 1);
	
	status.Set("Refining extrinsics (Stage C)...");
	if (solver.SolveExtrinsicsOnlyMicroRefine(params, bounds, lambda, per_eye)) {
		// Update Stage C deltas for display
		dyaw_c = (float)(params.yaw - base_yaw_r); // These are now deltas from Stage A
		dpitch_c = (float)(params.pitch - base_pitch_r);
		droll_c = (float)(params.roll - base_roll_r);
		// Note: params.yaw_l is also updated if per-eye
		
		// Update last_calibration
		// We need to decide what to store. The system generally expects relative R (outward).
		// If Left is rotated, the relative transform is R * L_inv.
		// Or we just store the new "Outward Angle" as the relative yaw?
		// SoftHMD generally assumes headset frame is Left frame? Or symmetrical?
		// If SoftHMD assumes L is identity, we must store the Relative transform.
		// Let's compute relative rotation: R_rel = R_abs * L_abs^-1.
		// Then extract yaw/pitch/roll from R_rel.
		// Simple approx for small angles: yaw_rel = yaw_r - yaw_l.
		
		last_calibration.outward_angle = (float)(params.yaw - params.yaw_l);
		last_calibration.right_pitch = (float)(params.pitch - params.pitch_l);
		last_calibration.right_roll = (float)(params.roll - params.roll_l);
		
		// We also store absolute L if we want to preview it correctly in "Refined" mode?
		// But PreparePreviewLens calculates relative if we are in Stage C mode?
		// Let's check PreparePreviewLens logic. It adds dyaw_c to last_calibration.outward.
		// If I update last_calibration with the NEW relative, then PreparePreviewLens should NOT add deltas again?
		// OR: PreparePreviewLens uses Stage A + Deltas.
		// If I update last_calibration, I am "Committing" the result.
		// If I want "Preview" style where I can toggle, I should perhaps NOT update last_calibration immediately?
		// But "RefineExtrinsics" implies we are updating the calibration.
		// Let's stick to: Refine updates last_calibration (the "Solved" state).
		// dyaw_c is just for info.
		
		SyncEditsFromCalibration(); // This might overwrite Stage A edits if we are not careful? 
		// SyncEditsFromCalibration updates UI fields (yaw_l, etc) from last_calibration? 
		// No, SyncEditsFromCalibration updates "calib_eye_dist", "calib_poly_a", etc.
		// It does NOT update yaw_l/yaw_r controls (those are Stage A inputs).
		// So Stage A inputs remain as "Baseline".
		
		// Wait, SyncEditsFromCalibration reads last_calibration.outward_angle -> calib_outward_angle.
		
		UpdatePreview();
		
		StereoCalibrationDiagnostics diag;
		solver.ComputeDiagnostics(params, diag);
		
		String report;
		report << "Stage C Refinement Success.\n";
		report << Format("Mode: %s\n", per_eye ? "Per-eye" : "Relative-only");
		report << Format("Bounds: %.1f, %.1f, %.1f deg\n", bounds[0], bounds[1], bounds[2]);
		report << Format("Lambda: %.4f\n", lambda);
		report << "Final Deltas (deg):\n";
		if (per_eye) {
			report << Format("  Left:  %.3f, %.3f, %.3f\n", 
				(params.yaw_l - base_yaw_l)*180/M_PI, (params.pitch_l - base_pitch_l)*180/M_PI, (params.roll_l - base_roll_l)*180/M_PI);
		}
		report << Format("  Right: %.3f, %.3f, %.3f\n", 
			(params.yaw - base_yaw_r)*180/M_PI, (params.pitch - base_pitch_r)*180/M_PI, (params.roll - base_roll_r)*180/M_PI);
			
		report << Format("Reproj RMS (L/R): %.3f / %.3f px\n", diag.reproj_rms_l, diag.reproj_rms_r);
		report << Format("Dist RMS (L/R): %.3f / %.3f mm\n", diag.dist_rms_l, diag.dist_rms_r);
		
		report_text <<= report;
		math_text <<= math_log + solver.GetTraceText();
		bottom_tabs.Set(1);
		status.Set("Extrinsics refined.");
	} else {
		PromptOK("Refinement failed.");
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
	hsplitter.Horz(left_panel, right_panel);
	hsplitter.SetPos(2000);
	vsplitter.Vert(hsplitter, bottom_tabs);
	vsplitter.SetPos(8000);
	Add(vsplitter.SizePos());

	BuildLeftPanel();
	BuildBottomTabs();
	right_panel.Add(preview.SizePos());
	status.Set("Status: idle");
}

void StereoCalibrationTool::BuildLeftPanel() {
	mode_lbl.SetLabel("Match mode");
	mode_list.Add(0, "Point pairs");
	mode_list.Add(1, "Line pairs");
	mode_list.Add(2, "Yaw center");
	mode_list.Add(3, "Pitch center");
	mode_list.SetIndex(0);

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
	clear_matches.WhenAction = THISBACK(ClearMatches);

	source_status.SetLabel("Status: idle");
	sep_source.SetLabel("Camera Controls");
	
	view_mode_lbl.SetLabel("View Mode");
	view_mode_list.Add(VIEW_RAW, "Raw");
	view_mode_list.Add(VIEW_BASIC, "Basic Undistort");
	view_mode_list.Add(VIEW_SOLVED, "Solved Undistort");
	view_mode_list.SetIndex(0);
	view_mode_list.WhenAction = THISBACK(OnReviewChanged);

	overlay_eyes.SetLabel("Overlay Eyes");
	overlay_eyes.WhenAction = THISBACK(OnReviewChanged);
	
	overlay_swap.SetLabel("Swap Order");
	overlay_swap.WhenAction = THISBACK(OnReviewChanged);
	
	show_difference.SetLabel("Show Diff");
	show_difference.WhenAction = THISBACK(OnReviewChanged);
	
	alpha_lbl.SetLabel("Alpha");
	alpha_slider.MinMax(0, 100);
	alpha_slider <<= 50;
	alpha_slider.WhenAction = THISBACK(OnReviewChanged);

	show_epipolar.SetLabel("Show epipolar line");
	show_epipolar.WhenAction = THISBACK(OnReviewChanged);

	int y = 4;
	pinned_camera_controls.Add(sep_source.TopPos(y, 18).HSizePos(4, 4));
	y += 22;
	pinned_camera_controls.Add(source_list.TopPos(y, 24).HSizePos(4, 4));
	y += 28;
	
	pinned_camera_controls.Add(mode_lbl.TopPos(y, 20).LeftPos(4, 70));
	pinned_camera_controls.Add(mode_list.TopPos(y, 20).HSizePos(78, 4));
	y += 28;

	pinned_camera_controls.Add(start_source.TopPos(y, 24).LeftPos(4, 60));
	pinned_camera_controls.Add(stop_source.TopPos(y, 24).LeftPos(68, 60));
	pinned_camera_controls.Add(live_view.TopPos(y, 24).LeftPos(132, 70));
	pinned_camera_controls.Add(capture_frame.TopPos(y, 24).LeftPos(206, 70));
	y += 28;
	pinned_camera_controls.Add(source_status.TopPos(y, 20).HSizePos(4, 4));
	y += 24;
	
	pinned_camera_controls.Add(view_mode_lbl.TopPos(y, 20).LeftPos(4, 70));
	pinned_camera_controls.Add(view_mode_list.TopPos(y, 20).HSizePos(78, 4));
	y += 24;
	
	pinned_camera_controls.Add(overlay_eyes.TopPos(y, 20).LeftPos(4, 100));
	pinned_camera_controls.Add(overlay_swap.TopPos(y, 20).LeftPos(110, 90));
	pinned_camera_controls.Add(show_difference.TopPos(y, 20).LeftPos(200, 80));
	y += 24;
	
	pinned_camera_controls.Add(alpha_lbl.TopPos(y, 20).LeftPos(4, 40));
	pinned_camera_controls.Add(alpha_slider.TopPos(y, 20).HSizePos(48, 4));
	y += 24;

	pinned_camera_controls.Add(show_epipolar.TopPos(y, 20).LeftPos(4, 180));
	y += 24;
	
	left_panel.Add(pinned_camera_controls.TopPos(0, y).HSizePos());
	left_panel.Add(stage_tabs.VSizePos(y, 0).HSizePos());

	BuildStageA();
	BuildStageB();
	BuildStageC();
	
	stage_tabs.Add(stage_a_ctrl.SizePos(), "Stage A (Basic)");
	stage_tabs.Add(stage_b_ctrl.SizePos(), "Stage B (Solve)");
	stage_tabs.Add(stage_c_ctrl.SizePos(), "Stage C (Refine)");

	sources.Clear();
	sources.Add(MakeOne<HmdStereoSource>());
	sources.Add(MakeOne<UsbStereoSource>());
	sources.Add(MakeOne<VideoStereoSource>());
}

void StereoCalibrationTool::BuildStageA() {
	int y = 8;
	stage_a_ctrl.Add(calib_eye_lbl.TopPos(y, 20).LeftPos(8, 80));
	stage_a_ctrl.Add(calib_eye_dist.TopPos(y, 20).LeftPos(96, 80));
	calib_eye_lbl.SetLabel("Eye dist (mm)");
	calib_eye_dist.WhenAction = THISBACK(SyncStageA);
	y += 24;

	eye_l_group.SetLabel("Left Eye");
	stage_a_ctrl.Add(eye_l_group.TopPos(y, 100).HSizePos(4, 4));
	int gy = y + 20;
	yaw_l_lbl.SetLabel("Yaw"); stage_a_ctrl.Add(yaw_l_lbl.TopPos(gy, 20).LeftPos(12, 40));
	stage_a_ctrl.Add(yaw_l.TopPos(gy, 20).LeftPos(52, 80));
	pitch_l_lbl.SetLabel("Pitch"); stage_a_ctrl.Add(pitch_l_lbl.TopPos(gy + 24, 20).LeftPos(12, 40));
	stage_a_ctrl.Add(pitch_l.TopPos(gy + 24, 20).LeftPos(52, 80));
	roll_l_lbl.SetLabel("Roll"); stage_a_ctrl.Add(roll_l_lbl.TopPos(gy + 48, 20).LeftPos(12, 40));
	stage_a_ctrl.Add(roll_l.TopPos(gy + 48, 20).LeftPos(52, 80));
	y += gy - y + 80;

	eye_r_group.SetLabel("Right Eye");
	stage_a_ctrl.Add(eye_r_group.TopPos(y, 100).HSizePos(4, 4));
	gy = y + 20;
	yaw_r_lbl.SetLabel("Yaw"); stage_a_ctrl.Add(yaw_r_lbl.TopPos(gy, 20).LeftPos(12, 40));
	stage_a_ctrl.Add(yaw_r.TopPos(gy, 20).LeftPos(52, 80));
	pitch_r_lbl.SetLabel("Pitch"); stage_a_ctrl.Add(pitch_r_lbl.TopPos(gy + 24, 20).LeftPos(12, 40));
	stage_a_ctrl.Add(pitch_r.TopPos(gy + 24, 20).LeftPos(52, 80));
	roll_r_lbl.SetLabel("Roll"); stage_a_ctrl.Add(roll_r_lbl.TopPos(gy + 48, 20).LeftPos(12, 40));
	stage_a_ctrl.Add(roll_r.TopPos(gy + 48, 20).LeftPos(52, 80));
	y += gy - y + 80;

	yaw_l.SetInc(0.01); yaw_l.MinMax(-1.2, 1.2); yaw_l.WhenAction = THISBACK(SyncStageA);
	pitch_l.SetInc(0.01); pitch_l.MinMax(-0.6, 0.6); pitch_l.WhenAction = THISBACK(SyncStageA);
	roll_l.SetInc(0.01); roll_l.MinMax(-1.2, 1.2); roll_l.WhenAction = THISBACK(SyncStageA);
	yaw_r.SetInc(0.01); yaw_r.MinMax(-1.2, 1.2); yaw_r.WhenAction = THISBACK(SyncStageA);
	pitch_r.SetInc(0.01); pitch_r.MinMax(-0.6, 0.6); pitch_r.WhenAction = THISBACK(SyncStageA);
	roll_r.SetInc(0.01); roll_r.MinMax(-1.2, 1.2); roll_r.WhenAction = THISBACK(SyncStageA);

	stage_a_ctrl.Add(preview_extrinsics.TopPos(y, 20).LeftPos(8, 180));
	preview_extrinsics.SetLabel("Preview extrinsics");
	preview_extrinsics <<= true;
	preview_extrinsics.WhenAction = THISBACK(SyncStageA);
	y += 24;

	stage_a_ctrl.Add(barrel_lbl.TopPos(y, 20).LeftPos(8, 120)); barrel_lbl.SetLabel("Barrel distortion");
	stage_a_ctrl.Add(barrel_strength.TopPos(y, 20).LeftPos(132, 60));
	barrel_strength.SetInc(0.1); barrel_strength <<= 0; barrel_strength.WhenAction = THISBACK(SyncStageA);
	y += 24;

	stage_a_ctrl.Add(fov_lbl.TopPos(y, 20).LeftPos(8, 120)); fov_lbl.SetLabel("FOV (deg)");
	stage_a_ctrl.Add(fov_deg.TopPos(y, 20).LeftPos(132, 60));
	fov_deg.SetInc(1.0); fov_deg <<= 90; fov_deg.WhenAction = THISBACK(SyncStageA);
	y += 24;

	stage_a_ctrl.Add(basic_params_doc.TopPos(y, 100).HSizePos(4, 4));
	basic_params_doc.SetReadOnly();
	y += 104;

	stage_a_ctrl.Add(yaw_center_btn.TopPos(y, 24).LeftPos(4, 90));
	yaw_center_btn.SetLabel("Yaw center");
	yaw_center_btn <<= THISBACK(OnYawCenter);
	stage_a_ctrl.Add(pitch_center_btn.TopPos(y, 24).LeftPos(98, 90));
	pitch_center_btn.SetLabel("Pitch center");
	pitch_center_btn <<= THISBACK(OnPitchCenter);
	y += 28;
}

void StereoCalibrationTool::BuildStageB() {
	int y = 8;
	stage_b_ctrl.Add(solve_calibration.TopPos(y, 24).HSizePos(4, 4));
	solve_calibration.SetLabel("Solve Intrinsics");
	solve_calibration <<= THISBACK(SolveCalibration);
	y += 32;

	stage_b_ctrl.Add(verbose_math_log.TopPos(y, 20).HSizePos(4, 4));
	verbose_math_log.SetLabel("Verbose math log");
	y += 24;

	stage_b_ctrl.Add(stage_b_compare_basic.TopPos(y, 20).HSizePos(4, 4));
	stage_b_compare_basic.SetLabel("Compare Basic Params");
	stage_b_compare_basic.WhenAction = THISBACK(OnReviewChanged);
	y += 24;

	stage_b_ctrl.Add(clear_matches.TopPos(y, 24).HSizePos(4, 4));
	y += 28;

	stage_b_ctrl.Add(sep_calib.TopPos(y, 18).HSizePos(4, 4)); y += 22;
	stage_b_ctrl.Add(load_calibration.TopPos(y, 24).LeftPos(4, 100));
	stage_b_ctrl.Add(export_calibration.TopPos(y, 24).LeftPos(108, 100));
	y += 28;
	stage_b_ctrl.Add(deploy_calibration.TopPos(y, 24).HSizePos(4, 4));
	y += 32;
	
	stage_b_ctrl.Add(calibration_preview.TopPos(y, 100).HSizePos(4, 4));
}

void StereoCalibrationTool::BuildStageC() {
	int y = 8;
	stage_c_ctrl.Add(enable_stage_c.TopPos(y, 20).HSizePos(4, 4));
	enable_stage_c.SetLabel("Enable micro-refine");
	enable_stage_c.WhenAction = THISBACK(OnReviewChanged);
	y += 24;

	stage_c_ctrl.Add(stage_c_mode_lbl.TopPos(y, 20).LeftPos(8, 60));
	stage_c_mode_lbl.SetLabel("Mode:");
	stage_c_ctrl.Add(stage_c_mode.TopPos(y, 20).LeftPos(72, 160));
	stage_c_mode.Add(0, "Relative-only");
	stage_c_mode.Add(1, "Per-eye");
	stage_c_mode <<= 0;
	y += 28;

	stage_c_ctrl.Add(max_dyaw_lbl.TopPos(y, 20).LeftPos(8, 80)); max_dyaw_lbl.SetLabel("Max dYaw");
	stage_c_ctrl.Add(max_dyaw.TopPos(y, 20).LeftPos(96, 80));
	max_dyaw.SetInc(0.1); max_dyaw <<= 3.0;
	y += 24;

	stage_c_ctrl.Add(max_dpitch_lbl.TopPos(y, 20).LeftPos(8, 80)); max_dpitch_lbl.SetLabel("Max dPitch");
	stage_c_ctrl.Add(max_dpitch.TopPos(y, 20).LeftPos(96, 80));
	max_dpitch.SetInc(0.1); max_dpitch <<= 2.0;
	y += 24;

	stage_c_ctrl.Add(max_droll_lbl.TopPos(y, 20).LeftPos(8, 80)); max_droll_lbl.SetLabel("Max dRoll");
	stage_c_ctrl.Add(max_droll.TopPos(y, 20).LeftPos(96, 80));
	max_droll.SetInc(0.1); max_droll <<= 3.0;
	y += 24;

	stage_c_ctrl.Add(lambda_lbl.TopPos(y, 20).LeftPos(8, 80)); lambda_lbl.SetLabel("Lambda");
	stage_c_ctrl.Add(lambda_edit.TopPos(y, 20).LeftPos(96, 80));
	lambda_edit.SetInc(0.01); lambda_edit <<= 0.1;
	y += 24;

	stage_c_ctrl.Add(refine_btn.TopPos(y, 24).HSizePos(4, 4));
	refine_btn.SetLabel("Run Stage C");
	refine_btn <<= THISBACK(RefineExtrinsics);
	y += 28;
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

void StereoCalibrationTool::SyncStageA() {
	// Derived lens params for Stage A
	// FOV -> focal length 'a'
	// a = (width/2) / tan(FOV_rad/2)
	// But our model uses a*theta = radius.
	// So radius_at_half_fov = a * (FOV_rad/2) => a = (width/2) / (FOV_rad/2)
	
	// We'll update the preview lens based on these Stage A values IF stage A is active OR "Preview extrinsics" is ON.
	
	// Map strength to b,c,d
	// b = strength * 0.01, c = strength * 0.01, d = strength * 0.01 as a simple stable mapping
	
	String doc;
	doc << "Stage A Basic Params:\n";
	doc << "  Eye dist: " << (double)calib_eye_dist << " mm\n";
	doc << Format("  Left Yaw/Pitch/Roll: %.3f, %.3f, %.3f\n", (double)yaw_l, (double)pitch_l, (double)roll_l);
	doc << Format("  Right Yaw/Pitch/Roll: %.3f, %.3f, %.3f\n", (double)yaw_r, (double)pitch_r, (double)roll_r);
	
	double fov_rad = (double)fov_deg * M_PI / 180.0;
	// We need a width to compute 'a'. Let's assume a default or use current image size.
	Size sz = preview.left_img.GetSize();
	if (sz.cx <= 0) sz = Size(1280, 720); // Fallback
	
	double a = (sz.cx * 0.5) / (fov_rad * 0.5);
	double s = (double)barrel_strength * 0.01;
	double b = a * s;
	double c = a * s;
	double d = a * s;
	
	doc << Format("  Derived a: %.3f\n", a);
	doc << Format("  Derived b,c,d: %.3f, %.3f, %.3f\n", b, c, d);
	
	basic_params_doc <<= doc;
	
	OnReviewChanged();
}

Pointf StereoCalibrationTool::MapClickToRaw(Pointf p) {
	int vmode = view_mode_list.GetIndex();
	if (vmode == VIEW_RAW) return p;
	
	if (preview.left_img.IsEmpty()) return p;
	Size isz = preview.left_img.GetSize();
	
	vec2 pp = preview_lens.GetPrincipalPoint();
	float cx = (pp[0] > 0) ? pp[0] : isz.cx * 0.5f;
	float cy = (pp[1] > 0) ? pp[1] : isz.cy * 0.5f;
	
	float dx = p.x * isz.cx - cx;
	float dy = p.y * isz.cy - cy;
	
	float max_radius = (float)sqrt(isz.cx*isz.cx*0.25f + isz.cy*isz.cy*0.25f);
	float max_angle = preview_lens.PixelToAngle(max_radius);
	if (max_angle <= 1e-6f) return p;
	float linear_scale = max_radius / max_angle;
	
	float r_undist = sqrt(dx*dx + dy*dy);
	float theta = r_undist / linear_scale;
	float roll = atan2(dy, dx);
	
	float r_raw = preview_lens.AngleToPixel(theta);
	float raw_x = cx + r_raw * cos(roll);
	float raw_y = cy + r_raw * sin(roll);
	
	return Pointf(raw_x / isz.cx, raw_y / isz.cy);
}

void StereoCalibrationTool::OnYawCenter() {
	int row = captures_list.GetCursor();
	if (row < 0 || row >= captured_frames.GetCount()) {
		status.Set("Select a captured frame first.");
		return;
	}
	CapturedFrame& frame = captured_frames[row];
	if (frame.matches.IsEmpty()) {
		status.Set("Add a match pair (same point in both eyes) to center.");
		return;
	}
	
	const MatchPair& m = frame.matches.Top();
	if (IsNull(m.left) || IsNull(m.right)) return;
	
	Size sz = frame.left_img.GetSize();
	if (sz.cx <= 0) return;
	
	// Construct LensPoly for Stage A
	double fov_rad = (double)fov_deg * M_PI / 180.0;
	double a = (sz.cx * 0.5) / (fov_rad * 0.5);
	double s = (double)barrel_strength * 0.01;
	double b = a * s;
	double c = a * s;
	double d = a * s;
	
	LensPoly lens;
	lens.SetAnglePixel((float)a, (float)b, (float)c, (float)d);
	lens.SetPrincipalPoint(sz.cx * 0.5f, sz.cy * 0.5f);
	
	auto GetHAngle = [&](Pointf p) -> double {
		float cx = sz.cx * 0.5f;
		float cy = sz.cy * 0.5f;
		float dx = p.x * sz.cx - cx;
		float dy = p.y * sz.cy - cy;
		float r = sqrt(dx*dx + dy*dy);
		float theta = lens.PixelToAngle(r);
		float roll = atan2(dy, dx);
		// Direction in camera space (z forward)
		// x = sin(theta)cos(roll)
		// z = cos(theta)
		double x = sin(theta) * cos(roll);
		double z = cos(theta);
		return atan2(x, z);
	};
	
	double angle_l = GetHAngle(m.left);
	double angle_r = GetHAngle(m.right);
	
	// angle_l + yaw_l should match angle_r + yaw_r in world
	// yaw_r = yaw_l + angle_l - angle_r
	
	double new_yaw_r = (double)yaw_l + angle_l - angle_r;
	double delta = new_yaw_r - (double)yaw_r;
	yaw_r <<= new_yaw_r;
	
	SyncStageA();
	status.Set(Format("Yaw aligned. Delta: %.3f deg (%.4f rad)", delta * 180.0 / M_PI, delta));
}

void StereoCalibrationTool::OnPitchCenter() {
	int row = captures_list.GetCursor();
	if (row < 0 || row >= captured_frames.GetCount()) {
		status.Set("Select a captured frame first.");
		return;
	}
	CapturedFrame& frame = captured_frames[row];
	if (frame.matches.IsEmpty()) {
		status.Set("Add a match pair (same point in both eyes) to center.");
		return;
	}
	
	const MatchPair& m = frame.matches.Top();
	if (IsNull(m.left) || IsNull(m.right)) return;
	
	Size sz = frame.left_img.GetSize();
	if (sz.cx <= 0) return;
	
	double fov_rad = (double)fov_deg * M_PI / 180.0;
	double a = (sz.cx * 0.5) / (fov_rad * 0.5);
	double s = (double)barrel_strength * 0.01;
	
	LensPoly lens;
	lens.SetAnglePixel((float)a, (float)(a*s), (float)(a*s), (float)(a*s));
	lens.SetPrincipalPoint(sz.cx * 0.5f, sz.cy * 0.5f);
	
	auto GetVAngle = [&](Pointf p) -> double {
		float cx = sz.cx * 0.5f;
		float cy = sz.cy * 0.5f;
		float dx = p.x * sz.cx - cx;
		float dy = p.y * sz.cy - cy;
		float r = sqrt(dx*dx + dy*dy);
		float theta = lens.PixelToAngle(r);
		float roll = atan2(dy, dx);
		// y = -sin(theta)sin(roll)
		// z = cos(theta)
		// pitch is approx atan2(y, z) ?
		// Actually pitch is rotation around X.
		// If we pitch up, y decreases?
		// Check coords: Y down?
		// If Y down, dy positive is down.
		// roll=90 => y = -sin(theta) * 1 = -sin(theta). (Up in 3D?)
		// Let's use simple atan2(y, z) for relative pitch.
		double y_val = -sin(theta) * sin(roll);
		double z_val = cos(theta);
		return atan2(y_val, z_val);
	};
	
	double angle_l = GetVAngle(m.left);
	double angle_r = GetVAngle(m.right);
	
	// pitch_r = pitch_l + angle_l - angle_r
	
	double new_pitch_r = (double)pitch_l + angle_l - angle_r;
	double delta = new_pitch_r - (double)pitch_r;
	pitch_r <<= new_pitch_r;
	
	SyncStageA();
	status.Set(Format("Pitch aligned. Delta: %.3f deg (%.4f rad)", delta * 180.0 / M_PI, delta));
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
	last_calibration.eye_dist = (float)((double)~calib_eye_dist / 1000.0);  // UI is mm, storage is m
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
	calib_eye_dist <<= (double)last_calibration.eye_dist * 1000.0;  // Storage is m, UI is mm
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
	s << Format("  eye_dist=%.6f (m)\n", last_calibration.eye_dist);
	s << Format("  outward_angle=%.6f\n", last_calibration.outward_angle);
	s << Format("  right_pitch=%.6f\n", last_calibration.right_pitch);
	s << Format("  right_roll=%.6f\n", last_calibration.right_roll);
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
	// undistort_view removed
	if (!can_review) {
		show_epipolar = false;
		preview.SetEpipolar(false);
		preview.SetResiduals(Vector<PreviewCtrl::ResidualSample>(), 0, false);
	}
}

bool StereoCalibrationTool::PreparePreviewLens(const Size& sz) {
	if (sz.cx <= 0 || sz.cy <= 0)
		return false;

	// Determine which intrinsics to use
	StereoCalibrationParams p;
	bool use_basic = false;
	
	int vmode = view_mode_list.GetIndex();
	if (vmode == VIEW_BASIC) use_basic = true;
	else if (vmode == VIEW_SOLVED) use_basic = false;
	else { // RAW
		// Heuristic: If in Stage A, use Basic. Else use Solved.
		if (stage_tabs.Get() == 0) use_basic = true;
		else use_basic = false;
	}
	
	// Stage B Override
	if (stage_b_compare_basic) use_basic = true;

	if (use_basic) {
		double fov_rad = (double)fov_deg * M_PI / 180.0;
		p.a = (sz.cx * 0.5) / (fov_rad * 0.5);
		double s = (double)barrel_strength * 0.01;
		p.b = p.a * s;
		p.c = p.a * s;
		p.d = p.a * s;
		p.cx = sz.cx * 0.5;
		p.cy = sz.cy * 0.5;
	} else {
		p.a = last_calibration.angle_to_pixel[0];
		p.b = last_calibration.angle_to_pixel[1];
		p.c = last_calibration.angle_to_pixel[2];
		p.d = last_calibration.angle_to_pixel[3];
		p.cx = last_calibration.principal_point[0];
		p.cy = last_calibration.principal_point[1];
	}

	// Determine which extrinsics to use
	// For Preview:
	// Stage A: Use User Controls (yaw_l, yaw_r...) if "Preview Extrinsics" is checked.
	// Stage B/C: Use Solved Extrinsics (last_calibration) + Stage C Deltas.
	// But if "Preview Extrinsics" is ON in Stage A, we should probably use them regardless of ViewMode?
	// Actually, Basic Mode usually implies Basic Extrinsics too?
	// Let's allow Stage A controls to override if we are in Stage A.
	
	double yaw = 0, pitch = 0, roll = 0;
	
	bool use_stage_a_extrinsics = (stage_tabs.Get() == 0 && preview_extrinsics);
	
	if (use_stage_a_extrinsics) {
		yaw = (double)yaw_r - (double)yaw_l;
		pitch = (double)pitch_r - (double)pitch_l;
		roll = (double)roll_r - (double)roll_l;
	} else {
		yaw = last_calibration.outward_angle;
		pitch = last_calibration.right_pitch;
		roll = last_calibration.right_roll;
		if (enable_stage_c) {
			yaw += dyaw_c;
			pitch += dpitch_c;
			roll += droll_c;
		}
	}
	p.yaw = yaw;
	p.pitch = pitch;
	p.roll = roll;

	vec4 poly((float)p.a, (float)p.b, (float)p.c, (float)p.d);
	vec2 pp((float)p.cx, (float)p.cy);
	vec2 tilt((float)p.pitch, (float)p.roll);
	
	bool needs = (preview_lens_size != sz) || !IsSamePoly(preview_lens_poly, poly) ||
		fabs(preview_lens_outward - (float)p.yaw) > 1e-6 ||
		fabs(preview_lens_pp[0] - pp[0]) > 1e-3 ||
		fabs(preview_lens_pp[1] - pp[1]) > 1e-3 ||
		fabs(preview_lens_tilt[0] - tilt[0]) > 1e-6 ||
		fabs(preview_lens_tilt[1] - tilt[1]) > 1e-6;

	if (needs) {
		preview_lens.SetAnglePixel(poly.data[0], poly.data[1], poly.data[2], poly.data[3]);
		preview_lens.SetEyeOutwardAngle((float)p.yaw);
		preview_lens.SetRightTilt((float)p.pitch, (float)p.roll);
		preview_lens.SetPrincipalPoint(pp[0], pp[1]);
		preview_lens.SetSize(sz);
		preview_lens_size = sz;
		preview_lens_poly = poly;
		preview_lens_outward = (float)p.yaw;
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
	
	int vmode = view_mode_list.GetIndex();
	if (vmode == VIEW_RAW) {
		frame.undist_valid = false;
		return false;
	}

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
	
	int vmode = view_mode_list.GetIndex();
	if (vmode == VIEW_RAW) {
		live_undist_valid = false;
		return false;
	}

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
	if (BuildUndistortCache(frame))
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
		
		// Note: preview_lens already has the active Stage A/B/C params if PreparePreviewLens was called.
		axes2 axesL = preview_lens.Unproject(0, lp);
		axes2 axesR = preview_lens.Unproject(1, rp);
		vec3 dL = GetAxesDir(axesL);
		vec3 dR = GetAxesDir(axesR);
		vec3 pL = vec3(-eye_dist / 2.0f, 0, 0);
		vec3 pR = vec3(eye_dist / 2.0f, 0, 0);
		
		vec3 pt = TriangulatePoint(pL, dL, pR, dR);

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
	
	// Sync Viewer Settings
	preview.SetOverlayMode(overlay_eyes, (float)(int)~alpha_slider / 100.0f, overlay_swap ? 1 : 0);
	preview.SetDifference(show_difference);
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
			Value v = ParseJSON(json);
			
			// Migration Logic
			Value state_val = v["state"];
			int ver = state_val["schema_version"];
			if (IsNull(state_val) || ver < 1) {
				Cout() << "Migrating project state from legacy/v0 to v1...\n";
				// Legacy to v1:
				// Map stage_a_undistort (if exists) to view_mode
				if (!IsNull(state_val["stage_a_undistort"]) && (bool)state_val["stage_a_undistort"]) {
					// We can't modify Value directly easily if it's const reference or map logic is complex.
					// Easier to load into struct then patch struct.
				}
			}

			LoadFromJsonValue(captured_frames, v["frames"]);
			LoadFromJsonValue(project_state, v["state"]);
			
			// Post-load patches for migration
			if (ver < 1) {
				// Restore view mode from legacy field if ViewMode is RAW (0)
				// Accessing legacy field from 'v["state"]'
				if (project_state.view_mode == VIEW_RAW && !IsNull(state_val["stage_a_undistort"]) && (bool)state_val["stage_a_undistort"]) {
					project_state.view_mode = VIEW_BASIC;
				}
				project_state.schema_version = 1;
			}
			
			// Sync UI from project_state
			// Stage A
			calib_eye_dist <<= project_state.eye_dist;
			yaw_l <<= project_state.yaw_l;
			pitch_l <<= project_state.pitch_l;
			roll_l <<= project_state.roll_l;
			yaw_r <<= project_state.yaw_r;
			pitch_r <<= project_state.pitch_r;
			roll_r <<= project_state.roll_r;
			barrel_strength <<= project_state.barrel_strength;
			fov_deg <<= project_state.fov_deg;
			preview_extrinsics <<= project_state.preview_extrinsics;
			
			// Stage B
			verbose_math_log <<= project_state.verbose_math_log;
			stage_b_compare_basic <<= project_state.compare_basic_params;
			
			// Stage C
			enable_stage_c <<= project_state.stage_c_enabled;
			stage_c_mode <<= project_state.stage_c_mode;
			max_dyaw <<= project_state.max_dyaw;
			max_dpitch <<= project_state.max_dpitch;
			max_droll <<= project_state.max_droll;
			lambda_edit <<= project_state.lambda;
			
			// Viewer
			view_mode_list.SetIndex(project_state.view_mode);
			overlay_eyes <<= project_state.overlay_eyes;
			alpha_slider <<= project_state.alpha;
			overlay_swap <<= project_state.overlay_swap;
			show_difference <<= project_state.show_difference;
			show_epipolar <<= project_state.show_epipolar;

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
	SyncStageA();
}

void StereoCalibrationTool::SaveState() {
	if (project_dir.IsEmpty()) return;
	
	// Sync project_state from UI
	// Stage A
	project_state.eye_dist = calib_eye_dist;
	project_state.yaw_l = yaw_l;
	project_state.pitch_l = pitch_l;
	project_state.roll_l = roll_l;
	project_state.yaw_r = yaw_r;
	project_state.pitch_r = pitch_r;
	project_state.roll_r = roll_r;
	project_state.barrel_strength = barrel_strength;
	project_state.fov_deg = fov_deg;
	project_state.preview_extrinsics = preview_extrinsics;
	
	// Stage B
	project_state.verbose_math_log = verbose_math_log;
	project_state.compare_basic_params = stage_b_compare_basic;
	
	// Stage C
	project_state.stage_c_enabled = enable_stage_c;
	project_state.stage_c_mode = stage_c_mode;
	project_state.max_dyaw = max_dyaw;
	project_state.max_dpitch = max_dpitch;
	project_state.max_droll = max_droll;
	project_state.lambda = lambda_edit;
	
	// Viewer
	project_state.view_mode = view_mode_list.GetIndex();
	project_state.overlay_eyes = overlay_eyes;
	project_state.alpha = ~alpha_slider;
	project_state.overlay_swap = overlay_swap;
	project_state.show_difference = show_difference;
	project_state.show_epipolar = show_epipolar;
	
	project_state.schema_version = 1; // Ensure version is current

	Json json;
	json("frames", StoreAsJsonValue(captured_frames));
	json("state", StoreAsJsonValue(project_state));
	
	String path = GetStatePath();
	SaveFile(path, json);
	
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

int StereoCalibrationTool::SolveHeadless(const String& project_dir) {
	// Load project
	String project_file = AppendFileName(project_dir, "project.json");
	if (!FileExists(project_file)) {
		Cerr() << "Error: Project file not found: " << project_file << "\n";
		return 1;
	}

	String json_text = LoadFile(project_file);
	if (json_text.IsEmpty()) {
		Cerr() << "Error: Failed to load project file\n";
		return 1;
	}

	if (!LoadFromJson(captured_frames, json_text)) {
		Cerr() << "Error: Failed to parse project JSON\n";
		return 1;
	}

	if (captured_frames.IsEmpty()) {
		Cerr() << "Error: No captured frames in project\n";
		return 1;
	}

	// Load calibration
	String calib_file = AppendFileName(project_dir, "calibration.stcal");
	if (FileExists(calib_file)) {
		if (!VisitFromJsonFile(last_calibration, calib_file)) {
			Cerr() << "Warning: Failed to load calibration file, using defaults\n";
		}
	}

	// Count matches
	int total_matches = 0;
	for (const auto& f : captured_frames) {
		for (const auto& m : f.matches) {
			if (!IsNull(m.left) && !IsNull(m.right))
				total_matches++;
		}
	}

	Cout() << "Loaded project from: " << project_dir << "\n";
	Cout() << "Captured frames: " << captured_frames.GetCount() << "\n";
	Cout() << "Total match pairs: " << total_matches << "\n";
	Cout() << "\n";

	if (total_matches < 5) {
		Cerr() << "Error: Need at least 5 match pairs, found " << total_matches << "\n";
		return 1;
	}

	// Run solver
	Cout() << "Running solver...\n";
	Cout() << "========================================\n";

	// Populate solver (copied from SolveCalibration)
	StereoCalibrationSolver solver;
	String math_log;
	solver.log = &math_log;
	solver.EnableTrace(true, 2, 20000);

	double eye_dist_m = (double)last_calibration.eye_dist;
	if (eye_dist_m <= 0) {
		eye_dist_m = 0.118;  // Default 118mm
		Cout() << "Using default eye distance: " << eye_dist_m << " m (118 mm)\n";
	}

	solver.eye_dist = eye_dist_m;
	solver.dist_weight = 0.1;
	solver.huber_px = 2.0;
	solver.huber_m = 0.030;
	solver.max_fevals = 0;

	// Configure GA bootstrap if enabled
	solver.use_ga_init = use_ga_bootstrap;
	solver.ga_population = ga_population;
	solver.ga_generations = ga_generations;
	solver.ga_top_candidates = 3;

	Size sz(640, 480);
	for (const auto& f : captured_frames) {
		if (f.matches.IsEmpty())
			continue;
		for (const auto& m : f.matches) {
			if (IsNull(m.left) || IsNull(m.right))
				continue;
			auto& p = solver.matches.Add();
			p.left_px = vec2((float)(m.left.x * sz.cx), (float)(m.left.y * sz.cy));
			p.right_px = vec2((float)(m.right.x * sz.cx), (float)(m.right.y * sz.cy));
			p.image_size = sz;
			// Convert distances from UI (millimeters) to solver (meters)
			p.dist_l = m.dist_l / 1000.0;
			p.dist_r = m.dist_r / 1000.0;
		}
	}

	// Initial parameters using heuristics (same as GUI version)
	StereoCalibrationParams params;
	params.a = 2.0 * sz.cx / M_PI;  // Heuristic: assumes ~180° FOV maps to image width
	params.b = 0.01;   // Small non-zero to help gradient computation
	params.c = 0.01;
	params.d = 0.01;
	params.cx = sz.cx * 0.5;
	params.cy = sz.cy * 0.5;
	params.yaw = 0.01;      // Small non-zero for extrinsics
	params.pitch = 0.01;
	params.roll = 0.01;

	// For headless mode with heuristic initialization, skip stage 1
	// and go straight to full optimization to avoid getting stuck
	Cout() << "Stage 1: Skipped (using heuristic init, going straight to full optimization)\n";
	Cout() << "Stage 2: Full optimization (all parameters)...\n";
	bool ok2 = solver.Solve(params, false);

	// Output math log
	Cout() << solver.GetTraceText();
	Cout() << "\n========================================\n";

	if (!ok2) {
		Cerr() << "Solver failed: " << solver.last_failure_reason << "\n";
		return 1;
	}

	// Compute diagnostics
	StereoCalibrationDiagnostics diag;
	solver.ComputeDiagnostics(params, diag);

	Cout() << "\nFinal Diagnostics:\n";
	Cout() << "========================================\n";
	Cout() << Format("Reprojection RMS L/R: %.3f / %.3f px\n", diag.reproj_rms_l, diag.reproj_rms_r);
	Cout() << Format("Distance RMS L/R: %.6f / %.6f m (%.2f / %.2f mm)\n",
		diag.dist_rms_l, diag.dist_rms_r, diag.dist_rms_l * 1000.0, diag.dist_rms_r * 1000.0);
	Cout() << Format("Behind camera L/R: %d / %d\n", diag.behind_left, diag.behind_right);
	Cout() << "\nFinal parameters:\n";
	Cout() << Format("  a=%.6f, b=%.6f, c=%.6f, d=%.6f\n", params.a, params.b, params.c, params.d);
	Cout() << Format("  cx=%.2f, cy=%.2f\n", params.cx, params.cy);
	Cout() << Format("  yaw=%.6f, pitch=%.6f, roll=%.6f (rad)\n", params.yaw, params.pitch, params.roll);
	Cout() << Format("  yaw=%.3f, pitch=%.3f, roll=%.3f (deg)\n",
		RAD2DEG(params.yaw), RAD2DEG(params.pitch), RAD2DEG(params.roll));

	// Pass/fail criteria
	double max_reproj_rms = max(diag.reproj_rms_l, diag.reproj_rms_r);
	double max_dist_rms = max(diag.dist_rms_l, diag.dist_rms_r);

	Cout() << "\n========================================\n";
	Cout() << "Quality Assessment:\n";
	Cout() << "========================================\n";

	bool pass = true;
	if (max_reproj_rms > 2.0) {
		Cout() << Format("FAIL: Reprojection RMS (%.3f px) exceeds threshold (2.0 px)\n", max_reproj_rms);
		pass = false;
	} else {
		Cout() << Format("PASS: Reprojection RMS (%.3f px) within threshold\n", max_reproj_rms);
	}

	if (max_dist_rms > 0.100) {  // 100mm = 0.1m
		Cout() << Format("FAIL: Distance RMS (%.3f mm) exceeds threshold (100 mm)\n", max_dist_rms * 1000.0);
		pass = false;
	} else {
		Cout() << Format("PASS: Distance RMS (%.3f mm) within threshold\n", max_dist_rms * 1000.0);
	}

	if (diag.behind_left > 0 || diag.behind_right > 0) {
		Cout() << Format("WARN: %d points behind camera\n", diag.behind_left + diag.behind_right);
	}

	if (!pass) {
		Cout() << "\n*** OVERALL: FAIL - Calibration quality insufficient ***\n";
		return 1;
	}

	Cout() << "\n*** OVERALL: PASS - Calibration acceptable ***\n";
	return 0;
}

END_UPP_NAMESPACE
