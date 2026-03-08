#include "Geometry.h"
#include <plugin/Eigen/Eigen.h>
#include <AI/Core/Base/Base.h>

NAMESPACE_UPP

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

bool IsParamsFinite(const StereoCalibrationParams& p) {
	return std::isfinite(p.a) && std::isfinite(p.b) && std::isfinite(p.c) && std::isfinite(p.d) &&
	       std::isfinite(p.cx) && std::isfinite(p.cy) &&
	       std::isfinite(p.yaw) && std::isfinite(p.pitch) && std::isfinite(p.roll) &&
	       std::isfinite(p.yaw_l) && std::isfinite(p.pitch_l) && std::isfinite(p.roll_l);
}

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

static inline double Huber(double r, double delta) {
	double a = fabs(r);
	if (a <= delta) return 0.5 * r * r;
	return delta * (a - 0.5 * delta);
}

static inline vec3 RotateVec(const mat4& m, const vec3& v) {
	return (m * v.Embed()).Splice();
}

static inline mat4 EyeRotation(const StereoCalibrationParams& p, int eye) {
	if (eye == 0)
		return AxesMat((float)p.yaw_l, (float)p.pitch_l, (float)p.roll_l);
	else
		return AxesMat((float)p.yaw, (float)p.pitch, (float)p.roll);
}

static inline mat4 EyeRotationInv(const StereoCalibrationParams& p, int eye) {
	return EyeRotation(p, eye).GetTransposed();
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
	dir_head = RotateVec(EyeRotation(p, eye), dir_cam);
	return true;
}

static bool ProjectPoint(const StereoCalibrationParams& p, const vec3& point, int eye, double eye_dist, vec2& out_pix, double& zf_out) {
	vec3 cam_center = eye == 0 ? vec3(-(float)eye_dist / 2.0f, 0, 0) : vec3((float)eye_dist / 2.0f, 0, 0);
	vec3 v = point - cam_center;
	v = RotateVec(EyeRotationInv(p, eye), v);
	
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

double StereoCalibrationSolver::ComputeRobustCost(const StereoCalibrationParams& params) const {
	if (!IsParamsFinite(params)) return 1e12; // Reject non-finite params immediately

	int N = matches.GetCount();
	if (N == 0 && lines.IsEmpty()) return 1e12; // Large finite penalty instead of DBL_MAX

	vec3 pL(-(float)eye_dist / 2.0f, 0, 0);
	vec3 pR((float)eye_dist / 2.0f, 0, 0);

	Vector<double> point_costs;
	if (N > 0) {
		point_costs.SetCount(N);
		for (int i = 0; i < N; i++) {
			const auto& m = matches[i];
			vec3 dL, dR;
			UnprojectDir(params, m.left_px, 0, dL);
			UnprojectDir(params, m.right_px, 1, dR);

			vec3 X = TriangulatePoint(pL, dL, pR, dR);
			
			double cost = 0;
			if (!std::isfinite(X[0]) || !std::isfinite(X[1]) || !std::isfinite(X[2])) {
				cost = 1e6;
			} else {
				vec2 projL, projR;
				double zL = 0, zR = 0;
				ProjectPoint(params, X, 0, eye_dist, projL, zL);
				ProjectPoint(params, X, 1, eye_dist, projR, zR);

				double eL = (projL - m.left_px).GetLength();
				double eR = (projR - m.right_px).GetLength();
				
				cost = Huber(eL, huber_px) + Huber(eR, huber_px);
				if (m.dist_l > 0) cost += dist_weight * Huber((X - pL).GetLength() - m.dist_l, huber_m);
				if (m.dist_r > 0) cost += dist_weight * Huber((X - pR).GetLength() - m.dist_r, huber_m);
				if (zL < 0.01 || zR < 0.01) cost += 1000.0;
			}
			point_costs[i] = cost;
		}
	}

	double total_cost = 0;
	if (N > 0) {
		if (ga_use_trimmed_loss) {
			Sort(point_costs);
			int count = (int)(N * (1.0 - ga_trim_percent / 100.0));
			count = max(1, count);
			for(int i = 0; i < count; i++) total_cost += point_costs[i];
			total_cost /= count;
		} else {
			for(int i = 0; i < N; i++) total_cost += point_costs[i];
			total_cost /= N;
		}
	}

	// Line straightness cost
	static bool dumped_invalid_line = false;
	static int line_debug_count = 0;
	bool enable_line_debug = (trace.enabled && trace.verbosity >= 2 && line_debug_count < 2);

	int line_idx = 0;
	for (const auto& line : lines) {
		if (line.raw_norm.GetCount() < 3) continue;

		if (enable_line_debug && line_idx == 0) {
			printf("\n=== LINE STRAIGHTNESS DEBUG (line %d, eye %d) ===\n", line_idx, line.eye);
			printf("  Params: a=%.2f b=%.2f c=%.2f d=%.2f cx=%.2f cy=%.2f\n",
			       params.a, params.b, params.c, params.d, params.cx, params.cy);
			printf("  Derived: k1=%.4f k2=%.4f (if c=a*k1, d=a*k2)\n",
			       params.a != 0 ? params.c/params.a : 0, params.a != 0 ? params.d/params.a : 0);
			fflush(stdout);
		}

		Vector<Pointf> pts;
		for (int i = 0; i < line.raw_norm.GetCount(); i++) {
			const auto& p_norm = line.raw_norm[i];
			Size sz = matches.GetCount() > 0 ? matches[0].image_size : Size(1280, 720);
			vec2 pix((float)p_norm[0] * sz.cx, (float)p_norm[1] * sz.cy);

			double dx = pix[0] - params.cx;
			double dy = pix[1] - params.cy;
			double r = sqrt(dx * dx + dy * dy);
			if (r < 1e-9) {
				pts.Add(Pointf(0, 0));
				continue;
			}
			double theta = PixelToAngle(params, r);
			double roll = atan2(dy, dx);

			// Rectilinear projection: r' = f * tan(theta)
			double r_rect = params.a * tan(theta);

			if (enable_line_debug && line_idx == 0 && i < 5) {
				printf("  Point %d: raw_pix=(%.1f,%.1f) r=%.2f theta=%.4f rad r_rect=%.2f\n",
				       i, pix[0], pix[1], r, theta, r_rect);
			}

			if (!std::isfinite(r_rect)) {
				if (!dumped_invalid_line) {
					printf("INVALID LINE POINT: f=%f, theta=%f, r_rect=%f, r=%f, dx=%f, dy=%f, cx=%f, cy=%f\n",
						params.a, theta, r_rect, r, dx, dy, params.cx, params.cy);
					dumped_invalid_line = true;
				}
				pts.Add(Pointf(1e6, 1e6)); // Large but finite error
			} else {
				pts.Add(Pointf(r_rect * cos(roll), r_rect * sin(roll)));
			}
		}

		line_idx++;
		
		// Fit line and calculate RMS error
		if (pts.GetCount() >= 3) {
			double sum_x = 0, sum_y = 0, sum_xx = 0, sum_xy = 0, sum_yy = 0;
			for (const auto& p : pts) {
				sum_x += p.x; sum_y += p.y;
				sum_xx += p.x * p.x; sum_xy += p.x * p.y; sum_yy += p.y * p.y;
			}
			int n = pts.GetCount();
			double var_x = sum_xx - sum_x * sum_x / n;
			double var_y = sum_yy - sum_y * sum_y / n;
			double cov_xy = sum_xy - sum_x * sum_y / n;

			double line_cost = 0;
			if (fabs(var_x) > fabs(var_y)) {
				double slope = cov_xy / var_x;
				double intercept = (sum_y - slope * sum_x) / n;
				for (const auto& p : pts) {
					double err = p.y - (slope * p.x + intercept);
					line_cost += err * err;
				}
			} else if (fabs(var_y) > 1e-9) {
				double inv_slope = cov_xy / var_y;
				double intercept = (sum_x - inv_slope * sum_y) / n;
				for (const auto& p : pts) {
					double err = p.x - (inv_slope * p.y + intercept);
					line_cost += err * err;
				}
			}

			if (enable_line_debug && line_idx - 1 == 0) {
				printf("  Line fit: rms_error=%.4f cost_contribution=%.2f\n",
				       sqrt(line_cost / n), 100.0 * sqrt(line_cost / n));
				fflush(stdout);
				line_debug_count++;
			}

			total_cost += 100.0 * sqrt(line_cost / n); // Weight lines heavily for intrinsics
		}
	}

	// Physical priors to break degeneracy
	// 1. Barrel distortion prior: k1 should be negative for typical lenses
	double k1 = params.a != 0 ? params.c / params.a : 0;
	if (k1 > 0) {
		// Penalize positive k1 (pincushion distortion is rare)
		total_cost += 500.0 * k1;  // Soft penalty, not hard constraint
	}

	// 2. r→θ monotonicity check: forward distortion function should be monotonic
	// Sample the polynomial at multiple radii and check if it's monotonically increasing
	bool monotonic = true;
	double max_theta = 1.5;  // Check up to ~85 degrees
	int n_samples = 10;
	double prev_r = 0;
	for (int i = 1; i <= n_samples; i++) {
		double theta = (i / (double)n_samples) * max_theta;
		double t2 = theta * theta;
		double t3 = t2 * theta;
		double t4 = t3 * theta;
		double r = params.a * theta + params.b * t2 + params.c * t3 + params.d * t4;

		if (r <= prev_r || r < 0 || !std::isfinite(r)) {
			monotonic = false;
			break;
		}
		prev_r = r;
	}
	if (!monotonic) {
		total_cost += 1e10;  // Heavy penalty for non-monotonic distortion
	}

	// 3. Raw pixel curvature cost: measure distortion BEFORE undistortion
	// This directly penalizes the amount of barrel/pincushion distortion
	// and prevents focal-distortion compensation
	for (const auto& line : lines) {
		if (line.raw_norm.GetCount() < 5) continue;

		// Get raw distorted pixel coordinates
		Vector<vec2> raw_pixels;
		Size sz = matches.GetCount() > 0 ? matches[0].image_size : Size(1280, 720);
		for (const auto& p_norm : line.raw_norm) {
			vec2 pix((float)p_norm[0] * sz.cx, (float)p_norm[1] * sz.cy);
			raw_pixels.Add(pix);
		}

		if (raw_pixels.GetCount() >= 5) {
			// Fit line to RAW pixels (should be curved if distortion exists)
			double sum_x = 0, sum_y = 0;
			for (const auto& p : raw_pixels) {
				sum_x += p[0]; sum_y += p[1];
			}
			int n = raw_pixels.GetCount();
			double mean_x = sum_x / n;
			double mean_y = sum_y / n;

			double sum_xx = 0, sum_yy = 0, sum_xy = 0;
			for (const auto& p : raw_pixels) {
				double dx = p[0] - mean_x;
				double dy = p[1] - mean_y;
				sum_xx += dx * dx;
				sum_yy += dy * dy;
				sum_xy += dx * dy;
			}

			double var_x = sum_xx / n;
			double var_y = sum_yy / n;
			double cov_xy = sum_xy / n;

			// Measure curvature in raw pixels
			double raw_curvature = 0;
			if (var_x > var_y && var_x > 1e-9) {
				double slope = cov_xy / var_x;
				double intercept = mean_y - slope * mean_x;
				for (const auto& p : raw_pixels) {
					double err = p[1] - (slope * p[0] + intercept);
					raw_curvature += err * err;
				}
			} else if (var_y > 1e-9) {
				double inv_slope = cov_xy / var_y;
				double intercept = mean_x - inv_slope * mean_y;
				for (const auto& p : raw_pixels) {
					double err = p[0] - (inv_slope * p[1] + intercept);
					raw_curvature += err * err;
				}
			}

			double rms_raw_curvature = sqrt(raw_curvature / n);

			// Now check how much undistortion straightens the line
			// Correct distortion should significantly reduce curvature
			Vector<vec2> undistorted_pts;
			for (const auto& pix : raw_pixels) {
				double dx = pix[0] - params.cx;
				double dy = pix[1] - params.cy;
				double r = sqrt(dx * dx + dy * dy);
				if (r < 1e-9) continue;

				double theta = PixelToAngle(params, r);
				double roll = atan2(dy, dx);
				double r_rect = params.a * tan(theta);
				if (!std::isfinite(r_rect)) continue;

				undistorted_pts.Add(vec2((float)(r_rect * cos(roll)),
				                          (float)(r_rect * sin(roll))));
			}

			if (undistorted_pts.GetCount() >= 5) {
				// Measure curvature after undistortion
				sum_x = 0; sum_y = 0;
				for (const auto& p : undistorted_pts) {
					sum_x += p[0]; sum_y += p[1];
				}
				int n_undist = undistorted_pts.GetCount();
				mean_x = sum_x / n_undist;
				mean_y = sum_y / n_undist;

				sum_xx = 0; sum_yy = 0; sum_xy = 0;
				for (const auto& p : undistorted_pts) {
					double dx = p[0] - mean_x;
					double dy = p[1] - mean_y;
					sum_xx += dx * dx;
					sum_yy += dy * dy;
					sum_xy += dx * dy;
				}

				var_x = sum_xx / n_undist;
				var_y = sum_yy / n_undist;
				cov_xy = sum_xy / n_undist;

				double undist_curvature = 0;
				if (var_x > var_y && var_x > 1e-9) {
					double slope = cov_xy / var_x;
					double intercept = mean_y - slope * mean_x;
					for (const auto& p : undistorted_pts) {
						double err = p[1] - (slope * p[0] + intercept);
						undist_curvature += err * err;
					}
				} else if (var_y > 1e-9) {
					double inv_slope = cov_xy / var_y;
					double intercept = mean_x - inv_slope * mean_y;
					for (const auto& p : undistorted_pts) {
						double err = p[0] - (inv_slope * p[1] + intercept);
						undist_curvature += err * err;
					}
				}

				double rms_undist_curvature = sqrt(undist_curvature / n_undist);

				// Penalty: undistortion should reduce curvature significantly
				// If raw curvature is high but undistorted is still high, bad params
				// Normalize by image size to make scale-independent
				double normalization = sz.cx;  // Normalize by image width
				double normalized_undist_curv = rms_undist_curvature / normalization;

				// Strong penalty for residual curvature after undistortion
				total_cost += 5000.0 * normalized_undist_curv;
			}
		}
	}

	// 4. Epipolar constraint: corresponding points must satisfy epipolar geometry
	// This provides scale-independent constraint that fixes focal-distortion degeneracy
	double eye_dist = 0.055;  // Typical IPD in meters
	if (matches.GetCount() > 0) {
		// Use first 20 matches to check epipolar consistency
		int n_check = min(20, matches.GetCount());
		double epipolar_error_sum = 0;
		int valid_epipolar = 0;

		for (int i = 0; i < n_check; i++) {
			const auto& m = matches[i];
			Size sz = m.image_size;

			// Undistort both points
			vec2 left_px = m.left_px;
			vec2 right_px = m.right_px;

			double dx_l = left_px[0] - params.cx;
			double dy_l = left_px[1] - params.cy;
			double r_l = sqrt(dx_l * dx_l + dy_l * dy_l);

			double dx_r = right_px[0] - params.cx;
			double dy_r = right_px[1] - params.cy;
			double r_r = sqrt(dx_r * dx_r + dy_r * dy_r);

			if (r_l < 1e-9 || r_r < 1e-9) continue;

			double theta_l = PixelToAngle(params, r_l);
			double theta_r = PixelToAngle(params, r_r);

			if (!std::isfinite(theta_l) || !std::isfinite(theta_r)) continue;

			// Undistorted rays in camera space (normalized by focal length for scale invariance)
			double roll_l = atan2(dy_l, dx_l);
			double roll_r = atan2(dy_r, dx_r);

			vec3 ray_l((float)(tan(theta_l) * cos(roll_l)),
			          (float)(tan(theta_l) * sin(roll_l)),
			          1.0f);

			vec3 ray_r((float)(tan(theta_r) * cos(roll_r)),
			          (float)(tan(theta_r) * sin(roll_r)),
			          1.0f);

			// Epipolar constraint: rays from L and R should intersect in 3D
			// Approximate check: vertical disparity should be near zero after rotation correction
			// For simplified check without full extrinsics: horizontal disparity should increase with depth
			// More robust: check that triangulated depth is consistent

			// Simple check: rectified y-coordinates should match (vertical disparity ~ 0)
			double y_disparity = fabs(ray_l[1] - ray_r[1]);

			// Horizontal disparity should be positive (right image shifted left)
			double x_disparity = ray_l[0] - ray_r[0];

			// Penalty for large vertical disparity (should be near 0 for calibrated system)
			epipolar_error_sum += y_disparity;

			// Penalty if horizontal disparity is wrong sign or unreasonable
			if (x_disparity < 0 || x_disparity > 1.0) {
				epipolar_error_sum += 0.5;  // Wrong disparity direction or too large
			}

			valid_epipolar++;
		}

		if (valid_epipolar > 0) {
			double avg_epipolar_error = epipolar_error_sum / valid_epipolar;
			// Strong penalty for epipolar violations (scale-independent)
			total_cost += 10000.0 * avg_epipolar_error;
		}
	}

	// Heavy penalties for invalid focal lengths
	if (params.a <= 0) return 1e12;  // Negative or zero focal is completely invalid
	if (params.a < 10.0) total_cost += 1e9;  // Very small focal is suspicious
	if (!std::isfinite(total_cost)) return 1e12;
	return total_cost;
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

void StereoCalibrationSolver::GABootstrapExtrinsics(StereoCalibrationParams& params) {
	if (log) *log << "  Step: Genetic algorithm bootstrap (extrinsics)...\n";
	
	// Reparameterization for symmetry:
	// 0: base_yaw, 1: base_pitch, 2: base_roll (common offset)
	// 3: toe_yaw (symmetric horizontal angle: L=-toe, R=+toe)
	// 4,5,6: asym_yaw, asym_pitch, asym_roll (small asymmetry deltas)
	const int dimension = 7;
	GeneticOptimizer ga;
	ga.SetMaxGenerations(ga_generations);
	ga.SetRandomTypeUniform();
	ga.MinMax(-1.0, 1.0); 
	ga.Init(dimension, ga_population, StrategyBest1Exp);

	// Base angles: +/- 15 deg
	ga.min_values[0] = -15 * M_PI / 180.0; ga.max_values[0] = 15 * M_PI / 180.0;
	ga.min_values[1] = -15 * M_PI / 180.0; ga.max_values[1] = 15 * M_PI / 180.0;
	ga.min_values[2] = -15 * M_PI / 180.0; ga.max_values[2] = 15 * M_PI / 180.0;
	
	// Toe yaw: 0 to 45 deg
	ga.min_values[3] = 0; ga.max_values[3] = 45 * M_PI / 180.0;
	
	// Asymmetry deltas: +/- 5 deg
	ga.min_values[4] = -5 * M_PI / 180.0; ga.max_values[4] = 5 * M_PI / 180.0;
	ga.min_values[5] = -5 * M_PI / 180.0; ga.max_values[5] = 5 * M_PI / 180.0;
	ga.min_values[6] = -5 * M_PI / 180.0; ga.max_values[6] = 5 * M_PI / 180.0;

	auto Map = [&](const Vector<double>& v) {
		StereoCalibrationParams p = params;
		double by = v[0], bp = v[1], br = v[2];
		double ty = v[3];
		double ay = v[4], ap = v[5], ar = v[6];
		
		// Left eye: base - toe - delta/2
		p.yaw_l   = by - ty - ay * 0.5;
		p.pitch_l = bp      - ap * 0.5;
		p.roll_l  = br      - ar * 0.5;
		
		// Right eye: base + toe + delta/2
		p.yaw     = by + ty + ay * 0.5;
		p.pitch   = bp      + ap * 0.5;
		p.roll    = br      + ar * 0.5;
		
		return p;
	};

	ga.best_energy = 1e30;
	bool dumped_invalid = false;
	for (int i = 0; i < ga_population; i++) {
		for (int j = 0; j < dimension; j++) ga.population[i][j] = ga.RandomUniform(ga.min_values[j], ga.max_values[j]);
		StereoCalibrationParams p = Map(ga.population[i]);
		double cost = ComputeRobustCost(p);
		if (!std::isfinite(cost) || !IsParamsFinite(p)) {
			if (!dumped_invalid) {
				printf("GA EXTRINSICS: Invalid candidate at init %d. cost=%f, params finite=%d\n", i, cost, IsParamsFinite(p));
				dumped_invalid = true;
			}
			cost = 1e12;
		}
		ga.pop_energy[i] = cost;
		if (ga.pop_energy[i] < ga.best_energy) { 
			ga.best_energy = ga.pop_energy[i]; 
			ga.best_solution = clone(ga.population[i]);
			StereoCalibrationParams p_best = Map(ga.best_solution);
			printf("%s\n", ~Format("NEW BEST (init): eval=%d cost=%.4f L_y/p/r: %.2f/%.2f/%.2f R_y/p/r: %.2f/%.2f/%.2f", 
				i, ga.best_energy, 
				p_best.yaw_l*180/M_PI, p_best.pitch_l*180/M_PI, p_best.roll_l*180/M_PI,
				p_best.yaw*180/M_PI, p_best.pitch*180/M_PI, p_best.roll*180/M_PI));
		}
	}

	int current_gen = 0;
	double last_best = ga.best_energy;
	while (!ga.IsEnd()) {
		ga.Start();
		current_gen++;
		Vector<double> trial = clone(ga.GetTrialSolution());
		for(int j=0; j<dimension; j++) trial[j] = Clamp(trial[j], ga.min_values[j], ga.max_values[j]);
		StereoCalibrationParams p_trial = Map(trial);
		double cost = ComputeRobustCost(p_trial);
		if (!std::isfinite(cost) || !IsParamsFinite(p_trial)) {
			if (!dumped_invalid) {
				printf("GA EXTRINSICS: Invalid candidate at gen %d. cost=%f, params finite=%d\n", current_gen, cost, IsParamsFinite(p_trial));
				dumped_invalid = true;
			}
			cost = 1e12;
		}
		ga.Stop(cost); 
		if (ga.best_energy < last_best && IsParamsFinite(Map(ga.best_solution))) {
			last_best = ga.best_energy;
			StereoCalibrationParams p_best = Map(ga.best_solution);
			printf("%s\n", ~Format("NEW BEST: gen=%d eval=%d cost=%.4f L_y/p/r: %.2f/%.2f/%.2f R_y/p/r: %.2f/%.2f/%.2f", 
				current_gen, ga.GetRound(), last_best,
				p_best.yaw_l*180/M_PI, p_best.pitch_l*180/M_PI, p_best.roll_l*180/M_PI,
				p_best.yaw*180/M_PI, p_best.pitch*180/M_PI, p_best.roll*180/M_PI));
		}
		if (ga_step_cb) {
			if (!ga_step_cb(current_gen, ga.best_energy, Map(ga.best_solution))) return;
		}
	}
	params = Map(ga.best_solution);
}

void StereoCalibrationSolver::GABootstrapIntrinsics(StereoCalibrationParams& params) {
	if (log) *log << "  Step: Genetic algorithm bootstrap (intrinsics) with multi-start...\n";

	// Enable trace for first 2 evaluations to debug line straightness
	EnableTrace(true, 2, 10000);

	const int dimension = 5;

	auto Map = [&](const Vector<double>& v, const StereoCalibrationParams& base_params) {
		StereoCalibrationParams p = base_params;
		// Clamp FOV to reasonable range to avoid negative/tiny focal lengths
		double fov_deg = Clamp(v[0], 80.0, 150.0);  // Max 150° to keep focal > 0
		double fov_rad = fov_deg * M_PI / 180.0;
		double w = matches.GetCount() > 0 ? matches[0].image_size.cx : 1280;
		p.a = (w * 0.5) / tan(fov_rad * 0.5);
		// Ensure focal is always positive and reasonable
		if (p.a <= 0 || !std::isfinite(p.a)) p.a = 50.0;  // Fallback
		if (p.a < 20.0) p.a = 20.0;  // Minimum reasonable focal
		p.cx = v[1]; p.cy = v[2];
		p.c = p.a * v[3]; p.d = p.a * v[4];
		p.b = 0;
		// Intrinsics are shared; extrinsics remain as-is (e.g. from current bootstrap state)
		return p;
	};

	// Multi-start optimization: run GA from 3 different initial guesses
	const int num_starts = 3;
	Vector<double> start_k1_values;
	start_k1_values.Add(-0.4);  // Strong barrel (typical for wide-angle)
	start_k1_values.Add(-0.1);  // Moderate barrel
	start_k1_values.Add(0.0);   // No distortion (fallback)

	double global_best_cost = 1e30;
	Vector<double> global_best_solution;
	StereoCalibrationParams global_best_params;

	for (int start_idx = 0; start_idx < num_starts; start_idx++) {
		printf("\n=== MULTI-START %d/%d: k1_init=%.3f ===\n", start_idx + 1, num_starts, start_k1_values[start_idx]);

		GeneticOptimizer ga;
		ga.SetMaxGenerations(ga_generations);
		ga.SetRandomTypeUniform();
		ga.MinMax(-1.0, 1.0);
		ga.Init(dimension, ga_population, StrategyBest1Exp);

		ga.min_values[0] = ga_bounds_intr.fov_min;
		ga.max_values[0] = ga_bounds_intr.fov_max;
		ga.min_values[1] = params.cx - ga_bounds_intr.cx_delta;
		ga.max_values[1] = params.cx + ga_bounds_intr.cx_delta;
		ga.min_values[2] = params.cy - ga_bounds_intr.cy_delta;
		ga.max_values[2] = params.cy + ga_bounds_intr.cy_delta;
		ga.min_values[3] = ga_bounds_intr.k1_min;
		ga.max_values[3] = ga_bounds_intr.k1_max;
		ga.min_values[4] = ga_bounds_intr.k2_min;
		ga.max_values[4] = ga_bounds_intr.k2_max;

		ga.best_energy = 1e30;
		bool dumped_invalid = false;

		// Initialize population with bias toward current start_k1
		for (int i = 0; i < ga_population; i++) {
			for (int j = 0; j < dimension; j++) {
				if (j == 3 && i < ga_population / 2) {
					// First half of population: bias toward this start's k1 value
					double k1_center = start_k1_values[start_idx];
					double k1_range = 0.2;  // +/- 0.2 around center
					ga.population[i][j] = Clamp(k1_center + (ga.RandomUniform(-1, 1) * k1_range),
					                             ga.min_values[j], ga.max_values[j]);
				} else {
					ga.population[i][j] = ga.RandomUniform(ga.min_values[j], ga.max_values[j]);
				}
			}

			StereoCalibrationParams p = Map(ga.population[i], params);
			double cost = ComputeRobustCost(p);
			if (!std::isfinite(cost) || !IsParamsFinite(p)) {
				if (!dumped_invalid) {
					printf("GA INTRINSICS (start %d): Invalid candidate at init %d. cost=%f, params finite=%d\n",
					       start_idx, i, cost, IsParamsFinite(p));
					dumped_invalid = true;
				}
				cost = 1e12;
			}
			ga.pop_energy[i] = cost;
			if (ga.pop_energy[i] < ga.best_energy) {
				ga.best_energy = ga.pop_energy[i];
				ga.best_solution = clone(ga.population[i]);
				StereoCalibrationParams p_best = Map(ga.best_solution, params);
				printf("%s\n", ~Format("NEW BEST (start %d, init): eval=%d cost=%.4f f=%.2f cx=%.2f cy=%.2f k1=%.4f k2=%.4f",
					start_idx, i, ga.best_energy, p_best.a, p_best.cx, p_best.cy,
					p_best.c/p_best.a, p_best.d/p_best.a));
			}
		}

		int current_gen = 0;
		double last_best = ga.best_energy;
		while (!ga.IsEnd()) {
			ga.Start();
			current_gen++;
			Vector<double> trial = clone(ga.GetTrialSolution());
			for(int j=0; j<dimension; j++) trial[j] = Clamp(trial[j], ga.min_values[j], ga.max_values[j]);
			StereoCalibrationParams p_trial = Map(trial, params);
			double cost = ComputeRobustCost(p_trial);
			if (!std::isfinite(cost) || !IsParamsFinite(p_trial)) {
				if (!dumped_invalid) {
					printf("GA INTRINSICS (start %d): Invalid candidate at gen %d. cost=%f, params finite=%d\n",
					       start_idx, current_gen, cost, IsParamsFinite(p_trial));
					dumped_invalid = true;
				}
				cost = 1e12;
			}
			ga.Stop(cost);
			if (ga.best_energy < last_best && IsParamsFinite(Map(ga.best_solution, params))) {
				last_best = ga.best_energy;
				StereoCalibrationParams p_best = Map(ga.best_solution, params);
				printf("%s\n", ~Format("NEW BEST (start %d): gen=%d eval=%d cost=%.4f f=%.2f cx=%.2f cy=%.2f k1=%.4f k2=%.4f",
					start_idx, current_gen, ga.GetRound(), last_best, p_best.a, p_best.cx, p_best.cy,
					p_best.c/p_best.a, p_best.d/p_best.a));
			}
			if (ga_step_cb) {
				if (!ga_step_cb(current_gen, ga.best_energy, Map(ga.best_solution, params))) {
					// Update global best if this start found better solution
					if (ga.best_energy < global_best_cost) {
						global_best_cost = ga.best_energy;
						global_best_solution = clone(ga.best_solution);
						global_best_params = Map(ga.best_solution, params);
					}
					params = global_best_params;
					EnableTrace(false);
					return;
				}
			}
		}

		// Check if this start found the global best
		if (ga.best_energy < global_best_cost) {
			global_best_cost = ga.best_energy;
			global_best_solution = clone(ga.best_solution);
			global_best_params = Map(ga.best_solution, params);
			printf("\n>>> NEW GLOBAL BEST from start %d: cost=%.4f <<<\n", start_idx, global_best_cost);
		}
	}

	// Use the best solution across all starts
	params = global_best_params;
	printf("\n=== MULTI-START COMPLETE ===\n");
	printf("Global best: cost=%.4f f=%.2f cx=%.2f cy=%.2f k1=%.4f k2=%.4f\n",
	       global_best_cost, params.a, params.cx, params.cy,
	       params.c/params.a, params.d/params.a);

	// Disable trace after GA
	EnableTrace(false);
}

void StereoCalibrationSolver::GABootstrapJoint(StereoCalibrationParams& params) {
	if (log) *log << "  Step: Joint optimization (intrinsics + extrinsics simultaneously)...\n";
	printf("\n=== JOINT OPTIMIZATION: All parameters together ===\n");

	// 11 dimensions: [fov, cx, cy, k1, k2, base_yaw, base_pitch, base_roll, toe_yaw, asym_yaw, asym_pitch]
	// Symmetric parametrization to enforce near-symmetry
	const int dimension = 11;

	GeneticOptimizer ga;
	ga.SetMaxGenerations(ga_generations);
	ga.SetRandomTypeUniform();
	ga.MinMax(-1.0, 1.0);
	ga.Init(dimension, ga_population, StrategyBest1Exp);

	// Bounds
	ga.min_values[0] = ga_bounds_intr.fov_min;
	ga.max_values[0] = ga_bounds_intr.fov_max;
	ga.min_values[1] = params.cx - ga_bounds_intr.cx_delta;
	ga.max_values[1] = params.cx + ga_bounds_intr.cx_delta;
	ga.min_values[2] = params.cy - ga_bounds_intr.cy_delta;
	ga.max_values[2] = params.cy + ga_bounds_intr.cy_delta;
	ga.min_values[3] = ga_bounds_intr.k1_min;
	ga.max_values[3] = ga_bounds_intr.k1_max;
	ga.min_values[4] = ga_bounds_intr.k2_min;
	ga.max_values[4] = ga_bounds_intr.k2_max;

	// Extrinsics: base angles +/- 15 deg
	ga.min_values[5] = -15 * M_PI / 180.0; ga.max_values[5] = 15 * M_PI / 180.0;  // base_yaw
	ga.min_values[6] = -15 * M_PI / 180.0; ga.max_values[6] = 15 * M_PI / 180.0;  // base_pitch
	ga.min_values[7] = -15 * M_PI / 180.0; ga.max_values[7] = 15 * M_PI / 180.0;  // base_roll
	ga.min_values[8] = 0; ga.max_values[8] = 45 * M_PI / 180.0;  // toe_yaw (0 to 45 deg)
	ga.min_values[9] = -5 * M_PI / 180.0; ga.max_values[9] = 5 * M_PI / 180.0;   // asym_yaw
	ga.min_values[10] = -5 * M_PI / 180.0; ga.max_values[10] = 5 * M_PI / 180.0; // asym_pitch

	auto Map = [&](const Vector<double>& v) {
		StereoCalibrationParams p;

		// Intrinsics
		double fov_deg = Clamp(v[0], 80.0, 150.0);
		double fov_rad = fov_deg * M_PI / 180.0;
		double w = matches.GetCount() > 0 ? matches[0].image_size.cx : 1280;
		p.a = (w * 0.5) / tan(fov_rad * 0.5);
		if (p.a <= 0 || !std::isfinite(p.a)) p.a = 50.0;
		if (p.a < 20.0) p.a = 20.0;

		p.cx = v[1];
		p.cy = v[2];
		p.c = p.a * v[3];  // k1
		p.d = p.a * v[4];  // k2
		p.b = 0;

		// Extrinsics (symmetric)
		double base_yaw = v[5];
		double base_pitch = v[6];
		double base_roll = v[7];
		double toe_yaw = v[8];
		double asym_yaw = v[9];
		double asym_pitch = v[10];

		// Left: base - toe - asym/2
		p.yaw_l = base_yaw - toe_yaw - asym_yaw * 0.5;
		p.pitch_l = base_pitch - asym_pitch * 0.5;
		p.roll_l = base_roll;

		// Right: base + toe + asym/2
		p.yaw = base_yaw + toe_yaw + asym_yaw * 0.5;
		p.pitch = base_pitch + asym_pitch * 0.5;
		p.roll = base_roll;

		return p;
	};

	// Initialize population with bias toward barrel distortion
	ga.best_energy = 1e30;
	bool dumped_invalid = false;
	for (int i = 0; i < ga_population; i++) {
		for (int j = 0; j < dimension; j++) {
			if (j == 3 && i < ga_population / 3) {
				// First third: bias toward strong barrel
				ga.population[i][j] = Clamp(-0.4 + ga.RandomUniform(-0.2, 0.2),
				                             ga.min_values[j], ga.max_values[j]);
			} else if (j == 3 && i < 2 * ga_population / 3) {
				// Second third: moderate barrel
				ga.population[i][j] = Clamp(-0.15 + ga.RandomUniform(-0.1, 0.1),
				                             ga.min_values[j], ga.max_values[j]);
			} else {
				ga.population[i][j] = ga.RandomUniform(ga.min_values[j], ga.max_values[j]);
			}
		}

		StereoCalibrationParams p = Map(ga.population[i]);
		double cost = ComputeRobustCost(p);
		if (!std::isfinite(cost) || !IsParamsFinite(p)) {
			if (!dumped_invalid) {
				printf("GA JOINT: Invalid at init %d. cost=%f\n", i, cost);
				dumped_invalid = true;
			}
			cost = 1e12;
		}
		ga.pop_energy[i] = cost;
		if (ga.pop_energy[i] < ga.best_energy) {
			ga.best_energy = ga.pop_energy[i];
			ga.best_solution = clone(ga.population[i]);
			StereoCalibrationParams p_best = Map(ga.best_solution);
			printf("NEW BEST (init): eval=%d cost=%.2f f=%.1f k1=%.3f L_yaw=%.1f R_yaw=%.1f\n",
			       i, ga.best_energy, p_best.a, p_best.c/p_best.a,
			       p_best.yaw_l*180/M_PI, p_best.yaw*180/M_PI);
		}
	}

	int current_gen = 0;
	double last_best = ga.best_energy;
	while (!ga.IsEnd()) {
		ga.Start();
		current_gen++;
		Vector<double> trial = clone(ga.GetTrialSolution());
		for(int j=0; j<dimension; j++) trial[j] = Clamp(trial[j], ga.min_values[j], ga.max_values[j]);

		StereoCalibrationParams p_trial = Map(trial);
		double cost = ComputeRobustCost(p_trial);
		if (!std::isfinite(cost) || !IsParamsFinite(p_trial)) {
			cost = 1e12;
		}
		ga.Stop(cost);

		if (ga.best_energy < last_best && IsParamsFinite(Map(ga.best_solution))) {
			last_best = ga.best_energy;
			StereoCalibrationParams p_best = Map(ga.best_solution);
			printf("NEW BEST: gen=%d eval=%d cost=%.2f f=%.1f k1=%.3f k2=%.3f L_yaw=%.1f R_yaw=%.1f\n",
			       current_gen, ga.GetRound(), last_best, p_best.a,
			       p_best.c/p_best.a, p_best.d/p_best.a,
			       p_best.yaw_l*180/M_PI, p_best.yaw*180/M_PI);
		}
		if (ga_step_cb) {
			if (!ga_step_cb(current_gen, ga.best_energy, Map(ga.best_solution))) {
				params = Map(ga.best_solution);
				return;
			}
		}
	}

	params = Map(ga.best_solution);
	printf("\n=== JOINT OPTIMIZATION COMPLETE ===\n");
	printf("Final: f=%.2f cx=%.1f cy=%.1f k1=%.4f k2=%.4f\n",
	       params.a, params.cx, params.cy, params.c/params.a, params.d/params.a);
	printf("       L: yaw=%.2f pitch=%.2f roll=%.2f\n",
	       params.yaw_l*180/M_PI, params.pitch_l*180/M_PI, params.roll_l*180/M_PI);
	printf("       R: yaw=%.2f pitch=%.2f roll=%.2f\n",
	       params.yaw*180/M_PI, params.pitch*180/M_PI, params.roll*180/M_PI);
}

void StereoCalibrationSolver::GABootstrapPipeline(StereoCalibrationParams& params, GAPhase phase) {
	if (phase == GA_PHASE_BOTH_JOINT) {
		if (log) *log << "Starting GA Joint Phase...\n";
		printf("GA: Starting Joint Phase (pop=%d, gens=%d)\n", ga_population, ga_generations);
		GABootstrapJoint(params);
		printf("GA: Joint Phase Finished.\n");
		return;
	}

	if (phase == GA_PHASE_INTRINSICS || phase == GA_PHASE_BOTH) {
		if (log) *log << "Starting GA Intrinsics Phase...\n";
		printf("GA: Starting Intrinsics Phase (pop=%d, gens=%d)\n", ga_population, ga_generations);
		GABootstrapIntrinsics(params);
		printf("%s\n", ~Format("GA: Intrinsics Phase Finished. f=%.2f, cx=%.2f, cy=%.2f, k1=%.4f, k2=%.4f",
			params.a, params.cx, params.cy, params.c/params.a, params.d/params.a));
	}
	if (phase == GA_PHASE_EXTRINSICS || phase == GA_PHASE_BOTH) {
		if (log) *log << "Starting GA Extrinsics Phase...\n";
		printf("GA: Starting Extrinsics Phase (pop=%d, gens=%d)\n", ga_population, ga_generations);
		GABootstrapExtrinsics(params);
		printf("%s\n", ~Format("GA: Extrinsics Phase Finished. yaw=%.3f, pitch=%.3f, roll=%.3f",
			params.yaw * 180.0/M_PI, params.pitch * 180.0/M_PI, params.roll * 180.0/M_PI));
	}
}

bool StereoCalibrationSolver::SolveIntrinsicsOnly(StereoCalibrationParams& params) {
	last_failure_reason.Clear();
	int N = matches.GetCount();
	if (N < 5) return false;

	if (use_ga_init) {
		GABootstrapIntrinsics(params);
	}

	Eigen::VectorXd y(6 + 3 * N);
	y[0] = params.a;
	y[1] = params.b;
	y[2] = params.c;
	y[3] = params.d;
	y[4] = params.cx;
	y[5] = params.cy;
	
	StereoCalibrationParams base_params = params;
	vec3 pL(-(float)eye_dist / 2.0f, 0, 0);
	vec3 pR((float)eye_dist / 2.0f, 0, 0);
	
	for(int i = 0; i < N; i++) {
		vec3 dL, dR;
		UnprojectDir(base_params, matches[i].left_px, 0, dL);
		UnprojectDir(base_params, matches[i].right_px, 1, dR);
		vec3 pt = TriangulatePoint(pL, dL, pR, dR);
		y[6 + i * 3 + 0] = pt[0];
		y[6 + i * 3 + 1] = pt[1];
		y[6 + i * 3 + 2] = pt[2];
	}

	int iter = 0;
	auto residual = [&](const Eigen::VectorXd& x, Eigen::VectorXd& res) {
		iter++;
		StereoCalibrationParams cur = base_params;
		cur.a = x[0];
		cur.b = x[1];
		cur.c = x[2];
		cur.d = x[3];
		cur.cx = x[4];
		cur.cy = x[5];

		res.resize(N * 6);
		for (int i = 0; i < N; i++) {
			const auto& m = matches[i];
			vec3 X((float)x[6 + i * 3 + 0], (float)x[6 + i * 3 + 1], (float)x[6 + i * 3 + 2]);
			vec2 projL, projR;
			double zL = 0, zR = 0;
			ProjectPoint(cur, X, 0, eye_dist, projL, zL);
			ProjectPoint(cur, X, 1, eye_dist, projR, zR);

			res[i * 6 + 0] = projL[0] - m.left_px[0];
			res[i * 6 + 1] = projL[1] - m.left_px[1];
			res[i * 6 + 2] = projR[0] - m.right_px[0];
			res[i * 6 + 3] = projR[1] - m.right_px[1];
			res[i * 6 + 4] = (m.dist_l > 0) ? ((X - pL).GetLength() - m.dist_l) * dist_weight : 0;
			res[i * 6 + 5] = (m.dist_r > 0) ? ((X - pR).GetLength() - m.dist_r) * dist_weight : 0;
		}
		return 0;
	};

	bool opt_success = NonLinearOptimization(y, N * 6, residual, 1e-10, 1e-10, 1000);
	if (opt_success) {
		params.a = y[0];
		params.b = y[1];
		params.c = y[2];
		params.d = y[3];
		params.cx = y[4];
		params.cy = y[5];
		
		last_points.SetCount(N);
		for (int i = 0; i < N; i++) {
			last_points[i][0] = (float)y[6 + i * 3 + 0];
			last_points[i][1] = (float)y[6 + i * 3 + 1];
			last_points[i][2] = (float)y[6 + i * 3 + 2];
		}
	}
	return opt_success;
}

bool StereoCalibrationSolver::SolveExtrinsicsOnlyMicroRefine(StereoCalibrationParams& params, 
                                                             const vec3& bounds_deg, 
                                                             double lambda, 
                                                             bool per_eye_mode) {
	last_failure_reason.Clear();
	int N = matches.GetCount();
	if (N < 5) return false;

	int num_delta = per_eye_mode ? 6 : 3;
	// Layout: 
	// If relative: 0=dyaw, 1=dpitch, 2=droll (for Right)
	// If per-eye: 0..2 (Left dyaw,dpitch,droll), 3..5 (Right dyaw,dpitch,droll)
	// + 3 * N point coords
	Eigen::VectorXd y(num_delta + 3 * N);
	for(int i=0; i<num_delta; i++) y[i] = 0;
	
	StereoCalibrationParams base_params = params;
	vec3 pL(-(float)eye_dist / 2.0f, 0, 0);
	vec3 pR((float)eye_dist / 2.0f, 0, 0);
	
	// Initialize 3D points
	for(int i = 0; i < N; i++) {
		vec3 dL, dR;
		UnprojectDir(base_params, matches[i].left_px, 0, dL);
		UnprojectDir(base_params, matches[i].right_px, 1, dR);
		vec3 pt = TriangulatePoint(pL, dL, pR, dR);
		y[num_delta + i * 3 + 0] = pt[0];
		y[num_delta + i * 3 + 1] = pt[1];
		y[num_delta + i * 3 + 2] = pt[2];
	}

	double max_dy = bounds_deg[0] * M_PI / 180.0;
	double max_dp = bounds_deg[1] * M_PI / 180.0;
	double max_dr = bounds_deg[2] * M_PI / 180.0;

	if (trace.enabled) {
		trace.Add("========================================\n");
		trace.Add("Stage C Micro-Refine\n");
		trace.Add("========================================\n\n");
		trace.Addf("Mode: %s\n", per_eye_mode ? "Per-eye" : "Relative-only");
		trace.Addf("Lambda: %.6f\n", lambda);
		trace.Addf("Bounds (deg): yaw=%.2f, pitch=%.2f, roll=%.2f\n", bounds_deg[0], bounds_deg[1], bounds_deg[2]);
		trace.Addf("Bounds (rad): yaw=%.6f, pitch=%.6f, roll=%.6f\n", max_dy, max_dp, max_dr);
		trace.Addf("Initial Extrinsics:\n");
		trace.Addf("  L: %.6f, %.6f, %.6f\n", base_params.yaw_l, base_params.pitch_l, base_params.roll_l);
		trace.Addf("  R: %.6f, %.6f, %.6f\n", base_params.yaw, base_params.pitch, base_params.roll);
	}

	int iter = 0;
	auto residual = [&](const Eigen::VectorXd& x, Eigen::VectorXd& res) {
		iter++;
		StereoCalibrationParams cur = base_params;
		
		double dy_L = 0, dp_L = 0, dr_L = 0;
		double dy_R = 0, dp_R = 0, dr_R = 0;
		
		if (per_eye_mode) {
			dy_L = Clamp(x[0], -max_dy, max_dy);
			dp_L = Clamp(x[1], -max_dp, max_dp);
			dr_L = Clamp(x[2], -max_dr, max_dr);
			dy_R = Clamp(x[3], -max_dy, max_dy);
			dp_R = Clamp(x[4], -max_dp, max_dp);
			dr_R = Clamp(x[5], -max_dr, max_dr);
		} else {
			dy_R = Clamp(x[0], -max_dy, max_dy);
			dp_R = Clamp(x[1], -max_dp, max_dp);
			dr_R = Clamp(x[2], -max_dr, max_dr);
		}
		
		cur.yaw_l += dy_L;
		cur.pitch_l += dp_L;
		cur.roll_l += dr_L;
		cur.yaw += dy_R;
		cur.pitch += dp_R;
		cur.roll += dr_R;

		res.resize(N * 6 + num_delta); // N matches * 6 residuals + regularization
		
		for (int i = 0; i < N; i++) {
			const auto& m = matches[i];
			vec3 X((float)x[num_delta + i * 3 + 0], (float)x[num_delta + i * 3 + 1], (float)x[num_delta + i * 3 + 2]);
			vec2 projL, projR;
			double zL = 0, zR = 0;
			ProjectPoint(cur, X, 0, eye_dist, projL, zL);
			ProjectPoint(cur, X, 1, eye_dist, projR, zR);

			res[i * 6 + 0] = projL[0] - m.left_px[0];
			res[i * 6 + 1] = projL[1] - m.left_px[1];
			res[i * 6 + 2] = projR[0] - m.right_px[0];
			res[i * 6 + 3] = projR[1] - m.right_px[1];
			res[i * 6 + 4] = (m.dist_l > 0) ? ((X - pL).GetLength() - m.dist_l) * dist_weight : 0;
			res[i * 6 + 5] = (m.dist_r > 0) ? ((X - pR).GetLength() - m.dist_r) * dist_weight : 0;
		}
		
		// Regularization: lambda * ||delta||^2 -> residual term sqrt(lambda) * delta
		// We want cost += lambda * delta^2. LM minimizes sum(res^2).
		// So res = sqrt(lambda) * delta.
		double sqrt_lambda = sqrt(max(0.0, lambda));
		if (per_eye_mode) {
			res[N * 6 + 0] = x[0] * sqrt_lambda;
			res[N * 6 + 1] = x[1] * sqrt_lambda;
			res[N * 6 + 2] = x[2] * sqrt_lambda;
			res[N * 6 + 3] = x[3] * sqrt_lambda;
			res[N * 6 + 4] = x[4] * sqrt_lambda;
			res[N * 6 + 5] = x[5] * sqrt_lambda;
		} else {
			res[N * 6 + 0] = x[0] * sqrt_lambda;
			res[N * 6 + 1] = x[1] * sqrt_lambda;
			res[N * 6 + 2] = x[2] * sqrt_lambda;
		}

		return 0;
	};

	int num_residuals = N * 6 + num_delta;
	bool opt_success = NonLinearOptimization(y, num_residuals, residual, 1e-10, 1e-10, 1000);
	if (opt_success) {
		double dy_L = 0, dp_L = 0, dr_L = 0;
		double dy_R = 0, dp_R = 0, dr_R = 0;
		
		if (per_eye_mode) {
			dy_L = Clamp(y[0], -max_dy, max_dy);
			dp_L = Clamp(y[1], -max_dp, max_dp);
			dr_L = Clamp(y[2], -max_dr, max_dr);
			dy_R = Clamp(y[3], -max_dy, max_dy);
			dp_R = Clamp(y[4], -max_dp, max_dp);
			dr_R = Clamp(y[5], -max_dr, max_dr);
		} else {
			dy_R = Clamp(y[0], -max_dy, max_dy);
			dp_R = Clamp(y[1], -max_dp, max_dp);
			dr_R = Clamp(y[2], -max_dr, max_dr);
		}
		
		params.yaw_l += dy_L;
		params.pitch_l += dp_L;
		params.roll_l += dr_L;
		params.yaw += dy_R;
		params.pitch += dp_R;
		params.roll += dr_R;
		
		last_points.SetCount(N);
		for (int i = 0; i < N; i++) {
			last_points[i][0] = (float)y[num_delta + i * 3 + 0];
			last_points[i][1] = (float)y[num_delta + i * 3 + 1];
			last_points[i][2] = (float)y[num_delta + i * 3 + 2];
		}

		if (trace.enabled) {
			trace.Addf("Final Extrinsics:\n");
			trace.Addf("  L: %.6f, %.6f, %.6f (Delta: %.6f, %.6f, %.6f)\n", 
				params.yaw_l, params.pitch_l, params.roll_l, dy_L, dp_L, dr_L);
			trace.Addf("  R: %.6f, %.6f, %.6f (Delta: %.6f, %.6f, %.6f)\n", 
				params.yaw, params.pitch, params.roll, dy_R, dp_R, dr_R);
		}
	} else {
		if (trace.enabled) trace.Add("Stage C Optimization FAILED.\n");
	}
	return opt_success;
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
		GABootstrapExtrinsics(params);

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

void StereoCalibrationSolver::ComputeGADiagnostics(const StereoCalibrationParams& params, double initial_cost, double final_cost, StereoCalibrationGADiagnostics& out) const {
	out.best_cost = final_cost;
	out.initial_cost = initial_cost;
	out.cost_improvement_ratio = (initial_cost > 1e-9) ? (initial_cost / final_cost) : 1.0;
	out.num_matches_used = matches.GetCount();
	// num_frames_used is unknown here, caller sets it.

	// Calculate stats
	StereoCalibrationDiagnostics diag;
	ComputeDiagnostics(params, diag); // Re-use existing diagnostics logic for reprojection errors

	// Aggregate from diag.residuals
	Vector<double> errors;
	double sum_err = 0;
	double max_err = 0;
	for(const auto& r : diag.residuals) {
		errors.Add(r.err_l_px);
		errors.Add(r.err_r_px);
		sum_err += r.err_l_px + r.err_r_px;
		max_err = max(max_err, max(r.err_l_px, r.err_r_px));
	}
	
	if (errors.GetCount() > 0) {
		out.mean_reproj_error_px = sum_err / errors.GetCount();
		out.max_reproj_error_px = max_err;
		Sort(errors);
		out.median_reproj_error_px = errors[errors.GetCount() / 2];
	} else {
		out.mean_reproj_error_px = 0;
		out.max_reproj_error_px = 0;
		out.median_reproj_error_px = 0;
	}
}

END_UPP_NAMESPACE
