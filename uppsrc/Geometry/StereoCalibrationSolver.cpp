#include "Geometry.h"
#include <plugin/Eigen/Eigen.h>
#include <AI/Core/Base/Base.h>

NAMESPACE_UPP

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void StereoCalibrationTrace::Add(const String& s) {
	if (!enabled || s.IsEmpty())
		return;
	int64 sz = s.GetCount();
	if (total_bytes + sz > max_bytes || lines.GetCount() >= max_lines) {
		if (lines.IsEmpty() || lines.Top() != "... (trace truncated)\n") {
			lines.Add("... (trace truncated)\n");
			total_bytes += 24;
		}
		return;
	}
	lines.Add(s);
	total_bytes += sz;
}

void StereoCalibrationTrace::Addf(const char* fmt, ...) {
	if (!enabled)
		return;
	va_list args;
	va_start(args, fmt);
	char buffer[4096];
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	Add(String(buffer));
}

String StereoCalibrationTrace::GetText() const {
	String out;
	for (const String& line : lines)
		out << line;
	return out;
}

void StereoCalibrationTrace::Clear() {
	lines.Clear();
	total_bytes = 0;
}

void StereoCalibrationSolver::EnableTrace(bool on, int verbosity, int max_lines) {
	trace.enabled = on;
	trace.verbosity = verbosity;
	trace.max_lines = max_lines;
	trace.Clear();
}

static inline double HuberWeight(double r, double delta) {
	if (delta <= 0)
		return 1.0;
	double a = fabs(r);
	if (a <= delta)
		return 1.0;
	return sqrt(delta / a);
}

static inline vec3 RotateVec(const mat4& m, const vec3& v) {
	return (m * v.Embed()).Splice();
}

static inline mat4 RightRotation(const StereoCalibrationParams& p) {
	return AxesMat((float)p.yaw, (float)p.pitch, (float)p.roll);
}

static inline mat4 RightRotationInv(const StereoCalibrationParams& p) {
	mat4 rot = RightRotation(p);
	return rot.GetTransposed();
}

static inline vec3 DirectionFromThetaRoll(double theta, double roll) {
	double sin_theta = sin(theta);
	double cos_theta = cos(theta);
	double x = sin_theta * cos(roll);
	double y = -sin_theta * sin(roll);
	double z = IS_NEGATIVE_Z ? -cos_theta : cos_theta;
	return vec3((float)x, (float)y, (float)z);
}

static inline bool ThetaRollFromDirection(const vec3& dir, double& theta, double& roll, double& zf) {
	vec3 d = dir;
	d.Normalize();
	zf = IS_NEGATIVE_Z ? -d[2] : d[2];
	double xy = sqrt((double)d[0] * d[0] + (double)d[1] * d[1]);
	double zf_safe = fabs(zf) < 1e-9 ? (zf < 0 ? -1e-9 : 1e-9) : zf;
	theta = atan2(xy, zf_safe);
	roll = atan2(-d[1], d[0]);
	return zf > 0;
}

static inline double AngleToPixel(const StereoCalibrationParams& p, double theta) {
	double t2 = theta * theta;
	double t3 = t2 * theta;
	double t4 = t3 * theta;
	return p.a * theta + p.b * t2 + p.c * t3 + p.d * t4;
}

static inline double PixelToAngle(const StereoCalibrationParams& p, double r) {
	if (r <= 0)
		return 0.0;
	double denom = fabs(p.a) > 1e-9 ? p.a : 1e-9;
	double theta = r / denom;
	for (int it = 0; it < 6; it++) {
		double t2 = theta * theta;
		double t3 = t2 * theta;
		double t4 = t3 * theta;
		double f = p.a * theta + p.b * t2 + p.c * t3 + p.d * t4 - r;
		double df = p.a + 2.0 * p.b * theta + 3.0 * p.c * t2 + 4.0 * p.d * t3;
		if (fabs(df) < 1e-9)
			break;
		theta -= f / df;
	}
	return theta;
}

static inline bool PixelToAngleTrace(const StereoCalibrationParams& p, double r, double& theta_out, StereoCalibrationTrace* trace) {
	if (r <= 0) {
		theta_out = 0.0;
		if (trace && trace->enabled && trace->verbosity >= 3)
			trace->Addf("      PixelToAngle: r=%.4f <= 0, returning theta=0\n", r);
		return true;
	}
	double denom = fabs(p.a) > 1e-9 ? p.a : 1e-9;
	double theta = r / denom;
	if (trace && trace->enabled && trace->verbosity >= 3)
		trace->Addf("      PixelToAngle: r=%.4f, initial theta=%.6f (r/a)\n", r, theta);

	bool converged = false;
	for (int it = 0; it < 6; it++) {
		double t2 = theta * theta;
		double t3 = t2 * theta;
		double t4 = t3 * theta;
		double f = p.a * theta + p.b * t2 + p.c * t3 + p.d * t4 - r;
		double df = p.a + 2.0 * p.b * theta + 3.0 * p.c * t2 + 4.0 * p.d * t3;
		if (trace && trace->enabled && trace->verbosity >= 3)
			trace->Addf("        Newton it %d: theta=%.6f, f=%.6f, df=%.6f\n", it, theta, f, df);
		if (fabs(df) < 1e-9) {
			if (trace && trace->enabled && trace->verbosity >= 3)
				trace->Add("        derivative too small, stopping\n");
			break;
		}
		double delta = f / df;
		theta -= delta;
		if (fabs(delta) < 1e-9 && fabs(f) < 1e-6) {
			converged = true;
			if (trace && trace->enabled && trace->verbosity >= 3)
				trace->Addf("        converged at it %d\n", it);
			break;
		}
	}
	theta_out = theta;
	if (!converged && trace && trace->enabled && trace->verbosity >= 2)
		trace->Addf("      WARNING: PixelToAngle did not converge for r=%.4f, final theta=%.6f\n", r, theta);
	return converged || fabs(theta) < M_PI;
}

static bool UnprojectDir(const StereoCalibrationParams& p, const vec2& pix, int eye, vec3& dir_head) {
	double dx = pix[0] - p.cx;
	double dy = pix[1] - p.cy;
	double r = sqrt(dx * dx + dy * dy);
	if (r < 1e-9) {
		dir_head = vec3(0, 0, IS_NEGATIVE_Z ? -1.0f : 1.0f);
		return true;
	}
	double theta = PixelToAngle(p, r);
	double roll = atan2(dy, dx);
	vec3 dir_cam = DirectionFromThetaRoll(theta, roll);
	if (eye == 1)
		dir_head = RotateVec(RightRotation(p), dir_cam);
	else
		dir_head = dir_cam;
	return true;
}

static bool ProjectPoint(const StereoCalibrationParams& p, const vec3& point, int eye, double eye_dist, vec2& out_pix, double& zf_out) {
	vec3 cam_center = eye == 0 ? vec3(-(float)eye_dist / 2.0f, 0, 0) : vec3((float)eye_dist / 2.0f, 0, 0);
	vec3 v = point - cam_center;
	if (eye == 1)
		v = RotateVec(RightRotationInv(p), v);
	if (v.GetLength() < 1e-9) {
		zf_out = 0;
		out_pix = vec2((float)p.cx, (float)p.cy);
		return false;
	}
	vec3 dir = v.GetNormalized();
	double theta = 0;
	double roll = 0;
	bool ok = ThetaRollFromDirection(dir, theta, roll, zf_out);
	double r = AngleToPixel(p, theta);
	double dx = r * cos(roll);
	double dy = r * sin(roll);
	out_pix = vec2((float)(p.cx + dx), (float)(p.cy + dy));
	return ok;
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

static double ComputeCalibrationCost(const StereoCalibrationSolver& solver,
                                      const StereoCalibrationParams& params,
                                      double eye_dist, double dist_weight,
                                      double huber_px, double huber_m) {
	const auto& matches = solver.matches;
	int N = matches.GetCount();
	if (N == 0)
		return DBL_MAX;

	vec3 pL(-(float)eye_dist / 2.0f, 0, 0);
	vec3 pR((float)eye_dist / 2.0f, 0, 0);

	double total_cost = 0;
	int valid_count = 0;

	for (int i = 0; i < N; i++) {
		const auto& m = matches[i];

		// Unproject rays
		vec3 dL, dR;
		if (!UnprojectDir(params, m.left_px, 0, dL))
			continue;
		if (!UnprojectDir(params, m.right_px, 1, dR))
			continue;

		// Triangulate 3D point
		vec3 X = TriangulatePoint(pL, dL, pR, dR);

		// Check validity
		if (!std::isfinite(X[0]) || !std::isfinite(X[1]) || !std::isfinite(X[2]))
			continue;
		if (X[2] < -1.0 || X[2] > 10.0)  // Reasonable depth range in meters
			continue;

		// Reproject and compute residuals
		vec2 projL, projR;
		double zL = 0, zR = 0;
		ProjectPoint(params, X, 0, eye_dist, projL, zL);
		ProjectPoint(params, X, 1, eye_dist, projR, zR);

		double r0 = projL[0] - m.left_px[0];
		double r1 = projL[1] - m.left_px[1];
		double r2 = projR[0] - m.right_px[0];
		double r3 = projR[1] - m.right_px[1];

		double r4 = 0;
		double r5 = 0;
		if (m.dist_l > 0)
			r4 = (X - pL).GetLength() - m.dist_l;
		if (m.dist_r > 0)
			r5 = (X - pR).GetLength() - m.dist_r;

		// Apply weights and Huber loss
		double w_px = 1.0;
		double w_mm = dist_weight;

		double r0s = r0 * w_px;
		double r1s = r1 * w_px;
		double r2s = r2 * w_px;
		double r3s = r3 * w_px;
		double r4s = r4 * w_mm;
		double r5s = r5 * w_mm;

		double h0 = HuberWeight(r0s, huber_px * w_px);
		double h1 = HuberWeight(r1s, huber_px * w_px);
		double h2 = HuberWeight(r2s, huber_px * w_px);
		double h3 = HuberWeight(r3s, huber_px * w_px);
		double h4 = HuberWeight(r4s, huber_m * w_mm);
		double h5 = HuberWeight(r5s, huber_m * w_mm);

		double weighted_r0 = r0s * h0;
		double weighted_r1 = r1s * h1;
		double weighted_r2 = r2s * h2;
		double weighted_r3 = r3s * h3;
		double weighted_r4 = r4s * h4;
		double weighted_r5 = r5s * h5;

		if (!std::isfinite(weighted_r0) || !std::isfinite(weighted_r1) ||
		    !std::isfinite(weighted_r2) || !std::isfinite(weighted_r3) ||
		    !std::isfinite(weighted_r4) || !std::isfinite(weighted_r5))
			continue;

		double cost = weighted_r0 * weighted_r0 + weighted_r1 * weighted_r1 +
		              weighted_r2 * weighted_r2 + weighted_r3 * weighted_r3 +
		              weighted_r4 * weighted_r4 + weighted_r5 * weighted_r5;

		total_cost += cost;
		valid_count++;
	}

	if (valid_count == 0)
		return DBL_MAX;

	return total_cost / valid_count;  // Average cost per match
}

static void GABootstrapExtrinsics(StereoCalibrationSolver& solver,
                                    StereoCalibrationParams& params,
                                    int population, int generations, int top_candidates) {
	// Log to GUI report
	if (solver.log) {
		*solver.log << "  Step: Genetic algorithm bootstrap (extrinsics initialization)...\n";
	}

	if (solver.trace.enabled && solver.trace.verbosity >= 1) {
		solver.trace.Add("========================================\n");
		solver.trace.Add("Genetic Algorithm Bootstrap\n");
		solver.trace.Add("========================================\n\n");
		solver.trace.Addf("Configuration:\n");
		solver.trace.Addf("  Population: %d\n", population);
		solver.trace.Addf("  Generations: %d\n", generations);
		solver.trace.Addf("  Top candidates for LM: %d\n\n", top_candidates);
	}

	// Dimension: 3 extrinsic parameters (yaw, pitch, roll)
	const int dimension = 3;

	GeneticOptimizer ga;
	ga.SetMaxGenerations(generations);
	ga.SetRandomTypeUniform();
	ga.MinMax(-M_PI, M_PI);  // All rotation parameters in [-pi, pi]
	ga.Init(dimension, population, StrategyBest1Exp);

	// Initialize population with random perturbations around current values
	for (int i = 0; i < population; i++) {
		ga.population[i][0] = params.yaw + (Randomf() - 0.5) * M_PI / 2.0;    // ±45 degrees
		ga.population[i][1] = params.pitch + (Randomf() - 0.5) * M_PI / 4.0;  // ±22.5 degrees
		ga.population[i][2] = params.roll + (Randomf() - 0.5) * M_PI / 4.0;   // ±22.5 degrees
	}

	// Run GA
	int iter = 0;
	while (!ga.IsEnd()) {
		const Vector<double>& trial = ga.GetTrialSolution();

		// Create trial parameters
		StereoCalibrationParams trial_params = params;
		trial_params.yaw = trial[0];
		trial_params.pitch = trial[1];
		trial_params.roll = trial[2];

		// Evaluate fitness
		ga.Start();
		double cost = ComputeCalibrationCost(solver, trial_params, solver.eye_dist,
		                                      solver.dist_weight, solver.huber_px, solver.huber_m);
		double energy = -cost;  // GA maximizes energy, we minimize cost
		ga.Stop(energy);

		if (solver.trace.enabled && solver.trace.verbosity >= 2 && iter % population == 0) {
			int gen = iter / population;
			solver.trace.Addf("Generation %d/%d: best energy=%.6f (cost=%.6f)\n",
			                   gen + 1, generations, ga.Energy(), -ga.Energy());
		}

		iter++;
	}

	// Extract best solution
	const Vector<double>& best = ga.GetBestSolution();
	params.yaw = best[0];
	params.pitch = best[1];
	params.roll = best[2];

	// Log completion to GUI report
	if (solver.log) {
		*solver.log << "  GA bootstrap completed. Best extrinsics: yaw=" << Format("%.4f", params.yaw)
		            << ", pitch=" << Format("%.4f", params.pitch)
		            << ", roll=" << Format("%.4f", params.roll) << "\n";
	}

	if (solver.trace.enabled && solver.trace.verbosity >= 1) {
		solver.trace.Addf("\nGA Bootstrap complete:\n");
		solver.trace.Addf("  Final best energy: %.6f (cost: %.6f)\n", ga.Energy(), -ga.Energy());
		solver.trace.Addf("  Best extrinsics: yaw=%.6f, pitch=%.6f, roll=%.6f\n\n",
		                   params.yaw, params.pitch, params.roll);
	}
}

bool StereoCalibrationSolver::Solve(StereoCalibrationParams& params, bool lock_distortion) {
	last_failure_reason.Clear();
	trace.Clear();

	int N = matches.GetCount();
	if (N < 5) {
		last_failure_reason = Format("Too few matches: %d (need at least 5)", N);
		if (trace.enabled)
			trace.Add("FAILURE: " + last_failure_reason + "\n");
		return false;
	}
	if (fabs(eye_dist) < 1e-9) {
		last_failure_reason = "Eye distance is zero or not set";
		if (trace.enabled)
			trace.Add("FAILURE: " + last_failure_reason + "\n");
		return false;
	}

	if (trace.enabled) {
		trace.Add("========================================\n");
		trace.Add("Stereo Calibration Solver Math Trace\n");
		trace.Add("========================================\n\n");
		trace.Addf("Configuration:\n");
		trace.Addf("  Matches: %d\n", N);
		trace.Addf("  Eye distance: %.6f m (%.3f mm)\n", eye_dist, eye_dist * 1000.0);
		trace.Addf("  Distance weight: %.6f\n", dist_weight);
		trace.Addf("  Huber threshold (px): %.2f\n", huber_px);
		trace.Addf("  Huber threshold (m): %.6f (%.2f mm)\n", huber_m, huber_m * 1000.0);
		trace.Addf("  Max function evals: %d\n", max_fevals > 0 ? max_fevals : 0);
		trace.Addf("  Lock distortion: %s\n", lock_distortion ? "yes" : "no");
		trace.Addf("  Trace verbosity: %d\n\n", trace.verbosity);

		trace.Addf("Initial parameters:\n");
		trace.Addf("  Distortion: a=%.6f, b=%.6f, c=%.6f, d=%.6f\n", params.a, params.b, params.c, params.d);
		trace.Addf("  Principal point: cx=%.2f, cy=%.2f\n", params.cx, params.cy);
		trace.Addf("  Right camera rotation: yaw=%.6f, pitch=%.6f, roll=%.6f (rad)\n\n", params.yaw, params.pitch, params.roll);
	}

	int global_params = lock_distortion ? 6 : 9;
	Eigen::VectorXd y(global_params + 3 * N);
	
	if (lock_distortion) {
		y[0] = params.a;
		y[1] = params.cx;
		y[2] = params.cy;
		y[3] = params.yaw;
		y[4] = params.pitch;
		y[5] = params.roll;
	}
	else {
		y[0] = params.a;
		y[1] = params.b;
		y[2] = params.c;
		y[3] = params.d;
		y[4] = params.cx;
		y[5] = params.cy;
		y[6] = params.yaw;
		y[7] = params.pitch;
		y[8] = params.roll;
	}

	StereoCalibrationParams init = params;
	vec3 pL(-(float)eye_dist / 2.0f, 0, 0);
	vec3 pR((float)eye_dist / 2.0f, 0, 0);

	if (trace.enabled && trace.verbosity >= 1) {
		trace.Add("========================================\n");
		trace.Add("Preflight Match Analysis\n");
		trace.Add("========================================\n\n");
	}

	int valid_count = 0;
	for (int i = 0; i < N; i++) {
		const auto& m = matches[i];

		if (trace.enabled && trace.verbosity >= 2) {
			trace.Addf("Match #%d:\n", i + 1);
			trace.Addf("  Input pixels: L=(%.2f, %.2f), R=(%.2f, %.2f)\n",
				m.left_px[0], m.left_px[1], m.right_px[0], m.right_px[1]);
			trace.Addf("  Image size: %dx%d\n", m.image_size.cx, m.image_size.cy);
			if (m.dist_l > 0 || m.dist_r > 0) {
				trace.Addf("  Measured distances: L=%.6f m (%.2f mm), R=%.6f m (%.2f mm)\n",
					m.dist_l, m.dist_l * 1000.0, m.dist_r, m.dist_r * 1000.0);
				// Sanity check for unit mismatches
				if (m.dist_l > 10.0 || m.dist_r > 10.0) {
					trace.Addf("  WARNING: Distance > 10m suspicious - check mm/m conversion!\n");
				}
				if ((m.dist_l > 0 && m.dist_l < 0.05) || (m.dist_r > 0 && m.dist_r < 0.05)) {
					trace.Addf("  WARNING: Distance < 50mm suspicious - check mm/m conversion!\n");
				}
			}
		}

		double dx_l = m.left_px[0] - init.cx;
		double dy_l = m.left_px[1] - init.cy;
		double r_pix_l = sqrt(dx_l * dx_l + dy_l * dy_l);
		double roll_l = atan2(dy_l, dx_l);

		double dx_r = m.right_px[0] - init.cx;
		double dy_r = m.right_px[1] - init.cy;
		double r_pix_r = sqrt(dx_r * dx_r + dy_r * dy_r);
		double roll_r = atan2(dy_r, dx_r);

		if (trace.enabled && trace.verbosity >= 2) {
			trace.Addf("  Left eye:\n");
			trace.Addf("    dx=%.2f, dy=%.2f (relative to principal point)\n", dx_l, dy_l);
			trace.Addf("    r_pix=%.2f, roll=%.6f rad (%.2f deg)\n", r_pix_l, roll_l, roll_l * 180.0 / M_PI);
		}

		double theta_l = 0;
		bool ok_l = PixelToAngleTrace(init, r_pix_l, theta_l, trace.enabled ? &trace : NULL);

		if (trace.enabled && trace.verbosity >= 2) {
			trace.Addf("    theta=%.6f rad (%.2f deg) %s\n", theta_l, theta_l * 180.0 / M_PI, ok_l ? "OK" : "FAILED");
		}

		if (trace.enabled && trace.verbosity >= 2) {
			trace.Addf("  Right eye:\n");
			trace.Addf("    dx=%.2f, dy=%.2f (relative to principal point)\n", dx_r, dy_r);
			trace.Addf("    r_pix=%.2f, roll=%.6f rad (%.2f deg)\n", r_pix_r, roll_r, roll_r * 180.0 / M_PI);
		}

		double theta_r = 0;
		bool ok_r = PixelToAngleTrace(init, r_pix_r, theta_r, trace.enabled ? &trace : NULL);

		if (trace.enabled && trace.verbosity >= 2) {
			trace.Addf("    theta=%.6f rad (%.2f deg) %s\n", theta_r, theta_r * 180.0 / M_PI, ok_r ? "OK" : "FAILED");
		}

		vec3 dL, dR;
		UnprojectDir(init, m.left_px, 0, dL);
		UnprojectDir(init, m.right_px, 1, dR);

		if (trace.enabled && trace.verbosity >= 2) {
			trace.Addf("  Ray directions (camera frame):\n");
			trace.Addf("    Left:  (%.6f, %.6f, %.6f)\n", dL[0], dL[1], dL[2]);
			trace.Addf("    Right: (%.6f, %.6f, %.6f) (after rotation yaw=%.4f, pitch=%.4f, roll=%.4f)\n",
				dR[0], dR[1], dR[2], init.yaw, init.pitch, init.roll);
		}

		vec3 pt = TriangulatePoint(pL, dL, pR, dR);

		vec3 vL = pt - pL;
		vec3 vR = pt - pR;
		double dist_l_calc = vL.GetLength();
		double dist_r_calc = vR.GetLength();

		vec3 w0 = pL - pR;
		double a = Dot(dL, dL);
		double b = Dot(dL, dR);
		double c = Dot(dR, dR);
		double d = Dot(dL, w0);
		double e = Dot(dR, w0);
		double denom = a * c - b * b;
		double sc = 0, tc = 0;
		if (fabs(denom) > 1e-9) {
			sc = (b * e - c * d) / denom;
			tc = (a * e - b * d) / denom;
		}
		vec3 closestL = pL + dL * (float)sc;
		vec3 closestR = pR + dR * (float)tc;
		double ray_ray_dist = (closestL - closestR).GetLength();

		if (trace.enabled && trace.verbosity >= 2) {
			trace.Addf("  Triangulation:\n");
			trace.Addf("    3D point: (%.6f, %.6f, %.6f) m  (%.2f, %.2f, %.2f) mm\n",
				pt[0], pt[1], pt[2], pt[0] * 1000.0, pt[1] * 1000.0, pt[2] * 1000.0);
			trace.Addf("    Ray-ray distance: %.6f m (%.3f mm)\n", ray_ray_dist, ray_ray_dist * 1000.0);
			trace.Addf("    Calculated distances: L=%.6f m (%.2f mm), R=%.6f m (%.2f mm)\n",
				dist_l_calc, dist_l_calc * 1000.0, dist_r_calc, dist_r_calc * 1000.0);
			if (m.dist_l > 0)
				trace.Addf("    Distance error L: %.6f m (%.2f mm) [measured %.6f m (%.2f mm)]\n",
					dist_l_calc - m.dist_l, (dist_l_calc - m.dist_l) * 1000.0, m.dist_l, m.dist_l * 1000.0);
			if (m.dist_r > 0)
				trace.Addf("    Distance error R: %.6f m (%.2f mm) [measured %.6f m (%.2f mm)]\n",
					dist_r_calc - m.dist_r, (dist_r_calc - m.dist_r) * 1000.0, m.dist_r, m.dist_r * 1000.0);
		}

		bool valid = ok_l && ok_r && std::isfinite(pt[0]) && std::isfinite(pt[1]) && std::isfinite(pt[2]);
		if (valid && (pt[2] < -1.0 || pt[2] > 10.0)) {  // Z should be in reasonable range (meters)
			valid = false;
			if (trace.enabled && trace.verbosity >= 1)
				trace.Addf("  WARNING: Match #%d has unreasonable Z=%.6f m (%.2f mm), marking invalid\n",
					i + 1, pt[2], pt[2] * 1000.0);
		}

		if (valid)
			valid_count++;

		if (!valid && trace.enabled && trace.verbosity >= 1) {
			trace.Addf("  INVALID: Match #%d failed preflight checks\n", i + 1);
		}

		if (trace.enabled && trace.verbosity >= 2)
			trace.Add("\n");

		y[global_params + i * 3 + 0] = pt[0];
		y[global_params + i * 3 + 1] = pt[1];
		y[global_params + i * 3 + 2] = pt[2];
	}

	// Unit consistency check
	Vector<double> measured_dists, calculated_dists;
	for (int i = 0; i < N; i++) {
		const auto& m = matches[i];
		vec3 pt(y[global_params + i * 3 + 0], y[global_params + i * 3 + 1], y[global_params + i * 3 + 2]);
		if (m.dist_l > 0) {
			measured_dists.Add(m.dist_l);
			calculated_dists.Add((pt - pL).GetLength());
		}
		if (m.dist_r > 0) {
			measured_dists.Add(m.dist_r);
			calculated_dists.Add((pt - pR).GetLength());
		}
	}

	if (measured_dists.GetCount() > 0 && trace.enabled && trace.verbosity >= 1) {
		Vector<double> sorted_measured = clone(measured_dists);
		Vector<double> sorted_calculated = clone(calculated_dists);
		Sort(sorted_measured);
		Sort(sorted_calculated);
		double median_measured = sorted_measured[sorted_measured.GetCount() / 2];
		double median_calculated = sorted_calculated[sorted_calculated.GetCount() / 2];
		double ratio = median_calculated / median_measured;

		trace.Add("========================================\n");
		trace.Add("Unit Consistency Check\n");
		trace.Add("========================================\n");
		trace.Addf("  Median measured distance:   %.6f m (%.2f mm)\n", median_measured, median_measured * 1000.0);
		trace.Addf("  Median calculated distance: %.6f m (%.2f mm)\n", median_calculated, median_calculated * 1000.0);
		trace.Addf("  Ratio (calculated/measured): %.6f\n", ratio);
		if (ratio < 0.001 || ratio > 1000.0) {
			trace.Add("  *** UNIT MISMATCH SUSPECTED ***\n");
			trace.Add("  Check mm↔m conversions at UI boundary!\n");
		} else if (fabs(ratio - 1.0) < 0.1) {
			trace.Add("  Units appear consistent.\n");
		}
		trace.Add("\n");
	}

	if (trace.enabled && trace.verbosity >= 1) {
		trace.Addf("Preflight summary: %d/%d matches valid\n\n", valid_count, N);
	}

	if (valid_count < 5) {
		last_failure_reason = Format("Too few valid matches after preflight: %d/%d (need at least 5)", valid_count, N);
		if (trace.enabled)
			trace.Add("FAILURE: " + last_failure_reason + "\n");
		return false;
	}

	// GA bootstrap for extrinsics (if enabled)
	if (use_ga_init) {
		GABootstrapExtrinsics(*this, params, ga_population, ga_generations, ga_top_candidates);

		// Update y vector with GA-optimized extrinsics
		if (lock_distortion) {
			y[3] = params.yaw;
			y[4] = params.pitch;
			y[5] = params.roll;
		}
		else {
			y[6] = params.yaw;
			y[7] = params.pitch;
			y[8] = params.roll;
		}
	}

	int iter = 0;
	bool had_nonfinite = false;

	if (trace.enabled && trace.verbosity >= 1) {
		trace.Add("========================================\n");
		trace.Add("Optimization Iterations\n");
		trace.Add("========================================\n\n");
	}

	auto residual = [&](const Eigen::VectorXd& x, Eigen::VectorXd& res) {
		iter++;
		StereoCalibrationParams cur = params;
		if (lock_distortion) {
			cur.a = x[0];
			cur.cx = x[1];
			cur.cy = x[2];
			cur.yaw = x[3];
			cur.pitch = x[4];
			cur.roll = x[5];
		}
		else {
			cur.a = x[0];
			cur.b = x[1];
			cur.c = x[2];
			cur.d = x[3];
			cur.cx = x[4];
			cur.cy = x[5];
			cur.yaw = x[6];
			cur.pitch = x[7];
			cur.roll = x[8];
		}

		if (log && (iter == 1 || iter % 10 == 0)) {
			*log << "    Iter " << iter << ": a=" << Format("%.4f", cur.a)
				<< ", cx=" << Format("%.2f", cur.cx) << ", cy=" << Format("%.2f", cur.cy)
				<< ", yaw=" << Format("%.4f", cur.yaw)
				<< ", pitch=" << Format("%.4f", cur.pitch)
				<< ", roll=" << Format("%.4f", cur.roll) << "\n";
		}

		if (trace.enabled && trace.verbosity >= 1 && (iter == 1 || iter % 10 == 0)) {
			trace.Addf("Iteration %d:\n", iter);
			if (!lock_distortion)
				trace.Addf("  Distortion: a=%.6f, b=%.6f, c=%.6f, d=%.6f\n", cur.a, cur.b, cur.c, cur.d);
			else
				trace.Addf("  Distortion: a=%.6f (locked)\n", cur.a);
			trace.Addf("  Principal point: cx=%.2f, cy=%.2f\n", cur.cx, cur.cy);
			trace.Addf("  Right rotation: yaw=%.6f, pitch=%.6f, roll=%.6f\n", cur.yaw, cur.pitch, cur.roll);
		}

		res.resize(N * 6);
		double total_cost = 0;
		int nonfinite_count = 0;

		for (int i = 0; i < N; i++) {
			const auto& m = matches[i];
			vec3 X((float)x[global_params + i * 3 + 0], (float)x[global_params + i * 3 + 1], (float)x[global_params + i * 3 + 2]);

			vec2 projL, projR;
			double zL = 0, zR = 0;
			ProjectPoint(cur, X, 0, eye_dist, projL, zL);
			ProjectPoint(cur, X, 1, eye_dist, projR, zR);

			double r0 = projL[0] - m.left_px[0];
			double r1 = projL[1] - m.left_px[1];
			double r2 = projR[0] - m.right_px[0];
			double r3 = projR[1] - m.right_px[1];

			double r4 = 0;
			double r5 = 0;
			if (m.dist_l > 0)
				r4 = (X - pL).GetLength() - m.dist_l;
			if (m.dist_r > 0)
				r5 = (X - pR).GetLength() - m.dist_r;

			double w_px = 1.0;
			double w_mm = dist_weight;
			double r0s = r0 * w_px;
			double r1s = r1 * w_px;
			double r2s = r2 * w_px;
			double r3s = r3 * w_px;
			double r4s = r4 * w_mm;
			double r5s = r5 * w_mm;

			double h0 = HuberWeight(r0s, huber_px * w_px);
			double h1 = HuberWeight(r1s, huber_px * w_px);
			double h2 = HuberWeight(r2s, huber_px * w_px);
			double h3 = HuberWeight(r3s, huber_px * w_px);
			double h4 = HuberWeight(r4s, huber_m * w_mm);
			double h5 = HuberWeight(r5s, huber_m * w_mm);

			double weighted_r0 = r0s * h0;
			double weighted_r1 = r1s * h1;
			double weighted_r2 = r2s * h2;
			double weighted_r3 = r3s * h3;
			double weighted_r4 = r4s * h4;
			double weighted_r5 = r5s * h5;

			res[i * 6 + 0] = weighted_r0;
			res[i * 6 + 1] = weighted_r1;
			res[i * 6 + 2] = weighted_r2;
			res[i * 6 + 3] = weighted_r3;
			res[i * 6 + 4] = weighted_r4;
			res[i * 6 + 5] = weighted_r5;

			if (!std::isfinite(weighted_r0) || !std::isfinite(weighted_r1) || !std::isfinite(weighted_r2) ||
				!std::isfinite(weighted_r3) || !std::isfinite(weighted_r4) || !std::isfinite(weighted_r5)) {
				nonfinite_count++;
			}

			double cost = weighted_r0 * weighted_r0 + weighted_r1 * weighted_r1 +
				weighted_r2 * weighted_r2 + weighted_r3 * weighted_r3 +
				weighted_r4 * weighted_r4 + weighted_r5 * weighted_r5;
			total_cost += cost;
		}

		if (nonfinite_count > 0) {
			had_nonfinite = true;
			if (trace.enabled && trace.verbosity >= 1)
				trace.Addf("  WARNING: %d matches produced non-finite residuals\n", nonfinite_count);
		}

		if (trace.enabled && trace.verbosity >= 1 && (iter == 1 || iter % 10 == 0)) {
			trace.Addf("  Total cost: %.6f\n\n", total_cost);
		}

		return 0;
	};

	if (log) {
		if (lock_distortion)
			*log << "  Step: Non-linear optimization Stage 1 (Levenberg-Marquardt, distortion locked)...\n";
		else
			*log << "  Step: Non-linear optimization Stage 2 (Levenberg-Marquardt, all parameters)...\n";
	}

	if (trace.enabled && trace.verbosity >= 1) {
		trace.Add("========================================\n");
		trace.Add("Starting Levenberg-Marquardt Optimization\n");
		trace.Add("========================================\n\n");
	}

	int maxfev = max_fevals > 0 ? max_fevals : max(200, 10 * (global_params + 3 * N));
	bool opt_success = NonLinearOptimization(y, N * 6, residual, 1e-10, 1e-10, maxfev);

	if (!opt_success) {
		if (log)
			*log << "  Optimization FAILED.\n\n";

		if (had_nonfinite) {
			last_failure_reason = "Non-finite residuals encountered during optimization";
		}
		else if (iter >= maxfev) {
			last_failure_reason = Format("Max function evaluations reached (%d)", maxfev);
		}
		else {
			last_failure_reason = "Levenberg-Marquardt optimization failed (did not converge)";
		}

		if (trace.enabled) {
			trace.Add("\n========================================\n");
			trace.Add("OPTIMIZATION FAILED\n");
			trace.Add("========================================\n\n");
			trace.Add("Reason: " + last_failure_reason + "\n");
			trace.Addf("Total iterations: %d\n", iter);
			trace.Add(String("Had non-finite residuals: ") + (had_nonfinite ? "yes" : "no") + "\n\n");
		}

		return false;
	}

	if (log)
		*log << "  Optimization converged in " << iter << " iterations.\n\n";

	if (trace.enabled && trace.verbosity >= 1) {
		trace.Add("\n========================================\n");
		trace.Add("Optimization Succeeded\n");
		trace.Add("========================================\n\n");
		trace.Addf("Total iterations: %d\n\n", iter);
	}

	if (lock_distortion) {
		params.a = y[0];
		params.cx = y[1];
		params.cy = y[2];
		params.yaw = y[3];
		params.pitch = y[4];
		params.roll = y[5];
	}
	else {
		params.a = y[0];
		params.b = y[1];
		params.c = y[2];
		params.d = y[3];
		params.cx = y[4];
		params.cy = y[5];
		params.yaw = y[6];
		params.pitch = y[7];
		params.roll = y[8];
	}

	last_points.SetCount(N);
	for (int i = 0; i < N; i++) {
		last_points[i][0] = (float)y[global_params + i * 3 + 0];
		last_points[i][1] = (float)y[global_params + i * 3 + 1];
		last_points[i][2] = (float)y[global_params + i * 3 + 2];
	}

	return true;
}

void StereoCalibrationSolver::ComputeDiagnostics(const StereoCalibrationParams& params, StereoCalibrationDiagnostics& out) const {
	out = StereoCalibrationDiagnostics();
	int N = matches.GetCount();
	if (N <= 0)
		return;
	out.residuals.SetCount(N);
	vec3 pL(-(float)eye_dist / 2.0f, 0, 0);
	vec3 pR((float)eye_dist / 2.0f, 0, 0);

	double sum_sq_l = 0;
	double sum_sq_r = 0;
	double dist_sq_l = 0;
	double dist_sq_r = 0;
	double sum_ray_ray_dist = 0;
	int ray_ray_count = 0;

	if (trace.enabled && trace.verbosity >= 1) {
		trace.Add("========================================\n");
		trace.Add("Final Diagnostics\n");
		trace.Add("========================================\n\n");
		trace.Addf("Final parameters:\n");
		trace.Addf("  Distortion: a=%.6f, b=%.6f, c=%.6f, d=%.6f\n", params.a, params.b, params.c, params.d);
		trace.Addf("  Principal point: cx=%.2f, cy=%.2f\n", params.cx, params.cy);
		trace.Addf("  Right rotation: yaw=%.6f, pitch=%.6f, roll=%.6f\n\n", params.yaw, params.pitch, params.roll);
	}

	for (int i = 0; i < N; i++) {
		const auto& m = matches[i];
		StereoCalibrationResidual res;
		res.match_index = i;
		res.measured_l = m.left_px;
		res.measured_r = m.right_px;
		vec3 X = i < last_points.GetCount() ? last_points[i] : vec3(0,0,0);
		res.point = X;

		vec2 projL, projR;
		double zL = 0, zR = 0;
		ProjectPoint(params, X, 0, eye_dist, projL, zL);
		ProjectPoint(params, X, 1, eye_dist, projR, zR);

		res.reproj_l = projL;
		res.reproj_r = projR;
		res.err_l_px = (projL - m.left_px).GetLength();
		res.err_r_px = (projR - m.right_px).GetLength();
		res.z_l = zL;
		res.z_r = zR;
		res.disparity_px = m.left_px[0] - m.right_px[0];

		if (zL <= 0)
			out.behind_left++;
		if (zR <= 0)
			out.behind_right++;

		sum_sq_l += res.err_l_px * res.err_l_px;
		sum_sq_r += res.err_r_px * res.err_r_px;
		out.reproj_count_l++;
		out.reproj_count_r++;

		if (m.dist_l > 0) {
			res.dist_l_err = (X - pL).GetLength() - m.dist_l;
			dist_sq_l += res.dist_l_err * res.dist_l_err;
			out.dist_count_l++;
		}
		if (m.dist_r > 0) {
			res.dist_r_err = (X - pR).GetLength() - m.dist_r;
			dist_sq_r += res.dist_r_err * res.dist_r_err;
			out.dist_count_r++;
		}

		vec3 dL, dR;
		if (UnprojectDir(params, m.left_px, 0, dL) && UnprojectDir(params, m.right_px, 1, dR)) {
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
				vec3 closestL = pL + dL * (float)sc;
				vec3 closestR = pR + dR * (float)tc;
				double ray_ray_dist = (closestL - closestR).GetLength();
				sum_ray_ray_dist += ray_ray_dist;
				ray_ray_count++;
			}
		}

		out.residuals[i] = res;
	}

	if (out.reproj_count_l > 0)
		out.reproj_rms_l = sqrt(sum_sq_l / out.reproj_count_l);
	if (out.reproj_count_r > 0)
		out.reproj_rms_r = sqrt(sum_sq_r / out.reproj_count_r);
	if (out.dist_count_l > 0)
		out.dist_rms_l = sqrt(dist_sq_l / out.dist_count_l);
	if (out.dist_count_r > 0)
		out.dist_rms_r = sqrt(dist_sq_r / out.dist_count_r);

	if (trace.enabled && trace.verbosity >= 1) {
		trace.Addf("Summary statistics:\n");
		trace.Addf("  Reproj RMS: L=%.3f px, R=%.3f px\n", out.reproj_rms_l, out.reproj_rms_r);
		if (out.dist_count_l > 0 || out.dist_count_r > 0)
			trace.Addf("  Distance RMS: L=%.2f mm (%d), R=%.2f mm (%d)\n",
				out.dist_rms_l, out.dist_count_l, out.dist_rms_r, out.dist_count_r);
		if (ray_ray_count > 0)
			trace.Addf("  Average ray-ray distance: %.3f mm\n", sum_ray_ray_dist / ray_ray_count);
		trace.Addf("  Behind camera: L=%d, R=%d\n\n", out.behind_left, out.behind_right);

		Vector<int> order;
		order.SetCount(N);
		for (int i = 0; i < N; i++)
			order[i] = i;

		Sort(order, [&](int a, int b) {
			const auto& ra = out.residuals[a];
			const auto& rb = out.residuals[b];
			double cost_a = ra.err_l_px * ra.err_l_px + ra.err_r_px * ra.err_r_px;
			if (matches[a].dist_l > 0)
				cost_a += dist_weight * ra.dist_l_err * ra.dist_l_err;
			if (matches[a].dist_r > 0)
				cost_a += dist_weight * ra.dist_r_err * ra.dist_r_err;
			double cost_b = rb.err_l_px * rb.err_l_px + rb.err_r_px * rb.err_r_px;
			if (matches[b].dist_l > 0)
				cost_b += dist_weight * rb.dist_l_err * rb.dist_l_err;
			if (matches[b].dist_r > 0)
				cost_b += dist_weight * rb.dist_r_err * rb.dist_r_err;
			return cost_a > cost_b;
		});

		int top_count = min(5, N);
		trace.Addf("Top %d matches by total cost:\n", top_count);
		for (int i = 0; i < top_count; i++) {
			int idx = order[i];
			const auto& r = out.residuals[idx];
			const auto& m = matches[idx];
			double cost = r.err_l_px * r.err_l_px + r.err_r_px * r.err_r_px;
			if (m.dist_l > 0)
				cost += dist_weight * r.dist_l_err * r.dist_l_err;
			if (m.dist_r > 0)
				cost += dist_weight * r.dist_r_err * r.dist_r_err;

			trace.Addf("  Match #%d (total cost=%.3f):\n", idx + 1, cost);
			trace.Addf("    Measured: L=(%.2f, %.2f), R=(%.2f, %.2f)\n",
				m.left_px[0], m.left_px[1], m.right_px[0], m.right_px[1]);
			trace.Addf("    Reproj:   L=(%.2f, %.2f), R=(%.2f, %.2f)\n",
				r.reproj_l[0], r.reproj_l[1], r.reproj_r[0], r.reproj_r[1]);
			trace.Addf("    Reproj error: L=%.2f px, R=%.2f px\n", r.err_l_px, r.err_r_px);
			if (m.dist_l > 0 || m.dist_r > 0)
				trace.Addf("    Distance error: L=%.6f m (%.2f mm), R=%.6f m (%.2f mm)\n",
					r.dist_l_err, r.dist_l_err * 1000.0, r.dist_r_err, r.dist_r_err * 1000.0);
			trace.Addf("    3D point: (%.6f, %.6f, %.6f) m  (%.2f, %.2f, %.2f) mm\n",
				r.point[0], r.point[1], r.point[2], r.point[0] * 1000.0, r.point[1] * 1000.0, r.point[2] * 1000.0);
			trace.Addf("    Z-depth: L=%.2f, R=%.2f\n", r.z_l, r.z_r);
			trace.Add("\n");
		}
	}
}

END_UPP_NAMESPACE
