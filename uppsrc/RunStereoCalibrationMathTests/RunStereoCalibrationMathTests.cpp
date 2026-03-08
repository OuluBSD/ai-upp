#include "../StereoCalibrationTool/StereoCalibrationTool.h"
#include <Geometry/Camera.h>

using namespace Upp;

// ============================================================================
// DEEP DIAGNOSTICS: First-Divergence Detection
// ============================================================================

struct DiagnosticContext {
	bool verbose = false;
	bool trace_pixels = false;
	int failures = 0;

	void PrintHeader(const String& title) {
		Cout() << "\n" << String('=', 70) << "\n";
		Cout() << title << "\n";
		Cout() << String('=', 70) << "\n";
	}

	void PrintSeparator() {
		Cout() << String('-', 70) << "\n";
	}
};

// ============================================================================
// Matrix and Math Invariant Checking
// ============================================================================

struct MatrixDiagnostics {
	static bool CheckIdentity(const mat4& m, double epsilon, DiagnosticContext& ctx) {
		double max_abs_diff = 0.0;
		bool is_identity = true;

		for(int r = 0; r < 4; r++) {
			for(int c = 0; c < 4; c++) {
				double expected = (r == c) ? 1.0 : 0.0;
				double diff = abs(m[r][c] - expected);
				max_abs_diff = max(max_abs_diff, diff);

				if(diff > epsilon) {
					if(ctx.verbose) {
						Cout() << "  Matrix[" << r << "][" << c << "]: expected=" << expected
						       << ", actual=" << m[r][c] << ", diff=" << diff << "\n";
					}
					is_identity = false;
				}
			}
		}

		if(ctx.verbose) {
			Cout() << "  max_abs(M - I) = " << max_abs_diff << "\n";
		}

		return is_identity;
	}

	static double ComputeDeterminant(const mat4& m) {
		// Simple determinant for 4x4 (simplified for diagnostics)
		// For a rotation matrix, det should be +1 or -1
		mat3 upper_left;
		for(int r = 0; r < 3; r++) {
			for(int c = 0; c < 3; c++) {
				upper_left[r][c] = m[r][c];
			}
		}
		return Determinant(upper_left);
	}

	static void CheckOrthonormality(const mat4& m, DiagnosticContext& ctx) {
		// For rotation matrix: R^T * R = I
		mat4 rt = m.GetTransposed();
		mat4 rt_r = rt * m;

		double max_ortho_err = 0.0;
		for(int r = 0; r < 3; r++) {
			for(int c = 0; c < 3; c++) {
				double expected = (r == c) ? 1.0 : 0.0;
				double diff = abs(rt_r[r][c] - expected);
				max_ortho_err = max(max_ortho_err, diff);
			}
		}

		double det = ComputeDeterminant(m);

		if(ctx.verbose) {
			Cout() << "  Orthonormality: ||R^T*R - I||_inf = " << max_ortho_err << "\n";
			Cout() << "  Determinant: det(R) = " << det << "\n";
		}
	}

	static void PrintMatrix(const String& name, const mat4& m) {
		Cout() << name << ":\n";
		for(int r = 0; r < 4; r++) {
			Cout() << "  [ ";
			for(int c = 0; c < 4; c++) {
				Cout() << Format("%9.6f ", m[r][c]);
			}
			Cout() << "]\n";
		}
	}
};

// ============================================================================
// Pixel-level Diagnostics
// ============================================================================

struct PixelDiagnostics {
	struct PixelTrace {
		Point input_coord;
		RGBA input_color;
		vec3 ray_camera;
		vec3 ray_rotated;
		vec2 projected_uv;
		RGBA sampled_color;
		RGBA output_color;
		double max_diff;
	};

	static void TracePixel(
		const Point& p,
		const Image& src,
		const LensPoly& lens,
		const mat4& rot,
		const mat4& rot_inv,
		float linear_scale,
		int lens_i,
		DiagnosticContext& ctx,
		PixelTrace& trace)
	{
		Size sz = src.GetSize();
		float cx = sz.cx * 0.5f, cy = sz.cy * 0.5f;

		trace.input_coord = p;
		trace.input_color = src[p.y][p.x];

		// Backproject pixel to ray
		float dx = p.x - cx, dy = p.y - cy;
		float r = sqrtf(dx * dx + dy * dy);

		if(r < 1e-6f) {
			trace.ray_camera = vec3(0, 0, 1);
		} else {
			float angle = r / linear_scale;
			float roll_angle = atan2(dy, dx);
			float sin_theta = sin(angle);
			float cos_theta = cos(angle);
			trace.ray_camera = vec3(
				sin_theta * cos(roll_angle),
				sin_theta * sin(roll_angle),
				cos_theta
			);
		}

		// Apply rotation
		trace.ray_rotated = (rot_inv * trace.ray_camera.Embed()).Splice();

		// Project back
		trace.projected_uv = const_cast<LensPoly&>(lens).Project(lens_i, GetDirAxes(trace.ray_rotated).Splice());

		// Sample (simplified bilinear)
		trace.sampled_color = SampleBilinear(src, trace.projected_uv[0], trace.projected_uv[1]);

		// Compute error
		trace.max_diff = max(
			max(abs(trace.input_color.r - trace.sampled_color.r),
			    abs(trace.input_color.g - trace.sampled_color.g)),
			abs(trace.input_color.b - trace.sampled_color.b)
		);
	}

	static RGBA SampleBilinear(const Image& img, float x, float y) {
		Size sz = img.GetSize();
		if(sz.cx <= 0 || sz.cy <= 0) return RGBA{0,0,0,0};

		x = Clamp(x, 0.0f, (float)(sz.cx - 1));
		y = Clamp(y, 0.0f, (float)(sz.cy - 1));

		int x0 = (int)floor(x), y0 = (int)floor(y);
		int x1 = min(x0 + 1, sz.cx - 1), y1 = min(y0 + 1, sz.cy - 1);
		float fx = x - x0, fy = y - y0;

		const RGBA *row0 = img[y0], *row1 = img[y1];
		auto lerp = [](float a, float b, float t) { return a + (b - a) * t; };

		RGBA c00 = row0[x0], c10 = row0[x1], c01 = row1[x0], c11 = row1[x1];
		RGBA out;
		out.r = (byte)Clamp((int)lerp(lerp(c00.r, c10.r, fx), lerp(c01.r, c11.r, fx), fy), 0, 255);
		out.g = (byte)Clamp((int)lerp(lerp(c00.g, c10.g, fx), lerp(c01.g, c11.g, fx), fy), 0, 255);
		out.b = (byte)Clamp((int)lerp(lerp(c00.b, c10.b, fx), lerp(c01.b, c11.b, fx), fy), 0, 255);
		out.a = (byte)Clamp((int)lerp(lerp(c00.a, c10.a, fx), lerp(c01.a, c11.a, fx), fy), 0, 255);

		return out;
	}

	static void PrintTrace(const PixelTrace& t, const String& eye_name) {
		Cout() << "\n  === Pixel Trace [" << eye_name << "]: (" << t.input_coord.x << "," << t.input_coord.y << ") ===\n";
		Cout() << "  Input color: RGB(" << (int)t.input_color.r << "," << (int)t.input_color.g << "," << (int)t.input_color.b << ")\n";
		Cout() << "  Ray (camera): (" << t.ray_camera[0] << ", " << t.ray_camera[1] << ", " << t.ray_camera[2] << ")\n";
		Cout() << "  Ray (rotated): (" << t.ray_rotated[0] << ", " << t.ray_rotated[1] << ", " << t.ray_rotated[2] << ")\n";
		Cout() << "  Projected UV: (" << t.projected_uv[0] << ", " << t.projected_uv[1] << ")\n";
		Cout() << "  Sampled color: RGB(" << (int)t.sampled_color.r << "," << (int)t.sampled_color.g << "," << (int)t.sampled_color.b << ")\n";
		Cout() << "  Max channel diff: " << t.max_diff << "\n";
	}
};

// ============================================================================
// Mock StereoCalibrationMath for Testing
// ============================================================================

struct MockMath : public StereoCalibrationMath {
	int stage = 0;
	bool preview_extrinsics = true;
	bool preview_intrinsics = false;
	double yaw_l = 0, pitch_l = 0, roll_l = 0;
	double yaw_r = 0, pitch_r = 0, roll_r = 0;

	int GetCurrentStage() const override { return stage; }
	bool IsPreviewExtrinsics() const override { return preview_extrinsics; }
	bool IsPreviewIntrinsics() const override { return preview_intrinsics; }

	void GetStageAExtrinsics(Eye eye, double& yaw, double& pitch, double& roll) const override {
		if(eye == Eye::Left) {
			yaw = yaw_l; pitch = pitch_l; roll = roll_l;
		} else {
			yaw = yaw_r; pitch = pitch_r; roll = roll_r;
		}
	}

	void GetSolvedExtrinsics(Eye eye, double& yaw, double& pitch, double& roll) const override {
		yaw = 0; pitch = 0; roll = 0;
	}
};

// ============================================================================
// TEST: AxesMat(0,0,0) is Identity
// ============================================================================

bool Test_AxesMat_Identity(DiagnosticContext& ctx) {
	ctx.PrintHeader("TEST A: AxesMat(0,0,0) -> Identity Matrix");

	mat4 r = AxesMat(0.0f, 0.0f, 0.0f);

	if(ctx.verbose) {
		MatrixDiagnostics::PrintMatrix("AxesMat(0,0,0)", r);
	}

	bool pass = MatrixDiagnostics::CheckIdentity(r, 1e-6, ctx);
	MatrixDiagnostics::CheckOrthonormality(r, ctx);

	if(pass) {
		Cout() << "PASSED: AxesMat(0,0,0) is identity within tolerance 1e-6\n";
	} else {
		Cout() << "FAILED: AxesMat(0,0,0) is NOT identity\n";
		ctx.failures++;
	}

	return pass;
}

// ============================================================================
// TEST: Stage A Preview at Zero Extrinsics is No-Op
// ============================================================================

Image CreateSyntheticTestImage(int width, int height) {
	ImageBuffer ib(width, height);

	// Create a gradient + high-frequency grid + corner markers
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			byte base_r = (byte)((x * 255) / width);
			byte base_g = (byte)((y * 255) / height);
			byte base_b = (byte)(((x + y) * 255) / (width + height));

			// Add high-frequency grid
			if((x % 10 == 0) || (y % 10 == 0)) {
				base_r = 255;
				base_g = 255;
				base_b = 255;
			}

			// Corner markers
			if((x < 5 && y < 5) || (x >= width-5 && y < 5) ||
			   (x < 5 && y >= height-5) || (x >= width-5 && y >= height-5)) {
				base_r = 255; base_g = 0; base_b = 0;
			}

			// Center cross
			if(abs(x - width/2) < 2 || abs(y - height/2) < 2) {
				base_r = 0; base_g = 255; base_b = 0;
			}

			ib[y][x] = RGBA(base_r, base_g, base_b, 255);
		}
	}

	return ib;
}

bool Test_StageA_ZeroExtrinsics_Identity(DiagnosticContext& ctx) {
	ctx.PrintHeader("TEST B: Stage A Preview at Zero Extrinsics -> Identity Mapping");

	// Setup test environment
	MockMath math;
	math.stage = 0;  // Stage A
	math.preview_extrinsics = true;
	math.preview_intrinsics = false;
	math.yaw_l = 0; math.pitch_l = 0; math.roll_l = 0;
	math.yaw_r = 0; math.pitch_r = 0; math.roll_r = 0;

	// Create lens
	int test_width = 640;
	int test_height = 480;
	LensPoly test_lens;

	// Simple linear lens for testing (a=100, no distortion)
	test_lens.SetAnglePixel(100.0f, 0.0f, 0.0f, 0.0f);
	test_lens.SetSize(Size(test_width, test_height));
	test_lens.SetPrincipalPoint(test_width / 2.0f, test_height / 2.0f);

	Cout() << "Test Image: " << test_width << "x" << test_height << "\n";
	Cout() << "Lens: a=100, b=c=d=0 (linear)\n";
	Cout() << "Principal Point: (" << (test_width/2) << ", " << (test_height/2) << ")\n";

	// Create synthetic test image
	Image src = CreateSyntheticTestImage(test_width, test_height);

	// Test both eyes
	struct EyeTest {
		StereoCalibrationMath::Eye eye;
		String name;
	};

	EyeTest eyes[] = {
		{StereoCalibrationMath::Eye::Left, "LEFT"},
		{StereoCalibrationMath::Eye::Right, "RIGHT"}
	};

	bool all_pass = true;

	for(auto& eye_test : eyes) {
		ctx.PrintSeparator();
		Cout() << "Testing Eye: " << eye_test.name << "\n";

		// Render preview
		Image dst = math.RenderEyePreview(eye_test.eye, src, test_lens, ctx.verbose);

		if(dst.IsEmpty()) {
			Cout() << "FAILED: RenderEyePreview returned empty image for " << eye_test.name << "\n";
			ctx.failures++;
			all_pass = false;
			continue;
		}

		// Compare pixels
		int total_pixels = test_width * test_height;
		int pixels_checked = 0;
		int pixels_different = 0;
		double max_diff = 0.0;
		Point first_fail_coord = Null;
		RGBA first_fail_src, first_fail_dst;

		// Sample strategy: check corners, center, and regular grid
		Vector<Point> sample_points;

		// Corners
		sample_points.Add(Point(5, 5));
		sample_points.Add(Point(test_width-6, 5));
		sample_points.Add(Point(5, test_height-6));
		sample_points.Add(Point(test_width-6, test_height-6));

		// Center
		sample_points.Add(Point(test_width/2, test_height/2));

		// Grid sampling (every 20 pixels)
		for(int y = 10; y < test_height; y += 20) {
			for(int x = 10; x < test_width; x += 20) {
				sample_points.Add(Point(x, y));
			}
		}

		for(const Point& p : sample_points) {
			RGBA c_src = src[p.y][p.x];
			RGBA c_dst = dst[p.y][p.x];

			double diff = max(max(abs(c_src.r - c_dst.r), abs(c_src.g - c_dst.g)), abs(c_src.b - c_dst.b));
			max_diff = max(max_diff, diff);

			// Allow 1 intensity unit for bilinear rounding, but not spatial shift
			if(diff > 1.0) {
				pixels_different++;
				if(IsNull(first_fail_coord)) {
					first_fail_coord = p;
					first_fail_src = c_src;
					first_fail_dst = c_dst;
				}
			}
			pixels_checked++;
		}

		double fail_rate = (double)pixels_different / pixels_checked;

		Cout() << "  Pixels checked: " << pixels_checked << "\n";
		Cout() << "  Pixels different (>1): " << pixels_different << "\n";
		Cout() << "  Fail rate: " << Format("%.2f%%", fail_rate * 100) << "\n";
		Cout() << "  Max pixel diff: " << max_diff << "\n";

		// Acceptance: <= 0.01% pixels with diff > 1
		bool eye_pass = (fail_rate <= 0.0001);

		if(!eye_pass || ctx.trace_pixels) {
			if(!IsNull(first_fail_coord)) {
				Cout() << "  First failure at (" << first_fail_coord.x << "," << first_fail_coord.y << ")\n";
				Cout() << "    Expected: RGB(" << (int)first_fail_src.r << "," << (int)first_fail_src.g << "," << (int)first_fail_src.b << ")\n";
				Cout() << "    Got:      RGB(" << (int)first_fail_dst.r << "," << (int)first_fail_dst.g << "," << (int)first_fail_dst.b << ")\n";

				// Deep trace for first failure
				if(ctx.trace_pixels) {
					mat4 rot = AxesMat(0.0f, 0.0f, 0.0f);
					mat4 rot_inv = rot.GetTransposed();
					Size sz = src.GetSize();
					float max_radius = (float)sqrt(sz.cx * sz.cx * 0.25f + sz.cy * sz.cy * 0.25f);
					float max_angle = test_lens.PixelToAngle(max_radius);
					float linear_scale = max_angle > 1e-6f ? max_radius / max_angle : 1.0f;

					PixelDiagnostics::PixelTrace trace;
					PixelDiagnostics::TracePixel(
						first_fail_coord, src, test_lens, rot, rot_inv,
						linear_scale, (int)eye_test.eye, ctx, trace
					);
					PixelDiagnostics::PrintTrace(trace, eye_test.name);
				}
			}
		}

		if(eye_pass) {
			Cout() << "PASSED: " << eye_test.name << " eye identity verified\n";
		} else {
			Cout() << "FAILED: " << eye_test.name << " eye has non-identity mapping\n";
			ctx.failures++;
			all_pass = false;
		}
	}

	return all_pass;
}

// ============================================================================
// TEST: Lens Project/Unproject at lens_i=0 and lens_i=1 with zero extrinsics
// ============================================================================

bool Test_LensProject_Symmetry(DiagnosticContext& ctx) {
	ctx.PrintHeader("TEST C: LensPoly::Project Symmetry at Zero Extrinsics");

	LensPoly lens;
	lens.SetAnglePixel(100.0f, 0.0f, 0.0f, 0.0f);
	lens.SetSize(Size(640, 480));
	lens.SetPrincipalPoint(320, 240);
	lens.SetEyeOutwardAngle(0.0f);
	lens.SetRightTilt(0.0f, 0.0f);

	// Test: same direction should give same pixel for both eyes when extrinsics are zero
	axes2 test_dir = {0.0f, 0.0f};  // Forward direction

	vec2 px_left = lens.Project(0, test_dir);
	vec2 px_right = lens.Project(1, test_dir);

	Cout() << "Test direction: yaw=0, pitch=0 (forward)\n";
	Cout() << "  Left  eye projects to: (" << px_left[0] << ", " << px_left[1] << ")\n";
	Cout() << "  Right eye projects to: (" << px_right[0] << ", " << px_right[1] << ")\n";

	double diff = sqrt((px_left[0] - px_right[0]) * (px_left[0] - px_right[0]) +
	                   (px_left[1] - px_right[1]) * (px_left[1] - px_right[1]));

	Cout() << "  Pixel difference: " << diff << "\n";

	bool pass = (diff < 0.1);  // Should be nearly identical

	if(pass) {
		Cout() << "PASSED: Both eyes project identically at zero extrinsics\n";
	} else {
		Cout() << "FAILED: Eyes project differently despite zero extrinsics\n";
		Cout() << "  This indicates right-eye special-casing in LensPoly::Project\n";
		ctx.failures++;
	}

	return pass;
}

// ============================================================================
// MAIN
// ============================================================================

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);

	Cout() << "\n";
	Cout() << "╔═══════════════════════════════════════════════════════════════════╗\n";
	Cout() << "║  Stereo Calibration Math Tests - Identity Invariant Validation   ║\n";
	Cout() << "╚═══════════════════════════════════════════════════════════════════╝\n";

	// Parse command-line args
	DiagnosticContext ctx;
	ctx.verbose = false;
	ctx.trace_pixels = false;

	const Vector<String>& args = CommandLine();
	for(const String& arg : args) {
		if(arg == "--verbose" || arg == "-v") {
			ctx.verbose = true;
			Cout() << "[Verbose mode enabled]\n";
		}
		if(arg == "--trace_preview_math" || arg == "--trace") {
			ctx.trace_pixels = true;
			ctx.verbose = true;
			Cout() << "[Deep pixel tracing enabled]\n";
		}
	}

	// Run tests
	bool pass_a = Test_AxesMat_Identity(ctx);
	bool pass_b = Test_StageA_ZeroExtrinsics_Identity(ctx);
	bool pass_c = Test_LensProject_Symmetry(ctx);

	// Summary
	ctx.PrintHeader("SUMMARY");
	Cout() << "Test A (AxesMat Identity):        " << (pass_a ? "PASS" : "FAIL") << "\n";
	Cout() << "Test B (Stage A No-Op):           " << (pass_b ? "PASS" : "FAIL") << "\n";
	Cout() << "Test C (LensProject Symmetry):    " << (pass_c ? "PASS" : "FAIL") << "\n";
	Cout() << "\n";

	if(ctx.failures == 0) {
		Cout() << "✓ ALL TESTS PASSED\n";
		SetExitCode(0);
	} else {
		Cout() << "✗ " << ctx.failures << " FAILURE(S) DETECTED\n";
		Cout() << "\n";
		Cout() << "NEXT STEPS:\n";
		Cout() << "1. Run with --trace_preview_math for deep pixel-level diagnostics\n";
		Cout() << "2. Check LensPoly::Project for right-eye special-casing (lens_i==1)\n";
		Cout() << "3. Verify UndistortImage doesn't apply rotation when yaw=pitch=roll=0\n";
		Cout() << "4. Add fast-path copy for identity case\n";
		SetExitCode(1);
	}
}
