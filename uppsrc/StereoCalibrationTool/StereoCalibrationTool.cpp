#include "StereoCalibrationTool.h"
#include <plugin/Eigen/Eigen.h>

NAMESPACE_UPP

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper for ray-ray distance
static double RayRayDistance(const vec3& p1, const vec3& d1, const vec3& p2, const vec3& d2) {
	vec3 w0 = p1 - p2;
	double a = Dot(d1, d1);
	double b = Dot(d1, d2);
	double c = Dot(d2, d2);
	double d = Dot(d1, w0);
	double e = Dot(d2, w0);
	double denom = a * c - b * b;
	if (fabs(denom) < 1e-9) return (w0 - d1 * (float)(Dot(w0, d1) / a)).GetLength(); // Parallel
	double sc = (b * e - c * d) / denom;
	double tc = (a * e - b * d) / denom;
	return (w0 + d1 * (float)sc - d2 * (float)tc).GetLength();
}

struct CalibrationSolver {
	struct PointPair : Moveable<PointPair> {
		Pointf l, r;
		Size sz;
		double dist_l, dist_r;
	};
	Vector<PointPair> pairs;
	double eye_dist;

	int Solve(double& a, double& b, double& c, double& d, double& outward_angle) {
		int N = pairs.GetCount();
		if (N < 5) return 0;

		Eigen::VectorXd y(5 + 3 * N);
		y[0] = a; y[1] = b; y[2] = c; y[3] = d; y[4] = outward_angle;

		auto unproject_dir = [&](Pointf pix, Size sz, double cur_a, double cur_b, double cur_c, double cur_d, double cur_phi, int eye) -> vec3 {
			double cx = sz.cx / 2.0, cy = sz.cy / 2.0;
			double dx = pix.x * sz.cx - cx, dy = pix.y * sz.cy - cy;
			double r = sqrt(dx * dx + dy * dy);
			if (r < 1e-6) return vec3(0, 0, -1);
			
			// Newton to find theta
			double theta = r / cur_a;
			for (int it = 0; it < 5; it++) {
				double t2 = theta * theta;
				double t3 = t2 * theta;
				double t4 = t3 * theta;
				double f = cur_a * theta + cur_b * t2 + cur_c * t3 + cur_d * t4 - r;
				double df = cur_a + 2 * cur_b * theta + 3 * cur_c * t2 + 4 * cur_d * t3;
				if (fabs(df) < 1e-9) break;
				theta -= f / df;
			}
			
			double roll = -atan2(dx, dy);
			vec3 dir = AxesDirRoll((float)theta, (float)roll);
			axes2 axes = GetDirAxes(dir).Splice();
			if (eye == 0) axes[0] += (float)cur_phi; else axes[0] -= (float)cur_phi;
			return GetAxesDir(axes);
		};

		// 1. Initial guess for 3D points using current rays
		for(int i = 0; i < N; i++) {
			const auto& p = pairs[i];
			vec3 pL = vec3(-(float)eye_dist / 2.0f, 0, 0), dL = unproject_dir(p.l, p.sz, a, b, c, d, outward_angle, 0);
			vec3 pR = vec3((float)eye_dist / 2.0f, 0, 0), dR = unproject_dir(p.r, p.sz, a, b, c, d, outward_angle, 1);
			
			vec3 w0 = pL - pR;
			double a_dot = Dot(dL, dL), b_dot = Dot(dL, dR), c_dot = Dot(dR, dR);
			double d_dot = Dot(dL, w0), e_dot = Dot(dR, w0);
			double denom = a_dot * c_dot - b_dot * b_dot;
			vec3 pt;
			if (fabs(denom) > 1e-9) {
				double sc = (b_dot * e_dot - c_dot * d_dot) / denom;
				double tc = (a_dot * e_dot - b_dot * d_dot) / denom;
				pt = (pL + dL * (float)sc + pR + dR * (float)tc) * 0.5f;
			} else {
				pt = (pL + pR) * 0.5f + dL * 1000.0f; // 1m away
			}
			y[5 + i*3 + 0] = pt[0];
			y[5 + i*3 + 1] = pt[1];
			y[5 + i*3 + 2] = pt[2];
		}

		// 2. Optimization loop
		auto residual = [&](const Eigen::VectorXd& x, Eigen::VectorXd& res) {
			double cur_a = x[0], cur_b = x[1], cur_c = x[2], cur_d = x[3], cur_phi = x[4];
			res.resize(N * 6);
			
			auto project = [&](vec3 P, int eye) -> Pointf {
				vec3 localP = P;
				if (eye == 0) localP[0] += (float)eye_dist / 2.0f;
				else localP[0] -= (float)eye_dist / 2.0f;
				
				vec3 dir = localP.GetNormalized();
				axes2 axes = GetDirAxes(dir).Splice();
				if (eye == 0) axes[0] -= (float)cur_phi;
				else axes[0] += (float)cur_phi;
				
				axes2 roll_axes = GetDirAxesRoll(GetAxesDir(axes));
				double theta = roll_axes.data[0];
				double roll = roll_axes.data[1];
				double r = cur_a * theta + cur_b * theta*theta + cur_c * theta*theta*theta + cur_d * theta*theta*theta*theta;
				double dx = r * -sin(roll);
				double dy = r * cos(roll);
				
				const auto& pp = pairs[0];
				return Pointf((dx + pp.sz.cx/2.0) / pp.sz.cx, (dy + pp.sz.cy/2.0) / pp.sz.cy);
			};

			for (int i = 0; i < N; i++) {
				const auto& p = pairs[i];
				vec3 X(x[5 + i*3 + 0], x[5 + i*3 + 1], x[5 + i*3 + 2]);
				
				Pointf uvL = project(X, 0);
				Pointf uvR = project(X, 1);
				
				res[i*6 + 0] = (uvL.x - p.l.x) * p.sz.cx;
				res[i*6 + 1] = (uvL.y - p.l.y) * p.sz.cy;
				res[i*6 + 2] = (uvR.x - p.r.x) * p.sz.cx;
				res[i*6 + 3] = (uvR.y - p.r.y) * p.sz.cy;
				
				vec3 pL = vec3(-(float)eye_dist / 2.0f, 0, 0);
				vec3 pR = vec3((float)eye_dist / 2.0f, 0, 0);
				double w_range = 0.02;
				if (p.dist_l > 0) res[i*6 + 4] = ((X - pL).GetLength() - p.dist_l) * w_range; else res[i*6 + 4] = 0;
				if (p.dist_r > 0) res[i*6 + 5] = ((X - pR).GetLength() - p.dist_r) * w_range; else res[i*6 + 5] = 0;
			}
			return 0;
		};

		if (NonLinearOptimization(y, N * 6, residual)) {
			a = y[0]; b = y[1]; c = y[2]; d = y[3]; outward_angle = y[4];
			return 1;
		}
		return 0;
	}
};

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

bool StereoCalibrationTool::HmdStereoSource::Start() {
	if (running)
		return true;
	if (!sys.Initialise())
		return false;
	cam.Create();
	if (!cam->Open()) {
		cam.Clear();
		sys.Uninitialise();
		return false;
	}
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
					preview.SetImages(left_img, right_img);
					preview.SetOverlay("Live view");
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
	CalibrationSolver solver;
	solver.eye_dist = (double)calib_eye_dist;
	
	for(const auto& f : captured_frames) {
		Size sz = f.left_img.GetSize();
		if (sz.cx <= 0 || sz.cy <= 0) continue;
		for(const auto& m : f.matches) {
			auto& p = solver.pairs.Add();
			p.l = m.left;
			p.r = m.right;
			p.sz = sz;
			p.dist_l = m.dist_l;
			p.dist_r = m.dist_r;
		}
	}
	
	if (solver.pairs.GetCount() < 5) {
		PromptOK("At least 5 match pairs across all frames are required to solve for 5 parameters.");
		return;
	}
	
	double a = calib_poly_a, b = calib_poly_b, c = calib_poly_c, d = calib_poly_d;
	double phi = calib_outward_angle;
	
	// Initial guesses if they are zero
	if (a == 0) a = 2.0 * solver.pairs[0].sz.cx / M_PI; 
	
	status.Set("Solving calibration...");
	if (solver.Solve(a, b, c, d, phi)) {
		calib_poly_a <<= a;
		calib_poly_b <<= b;
		calib_poly_c <<= c;
		calib_poly_d <<= d;
		calib_outward_angle <<= phi;
		
		SyncCalibrationFromEdits();
		
		String report;
		report << "Solver converged successfully.\n";
		report << "Matches: " << solver.pairs.GetCount() << "\n";
		report << "Final parameters:\n";
		report << "  a: " << a << "\n";
		report << "  b: " << b << "\n";
		report << "  c: " << c << "\n";
		report << "  d: " << d << "\n";
		report << "  phi: " << phi << " rad (" << phi * 180 / M_PI << " deg)\n";
		
		report_text <<= report;
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
	sep_diag.SetLabel("Diagnostics");
	mode_lbl.SetLabel("Match mode");
	mode_list.Add(0, "Point pairs");
	mode_list.Add(1, "Line pairs");
	mode_list.SetIndex(0);

	export_calibration.SetLabel("Export .stcal");
	export_calibration <<= THISBACK(ExportCalibration);
	load_calibration.SetLabel("Load .stcal");
	load_calibration <<= THISBACK(LoadCalibration);
	calib_enabled_lbl.SetLabel("Enabled");
	calib_enabled.WhenAction = THISBACK(SyncCalibrationFromEdits);
	calib_eye_lbl.SetLabel("Eye dist (mm)");
	calib_eye_dist.WhenAction = THISBACK(SyncCalibrationFromEdits);
	calib_outward_lbl.SetLabel("Outward angle");
	calib_outward_angle.WhenAction = THISBACK(SyncCalibrationFromEdits);
	calib_poly_lbl.SetLabel("Angle poly");
	calib_poly_a.WhenAction = THISBACK(SyncCalibrationFromEdits);
	calib_poly_b.WhenAction = THISBACK(SyncCalibrationFromEdits);
	calib_poly_c.WhenAction = THISBACK(SyncCalibrationFromEdits);
	calib_poly_d.WhenAction = THISBACK(SyncCalibrationFromEdits);

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
	y += 32;

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

	captures_split.Horz(captures_list, matches_list);
	captures_split.SetPos(4000);

	bottom_tabs.Add(captures_split.SizePos(), "Captured Frames");
	bottom_tabs.Add(report_text.SizePos(), "Report");
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
	
	const CapturedFrame& frame = captured_frames[row];
	matches_list.Clear();
	for (const MatchPair& pair : frame.matches)
		matches_list.Add(pair.left_text, pair.right_text, pair.dist_l, pair.dist_r);
	preview.SetImages(frame.left_img, frame.right_img);
	preview.SetMatches(frame.matches);
	String time = AsString(captures_list.Get(row, 0));
	String source = AsString(captures_list.Get(row, 1));
	int samples = frame.matches.GetCount();
	captures_list.Set(row, 2, samples);
	preview.SetOverlay(Format("Capture %s (%s), samples %d", time, source, samples));
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
	if (idx < 0 || idx >= sources.GetCount()) return;
	String name = AsString(source_list.GetValue());
	if (verbose) Cout() << "StartSource: Starting " << name << "\n";
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
	if (idx < 0 || idx >= sources.GetCount()) return;
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
	s << "  angle_poly=" << last_calibration.angle_to_pixel[0] << ", "
	  << 	last_calibration.angle_to_pixel[1] << ", "
	  << 	last_calibration.angle_to_pixel[2] << ", "
	  << 	last_calibration.angle_to_pixel[3] << "\n";
	calibration_preview.SetLabel(s);
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
