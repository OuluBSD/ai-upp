/*
 * GenerateStereoCalibSynth - Synthetic Stereo Calibration Dataset Generator
 *
 * Purpose:
 *   Generate ground-truth synthetic stereo calibration datasets for validating
 *   the GA intrinsics solver in StereoCalibrationTool.
 *
 * Outputs:
 *   - project.json (matches + line annotations compatible with StereoCalibrationTool)
 *   - ground_truth.json (all GT parameters: intrinsics + extrinsics + baseline)
 *   - captures/frame_*_l.png, frame_*_r.png (synthetic images with checkerboard pattern)
 *
 * Distortion Model:
 *   Uses the same polynomial model as StereoCalibrationSolver:
 *     r_distorted = a*theta + b*theta^2 + c*theta^3 + d*theta^4
 *   where:
 *     a ~ focal length f (approximately f for small angles)
 *     c = f * k1 (radial distortion coefficient)
 *     d = f * k2 (second radial distortion coefficient)
 *
 * Line Annotations:
 *   Generates straight lines in 3D space that appear curved in distorted images.
 *   After correct undistortion, these lines should become straight.
 *
 * Usage:
 *   ./bin/GenerateStereoCalibSynth --out share/calibration/test1 --seed 42
 */

#include <Core/Core.h>
#include <Geometry/Geometry.h>
#include <Draw/Draw.h>
#include <plugin/png/png.h>

using namespace Upp;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Ground truth parameters structure
struct GTParams {
	// Image dimensions
	int width = 640;
	int height = 480;

	// Baseline (eye distance) in millimeters
	double eye_dist_mm = 63.0;

	// Intrinsics (polynomial model: r = a*theta + b*theta^2 + c*theta^3 + d*theta^4)
	double focal_f = 400.0;  // pixels
	double cx = 320.0;        // principal point x
	double cy = 240.0;        // principal point y
	double k1 = -0.3;         // radial distortion coefficient
	double k2 = 0.05;         // second radial distortion coefficient

	// Polynomial coefficients (derived from f, k1, k2)
	double a = 400.0;  // ~ focal length
	double b = 0.0;    // typically 0 or small
	double c = -120.0; // f * k1
	double d = 20.0;   // f * k2

	// Extrinsics (degrees)
	double base_yaw = 0.0;
	double base_pitch = 0.0;
	double base_roll = 0.0;
	double toe_out = 10.0;     // toe-out yaw (left=-toe, right=+toe)
	double left_yaw = -10.0;
	double left_pitch = 0.0;
	double left_roll = 0.0;
	double right_yaw = 10.0;
	double right_pitch = 0.0;
	double right_roll = 0.0;

	void Jsonize(JsonIO& jio) {
		jio("width", width)("height", height);
		jio("eye_dist_mm", eye_dist_mm);
		jio("focal_f", focal_f)("cx", cx)("cy", cy);
		jio("k1", k1)("k2", k2);
		jio("a", a)("b", b)("c", c)("d", d);
		jio("base_yaw", base_yaw)("base_pitch", base_pitch)("base_roll", base_roll);
		jio("toe_out", toe_out);
		jio("left_yaw", left_yaw)("left_pitch", left_pitch)("left_roll", left_roll);
		jio("right_yaw", right_yaw)("right_pitch", right_pitch)("right_roll", right_roll);
	}
};

// Match pair (normalized [0,1] coordinates)
struct MatchPair : Moveable<MatchPair> {
	Pointf left;
	Pointf right;
	String left_text;
	String right_text;
	double dist_l = 0;  // mm
	double dist_r = 0;  // mm

	void Jsonize(JsonIO& jio) {
		jio("left", left)("right", right);
		jio("left_text", left_text)("right_text", right_text);
		jio("dist_l", dist_l)("dist_r", dist_r);
	}
};

// Frame structure
struct Frame : Moveable<Frame> {
	Time time;
	String source;
	int samples = 0;
	Vector<MatchPair> matches;
	Vector<Vector<Pointf>> lines_l;
	Vector<Vector<Pointf>> lines_r;

	void Jsonize(JsonIO& jio) {
		jio("time", time)("source", source)("samples", samples);
		jio("matches", matches);
		jio("lines_l", lines_l)("lines_r", lines_r);
	}
};

// Project structure
struct ProjectData : Moveable<ProjectData> {
	Vector<Frame> frames;

	void Jsonize(JsonIO& jio) {
		jio("frames", frames);
	}
};

// Helper: Convert degrees to radians
double DegToRad(double deg) {
	return deg * M_PI / 180.0;
}

// Helper: Angle-to-pixel using polynomial model
double AngleToPixel(const GTParams& gt, double theta) {
	double t2 = theta * theta;
	double t3 = t2 * theta;
	double t4 = t3 * theta;
	return gt.a * theta + gt.b * t2 + gt.c * t3 + gt.d * t4;
}

// Helper: Direction from theta and roll
vec3 DirectionFromThetaRoll(double theta, double roll) {
	double sin_theta = sin(theta);
	double cos_theta = cos(theta);
	double x = sin_theta * cos(roll);
	double y = -sin_theta * sin(roll);
	double z = IS_NEGATIVE_Z ? -cos_theta : cos_theta;
	return vec3((float)x, (float)y, (float)z);
}

// Helper: Theta and roll from direction
bool ThetaRollFromDirection(const vec3& dir, double& theta, double& roll, double& zf) {
	vec3 d = dir;
	d.Normalize();
	zf = IS_NEGATIVE_Z ? -d[2] : d[2];
	double xy = sqrt((double)d[0] * d[0] + (double)d[1] * d[1]);
	double zf_safe = fabs(zf) < 1e-9 ? (zf < 0 ? -1e-9 : 1e-9) : zf;
	theta = atan2(xy, zf_safe);
	roll = atan2(-d[1], d[0]);
	return zf > 0;
}

// Project a 3D point to pixel coordinates using GT parameters
// eye: 0=left, 1=right
// Returns false if point is behind camera
bool ProjectPoint(const GTParams& gt, const vec3& point, int eye, vec2& out_pix, double& zf_out) {
	// Camera position (left at -eye_dist/2, right at +eye_dist/2)
	double eye_dist_m = gt.eye_dist_mm / 1000.0;
	vec3 cam_center = eye == 0 ? vec3(-(float)eye_dist_m / 2.0f, 0, 0)
	                            : vec3((float)eye_dist_m / 2.0f, 0, 0);

	// Vector from camera to point
	vec3 v = point - cam_center;

	// Apply camera rotation
	double yaw_rad, pitch_rad, roll_rad;
	if (eye == 0) {
		yaw_rad = DegToRad(gt.left_yaw);
		pitch_rad = DegToRad(gt.left_pitch);
		roll_rad = DegToRad(gt.left_roll);
	} else {
		yaw_rad = DegToRad(gt.right_yaw);
		pitch_rad = DegToRad(gt.right_pitch);
		roll_rad = DegToRad(gt.right_roll);
	}

	mat4 rot = AxesMat((float)yaw_rad, (float)pitch_rad, (float)roll_rad);
	v = (rot.GetTransposed() * v.Embed()).Splice();

	// Check if point is too close
	if (v.GetLength() < 1e-9) {
		zf_out = 0;
		return false;
	}

	// Get direction
	vec3 dir = v.GetNormalized();
	double theta = 0, roll = 0, zf = 0;
	if (!ThetaRollFromDirection(dir, theta, roll, zf)) {
		zf_out = zf;
		return false;
	}

	// Apply distortion model
	double r = AngleToPixel(gt, theta);

	// Convert to pixel coordinates
	out_pix = vec2((float)(gt.cx + r * cos(roll)), (float)(gt.cy + r * sin(roll)));
	zf_out = zf;
	return zf > 0;
}

// Generate random 3D points in front of cameras
void GenerateRandomPoints(int count, Vector<vec3>& points, int seed) {
	SeedRandom((dword)seed);
	for (int i = 0; i < count; i++) {
		double x = (Randomf() * 0.8 - 0.4);  // -0.4 to 0.4 m
		double y = (Randomf() * 0.6 - 0.3);  // -0.3 to 0.3 m
		double z = (Randomf() * 2.1 + 0.4);  // 0.4 to 2.5 m
		if (IS_NEGATIVE_Z)
			z = -z;
		points.Add(vec3((float)x, (float)y, (float)z));
	}
}

// Generate straight line in 3D and project to both eyes
void GenerateStraightLine(const GTParams& gt, const vec3& start, const vec3& end,
                          int num_points, Vector<Pointf>& line_l, Vector<Pointf>& line_r) {
	for (int i = 0; i < num_points; i++) {
		float t = (float)i / (num_points - 1);
		vec3 pt = start * (1.0f - t) + end * t;

		vec2 pix_l, pix_r;
		double zf_l, zf_r;
		if (ProjectPoint(gt, pt, 0, pix_l, zf_l) && ProjectPoint(gt, pt, 1, pix_r, zf_r)) {
			if (zf_l > 0 && zf_r > 0) {
				// Check if pixels are within image bounds
				if (pix_l[0] >= 0 && pix_l[0] < gt.width && pix_l[1] >= 0 && pix_l[1] < gt.height &&
				    pix_r[0] >= 0 && pix_r[0] < gt.width && pix_r[1] >= 0 && pix_r[1] < gt.height) {
					line_l.Add(Pointf(pix_l[0] / gt.width, pix_l[1] / gt.height));
					line_r.Add(Pointf(pix_r[0] / gt.width, pix_r[1] / gt.height));
				}
			}
		}
	}
}

// Generate synthetic checkerboard pattern image
Image GenerateCheckerboardImage(int width, int height, int checker_size = 40) {
	ImageBuffer ib(width, height);
	for (int y = 0; y < height; y++) {
		RGBA* row = ib[y];
		for (int x = 0; x < width; x++) {
			int cx = x / checker_size;
			int cy = y / checker_size;
			bool black = ((cx + cy) & 1) == 0;
			byte val = black ? 40 : 200;
			row[x].r = val;
			row[x].g = val;
			row[x].b = val;
			row[x].a = 255;
		}
	}
	return ib;
}

// Generate random but plausible GT parameters
void GenerateRandomGT(GTParams& gt, int seed) {
	SeedRandom((dword)seed);

	// Baseline (55-75mm)
	gt.eye_dist_mm = 55.0 + Randomf() * 20.0;

	// FOV in range [90, 140] degrees
	double fov_deg = 90.0 + Randomf() * 50.0;
	double fov_rad = DegToRad(fov_deg);
	gt.focal_f = (gt.width * 0.5) / tan(fov_rad * 0.5);

	// Principal point (slight offset from center)
	gt.cx = gt.width * 0.5 + (Randomf() * 30.0 - 15.0);
	gt.cy = gt.height * 0.5 + (Randomf() * 30.0 - 15.0);

	// Radial distortion (barrel convention: negative k1)
	gt.k1 = -0.8 + Randomf() * 0.75;  // [-0.8, -0.05]
	gt.k2 = -0.2 + Randomf() * 0.4;   // [-0.2, 0.2]

	// Polynomial coefficients
	gt.a = gt.focal_f;
	gt.b = 0.0;  // Keep b=0 for simplicity
	gt.c = gt.focal_f * gt.k1;
	gt.d = gt.focal_f * gt.k2;

	// Extrinsics: base offset + toe-out
	gt.base_yaw = (Randomf() * 20.0 - 10.0);    // [-10, 10]
	gt.base_pitch = (Randomf() * 20.0 - 10.0);  // [-10, 10]
	gt.base_roll = (Randomf() * 20.0 - 10.0);   // [-10, 10]
	gt.toe_out = Randomf() * 25.0;               // [0, 25]

	// Asymmetry deltas
	double delta_yaw = (Randomf() * 6.0 - 3.0);
	double delta_pitch = (Randomf() * 6.0 - 3.0);
	double delta_roll = (Randomf() * 6.0 - 3.0);

	gt.left_yaw = gt.base_yaw - gt.toe_out + delta_yaw;
	gt.left_pitch = gt.base_pitch + delta_pitch;
	gt.left_roll = gt.base_roll + delta_roll;

	gt.right_yaw = gt.base_yaw + gt.toe_out - delta_yaw;
	gt.right_pitch = gt.base_pitch - delta_pitch;
	gt.right_roll = gt.base_roll - delta_roll;
}

CONSOLE_APP_MAIN
{
	const Vector<String>& cmdline = CommandLine();
	String out_dir;
	int seed = 12345;
	int num_frames = 3;
	int num_matches = 20;
	int num_lines = 6;

	for (int i = 0; i < cmdline.GetCount(); i++) {
		if (cmdline[i] == "--out" && i + 1 < cmdline.GetCount())
			out_dir = cmdline[++i];
		else if (cmdline[i].StartsWith("--out="))
			out_dir = cmdline[i].Mid(6);
		else if (cmdline[i] == "--seed" && i + 1 < cmdline.GetCount())
			seed = atoi(cmdline[++i]);
		else if (cmdline[i].StartsWith("--seed="))
			seed = atoi(cmdline[i].Mid(7));
		else if (cmdline[i] == "--frames" && i + 1 < cmdline.GetCount())
			num_frames = atoi(cmdline[++i]);
		else if (cmdline[i] == "--matches" && i + 1 < cmdline.GetCount())
			num_matches = atoi(cmdline[++i]);
		else if (cmdline[i] == "--lines" && i + 1 < cmdline.GetCount())
			num_lines = atoi(cmdline[++i]);
	}

	if (out_dir.IsEmpty()) {
		Cerr() << "Usage: GenerateStereoCalibSynth --out <dir> [--seed <n>] [--frames <n>] [--matches <n>] [--lines <n>]\n";
		SetExitCode(1);
		return;
	}

	// Generate ground truth parameters
	GTParams gt;
	GenerateRandomGT(gt, seed);

	Cout() << "Generating synthetic stereo calibration dataset...\n";
	Cout() << Format("  Output: %s\n", out_dir);
	Cout() << Format("  Seed: %d\n", seed);
	Cout() << Format("  Image size: %dx%d\n", gt.width, gt.height);
	Cout() << Format("  Baseline: %.2f mm\n", gt.eye_dist_mm);
	Cout() << Format("  Focal: %.2f px, cx=%.1f, cy=%.1f\n", gt.focal_f, gt.cx, gt.cy);
	Cout() << Format("  Distortion: k1=%.4f, k2=%.4f\n", gt.k1, gt.k2);
	Cout() << Format("  Extrinsics L: yaw=%.2f, pitch=%.2f, roll=%.2f\n", gt.left_yaw, gt.left_pitch, gt.left_roll);
	Cout() << Format("  Extrinsics R: yaw=%.2f, pitch=%.2f, roll=%.2f\n", gt.right_yaw, gt.right_pitch, gt.right_roll);

	// Create output directory
	RealizeDirectory(out_dir);
	String captures_dir = AppendFileName(out_dir, "captures");
	RealizeDirectory(captures_dir);

	// Generate project
	ProjectData dataset;

	// Generate frames
	for (int frame_idx = 0; frame_idx < num_frames; frame_idx++) {
		Frame frame;
		frame.time = GetSysTime();
		frame.source = "Synthetic Generator";
		frame.samples = num_matches;

		// Generate random 3D points for matches
		Vector<vec3> points;
		GenerateRandomPoints(num_matches * 2, points, seed + frame_idx * 1000);

		// Project points and create matches
		for (int i = 0; i < points.GetCount() && frame.matches.GetCount() < num_matches; i++) {
			vec2 pix_l, pix_r;
			double zf_l, zf_r;
			if (!ProjectPoint(gt, points[i], 0, pix_l, zf_l))
				continue;
			if (!ProjectPoint(gt, points[i], 1, pix_r, zf_r))
				continue;
			if (zf_l <= 0 || zf_r <= 0)
				continue;

			// Check bounds
			if (pix_l[0] < 0 || pix_l[0] >= gt.width || pix_l[1] < 0 || pix_l[1] >= gt.height)
				continue;
			if (pix_r[0] < 0 || pix_r[0] >= gt.width || pix_r[1] < 0 || pix_r[1] >= gt.height)
				continue;

			MatchPair m;
			m.left = Pointf(pix_l[0] / gt.width, pix_l[1] / gt.height);
			m.right = Pointf(pix_r[0] / gt.width, pix_r[1] / gt.height);
			m.left_text = Format("L%d", frame.matches.GetCount() + 1);
			m.right_text = Format("R%d", frame.matches.GetCount() + 1);

			// Compute distances in mm
			vec3 cam_l(-(float)gt.eye_dist_mm / 2000.0f, 0, 0);
			vec3 cam_r((float)gt.eye_dist_mm / 2000.0f, 0, 0);
			m.dist_l = (points[i] - cam_l).GetLength() * 1000.0;
			m.dist_r = (points[i] - cam_r).GetLength() * 1000.0;

			frame.matches.Add(m);
		}

		// Generate line annotations (straight lines in 3D that appear curved in distorted image)
		for (int line_idx = 0; line_idx < num_lines; line_idx++) {
			vec3 start, end;
			int attempts = 0;
			bool valid_line = false;

			while (attempts < 50 && !valid_line) {
				attempts++;
				// Generate random line endpoints
				double x1 = (Randomf() * 0.6 - 0.3);
				double y1 = (Randomf() * 0.4 - 0.2);
				double z1 = (Randomf() * 1.5 + 0.5);
				double x2 = (Randomf() * 0.6 - 0.3);
				double y2 = (Randomf() * 0.4 - 0.2);
				double z2 = (Randomf() * 1.5 + 0.5);
				if (IS_NEGATIVE_Z) { z1 = -z1; z2 = -z2; }

				start = vec3((float)x1, (float)y1, (float)z1);
				end = vec3((float)x2, (float)y2, (float)z2);

				Vector<Pointf> line_l, line_r;
				GenerateStraightLine(gt, start, end, 12, line_l, line_r);

				if (line_l.GetCount() >= 4 && line_r.GetCount() >= 4) {
					frame.lines_l.Add(pick(line_l));
					frame.lines_r.Add(pick(line_r));
					valid_line = true;
				}
			}
		}

		// Generate synthetic images (simple checkerboard for visualization)
		Image img_l = GenerateCheckerboardImage(gt.width, gt.height);
		Image img_r = GenerateCheckerboardImage(gt.width, gt.height);

		// Save images
		String left_path = AppendFileName(captures_dir, Format("frame_%d_l.png", frame_idx));
		String right_path = AppendFileName(captures_dir, Format("frame_%d_r.png", frame_idx));
		PNGEncoder().SaveFile(left_path, img_l);
		PNGEncoder().SaveFile(right_path, img_r);

		Cout() << Format("  Frame %d: %d matches, %d+%d lines\n",
		                 frame_idx, frame.matches.GetCount(),
		                 frame.lines_l.GetCount(), frame.lines_r.GetCount());

		dataset.frames.Add(pick(frame));
	}

	// Save project.json
	String project_path = AppendFileName(out_dir, "project.json");
	Json json;
	json("frames", StoreAsJsonValue(dataset.frames));
	String json_str = json;
	SaveFile(project_path, json_str);
	Cout() << Format("  Saved: %s\n", project_path);

	// Save ground_truth.json
	String gt_path = AppendFileName(out_dir, "ground_truth.json");
	String gt_json_str = StoreAsJson(gt, true);
	SaveFile(gt_path, gt_json_str);
	Cout() << Format("  Saved: %s\n", gt_path);

	Cout() << "Dataset generation complete!\n";
	SetExitCode(0);
}
