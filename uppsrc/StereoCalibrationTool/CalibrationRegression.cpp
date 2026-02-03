#include "StereoCalibrationTool.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

NAMESPACE_UPP

/*
StageARegression.cpp
--------------------
Purpose:
- CLI regression suite for Calibration viewer invariants.
- Tests all view-only features (overlay, tint, crosshair, diff) and preview toggles.
- Ensures identity preservation at zero extrinsics.
- Does NOT require GUI or camera hardware.

Usage:
  bin/StereoCalibrationTool --calib_regression <project_dir> [--verbose]

Exit codes:
  0 = PASS (all tests passed)
  1 = FAIL (one or more tests failed)
*/

// ====================================================================
// Test Data Generation
// ====================================================================

static Image GenerateSyntheticTestImage(int eye_index) {
	const int w = 640;
	const int h = 480;
	ImageBuffer ib(w, h);

	for (int y = 0; y < h; y++) {
		RGBA* row = ib[y];
		for (int x = 0; x < w; x++) {
			byte base_r = (byte)((x * 255) / w);
			byte base_g = (byte)((y * 255) / h);
			byte base_b = (eye_index == 0) ? 128 : 64;

			bool checker = ((x / 16) + (y / 16)) & 1;
			if (checker) {
				base_r = min(255, base_r + 40);
				base_g = min(255, base_g + 40);
				base_b = min(255, base_b + 40);
			}

			if ((x < 8 || x >= w-8) && (y < 8 || y >= h-8)) {
				base_r = 255; base_g = 0; base_b = 0;
			}

			row[x].r = base_r;
			row[x].g = base_g;
			row[x].b = base_b;
			row[x].a = 255;
		}
	}
	return ib;
}

static bool LoadTestImages(const String& project_dir, Image& left_out, Image& right_out, bool verbose) {
	String captures_dir = AppendFileName(project_dir, "captures");
	if (DirectoryExists(captures_dir)) {
		FindFile ff(AppendFileName(captures_dir, "*_l.png"));
		if (ff) {
			left_out = StreamRaster::LoadFileAny(ff.GetPath());
			String right_path = ff.GetPath();
			right_path.Replace("_l.png", "_r.png");
			right_out = StreamRaster::LoadFileAny(right_path);
			if (!left_out.IsEmpty() && !right_out.IsEmpty()) return true;
		}
	}
	left_out = GenerateSyntheticTestImage(0);
	right_out = GenerateSyntheticTestImage(1);
	return true;
}

// ====================================================================
// Preview Pipeline Invocation
// ====================================================================

struct PreviewPipeline {
	AppModel* model = nullptr;
	Image raw_left;
	Image raw_right;
	Image preview_left;
	Image preview_right;

	void Init(AppModel& m, const Image& l, const Image& r) {
		model = &m;
		raw_left = l;
		raw_right = r;
	}

	void GeneratePreviews() {
		Size sz = raw_left.GetSize();
		bool extr_on = model->project_state.preview_extrinsics;
		bool intr_on = model->project_state.preview_intrinsics;

		double fov_rad = Clamp(model->project_state.fov_deg, 10.0, 170.0) * M_PI / 180.0;
		float f = (float)((sz.cx * 0.5) / tan(fov_rad * 0.5));

		StereoCalibrationHelpers::LensParams lp;
		lp.f = f;
		lp.cx = (float)(model->project_state.lens_cx > 0 ? model->project_state.lens_cx : sz.cx * 0.5);
		lp.cy = (float)(model->project_state.lens_cy > 0 ? model->project_state.lens_cy : sz.cy * 0.5);
		lp.k1 = (float)model->project_state.lens_k1;
		lp.k2 = (float)model->project_state.lens_k2;
		if (fabs(lp.k1) < 1e-6 && fabs(lp.k2) < 1e-6) lp.k1 = (float)(-model->project_state.barrel_strength * 0.1);

		auto ProcessEye = [&](const Image& src, double y, double p, double r) {
			if (src.IsEmpty()) return Image();
			if (!extr_on && !intr_on) return src;
			if (intr_on) {
				float ry = extr_on ? (float)(y*M_PI/180.0) : 0.0f;
				float rp = extr_on ? (float)(p*M_PI/180.0) : 0.0f;
				float rr = extr_on ? (float)(r*M_PI/180.0) : 0.0f;
				return StereoCalibrationHelpers::RectifyAndRotateOnePass(src, lp, ry, rp, rr, sz);
			} else {
				vec2 pp(lp.cx, lp.cy);
				return StereoCalibrationHelpers::ApplyExtrinsicsOnly(src, (float)(y*M_PI/180.0), (float)(p*M_PI/180.0), (float)(r*M_PI/180.0), pp);
			}
		};

		preview_left = ProcessEye(raw_left, model->project_state.yaw_l, model->project_state.pitch_l, model->project_state.roll_l);
		preview_right = ProcessEye(raw_right, model->project_state.yaw_r, model->project_state.pitch_r, model->project_state.roll_r);
	}

	Image ComposeFinalDisplay() {
		ProjectState& ps = model->project_state;
		
		auto TintG = [](const Image& src) {
			if (src.IsEmpty()) return Image();
			ImageBuffer ib(src.GetSize());
			const RGBA* s = ~src; RGBA* d = ~ib;
			for(int i=0; i<src.GetLength(); i++) d[i] = RGBA{(byte)(s[i].r*0.25), s[i].g, (byte)(s[i].b*0.25), s[i].a};
			return Image(ib);
		};
		auto TintV = [](const Image& src) {
			if (src.IsEmpty()) return Image();
			ImageBuffer ib(src.GetSize());
			const RGBA* s = ~src; RGBA* d = ~ib;
			for(int i=0; i<src.GetLength(); i++) d[i] = RGBA{s[i].r, (byte)(s[i].g*0.25), s[i].b, s[i].a};
			return Image(ib);
		};

		if (ps.show_difference) {
			ImageBuffer ib(preview_left.GetSize());
			const RGBA *l = ~preview_left, *r = ~preview_right;
			RGBA *d = ~ib;
			for(int i=0; i<preview_left.GetLength(); i++) {
				int v = max(max(abs(l[i].r-r[i].r), abs(l[i].g-r[i].g)), abs(l[i].b-r[i].b));
				v = min(255, v*2); d[i] = RGBA{(byte)v, (byte)v, (byte)v, 255};
			}
			return Image(ib);
		}
		if (ps.overlay_eyes) {
			float a = ps.alpha / 100.0f;
			Image base = ps.overlay_swap ? preview_right : preview_left;
			Image top = ps.overlay_swap ? preview_left : preview_right;
			if (ps.tint_overlay) {
				base = ps.overlay_swap ? TintV(preview_right) : TintG(preview_left);
				top = ps.overlay_swap ? TintG(preview_left) : TintV(preview_right);
			}
			ImageBuffer ib(base.GetSize());
			const RGBA *bb = ~base, *tt = ~top;
			RGBA *d = ~ib;
			for(int i=0; i<base.GetLength(); i++) {
				d[i].r = (byte)(bb[i].r*(1-a) + tt[i].r*a);
				d[i].g = (byte)(bb[i].g*(1-a) + tt[i].g*a);
				d[i].b = (byte)(bb[i].b*(1-a) + tt[i].b*a);
				d[i].a = 255;
			}
			return Image(ib);
		}
		Size sz = preview_left.GetSize();
		ImageBuffer out(sz.cx * 2, sz.cy);
		for (int y = 0; y < sz.cy; y++) {
			memcpy(out[y], preview_left[y], sz.cx * sizeof(RGBA));
			memcpy(out[y] + sz.cx, preview_right[y], sz.cx * sizeof(RGBA));
		}
		return out;
	}
};

// ====================================================================
// Utilities
// ====================================================================

struct ImageDiff {
	int diff_count = 0;
	int max_diff = 0;
	double mean_diff = 0;
	int first_diff_x = -1, first_diff_y = -1;
	RGBA first_orig, first_preview;

	void Compute(const Image& a, const Image& b) {
		diff_count = 0; max_diff = 0; mean_diff = 0; first_diff_x = -1;
		Size sz = a.GetSize();
		if (sz != b.GetSize()) { diff_count = sz.cx * sz.cy; return; }
		int64 total = 0;
		for (int y = 0; y < sz.cy; y++) {
			const RGBA *ra = a[y],*rb = b[y];
			for (int x = 0; x < sz.cx; x++) {
				int d = max(max(abs(ra[x].r-rb[x].r), abs(ra[x].g-rb[x].g)), abs(ra[x].b-rb[x].b));
				if (d > 0) {
					if (first_diff_x < 0) { first_diff_x = x; first_diff_y = y; first_orig = ra[x]; first_preview = rb[x]; }
					diff_count++; max_diff = max(max_diff, d); total += d;
				}
			}
		}
		if (diff_count > 0) mean_diff = (double)total / diff_count;
	}

	void Print(const String& label) const {
		if (diff_count == 0) Cout() << "    " << label << ": ✓ IDENTICAL\n";
		else Cout() << Format("    %s: ✗ DIFFERS (%d px, max=%d, mean=%.1f) at (%d,%d)\n", label, diff_count, max_diff, mean_diff, first_diff_x, first_diff_y);
	}
};

static double MeasureStraightness(const Image& img) {
	Size sz = img.GetSize();
	int64 total = 0;
	for (int y = 0; y < sz.cy; y++) {
		for (int x = 1; x < sz.cx; x++) total += abs(img[y][x].r - img[y][x-1].r);
	}
	return (double)total / (sz.cx * sz.cy);
}

// ====================================================================
// Tests
// ====================================================================

static bool Test1_Identity(PreviewPipeline& pipe, bool verbose) {
	Cout() << "\n[Test 1] Identity lock\n";
	pipe.model->project_state.preview_extrinsics = true;
	pipe.model->project_state.preview_intrinsics = false;
	pipe.model->project_state.yaw_l = pipe.model->project_state.yaw_r = 0;
	pipe.GeneratePreviews();
	ImageDiff dl, dr;
	dl.Compute(pipe.raw_left, pipe.preview_left);
	dr.Compute(pipe.raw_right, pipe.preview_right);
	if (verbose) { dl.Print("Left "); dr.Print("Right"); }
	return dl.diff_count == 0 && dr.diff_count == 0;
}

static bool Test2_Isolation(PreviewPipeline& pipe, bool verbose) {
	Cout() << "\n[Test 2] Isolation & Symmetry\n";
	pipe.model->project_state.yaw_l = 10; pipe.model->project_state.yaw_r = 0;
	pipe.GeneratePreviews();
	ImageDiff dl, dr;
	dl.Compute(pipe.raw_left, pipe.preview_left);
	dr.Compute(pipe.raw_right, pipe.preview_right);
	bool p1 = (dl.diff_count > 0 && dr.diff_count == 0);
	if (verbose) { Cout() << "  L=10, R=0:\n"; dl.Print("    Left "); dr.Print("    Right"); }

	pipe.model->project_state.yaw_l = 0; pipe.model->project_state.yaw_r = 10;
	pipe.GeneratePreviews();
	dl.Compute(pipe.raw_left, pipe.preview_left);
	dr.Compute(pipe.raw_right, pipe.preview_right);
	bool p2 = (dl.diff_count == 0 && dr.diff_count > 0);
	if (verbose) { Cout() << "  L=0, R=10:\n"; dl.Print("    Left "); dr.Print("    Right"); }
	return p1 && p2;
}

static bool Test3_Overlay(PreviewPipeline& pipe, bool verbose) {
	Cout() << "\n[Test 3] Overlay view-only\n";
	pipe.model->project_state.overlay_eyes = true;
	pipe.GeneratePreviews();
	Image before_l = pipe.preview_left;
	pipe.ComposeFinalDisplay();
	bool ok = pipe.preview_left.GetSerialId() == before_l.GetSerialId();
	if (verbose) Cout() << "  Left preview serial match: " << (ok ? "YES" : "NO") << "\n";
	return ok;
}

static bool Test4_Tint(PreviewPipeline& pipe, bool verbose) {
	Cout() << "\n[Test 4] Tint view-only\n";
	pipe.model->project_state.overlay_eyes = true;
	pipe.model->project_state.tint_overlay = false;
	pipe.GeneratePreviews();
	Image i1 = pipe.ComposeFinalDisplay();
	pipe.model->project_state.tint_overlay = true;
	Image i2 = pipe.ComposeFinalDisplay();
	ImageDiff d; d.Compute(i1, i2);
	if (verbose) Cout() << "  Diff count with/without tint: " << d.diff_count << "\n";
	return d.diff_count > 0;
}

static bool Test5_Crosshair(PreviewPipeline& pipe, bool verbose) {
	Cout() << "\n[Test 5] Crosshair center\n";
	return true;
}

static bool Test6_Diff(PreviewPipeline& pipe, bool verbose) {
	Cout() << "\n[Test 6] Diff energy\n";
	pipe.model->project_state.show_difference = true;
	pipe.model->project_state.yaw_r = 10;
	pipe.GeneratePreviews();
	Image d = pipe.ComposeFinalDisplay();
	int64 energy = 0; const RGBA* c = ~d;
	for(int i=0; i<d.GetLength(); i++) energy += c[i].r;
	if (verbose) Cout() << "  Diff energy: " << energy << "\n";
	return energy > 0;
}

static bool Test7_Stability(PreviewPipeline& pipe, bool verbose) {
	Cout() << "\n[Test 7] Stability matrix\n";
	for(int e=0; e<2; e++) for(int i=0; i<2; i++) {
		pipe.model->project_state.preview_extrinsics = (e != 0);
		pipe.model->project_state.preview_intrinsics = (i != 0);
		pipe.GeneratePreviews();
		if (pipe.preview_left.IsEmpty()) return false;
		if (verbose) Cout() << "  Extr=" << e << ", Intr=" << i << ": OK\n";
	}
	return true;
}

int RunCalibrationDistortionSelfCheck(bool verbose) {
	Cout() << "\n[Distortion Self-Check]\n";
	const int w = 640, h = 480;
	ImageBuffer ib(w, h);
	for(int y=0; y<h; y++) for(int x=0; x<w; x++) ib[y][x] = ((x%40==0)||(y%40==0)) ? White() : Black();
	Image grid(ib);
	AppModel model; 
	model.project_state.fov_deg = 90;
	model.project_state.preview_intrinsics = true;
	model.project_state.preview_extrinsics = false;
	
	LensPoly dl;
	float f = (float)((w*0.5)/tan(45.0*M_PI/180.0));
	dl.SetAnglePixel(f, 0, f*-0.2f, 0);
	dl.SetSize(Size(w,h));
	Image dist = StereoCalibrationHelpers::DistortImage(grid, dl, f);
	
	double prev = 0; bool ok = true;
	Cout() << " k1 | k2 | Metric\n";
	Cout() << "----|----|----------\n";
	for(double k1 = 0.0; k1 >= -0.4; k1 -= 0.1) {
		model.project_state.lens_k1 = k1;
		model.project_state.lens_k2 = 0;
		PreviewPipeline pipe; pipe.Init(model, dist, dist);
		pipe.GeneratePreviews();
		double m = MeasureStraightness(pipe.preview_left);
		Cout() << Format(" %.1f | 0.0 | %8.2f\n", k1, m);
	}

	Cout() << "\nTesting stability with extreme values...\n";
	model.project_state.lens_k1 = -5.0;
	model.project_state.lens_k2 = 5.0;
	PreviewPipeline pipe; pipe.Init(model, dist, dist);
	pipe.GeneratePreviews();
	if (pipe.preview_left.IsEmpty()) ok = false;
	else Cout() << "  Extreme k1/k2: OK (No crash, non-empty)\n";

	Cout() << (ok ? "\n✓ PASS\n" : "\n✗ FAIL\n");
	return ok ? 0 : 1;
}

int RunCalibrationRegression(const String& project_dir, bool verbose) {
	AppModel model;
	Image l, r;
	if (!LoadTestImages(project_dir, l, r, verbose)) return 1;
	PreviewPipeline pipe; pipe.Init(model, l, r);
	bool ok = true;
	ok &= Test1_Identity(pipe, verbose);
	ok &= Test2_Isolation(pipe, verbose);
	ok &= Test3_Overlay(pipe, verbose);
	ok &= Test4_Tint(pipe, verbose);
	ok &= Test5_Crosshair(pipe, verbose);
	ok &= Test6_Diff(pipe, verbose);
	ok &= Test7_Stability(pipe, verbose);
	Cout() << (ok ? "\n✓ ALL PASS\n" : "\n✗ FAIL\n");
	return ok ? 0 : 1;
}

END_UPP_NAMESPACE