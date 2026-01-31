#include "StereoCalibrationTool.h"

NAMESPACE_UPP

// Stage A Identity Test Implementation
// Tests that zero extrinsics produce identity mapping (pixel-perfect copy)

int TestStageAIdentity(AppModel& model, const String& proj_dir, const String& img_path) {
	Cout() << "\n";
	Cout() << "╔═══════════════════════════════════════════════════════════════════╗\n";
	Cout() << "║    Stage A Preview Identity Test (Zero Extrinsics)              ║\n";
	Cout() << "╚═══════════════════════════════════════════════════════════════════╝\n\n";

	// Set project directory and load calibration
	model.project_dir = proj_dir;
	StereoCalibrationHelpers::LoadLastCalibration(model);
	StereoCalibrationHelpers::LoadState(model);

	// Force identity conditions
	model.project_state.yaw_l = 0;
	model.project_state.pitch_l = 0;
	model.project_state.roll_l = 0;
	model.project_state.yaw_r = 0;
	model.project_state.pitch_r = 0;
	model.project_state.roll_r = 0;
	model.project_state.preview_extrinsics = true;
	model.project_state.preview_intrinsics = false;

	// Load test images
	Image test_left, test_right;

	if(!img_path.IsEmpty()) {
		Cout() << "Loading test image from: " << img_path << "\n";
		test_left = StreamRaster::LoadFileAny(img_path);
		test_right = test_left;
	} else {
		String captures_dir = AppendFileName(proj_dir, "captures");
		if(!DirectoryExists(captures_dir)) {
			Cerr() << "Error: No captures directory found\n";
			return 1;
		}

		FindFile ff(AppendFileName(captures_dir, "*_l.png"));
		if(ff) {
			String left_path = ff.GetPath();
			Cout() << "Loading test image: " << left_path << "\n";
			test_left = StreamRaster::LoadFileAny(left_path);
			test_right = test_left;
		} else {
			Cerr() << "Error: No test images found\n";
			return 1;
		}
	}

	if(test_left.IsEmpty()) {
		Cerr() << "Error: Failed to load test image\n";
		return 1;
	}

	Size sz = test_left.GetSize();
	Cout() << "Test image size: " << sz.cx << "x" << sz.cy << "\n";
	Cout() << "Test conditions:\n";
	Cout() << "  - preview_extrinsics: ON\n";
	Cout() << "  - preview_intrinsics: OFF\n";
	Cout() << "  - yaw/pitch/roll (all eyes): 0\n\n";

	// Test: Apply preview with zero extrinsics and intrinsics OFF
	vec2 pp(sz.cx * 0.5f, sz.cy * 0.5f);
	Image preview_left = StereoCalibrationHelpers::ApplyExtrinsicsOnly(test_left, 0, 0, 0, pp);
	Image preview_right = StereoCalibrationHelpers::ApplyExtrinsicsOnly(test_right, 0, 0, 0, pp);

	// Verify identity (pixel-perfect match)
	bool left_identical = true;
	bool right_identical = true;
	int left_diff_count = 0;
	int right_diff_count = 0;
	int max_diff_left = 0;
	int max_diff_right = 0;

	for (int y = 0; y < sz.cy; y++) {
		for (int x = 0; x < sz.cx; x++) {
			RGBA orig_l = test_left[y][x];
			RGBA prev_l = preview_left[y][x];
			int diff_r = abs(orig_l.r - prev_l.r);
			int diff_g = abs(orig_l.g - prev_l.g);
			int diff_b = abs(orig_l.b - prev_l.b);
			int diff_a = abs(orig_l.a - prev_l.a);
			int max_diff = max(max(diff_r, diff_g), max(diff_b, diff_a));
			if (max_diff > 0) {
				left_identical = false;
				left_diff_count++;
				max_diff_left = max(max_diff_left, max_diff);
			}

			RGBA orig_r = test_right[y][x];
			RGBA prev_r = preview_right[y][x];
			diff_r = abs(orig_r.r - prev_r.r);
			diff_g = abs(orig_r.g - prev_r.g);
			diff_b = abs(orig_r.b - prev_r.b);
			diff_a = abs(orig_r.a - prev_r.a);
			max_diff = max(max(diff_r, diff_g), max(diff_b, diff_a));
			if (max_diff > 0) {
				right_identical = false;
				right_diff_count++;
				max_diff_right = max(max_diff_right, max_diff);
			}
		}
	}

	Cout() << "Results:\n";
	Cout() << "  Left eye:  " << (left_identical ? "✓ IDENTICAL" : "✗ DIFFERS") << "\n";
	if (!left_identical)
		Cout() << "    Diff pixels: " << left_diff_count << " / " << (sz.cx * sz.cy) << ", max diff: " << max_diff_left << "\n";
	Cout() << "  Right eye: " << (right_identical ? "✓ IDENTICAL" : "✗ DIFFERS") << "\n";
	if (!right_identical)
		Cout() << "    Diff pixels: " << right_diff_count << " / " << (sz.cx * sz.cy) << ", max diff: " << max_diff_right << "\n";

	if (!left_identical || !right_identical) {
		Cout() << "\n✗ FAIL: Identity NOT preserved at zero extrinsics\n";
		return 1;
	}

	Cout() << "\n✓ PASS: Identity preserved at zero extrinsics\n\n";

	// Test 2: Verify that non-zero extrinsics produce visible changes
	Cout() << "╔═══════════════════════════════════════════════════════════════════╗\n";
	Cout() << "║    Test 2: Extrinsics Preview Visibility (Non-Zero Values)      ║\n";
	Cout() << "╚═══════════════════════════════════════════════════════════════════╝\n\n";

	float test_yaw = 0.2f; // radians (approx 11.5 degrees)
	Cout() << "Test conditions:\n";
	Cout() << "  - preview_extrinsics: ON\n";
	Cout() << "  - preview_intrinsics: OFF\n";
	Cout() << Format("  - Left yaw: %.3f rad, Right yaw: 0\n\n", test_yaw);

	Image preview_left_yawed = StereoCalibrationHelpers::ApplyExtrinsicsOnly(test_left, test_yaw, 0, 0, pp);

	// Count differences
	int diff_count = 0;
	int max_diff = 0;
	for (int y = 0; y < sz.cy; y++) {
		for (int x = 0; x < sz.cx; x++) {
			RGBA orig = test_left[y][x];
			RGBA prev = preview_left_yawed[y][x];
			int diff_r = abs(orig.r - prev.r);
			int diff_g = abs(orig.g - prev.g);
			int diff_b = abs(orig.b - prev.b);
			int diff = max(diff_r, max(diff_g, diff_b));
			if (diff > 0) {
				diff_count++;
				max_diff = max(max_diff, diff);
			}
		}
	}

	Cout() << "Results:\n";
	Cout() << Format("  Diff pixels: %d / %d (%.1f%%)\n", diff_count, sz.cx * sz.cy, 100.0 * diff_count / (sz.cx * sz.cy));
	Cout() << Format("  Max diff: %d\n", max_diff);

	if (diff_count > (sz.cx * sz.cy / 100)) { // At least 1% of pixels should change
		Cout() << "\n✓ PASS: Extrinsics preview is visible (non-zero values produce changes)\n";
		return 0;
	} else {
		Cout() << "\n✗ FAIL: Extrinsics preview NOT visible (too few pixels changed)\n";
		return 1;
	}
}

END_UPP_NAMESPACE
