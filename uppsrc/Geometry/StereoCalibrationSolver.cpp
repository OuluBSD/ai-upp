#include "Geometry.h"
#include <plugin/Eigen/Eigen.h>

NAMESPACE_UPP

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

bool StereoCalibrationSolver::Solve(StereoCalibrationParams& params, bool lock_distortion) {
	int N = matches.GetCount();
	if (N < 5)
		return false;
	if (fabs(eye_dist) < 1e-9)
		return false;

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

	for (int i = 0; i < N; i++) {
		const auto& m = matches[i];
		vec3 dL, dR;
		UnprojectDir(init, m.left_px, 0, dL);
		UnprojectDir(init, m.right_px, 1, dR);
		vec3 pt = TriangulatePoint(pL, dL, pR, dR);
		y[global_params + i * 3 + 0] = pt[0];
		y[global_params + i * 3 + 1] = pt[1];
		y[global_params + i * 3 + 2] = pt[2];
	}

	int iter = 0;
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

		res.resize(N * 6);
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
			double h4 = HuberWeight(r4s, huber_mm * w_mm);
			double h5 = HuberWeight(r5s, huber_mm * w_mm);

			res[i * 6 + 0] = r0s * h0;
			res[i * 6 + 1] = r1s * h1;
			res[i * 6 + 2] = r2s * h2;
			res[i * 6 + 3] = r3s * h3;
			res[i * 6 + 4] = r4s * h4;
			res[i * 6 + 5] = r5s * h5;
		}
		return 0;
	};

	if (log) {
		*log << "  Step: Non-linear optimization (Levenberg-Marquardt)...\n";
	}

	int maxfev = max_fevals > 0 ? max_fevals : max(200, 10 * (global_params + 3 * N));
	if (!NonLinearOptimization(y, N * 6, residual, 1e-10, 1e-10, maxfev)) {
		if (log)
			*log << "  Optimization FAILED.\n\n";
		return false;
	}

	if (log)
		*log << "  Optimization converged in " << iter << " iterations.\n\n";

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
}

END_UPP_NAMESPACE
