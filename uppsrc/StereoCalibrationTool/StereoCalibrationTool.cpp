#include "StereoCalibrationTool.h"

NAMESPACE_UPP

/*
StereoCalibrationTool.cpp
=========================
Purpose:
- Shared helpers and non-UI controller glue for the StereoCalibrationTool.
- Implements shared camera source backends.

Key classes:
- HmdStereoSource / UsbStereoSource: stereo camera backends.

Data flow:
- Persistence helpers read/write project.json + calibration.stcal.
*/

// ------------------------------------------------------------
// StereoRectificationCache implementation
// ------------------------------------------------------------

bool StereoRectificationCache::IsValid(const cv::Mat& k1, const cv::Mat& d1,
                                        const cv::Mat& k2, const cv::Mat& d2,
                                        const cv::Mat& r, const cv::Mat& t,
                                        const cv::Size& sz, double a) const {
	if (!valid) return false;
	if (image_size != sz) return false;
	if (fabs(alpha - a) > 1e-9) return false;

	// Compare matrices using cv::norm (L2 norm)
	auto mat_eq = [](const cv::Mat& m1, const cv::Mat& m2) -> bool {
		if (m1.size() != m2.size()) return false;
		if (m1.type() != m2.type()) return false;
		double diff = cv::norm(m1, m2, cv::NORM_L2);
		return diff < 1e-6;
	};

	return mat_eq(K1, k1) && mat_eq(D1, d1) &&
	       mat_eq(K2, k2) && mat_eq(D2, d2) &&
	       mat_eq(R, r) && mat_eq(T, t);
}

namespace StereoCalibrationHelpers {

String GetCalibrationStateText(int state) {
	switch(state) {
		case CALIB_RAW: return "RAW";
		case CALIB_MANUAL: return "CALIB (MANUAL)";
		case CALIB_SOLVED: return "SOLVED";
		default: return "UNKNOWN";
	}
}

// ------------------------------------------------------------
// Image conversion helpers
// ------------------------------------------------------------

bool SplitStereoImage(const Image& src, Image& left, Image& right) {
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

Image CopyFrameImage(const VisualFrame& frame) {
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

bool IsFrameNonBlack(const Image& img) {
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

Image ConvertRgb24ToImage(const byte* data, int width, int height) {
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

Image ConvertYuyvToImage(const byte* data, int width, int height) {
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

Image ConvertMjpegToImage(const byte* data, int bytes) {
	if (!data || bytes <= 0)
		return Image();
	String s((const char*)data, bytes);
	return StreamRaster::LoadStringAny(s);
}

// ------------------------------------------------------------
// Undistort helpers
// ------------------------------------------------------------

RGBA SampleBilinear(const Image& img, float x, float y) {
	Size sz = img.GetSize();
	if (sz.cx <= 0 || sz.cy <= 0 || !std::isfinite(x) || !std::isfinite(y))
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

Image UndistortImage(const Image& src, const LensPoly& lens, float linear_scale) {
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
			float r_out = sqrtf(dx * dx + dy * dy);
			if (r_out < 1e-6f) {
				dst[x] = SampleBilinear(src, cx, cy);
				continue;
			}
			float theta = atanf(r_out / linear_scale);
			float r_src = lens.AngleToPixel(theta);
			if (!isfinite(r_src) || r_src < 0) {
				dst[x] = RGBA{0,0,0,255};
				continue;
			}
			float scale = r_src / r_out;
			float sx = cx + dx * scale;
			float sy = cy + dy * scale;
			dst[x] = SampleBilinear(src, sx, sy);
		}
	}
	return out;
}

Image DistortImage(const Image& src, const LensPoly& lens, float linear_scale) {
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
			float r_out = sqrtf(dx * dx + dy * dy);
			if (r_out < 1e-6f) {
				dst[x] = SampleBilinear(src, cx, cy);
				continue;
			}
			float theta = lens.PixelToAngle(r_out);
			if (!isfinite(theta) || theta < 0) {
				dst[x] = RGBA{0,0,0,255};
				continue;
			}
			float r_src = linear_scale * tanf(theta);
			if (!isfinite(r_src) || r_src < 0) {
				dst[x] = RGBA{0,0,0,255};
				continue;
			}
			float scale = r_src / r_out;
			float sx = cx + dx * scale;
			float sy = cy + dy * scale;
			dst[x] = SampleBilinear(src, sx, sy);
		}
	}
	return out;
}

Image ApplyExtrinsicsOnly(const Image& src, float yaw, float pitch, float roll, const vec2& pp) {
	if (src.IsEmpty())
		return Image();
	if (fabs(yaw) < 1e-6f && fabs(pitch) < 1e-6f && fabs(roll) < 1e-6f)
		return src;

	Size sz = src.GetSize();
	ImageBuffer out(sz);
	float cx = pp[0] > 0 ? pp[0] : sz.cx * 0.5f;
	float cy = pp[1] > 0 ? pp[1] : sz.cy * 0.5f;
	mat4 rot = AxesMat(yaw, pitch, roll);
	mat4 rot_inv = rot.GetTransposed();
	float f = cx; 

	for (int y = 0; y < sz.cy; y++) {
		RGBA* dst = out[y];
		float dy = y - cy;
		for (int x = 0; x < sz.cx; x++) {
			float dx = x - cx;
			vec3 dir_out(dx, -dy, IS_NEGATIVE_Z ? -f : f);
			dir_out.Normalize();
			vec3 dir_src = (rot_inv * dir_out.Embed()).Splice();
			dir_src.Normalize();
			float zf = IS_NEGATIVE_Z ? -dir_src[2] : dir_src[2];
			if (zf <= 1e-3f) {
				dst[x] = RGBA{0,0,0,255};
				continue;
			}
			float scale = f / zf;
			float sx = cx + dir_src[0] * scale;
			float sy = cy - dir_src[1] * scale;
			dst[x] = SampleBilinear(src, sx, sy);
		}
	}
	return out;
}

Image ApplyIntrinsicsOnly(const Image& src, const LensPoly& lens, float linear_scale, bool undistort) {
	if (undistort)
		return UndistortImage(src, lens, linear_scale);
	else
		return DistortImage(src, lens, linear_scale);
}

Image RectifyAndRotateOnePass(const Image& src, const LensParams& lp, float yaw, float pitch, float roll, Size out_sz) {
	if (src.IsEmpty() || out_sz.cx <= 0 || out_sz.cy <= 0 || lp.f <= 1e-6f)
		return Image();

	ImageBuffer out(out_sz);
	float cx_out = out_sz.cx * 0.5f;
	float cy_out = out_sz.cy * 0.5f;
	mat4 rot = AxesMat(yaw, pitch, roll);
	mat4 rot_inv = rot.GetTransposed();

	for (int y = 0; y < out_sz.cy; y++) {
		RGBA* dst = out[y];
		float dy_out = y - cy_out;
		for (int x = 0; x < out_sz.cx; x++) {
			float dx_out = x - cx_out;
			vec3 dir_out(dx_out, -dy_out, IS_NEGATIVE_Z ? -lp.f : lp.f);
			dir_out.Normalize();
			vec3 dir_cam = (rot_inv * dir_out.Embed()).Splice();
			dir_cam.Normalize();
			float zf = IS_NEGATIVE_Z ? -dir_cam[2] : dir_cam[2];
			if (zf <= 1e-3f) {
				dst[x] = RGBA{0,0,0,255};
				continue;
			}
			float nx = dir_cam[0] / zf;
			float ny = -dir_cam[1] / zf;
			float r2 = nx*nx + ny*ny;
			float dist_scale = 1.0f + lp.k1 * r2 + lp.k2 * r2 * r2;
			if (dist_scale < 0.1f) dist_scale = 0.1f;
			if (dist_scale > 10.0f) dist_scale = 10.0f;
			float sx = lp.cx + nx * dist_scale * lp.f;
			float sy = lp.cy + ny * dist_scale * lp.f;
			dst[x] = SampleBilinear(src, sx, sy);
		}
	}
	return out;
}

Pointf ProjectPointOnePass(Pointf src_norm, Size src_sz, const LensParams& lp, float yaw, float pitch, float roll) {
	if (src_sz.cx <= 0 || src_sz.cy <= 0 || lp.f <= 1e-6f)
		return Pointf(-1, -1);

	// 1. Center the source point (relative to principal point)
	float sx = (float)(src_norm.x * src_sz.cx);
	float sy = (float)(src_norm.y * src_sz.cy);
	
	// Normalize to "distorted radius" space (relative to focal length f)
	float dx = (sx - lp.cx) / lp.f;
	float dy = (sy - lp.cy) / lp.f;
	double rd = sqrt(dx*dx + dy*dy);
	
	// 2. Solve for undistorted radius r_u
	// Model: r_d = r_u(1 + k1*r_u^2 + k2*r_u^4)
	// f(r_u) = r_u + k1*r_u^3 + k2*r_u^5 - r_d = 0
	// f'(r_u) = 1 + 3*k1*r_u^2 + 5*k2*r_u^4
	double ru = rd; // Initial guess
	for(int i=0; i<10; i++) {
		double r2 = ru*ru;
		double val = ru * (1.0 + lp.k1*r2 + lp.k2*r2*r2) - rd;
		double deriv = 1.0 + 3.0*lp.k1*r2 + 5.0*lp.k2*r2*r2;
		if (fabs(deriv) < 1e-6) break;
		double step = val / deriv;
		ru -= step;
		if (fabs(step) < 1e-7) break;
	}
	
	// 3. Recover camera ray (before rotation)
	// Direction vector (nx, ny, 1). We know direction is same as (dx, dy).
	double scale = (rd > 1e-9) ? (ru / rd) : 1.0;
	vec3 dir_cam((float)(dx * scale), (float)(dy * scale), (float)(IS_NEGATIVE_Z ? -1.0 : 1.0));
	dir_cam.Normalize();
	
	// 4. Apply forward rotation (Cam -> Rect)
	mat4 rot = AxesMat(yaw, pitch, roll);
	vec3 dir_rect = (rot * dir_cam.Embed()).Splice();
	
	// 5. Project to rectilinear pixels
	float zf = IS_NEGATIVE_Z ? -dir_rect[2] : dir_rect[2];
	if (zf <= 1e-3f) return Pointf(-1, -1);
	
	float px = dir_rect[0] / zf * lp.f + lp.cx; // Assuming output cx/cy matches input
	float py = -dir_rect[1] / zf * lp.f + lp.cy;
	
	// Return as un-normalized pixels
	return Pointf(px, py);
}

// Inverse of ProjectPointOnePass: Output(Rectified, Linear) -> Input(Source, Distorted)
Pointf UnprojectPointOnePass(Pointf rect_px, Size rect_sz, const LensParams& lp, float yaw, float pitch, float roll) {
	if (rect_sz.cx <= 0 || rect_sz.cy <= 0 || lp.f <= 1e-6f)
		return Pointf(-1, -1);

	float cx_out = rect_sz.cx * 0.5f;
	float cy_out = rect_sz.cy * 0.5f;
	float dx_out = (float)rect_px.x - cx_out;
	float dy_out = (float)rect_px.y - cy_out;

	// 1. Ray direction in output (rectilinear) space
	vec3 dir_out(dx_out, -dy_out, IS_NEGATIVE_Z ? -lp.f : lp.f);
	dir_out.Normalize();

	// 2. Apply inverse rotation to get ray in camera (distorted) space
	mat4 rot = AxesMat(yaw, pitch, roll);
	mat4 rot_inv = rot.GetTransposed();
	vec3 dir_cam = (rot_inv * dir_out.Embed()).Splice();
	dir_cam.Normalize();

	// 3. Project to normalized camera plane (z=1)
	float zf = IS_NEGATIVE_Z ? -dir_cam[2] : dir_cam[2];
	if (zf <= 1e-3f) return Pointf(-1, -1);
	
	float nx = dir_cam[0] / zf;
	float ny = -dir_cam[1] / zf;

	// 4. Apply forward distortion model
	float r2 = nx*nx + ny*ny;
	float dist_scale = 1.0f + lp.k1 * r2 + lp.k2 * r2 * r2;

	// 5. Map to input pixel coordinates
	float sx = lp.cx + nx * dist_scale * lp.f;
	float sy = lp.cy + ny * dist_scale * lp.f;

	return Pointf(sx, sy);
}

// ------------------------------------------------------------
// Persistence helpers
// ------------------------------------------------------------

String GetPersistPath(const AppModel& model) {
	return AppendFileName(model.project_dir, "calibration.stcal");
}

String GetStatePath(const AppModel& model) {
	return AppendFileName(model.project_dir, "project.json");
}

String GetReportPath(const AppModel& model) {
	return AppendFileName(model.project_dir, "report.txt");
}

void LoadLastCalibration(AppModel& model) {
	StereoCalibrationData data;
	String path = GetPersistPath(model);
	if (!model.project_dir.IsEmpty() && FileExists(path) && HMD::StereoTracker::LoadCalibrationFile(path, data))
		model.last_calibration = data;
}

void SaveLastCalibration(AppModel& model) {
	if (model.project_dir.IsEmpty())
		return;
	HMD::StereoTracker::SaveCalibrationFile(GetPersistPath(model), model.last_calibration);
}

void LoadState(AppModel& model) {
	if (model.project_dir.IsEmpty())
		return;
	String path = GetStatePath(model);
	if (!FileExists(path))
		return;
	String json = LoadFile(path);
	if (json.IsEmpty())
		return;
	Value v = ParseJSON(json);
	LoadFromJsonValue(model.captured_frames, v["frames"]);
	LoadFromJsonValue(model.project_state, v["state"]);
	model.project_state.schema_version = 1;
	String dir = AppendFileName(model.project_dir, "captures");
	for (int i = 0; i < model.captured_frames.GetCount(); i++) {
		auto& f = model.captured_frames[i];
		f.left_img = StreamRaster::LoadFileAny(AppendFileName(dir, Format("frame_%d_l.png", i)));
		f.right_img = StreamRaster::LoadFileAny(AppendFileName(dir, Format("frame_%d_r.png", i)));
	}
}

void SaveState(const AppModel& model) {
	if (model.project_dir.IsEmpty())
		return;
	Json json;
	ProjectState state = model.project_state;
	state.schema_version = 1;
	json("frames", StoreAsJsonValue(model.captured_frames));
	json("state", StoreAsJsonValue(state));
	SaveFile(GetStatePath(model), json);
	String dir = AppendFileName(model.project_dir, "captures");
	RealizeDirectory(dir);
	for (int i = 0; i < model.captured_frames.GetCount(); i++) {
		const auto& f = model.captured_frames[i];
		PNGEncoder().SaveFile(AppendFileName(dir, Format("frame_%d_l.png", i)), f.left_img);
		PNGEncoder().SaveFile(AppendFileName(dir, Format("frame_%d_r.png", i)), f.right_img);
	}
}

void ShowInstructions() {
	String text;
	text << "Stereo Calibration Instructions (Unified Workflow)\n";
	text << "==================================================\n\n";
	text << "1. Print the Calibration Board:\n";
	text << "   - In the left pane (Camera), click 'Generate Board...'.\n";
	text << "   - Specifying 9x6 squares results in 8x5 inner corners.\n";
	text << "   - Print the generated PNG at 100% scale (no fit-to-page).\n\n";
	text << "2. Capture Calibration Frames:\n";
	text << "   - Select your camera source and click 'Start'.\n";
	text << "   - Hold the board in front of the camera in various positions.\n";
	text << "   - Ensure the board is visible in BOTH eyes.\n";
	text << "   - Click 'Capture' to save a frame. Aim for 20-40 frames covering the full sensor area.\n\n";
	text << "3. Detect and Solve:\n";
	text << "   - In the right tab (Calibration), go to 'Board' tab.\n";
	text << "   - Click 'Detect Corners'. The frame list will update with detection results.\n";
	text << "   - Go to 'Solve' tab.\n";
	text << "   - Click 'Solve Intrinsics' to find lens focal length and distortion.\n";
	text << "   - Click 'Solve Stereo' to find camera baseline and relative rotation.\n\n";
	text << "4. Verify Results:\n";
	text << "   - Use 'Preview intrinsics' and 'Preview extrinsics' toggles to see rectification.\n";
	text << "   - Use 'Rectified Overlay' to verify epipolar alignment (lines should match horizontally).\n";
	text << "   - Check the 'Report' tab for RMS errors (lower is better, < 1.0 px is good).\n";

	TopWindow dlg;
	dlg.Title("Calibration Instructions");
	dlg.SetRect(0, 0, 500, 600);
	DocEdit edit;
	edit.SetReadOnly();
	edit <<= text;
	dlg.Add(edit.SizePos());
	dlg.RunAppModal();
}

} // namespace StereoCalibrationHelpers

// ------------------------------------------------------------
// Stereo sources (HMD / USB)
// ------------------------------------------------------------

bool HmdStereoSource::Start() {
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
	quit = false;
	background_thread.Start(THISBACK(BackgroundProcess));
	running = true;
	return true;
}

void HmdStereoSource::Stop() {
	if (running) {
		quit = true;
		background_thread.Wait();
	}
	if (cam)
		cam->Close();
	cam.Clear();
	if (running)
		sys.Uninitialise();
	running = false;
	last_left = Image();
	last_right = Image();
	{
		Mutex::Lock __(mutex);
		bg_left_bright = Image();
		bg_right_bright = Image();
		bg_left_dark = Image();
		bg_right_dark = Image();
		bg_serial_bright = -1;
		bg_serial_dark = -1;
	}
}

void HmdStereoSource::BackgroundProcess() {
	while (!quit) {
		sys.UpdateData();
		if (cam && cam->IsOpen()) {
			Vector<HMD::CameraFrame> frames;
			cam->PopFrames(frames);
			if (frames.IsEmpty()) {
				Sleep(1);
				continue;
			}
			for (const auto& f : frames) {
				Image L, R;
				if (StereoCalibrationHelpers::SplitStereoImage(f.img, L, R)) {
					Mutex::Lock __(mutex);
					if (f.is_bright) {
						bg_left_bright = L;
						bg_right_bright = R;
						bg_serial_bright = f.serial;
					} else {
						bg_left_dark = L;
						bg_right_dark = R;
						bg_serial_dark = f.serial;
					}
				}
			}
		}
		else {
			Sleep(10);
		}
	}
}

bool HmdStereoSource::ReadFrame(VisualFrame& left, VisualFrame& right, bool prefer_bright) {
	if (!running)
		return false;

	Image L, R;
	bool is_bright;
	int64 serial;

	{
		Mutex::Lock __(mutex);
		
		bool has_new_bright = (bg_serial_bright > last_serial);
		bool has_new_dark = (bg_serial_dark > last_serial);
		
		if (prefer_bright) {
			if (has_new_bright) {
				L = bg_left_bright; R = bg_right_bright;
				is_bright = true; serial = bg_serial_bright;
			} else if (has_new_dark) {
				L = bg_left_dark; R = bg_right_dark;
				is_bright = false; serial = bg_serial_dark;
			} else return false;
		} else {
			if (bg_serial_bright > bg_serial_dark) {
				if (has_new_bright) {
					L = bg_left_bright; R = bg_right_bright;
					is_bright = true; serial = bg_serial_bright;
				} else return false;
			} else {
				if (has_new_dark) {
					L = bg_left_dark; R = bg_right_dark;
					is_bright = false; serial = bg_serial_dark;
				} else return false;
			}
		}
		
		last_left = L;
		last_right = R;
		last_is_bright = is_bright;
		last_serial = serial;
	}

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
	left.serial = last_serial;

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
	right.serial = last_serial;

	return true;
}

bool HmdStereoSource::PeakFrame(VisualFrame& left, VisualFrame& right, bool prefer_bright) {
	if (!running)
		return false;

	Image L, R;
	bool is_bright;
	int64 serial;
	{
		Mutex::Lock __(mutex);
		if (prefer_bright && !bg_left_bright.IsEmpty()) {
			L = bg_left_bright; R = bg_right_bright;
			is_bright = true; serial = bg_serial_bright;
		} else if (!bg_left_dark.IsEmpty() && bg_serial_dark > bg_serial_bright) {
			L = bg_left_dark; R = bg_right_dark;
			is_bright = false; serial = bg_serial_dark;
		} else if (!bg_left_bright.IsEmpty()) {
			L = bg_left_bright; R = bg_right_bright;
			is_bright = true; serial = bg_serial_bright;
		} else return false;
	}

	left.timestamp_us = usecs();
	left.format = GEOM_EVENT_CAM_RGBA8;
	left.width = L.GetWidth();
	left.height = L.GetHeight();
	left.stride = left.width * (int)sizeof(RGBA);
	left.eye = 0;
	left.data = (const byte*)~L;
	left.data_bytes = L.GetLength() * (int)sizeof(RGBA);
	left.img = L;
	left.flags = is_bright ? VIS_FRAME_BRIGHT : VIS_FRAME_DARK;
	left.serial = serial;

	right.timestamp_us = left.timestamp_us;
	right.format = GEOM_EVENT_CAM_RGBA8;
	right.width = R.GetWidth();
	right.height = R.GetHeight();
	right.stride = right.width * (int)sizeof(RGBA);
	right.eye = 1;
	right.data = (const byte*)~R;
	right.data_bytes = R.GetLength() * (int)sizeof(RGBA);
	right.img = R;
	right.flags = left.flags;
	right.serial = serial;

	return true;
}

#ifdef flagLINUX
bool UsbStereoSource::Start() {
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
	width = capture->getWidth();
	height = capture->getHeight();
	pixfmt = capture->getFormat();
	running = true;
	return true;
}

void UsbStereoSource::Stop() {
	if (!running)
		return;
	capture.Clear();
	running = false;
}

bool UsbStereoSource::ReadFrame(VisualFrame& left, VisualFrame& right, bool prefer_bright) {
	if (!running || !capture)
		return false;
	unsigned int buffer_size = capture->getBufferSize();
	if (buffer_size == 0)
		return false;
	raw.SetCount(buffer_size);
	size_t bytes = capture->read((char*)raw.Begin(), raw.GetCount());
	if (bytes <= 0)
		return false;
	const byte* data = raw.Begin();
	Image img;
	if (pixfmt == V4L2_PIX_FMT_RGB24)
		img = StereoCalibrationHelpers::ConvertRgb24ToImage(data, width, height);
	else if (pixfmt == V4L2_PIX_FMT_YUYV)
		img = StereoCalibrationHelpers::ConvertYuyvToImage(data, width, height);
	else if (pixfmt == V4L2_PIX_FMT_MJPEG)
		img = StereoCalibrationHelpers::ConvertMjpegToImage(data, (int)bytes);
	if (img.IsEmpty())
		return false;
	return StereoCalibrationHelpers::SplitStereoImage(img, left.img, right.img);
}

bool UsbStereoSource::PeakFrame(VisualFrame& left, VisualFrame& right, bool prefer_bright) {
	return ReadFrame(left, right, prefer_bright);
}
#else
bool UsbStereoSource::Start() { return false; }
void UsbStereoSource::Stop() { running = false; }
bool UsbStereoSource::ReadFrame(VisualFrame&, VisualFrame&, bool) { return false; }
bool UsbStereoSource::PeakFrame(VisualFrame&, VisualFrame&, bool) { return false; }
#endif

END_UPP_NAMESPACE