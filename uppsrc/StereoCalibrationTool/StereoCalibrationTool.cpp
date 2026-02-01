#include "StereoCalibrationTool.h"

NAMESPACE_UPP

/*
StereoCalibrationTool.cpp
=========================
Purpose:
- Shared helpers and non-UI controller glue for the StereoCalibrationTool.
- Implements PreviewCtrl rendering and shared camera source backends.

Key classes:
- StereoCalibrationTool: legacy controller (currently disabled).
- PreviewCtrl: shared preview widget.
- HmdStereoSource / UsbStereoSource: stereo camera backends.

Data flow:
- AppModel is owned by the controller and passed by reference into windows.
- Persistence helpers read/write project.json + calibration.stcal.

Gotchas / invariants:
- Helper functions must remain UI-agnostic.
- Controller never mutates UI directly; it only delegates to windows.
*/

namespace StereoCalibrationHelpers {

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

// Undistorts an image: removes barrel distortion from source, producing rectilinear output.
// Source has barrel/pincushion distortion (from real camera), output is linear.
// For each LINEAR output pixel, find the DISTORTED source pixel.
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
			// Output pixel at radius r_out (linear) corresponds to angle theta
			float theta = r_out / linear_scale;
			// In distorted source, that angle is at radius r_src
			float r_src = lens.AngleToPixel(theta);
			// Guard against invalid lens calculations producing NaN/inf
			if (!isfinite(r_src) || r_src < 0) {
				dst[x] = src[y][x];  // Fallback: copy original pixel
				continue;
			}
			float scale = r_src / r_out;
			float sx = cx + dx * scale;
			float sy = cy + dy * scale;
			// Validate coordinates before sampling
			if (!isfinite(sx) || !isfinite(sy)) {
				dst[x] = src[y][x];  // Fallback: copy original pixel
				continue;
			}
			dst[x] = SampleBilinear(src, sx, sy);
		}
	}
	return out;
}

// Distorts an image: adds barrel distortion to source, producing distorted output.
// Source is linear/rectilinear, output has barrel/pincushion distortion.
// For each DISTORTED output pixel, find the LINEAR source pixel.
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
			// Output pixel at radius r_out (distorted) corresponds to angle theta
			float theta = lens.PixelToAngle(r_out);
			// Guard against invalid lens calculations producing NaN/inf
			if (!isfinite(theta) || theta < 0) {
				dst[x] = src[y][x];  // Fallback: copy original pixel
				continue;
			}
			// In linear source, that angle is at radius r_src
			float r_src = theta * linear_scale;
			float scale = r_src / r_out;
			float sx = cx + dx * scale;
			float sy = cy + dy * scale;
			// Validate coordinates before sampling
			if (!isfinite(sx) || !isfinite(sy)) {
				dst[x] = src[y][x];  // Fallback: copy original pixel
				continue;
			}
			dst[x] = SampleBilinear(src, sx, sy);
		}
	}
	return out;
}

// Applies ONLY extrinsics (yaw/pitch/roll rotation) without any lens distortion.
// This performs a 3D rotation of the camera rays and resamples the source image.
Image ApplyExtrinsicsOnly(const Image& src, float yaw, float pitch, float roll, const vec2& pp) {
	if (src.IsEmpty())
		return Image();

	// Fast-path: if all rotations are zero, return copy of source
	if (fabs(yaw) < 1e-6f && fabs(pitch) < 1e-6f && fabs(roll) < 1e-6f)
		return clone(src);

	Size sz = src.GetSize();
	ImageBuffer out(sz);
	float cx = pp[0] > 0 ? pp[0] : sz.cx * 0.5f;
	float cy = pp[1] > 0 ? pp[1] : sz.cy * 0.5f;

	// Build rotation matrix (matches LensPoly::Project/Unproject for lens_i==1)
	mat4 rot = AxesMat(yaw, pitch, roll);
	mat4 rot_inv = rot.GetTransposed(); // Inverse rotation for unprojecting

	for (int y = 0; y < sz.cy; y++) {
		RGBA* dst = out[y];
		float dy = y - cy;
		for (int x = 0; x < sz.cx; x++) {
			float dx = x - cx;

			// Compute output ray direction (pinhole projection, no distortion)
			float r = sqrtf(dx * dx + dy * dy);
			float theta = atanf(r / cx); // Approximate FOV based on image width
			float roll_angle = atan2f(dy, dx);

			float sin_theta = sinf(theta);
			float cos_theta = cosf(theta);
			vec3 dir_out(
				sin_theta * cosf(roll_angle),
				-sin_theta * sinf(roll_angle),
				IS_NEGATIVE_Z ? -cos_theta : cos_theta);

			// Apply inverse rotation to get source ray
			vec3 dir_src = (rot_inv * dir_out.Embed()).Splice();
			dir_src.Normalize();

			// Project source ray back to source image
			float zf = IS_NEGATIVE_Z ? -dir_src[2] : dir_src[2];
			zf = zf < -1.0f ? -1.0f : (zf > 1.0f ? 1.0f : zf);
			float theta_src = atan2f(sqrtf(dir_src[0] * dir_src[0] + dir_src[1] * dir_src[1]), zf);
			float roll_src = atan2f(-dir_src[1], dir_src[0]);

			float r_src = cx * tanf(theta_src);
			float sx = cx + r_src * cosf(roll_src);
			float sy = cy + r_src * sinf(roll_src);

			dst[x] = SampleBilinear(src, sx, sy);
		}
	}
	return out;
}

// Applies ONLY intrinsics (lens undistortion or distortion) without rotation.
// undistort=true: remove distortion (for viewing raw camera images)
// undistort=false: add distortion (for synthetic rendering)
Image ApplyIntrinsicsOnly(const Image& src, const LensPoly& lens, float linear_scale, bool undistort) {
	if (undistort)
		return UndistortImage(src, lens, linear_scale);
	else
		return DistortImage(src, lens, linear_scale);
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
	Value state_val = v["state"];
	int ver = state_val["schema_version"];

	LoadFromJsonValue(model.captured_frames, v["frames"]);
	LoadFromJsonValue(model.project_state, v["state"]);

	if (ver < 1) {
		if (model.project_state.view_mode == 0 && !IsNull(state_val["stage_a_undistort"]) && (bool)state_val["stage_a_undistort"]) {
			model.project_state.view_mode = 1;
		}
		model.project_state.schema_version = 1;
	}

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

} // namespace StereoCalibrationHelpers

// ------------------------------------------------------------
// PreviewCtrl implementation (disabled)
// ------------------------------------------------------------

#if 0
void PreviewCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	w.DrawRect(sz, Black());
	if (has_images) {
		int half = max(1, sz.cx / 2);

		if (show_difference && !IsNull(left_img) && !IsNull(right_img)) {
			Size isz = left_img.GetSize();
			if (right_img.GetSize() != isz) {
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
				for (RGBA* p = ib.Begin(), *e = ib.End(); p < e; p++)
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
			for (int y = 0; y < sz.cy; y += step)
				w.DrawLine(0, y, sz.cx, y, 1, LtRed());
		}

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
					} else {
						p0 = Point(int(half + r.measured.x * (sz.cx - half)), int(r.measured.y * sz.cy));
						p1 = Point(int(half + r.reproj.x * (sz.cx - half)), int(r.measured.y * sz.cy));
					}
				}
				w.DrawLine(p0.x, p0.y, p1.x, p1.y, 1, c);
				w.DrawEllipse(p1.x - 2, p1.y - 2, 4, 4, c);
			}
		}

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

void PreviewCtrl::LeftDown(Point p, dword flags) {
	Size sz = GetSize();
	if (sz.cx <= 0 || sz.cy <= 0)
		return;
	if (overlay_mode || show_difference) {
		Pointf img_p(float(p.x) / sz.cx, float(p.y) / sz.cy);
		WhenClick(img_p, IsNull(pending_left) ? 0 : 1);
	} else {
		int half = sz.cx / 2;
		if (p.x < half) {
			Pointf img_p(float(p.x) / half, float(p.y) / sz.cy);
			WhenClick(img_p, 0);
		} else {
			Pointf img_p(float(p.x - half) / (sz.cx - half), float(p.y) / sz.cy);
			WhenClick(img_p, 1);
		}
	}
}
#endif

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
	running = true;
	return true;
}

void HmdStereoSource::Stop() {
	if (cam)
		cam->Close();
	cam.Clear();
	if (running)
		sys.Uninitialise();
	running = false;
	last_left = Image();
	last_right = Image();
}

bool HmdStereoSource::ReadFrame(VisualFrame& left, VisualFrame& right, bool prefer_bright) {
	if (!cam || !cam->IsOpen())
		return false;
	Vector<HMD::CameraFrame> frames;
	cam->PopFrames(frames);
	if (frames.IsEmpty())
		return false;

	int best_idx = -1;
	if (prefer_bright) {
		for (int i = 0; i < frames.GetCount(); i++) {
			if (frames[i].is_bright) {
				best_idx = i;
			}
		}
	}

	if (best_idx < 0)
		best_idx = frames.GetCount() - 1;

	const HMD::CameraFrame& f = frames[best_idx];
	if (!StereoCalibrationHelpers::SplitStereoImage(f.img, last_left, last_right))
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

bool HmdStereoSource::PeakFrame(VisualFrame& left, VisualFrame& right, bool prefer_bright) {
	if (!cam || !cam->IsOpen())
		return false;
	Vector<HMD::CameraFrame> frames;
	cam->PeakFrames(frames);
	if (frames.IsEmpty())
		return false;

	int best_idx = -1;
	if (prefer_bright) {
		for (int i = 0; i < frames.GetCount(); i++) {
			if (frames[i].is_bright) {
				best_idx = i;
			}
		}
	}

	if (best_idx < 0)
		best_idx = frames.GetCount() - 1;

	const HMD::CameraFrame& f = frames[best_idx];
	if (!StereoCalibrationHelpers::SplitStereoImage(f.img, last_left, last_right))
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

// ------------------------------------------------------------
// Controller implementation (disabled)
// ------------------------------------------------------------

#if 0
StereoCalibrationTool::StereoCalibrationTool() {
	menu = new MenuWindow();
	camera = new CameraWindow();
	stage_a = new StageAWindow();
	stage_b = new StageBWindow();
	stage_c = new StageCWindow();
	live = new LiveResultWindow();

	menu->Init(model, *camera, *stage_a, *stage_b, *stage_c, *live);
	camera->Init(model);
	stage_a->Init(model);
	stage_b->Init(model);
	stage_c->Init(model);
	live->Init(model);
}

StereoCalibrationTool::~StereoCalibrationTool() {
	usb_test_cb.Kill();
	hmd_test_cb.Kill();
	live_test_cb.Kill();
	if (camera)
		camera->StopSource();
	if (live)
		live->StopSource();
	if (!model.project_dir.IsEmpty()) {
		StereoCalibrationHelpers::SaveLastCalibration(model);
		StereoCalibrationHelpers::SaveState(model);
	}
	delete menu;
	delete camera;
	delete stage_a;
	delete stage_b;
	delete stage_c;
	delete live;
}

// Opens the Menu window and starts the UI event loop.
void StereoCalibrationTool::Run() {
	if (menu)
		menu->Run();
}

// Sets global verbose flag and forwards it to windows that read live sources.
void StereoCalibrationTool::SetVerbose(bool v) {
	model.verbose = v;
	if (camera)
		camera->SetVerbose(v);
	if (live)
		live->SetVerbose(v);
}

// Applies project directory, loads persisted state, and refreshes windows.
void StereoCalibrationTool::SetProjectDir(const String& dir) {
	model.project_dir = dir;
	StereoCalibrationHelpers::LoadLastCalibration(model);
	StereoCalibrationHelpers::LoadState(model);
	if (camera)
		camera->RefreshFromModel();
	if (stage_a)
		stage_a->RefreshFromModel();
	if (stage_b)
		stage_b->RefreshFromModel();
	if (stage_c)
		stage_c->RefreshFromModel();
	if (menu)
		menu->RefreshFromModel();
}

// Enables/disables genetic algorithm bootstrap parameters for headless usage.
void StereoCalibrationTool::EnableGABootstrap(bool enable, int population, int generations) {
	model.use_ga_bootstrap = enable;
	model.ga_population = population;
	model.ga_generations = generations;
}

// Enables USB source capture test and schedules timer callback.
void StereoCalibrationTool::EnableUsbTest(const String& dev, int timeout_ms) {
	usb_test_enabled = true;
	usb_test_device = dev;
	if (camera && !usb_test_device.IsEmpty())
		camera->SetUsbDevicePath(usb_test_device);
	if (timeout_ms > 0)
		usb_test_timeout_ms = timeout_ms;
	PostCallback(THISBACK(StartUsbTest));
}

// Starts the USB test timer loop.
void StereoCalibrationTool::StartUsbTest() {
	if (!usb_test_enabled || usb_test_active)
		return;
	usb_test_active = true;
	usb_test_start_us = usecs();
	usb_test_last_start_us = 0;
	usb_test_attempts = 0;
	usb_test_cb.Set(-100, THISBACK(RunUsbTest));
}

// Periodic USB test tick; exits with 0 on success, 1 on timeout.
void StereoCalibrationTool::RunUsbTest() {
	if (!usb_test_active || !camera)
		return;
	int64 elapsed_ms = (usecs() - usb_test_start_us) / 1000;
	if (elapsed_ms > usb_test_timeout_ms) {
		usb_test_active = false;
		usb_test_cb.Kill();
		camera->StopSource();
		Exit(1);
		return;
	}

	int idx = 1; // USB
	if (!camera->StartSourceByIndex(idx))
		return;
	Image l, r;
	if (camera->PeekFrame(l, r, true) && StereoCalibrationHelpers::IsFrameNonBlack(l)) {
		usb_test_active = false;
		usb_test_cb.Kill();
		camera->StopSource();
		Exit(0);
	}
}

// Enables HMD source capture test and schedules timer callback.
void StereoCalibrationTool::EnableHmdTest(int timeout_ms) {
	hmd_test_enabled = true;
	if (timeout_ms > 0)
		hmd_test_timeout_ms = timeout_ms;
	PostCallback(THISBACK(StartHmdTest));
}

// Starts the HMD test timer loop.
void StereoCalibrationTool::StartHmdTest() {
	if (!hmd_test_enabled || hmd_test_active)
		return;
	hmd_test_active = true;
	hmd_test_start_us = usecs();
	hmd_test_last_start_us = 0;
	hmd_test_attempts = 0;
	hmd_test_cb.Set(-100, THISBACK(RunHmdTest));
}

// Periodic HMD test tick; exits with 0 on success, 1 on timeout.
void StereoCalibrationTool::RunHmdTest() {
	if (!hmd_test_active || !camera)
		return;
	int64 elapsed_ms = (usecs() - hmd_test_start_us) / 1000;
	if (elapsed_ms > hmd_test_timeout_ms) {
		hmd_test_active = false;
		hmd_test_cb.Kill();
		camera->StopSource();
		Exit(1);
		return;
	}

	int idx = 0; // HMD
	if (!camera->StartSourceByIndex(idx))
		return;
	Image l, r;
	if (camera->PeekFrame(l, r, true) && StereoCalibrationHelpers::IsFrameNonBlack(l)) {
		hmd_test_active = false;
		hmd_test_cb.Kill();
		camera->StopSource();
		Exit(0);
	}
}

// Enables live test (calibrated preview check) and schedules timer callback.
void StereoCalibrationTool::EnableLiveTest(int timeout_ms) {
	if (timeout_ms > 0)
		live_test_timeout_ms = timeout_ms;
	PostCallback(THISBACK(StartLiveTest));
}

// Starts the live test timer loop.
void StereoCalibrationTool::StartLiveTest() {
	if (live_test_active)
		return;
	live_test_active = true;
	live_test_start_us = usecs();
	live_test_cb.Set(-100, THISBACK(RunLiveTest));
}

// Periodic live test tick; exits with 0 on success, 1 on timeout.
void StereoCalibrationTool::RunLiveTest() {
	if (!live_test_active || !live)
		return;
	int64 elapsed_ms = (usecs() - live_test_start_us) / 1000;
	if (elapsed_ms > live_test_timeout_ms) {
		live_test_active = false;
		live_test_cb.Kill();
		live->StopSource();
		Exit(1);
		return;
	}

	int idx = 0; // HMD by default
	if (!live->StartSourceByIndex(idx))
		return;
	Image l, r;
	if (live->PeekFrame(l, r, true) && StereoCalibrationHelpers::IsFrameNonBlack(l)) {
		live_test_active = false;
		live_test_cb.Kill();
		live->StopSource();
		Exit(0);
	}
}

// Headless solve entry point used by CLI (--solve).
int StereoCalibrationTool::SolveHeadless(const String& project_dir) {
	model.project_dir = project_dir;
	StereoCalibrationHelpers::LoadLastCalibration(model);
	StereoCalibrationHelpers::LoadState(model);
	if (stage_b)
		stage_b->RefreshFromModel();
	if (stage_b && stage_b->SolveCalibration()) {
		StereoCalibrationHelpers::SaveLastCalibration(model);
		StereoCalibrationHelpers::SaveState(model);
		return 0;
	}
	return 1;
}

// Placeholder for Stage A identity test (requires UI preview context).
int StereoCalibrationTool::TestStageAIdentity(const String& project_dir, const String& image_path) {
	model.project_dir = project_dir;
	StereoCalibrationHelpers::LoadLastCalibration(model);
	StereoCalibrationHelpers::LoadState(model);
	Cout() << "\n";
	Cout() << "Stage A identity test is a UI-only diagnostic (preview required).\n";
	if (!image_path.IsEmpty())
		Cout() << "Image path: " << image_path << "\n";
	return 0;
}

#endif

END_UPP_NAMESPACE
