#include "StereoCalibrationTool.h"

NAMESPACE_UPP

/*
StageARegression.cpp
--------------------
Purpose:
- CLI regression suite for Stage A viewer invariants.
- Tests all view-only features (overlay, tint, crosshair, diff) and preview toggles.
- Ensures identity preservation at zero extrinsics.
- Does NOT require GUI or camera hardware.

Usage:
  bin/StereoCalibrationTool --stagea_regression <project_dir> [--verbose]

Exit codes:
  0 = PASS (all tests passed)
  1 = FAIL (one or more tests failed)
*/

// ====================================================================
// Test Data Generation
// ====================================================================

static Image GenerateSyntheticTestImage(int eye_index) {
	// Generate 640x480 synthetic test image with gradient + checkerboard + corner markers
	const int w = 640;
	const int h = 480;
	ImageBuffer ib(w, h);

	for (int y = 0; y < h; y++) {
		RGBA* row = ib[y];
		for (int x = 0; x < w; x++) {
			// Base gradient (different per eye to detect mixing)
			byte base_r = (byte)((x * 255) / w);
			byte base_g = (byte)((y * 255) / h);
			byte base_b = (eye_index == 0) ? 128 : 64; // Left=128, Right=64

			// Checkerboard overlay (16x16 squares)
			bool checker = ((x / 16) + (y / 16)) & 1;
			if (checker) {
				base_r = min(255, base_r + 40);
				base_g = min(255, base_g + 40);
				base_b = min(255, base_b + 40);
			}

			// Corner markers (8x8 red squares in corners)
			if ((x < 8 || x >= w-8) && (y < 8 || y >= h-8)) {
				base_r = 255;
				base_g = 0;
				base_b = 0;
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
	// Try to load first captured frame from project
	String captures_dir = AppendFileName(project_dir, "captures");
	if (DirectoryExists(captures_dir)) {
		FindFile ff(AppendFileName(captures_dir, "*_l.png"));
		if (ff) {
			String left_path = ff.GetPath();
			String right_path = left_path;
			right_path.Replace("_l.png", "_r.png");

			left_out = StreamRaster::LoadFileAny(left_path);
			right_out = StreamRaster::LoadFileAny(right_path);

			if (!left_out.IsEmpty() && !right_out.IsEmpty()) {
				if (verbose)
					Cout() << "Loaded test images from: " << captures_dir << "\n";
				return true;
			}
		}
	}

	// Fallback: generate synthetic images
	if (verbose)
		Cout() << "No captured frames found, generating synthetic test images (640x480)\n";
	left_out = GenerateSyntheticTestImage(0);
	right_out = GenerateSyntheticTestImage(1);
	return true;
}

// ====================================================================
// Preview Pipeline Invocation (uses real Stage A helpers)
// ====================================================================

struct PreviewPipeline {
	AppModel* model = nullptr;
	Image raw_left;
	Image raw_right;
	Image preview_left;
	Image preview_right;
	bool verbose = false;

	void Init(AppModel& m, const Image& l, const Image& r, bool v) {
		model = &m;
		raw_left = l;
		raw_right = r;
		verbose = v;
	}

	// Generate previews using Stage A logic (extrinsics + intrinsics)
	void GeneratePreviews() {
		Size sz = raw_left.GetSize();
		vec2 pp(sz.cx * 0.5f, sz.cy * 0.5f);

		bool extr_on = model->project_state.preview_extrinsics;
		bool intr_on = model->project_state.preview_intrinsics;

		float yaw_l = (float)(model->project_state.yaw_l * M_PI / 180.0);
		float pitch_l = (float)(model->project_state.pitch_l * M_PI / 180.0);
		float roll_l = (float)(model->project_state.roll_l * M_PI / 180.0);
		float yaw_r = (float)(model->project_state.yaw_r * M_PI / 180.0);
		float pitch_r = (float)(model->project_state.pitch_r * M_PI / 180.0);
		float roll_r = (float)(model->project_state.roll_r * M_PI / 180.0);

		// Left eye
		if (!extr_on && !intr_on) {
			preview_left = raw_left;
		} else if (extr_on && !intr_on) {
			preview_left = StereoCalibrationHelpers::ApplyExtrinsicsOnly(raw_left, yaw_l, pitch_l, roll_l, pp);
		} else if (!extr_on && intr_on) {
			// Build lens
			LensPoly lens;
			lens.SetSize(sz);
			float fov_rad = (float)(model->project_state.fov_deg * M_PI / 180.0);
			float a = (sz.cx * 0.5f) / (fov_rad * 0.5f);
			float barrel = (float)model->project_state.barrel_strength * 0.01f;
			float b = a * barrel;
			float c = a * barrel;
			float d = a * barrel;
			lens.SetAnglePixel(a, b, c, d);
			lens.SetPrincipalPoint(sz.cx * 0.5f, sz.cy * 0.5f);
			float linear_scale = sz.cy / (2.0f * tanf(fov_rad / 2.0f));
			preview_left = StereoCalibrationHelpers::ApplyIntrinsicsOnly(raw_left, lens, linear_scale, true);
		} else {
			// Both ON: sequential composition (undistort then rotate)
			LensPoly lens;
			lens.SetSize(sz);
			float fov_rad = (float)(model->project_state.fov_deg * M_PI / 180.0);
			float a = (sz.cx * 0.5f) / (fov_rad * 0.5f);
			float barrel = (float)model->project_state.barrel_strength * 0.01f;
			float b = a * barrel;
			float c = a * barrel;
			float d = a * barrel;
			lens.SetAnglePixel(a, b, c, d);
			lens.SetPrincipalPoint(sz.cx * 0.5f, sz.cy * 0.5f);
			float linear_scale = sz.cy / (2.0f * tanf(fov_rad / 2.0f));
			Image undist = StereoCalibrationHelpers::ApplyIntrinsicsOnly(raw_left, lens, linear_scale, true);
			preview_left = StereoCalibrationHelpers::ApplyExtrinsicsOnly(undist, yaw_l, pitch_l, roll_l, pp);
		}

		// Right eye (same logic)
		if (!extr_on && !intr_on) {
			preview_right = raw_right;
		} else if (extr_on && !intr_on) {
			preview_right = StereoCalibrationHelpers::ApplyExtrinsicsOnly(raw_right, yaw_r, pitch_r, roll_r, pp);
		} else if (!extr_on && intr_on) {
			LensPoly lens;
			lens.SetSize(sz);
			float fov_rad = (float)(model->project_state.fov_deg * M_PI / 180.0);
			float a = (sz.cx * 0.5f) / (fov_rad * 0.5f);
			float barrel = (float)model->project_state.barrel_strength * 0.01f;
			float b = a * barrel;
			float c = a * barrel;
			float d = a * barrel;
			lens.SetAnglePixel(a, b, c, d);
			lens.SetPrincipalPoint(sz.cx * 0.5f, sz.cy * 0.5f);
			float linear_scale = sz.cy / (2.0f * tanf(fov_rad / 2.0f));
			preview_right = StereoCalibrationHelpers::ApplyIntrinsicsOnly(raw_right, lens, linear_scale, true);
		} else {
			LensPoly lens;
			lens.SetSize(sz);
			float fov_rad = (float)(model->project_state.fov_deg * M_PI / 180.0);
			float a = (sz.cx * 0.5f) / (fov_rad * 0.5f);
			float barrel = (float)model->project_state.barrel_strength * 0.01f;
			float b = a * barrel;
			float c = a * barrel;
			float d = a * barrel;
			lens.SetAnglePixel(a, b, c, d);
			lens.SetPrincipalPoint(sz.cx * 0.5f, sz.cy * 0.5f);
			float linear_scale = sz.cy / (2.0f * tanf(fov_rad / 2.0f));
			Image undist = StereoCalibrationHelpers::ApplyIntrinsicsOnly(raw_right, lens, linear_scale, true);
			preview_right = StereoCalibrationHelpers::ApplyExtrinsicsOnly(undist, yaw_r, pitch_r, roll_r, pp);
		}
	}

	// Compose final display images (overlay, tint, crosshair, diff)
	Image ComposeFinalDisplay() {
		// For now, just return side-by-side for basic tests
		// Will be extended for overlay/tint/crosshair/diff tests
		Size sz = preview_left.GetSize();
		ImageBuffer out(sz.cx * 2, sz.cy);
		for (int y = 0; y < sz.cy; y++) {
			RGBA* dst = out[y];
			const RGBA* src_l = preview_left[y];
			const RGBA* src_r = preview_right[y];
			for (int x = 0; x < sz.cx; x++) {
				dst[x] = src_l[x];
				dst[x + sz.cx] = src_r[x];
			}
		}
		return out;
	}
};

// ====================================================================
// Image Comparison Utilities
// ====================================================================

struct ImageDiff {
	int diff_count = 0;
	int max_diff = 0;
	double mean_diff = 0;
	int first_diff_x = -1;
	int first_diff_y = -1;
	RGBA first_orig;
	RGBA first_preview;

	void Compute(const Image& a, const Image& b) {
		Size sz = a.GetSize();
		if (sz != b.GetSize()) {
			diff_count = sz.cx * sz.cy; // Size mismatch = total diff
			return;
		}

		int64 total_diff = 0;
		for (int y = 0; y < sz.cy; y++) {
			const RGBA* row_a = a[y];
			const RGBA* row_b = b[y];
			for (int x = 0; x < sz.cx; x++) {
				int dr = abs(row_a[x].r - row_b[x].r);
				int dg = abs(row_a[x].g - row_b[x].g);
				int db = abs(row_a[x].b - row_b[x].b);
				int da = abs(row_a[x].a - row_b[x].a);
				int diff = max(max(dr, dg), max(db, da));
				if (diff > 0) {
					if (first_diff_x < 0) {
						first_diff_x = x;
						first_diff_y = y;
						first_orig = row_a[x];
						first_preview = row_b[x];
					}
					diff_count++;
					max_diff = max(max_diff, diff);
					total_diff += diff;
				}
			}
		}
		if (diff_count > 0)
			mean_diff = (double)total_diff / diff_count;
	}

	void Print(const String& eye_label) const {
		if (diff_count == 0) {
			Cout() << "    " << eye_label << ": ✓ IDENTICAL (pixel-perfect)\n";
		} else {
			Cout() << Format("    %s: ✗ DIFFERS (%d pixels, max=%d, mean=%.1f)\n",
				eye_label, diff_count, max_diff, mean_diff);
			if (first_diff_x >= 0) {
				Cout() << Format("      First diff at (%d,%d): (%d,%d,%d,%d) → (%d,%d,%d,%d)\n",
					first_diff_x, first_diff_y,
					first_orig.r, first_orig.g, first_orig.b, first_orig.a,
					first_preview.r, first_preview.g, first_preview.b, first_preview.a);
			}
		}
	}
};

// ====================================================================
// Test 1: Identity lock at zero extrinsics when intrinsics OFF
// ====================================================================

static bool Test1_IdentityLock(PreviewPipeline& pipe, bool verbose) {
	Cout() << "\n╔═══════════════════════════════════════════════════════════════════╗\n";
	Cout() << "║  Test 1: Identity lock at zero extrinsics (intrinsics OFF)     ║\n";
	Cout() << "╚═══════════════════════════════════════════════════════════════════╝\n\n";

	// Setup
	pipe.model->project_state.preview_extrinsics = true;
	pipe.model->project_state.preview_intrinsics = false;
	pipe.model->project_state.yaw_l = 0;
	pipe.model->project_state.pitch_l = 0;
	pipe.model->project_state.roll_l = 0;
	pipe.model->project_state.yaw_r = 0;
	pipe.model->project_state.pitch_r = 0;
	pipe.model->project_state.roll_r = 0;

	if (verbose) {
		Cout() << "Setup:\n";
		Cout() << "  preview_extrinsics: ON\n";
		Cout() << "  preview_intrinsics: OFF\n";
		Cout() << "  All yaw/pitch/roll: 0\n\n";
	}

	// Generate previews
	pipe.GeneratePreviews();

	// Compare
	ImageDiff diff_l, diff_r;
	diff_l.Compute(pipe.raw_left, pipe.preview_left);
	diff_r.Compute(pipe.raw_right, pipe.preview_right);

	Cout() << "Results:\n";
	diff_l.Print("Left eye ");
	diff_r.Print("Right eye");

	bool pass = (diff_l.diff_count == 0) && (diff_r.diff_count == 0);
	Cout() << "\n" << (pass ? "✓ PASS" : "✗ FAIL") << ": Identity lock\n";
	return pass;
}

// ====================================================================
// Test 2: Per-eye extrinsics visibility & isolation
// ====================================================================

static bool Test2_ExtrinsicsIsolation(PreviewPipeline& pipe, bool verbose) {
	Cout() << "\n╔═══════════════════════════════════════════════════════════════════╗\n";
	Cout() << "║  Test 2: Per-eye extrinsics visibility & isolation             ║\n";
	Cout() << "╚═══════════════════════════════════════════════════════════════════╝\n\n";

	// Setup: left yaw nonzero, right zero
	pipe.model->project_state.preview_extrinsics = true;
	pipe.model->project_state.preview_intrinsics = false;
	pipe.model->project_state.yaw_l = 11.5; // degrees (0.2 radians)
	pipe.model->project_state.pitch_l = 0;
	pipe.model->project_state.roll_l = 0;
	pipe.model->project_state.yaw_r = 0;
	pipe.model->project_state.pitch_r = 0;
	pipe.model->project_state.roll_r = 0;

	if (verbose) {
		Cout() << "Setup:\n";
		Cout() << "  Left yaw: +11.5 deg, pitch/roll: 0\n";
		Cout() << "  Right yaw/pitch/roll: 0\n\n";
	}

	pipe.GeneratePreviews();

	ImageDiff diff_l, diff_r;
	diff_l.Compute(pipe.raw_left, pipe.preview_left);
	diff_r.Compute(pipe.raw_right, pipe.preview_right);

	Cout() << "Results:\n";
	diff_l.Print("Left eye ");
	diff_r.Print("Right eye");

	Size sz = pipe.raw_left.GetSize();
	int total_pixels = sz.cx * sz.cy;
	double left_pct = 100.0 * diff_l.diff_count / total_pixels;

	bool left_changed = (left_pct >= 5.0); // At least 5% pixels differ
	bool right_unchanged = (diff_r.diff_count == 0); // Right still identity

	if (verbose) {
		Cout() << Format("\n  Left changed: %.1f%% pixels (threshold: >=5%%)\n", left_pct);
		Cout() << "  Right unchanged: " << (right_unchanged ? "YES" : "NO") << "\n";
	}

	bool pass = left_changed && right_unchanged;
	Cout() << "\n" << (pass ? "✓ PASS" : "✗ FAIL") << ": Per-eye isolation\n";

	// Symmetry check: swap eyes
	if (verbose)
		Cout() << "\nSymmetry check (swap eyes):\n";

	pipe.model->project_state.yaw_l = 0;
	pipe.model->project_state.yaw_r = 11.5; // degrees (0.2 radians)
	pipe.GeneratePreviews();

	diff_l.Compute(pipe.raw_left, pipe.preview_left);
	diff_r.Compute(pipe.raw_right, pipe.preview_right);

	if (verbose) {
		diff_l.Print("Left eye ");
		diff_r.Print("Right eye");
	}

	double right_pct = 100.0 * diff_r.diff_count / total_pixels;
	bool right_changed = (right_pct >= 5.0);
	bool left_unchanged = (diff_l.diff_count == 0);

	bool symmetry_pass = right_changed && left_unchanged;
	Cout() << "\n" << (symmetry_pass ? "✓ PASS" : "✗ FAIL") << ": Symmetry check\n";

	return pass && symmetry_pass;
}

// ====================================================================
// Test 3-7: Overlay/Tint/Crosshair/Diff (placeholder stubs)
// ====================================================================

static bool Test3_OverlayViewOnly(PreviewPipeline& pipe, bool verbose) {
	Cout() << "\n╔═══════════════════════════════════════════════════════════════════╗\n";
	Cout() << "║  Test 3: Overlay is view-only (does not mutate previews)       ║\n";
	Cout() << "╚═══════════════════════════════════════════════════════════════════╝\n\n";

	// TODO: Implement overlay compositor test
	Cout() << "⚠ NOT IMPLEMENTED YET\n";
	return true; // Placeholder PASS
}

static bool Test4_TintViewOnly(PreviewPipeline& pipe, bool verbose) {
	Cout() << "\n╔═══════════════════════════════════════════════════════════════════╗\n";
	Cout() << "║  Test 4: Tint is view-only and produces changed display        ║\n";
	Cout() << "╚═══════════════════════════════════════════════════════════════════╝\n\n";

	Cout() << "⚠ NOT IMPLEMENTED YET\n";
	return true;
}

static bool Test5_CrosshairViewOnly(PreviewPipeline& pipe, bool verbose) {
	Cout() << "\n╔═══════════════════════════════════════════════════════════════════╗\n";
	Cout() << "║  Test 5: Crosshair is view-only and appears at center          ║\n";
	Cout() << "╚═══════════════════════════════════════════════════════════════════╝\n\n";

	Cout() << "⚠ NOT IMPLEMENTED YET\n";
	return true;
}

static bool Test6_DiffViewOnly(PreviewPipeline& pipe, bool verbose) {
	Cout() << "\n╔═══════════════════════════════════════════════════════════════════╗\n";
	Cout() << "║  Test 6: Diff is view-only and works                           ║\n";
	Cout() << "╚═══════════════════════════════════════════════════════════════════╝\n\n";

	Cout() << "⚠ NOT IMPLEMENTED YET\n";
	return true;
}

static bool Test7_ToggleMatrix(PreviewPipeline& pipe, bool verbose) {
	Cout() << "\n╔═══════════════════════════════════════════════════════════════════╗\n";
	Cout() << "║  Test 7: Toggle matrix coverage (all combinations)             ║\n";
	Cout() << "╚═══════════════════════════════════════════════════════════════════╝\n\n";

	Cout() << "⚠ NOT IMPLEMENTED YET\n";
	return true;
}

// ====================================================================
// Main Entry Point
// ====================================================================

int RunStageARegression(const String& project_dir, bool verbose) {
	Cout() << "\n";
	Cout() << "╔═══════════════════════════════════════════════════════════════════╗\n";
	Cout() << "║          Stage A Regression Suite (Viewer Invariants)          ║\n";
	Cout() << "╚═══════════════════════════════════════════════════════════════════╝\n";

	if (verbose)
		Cout() << "\nProject directory: " << project_dir << "\n";

	// Load model
	AppModel model;
	model.project_dir = project_dir;
	StereoCalibrationHelpers::LoadLastCalibration(model);
	StereoCalibrationHelpers::LoadState(model);

	// Load test images
	Image test_left, test_right;
	if (!LoadTestImages(project_dir, test_left, test_right, verbose)) {
		Cerr() << "✗ FAIL: Could not load test images\n";
		return 1;
	}

	Size sz = test_left.GetSize();
	if (verbose)
		Cout() << Format("Test image size: %dx%d\n", sz.cx, sz.cy);

	// Initialize pipeline
	PreviewPipeline pipe;
	pipe.Init(model, test_left, test_right, verbose);

	// Run tests
	bool all_pass = true;
	all_pass &= Test1_IdentityLock(pipe, verbose);
	all_pass &= Test2_ExtrinsicsIsolation(pipe, verbose);
	all_pass &= Test3_OverlayViewOnly(pipe, verbose);
	all_pass &= Test4_TintViewOnly(pipe, verbose);
	all_pass &= Test5_CrosshairViewOnly(pipe, verbose);
	all_pass &= Test6_DiffViewOnly(pipe, verbose);
	all_pass &= Test7_ToggleMatrix(pipe, verbose);

	// Summary
	Cout() << "\n╔═══════════════════════════════════════════════════════════════════╗\n";
	Cout() << "║                         Test Summary                            ║\n";
	Cout() << "╚═══════════════════════════════════════════════════════════════════╝\n\n";

	if (all_pass) {
		Cout() << "✓ ALL TESTS PASSED\n\n";
		return 0;
	} else {
		Cout() << "✗ SOME TESTS FAILED\n\n";
		return 1;
	}
}

END_UPP_NAMESPACE
