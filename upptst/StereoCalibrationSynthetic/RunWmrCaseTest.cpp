#include "StereoCalibrationSynthetic.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

NAMESPACE_UPP

static double RadToDeg(double rad) {
	return rad * 180.0 / M_PI;
}

static double DegToRad(double deg) {
	return deg * M_PI / 180.0;
}

static double AngleToPixel(const StereoCalibrationParams& p, double theta) {
	double t2 = theta * theta;
	double t3 = t2 * theta;
	double t4 = t3 * theta;
	return p.a * theta + p.b * t2 + p.c * t3 + p.d * t4;
}

static bool ThetaRollFromDirection(const vec3& dir, double& theta, double& roll, double& zf) {
	vec3 d = dir;
	d.Normalize();
	zf = IS_NEGATIVE_Z ? -d[2] : d[2];
	double xy = sqrt((double)d[0] * d[0] + (double)d[1] * d[1]);
	double zf_safe = fabs(zf) < 1e-9 ? (zf < 0 ? -1e-9 : 1e-9) : zf;
	theta = atan2(xy, zf_safe);
	roll = atan2(-d[1], d[0]);
	return zf > 0;
}

static bool ProjectPoint(const StereoCalibrationParams& p, const vec3& point, int eye, double eye_dist, vec2& out_pix, double& zf_out) {
	vec3 cam_center = eye == 0 ? vec3(-(float)eye_dist / 2.0f, 0, 0) : vec3((float)eye_dist / 2.0f, 0, 0);
	vec3 v = point - cam_center;
	if (eye == 1) {
		mat4 rot = AxesMat((float)p.yaw, (float)p.pitch, (float)p.roll);
		v = (rot.GetTransposed() * v.Embed()).Splice();
	}
	if (v.GetLength() < 1e-9)
		return false;
	vec3 dir = v.GetNormalized();
	double theta = 0, roll = 0, zf = 0;
	if (!ThetaRollFromDirection(dir, theta, roll, zf))
		return false;
	double r = AngleToPixel(p, theta);
	out_pix = vec2((float)(p.cx + r * cos(roll)), (float)(p.cy + r * sin(roll)));
	zf_out = zf;
	return true;
}

static void PrintParams(const char* label, const StereoCalibrationParams& p) {
	Cout() << label << "\n";
	Cout() << "  a=" << p.a << " b=" << p.b << " c=" << p.c << " d=" << p.d << "\n";
	Cout() << "  cx=" << p.cx << " cy=" << p.cy << "\n";
	Cout() << "  yaw=" << p.yaw << " rad (" << RadToDeg(p.yaw) << " deg)\n";
	Cout() << "  pitch=" << p.pitch << " rad (" << RadToDeg(p.pitch) << " deg)\n";
	Cout() << "  roll=" << p.roll << " rad (" << RadToDeg(p.roll) << " deg)\n";
}

static void ComputeResiduals(const Vector<WmrCaseMatch>& matches, const StereoCalibrationParams& p, double eye_dist,
		double& rms_l, double& rms_r, double& dist_l, double& dist_r, int& behind_l, int& behind_r) {
	int count = 0;
	double sum_l = 0;
	double sum_r = 0;
	double sum_dl = 0;
	double sum_dr = 0;
	int count_dl = 0;
	int count_dr = 0;
	behind_l = 0;
	behind_r = 0;
	for (const auto& m : matches) {
		vec2 projL, projR;
		double zf_l = 0, zf_r = 0;
		ProjectPoint(p, m.point, 0, eye_dist, projL, zf_l);
		ProjectPoint(p, m.point, 1, eye_dist, projR, zf_r);
		if (zf_l <= 0) behind_l++;
		if (zf_r <= 0) behind_r++;
		double errL = (projL - m.left_px).GetLength();
		double errR = (projR - m.right_px).GetLength();
		sum_l += errL * errL;
		sum_r += errR * errR;
		count++;
		if (m.dist_l > 0) {
			sum_dl += (m.dist_l - (m.point - vec3((float)-eye_dist/2.0f,0,0)).GetLength())
				* (m.dist_l - (m.point - vec3((float)-eye_dist/2.0f,0,0)).GetLength());
			count_dl++;
		}
		if (m.dist_r > 0) {
			sum_dr += (m.dist_r - (m.point - vec3((float)eye_dist/2.0f,0,0)).GetLength())
				* (m.dist_r - (m.point - vec3((float)eye_dist/2.0f,0,0)).GetLength());
			count_dr++;
		}
	}
	rms_l = count ? sqrt(sum_l / count) : 0;
	rms_r = count ? sqrt(sum_r / count) : 0;
	dist_l = count_dl ? sqrt(sum_dl / count_dl) : 0;
	dist_r = count_dr ? sqrt(sum_dr / count_dr) : 0;
}

bool RunWmrCaseTest(const String& dataset_path) {
	WmrCaseData data;
	if (!LoadWmrCase(data, dataset_path))
		return false;
	if (data.matches.GetCount() != 9)
		return false;

	StereoCalibrationSolver solver;
	solver.eye_dist = data.eye_dist;
	solver.dist_weight = 10.0;
	solver.huber_mm = 200.0;
	solver.max_fevals = 3000;
	String solve_log;
	solver.log = &solve_log;

	for (const auto& m : data.matches) {
		StereoCalibrationMatch sm;
		sm.left_px = m.left_px;
		sm.right_px = m.right_px;
		sm.image_size = data.image_size;
		sm.dist_l = m.dist_l;
		sm.dist_r = m.dist_r;
		solver.matches.Add(sm);
	}

	StereoCalibrationParams init;
	init.a = data.gt.a * 0.98;
	init.b = data.gt.b * 0.95;
	init.c = data.gt.c * 1.05;
	init.d = data.gt.d * 0.95;
	init.cx = data.gt.cx + 1.0;
	init.cy = data.gt.cy - 1.0;
	init.yaw = data.gt.yaw + DegToRad(1.0);
	init.pitch = data.gt.pitch + DegToRad(0.5);
	init.roll = data.gt.roll - DegToRad(1.0);

	bool ok_stage1 = solver.Solve(init, true);
	bool ok_stage2 = solver.Solve(init, false);
	if (!ok_stage1 || !ok_stage2) {
		Cout() << "Solver failed to converge.\n";
		Cout() << solve_log << "\n";
		return false;
	}

	double rms_l_gt = 0, rms_r_gt = 0, dist_l_gt = 0, dist_r_gt = 0;
	double rms_l_est_on_gt = 0, rms_r_est_on_gt = 0, dist_l_est_on_gt = 0, dist_r_est_on_gt = 0;
	int behind_l_gt = 0, behind_r_gt = 0;
	int behind_l_est_on_gt = 0, behind_r_est_on_gt = 0;

	ComputeResiduals(data.matches, data.gt, data.eye_dist, rms_l_gt, rms_r_gt, dist_l_gt, dist_r_gt, behind_l_gt, behind_r_gt);
	ComputeResiduals(data.matches, init, data.eye_dist, rms_l_est_on_gt, rms_r_est_on_gt, dist_l_est_on_gt, dist_r_est_on_gt, behind_l_est_on_gt, behind_r_est_on_gt);

	StereoCalibrationDiagnostics diag_est;
	solver.ComputeDiagnostics(init, diag_est);

	PrintParams("Ground Truth", data.gt);
	if (data.left_yaw_deg != 0 || data.left_pitch_deg != 0 || data.left_roll_deg != 0 ||
		data.right_yaw_deg != 0 || data.right_pitch_deg != 0 || data.right_roll_deg != 0) {
		Cout() << "  left yaw/pitch/roll deg = " << data.left_yaw_deg << "/" << data.left_pitch_deg
			<< "/" << data.left_roll_deg << "\n";
		Cout() << "  right yaw/pitch/roll deg = " << data.right_yaw_deg << "/" << data.right_pitch_deg
			<< "/" << data.right_roll_deg << "\n";
	}
	PrintParams("Estimated", init);

	Cout() << "Residuals (GT): reproj L/R=" << rms_l_gt << "/" << rms_r_gt
		<< " px, dist L/R=" << dist_l_gt << "/" << dist_r_gt << " mm\n";
	Cout() << "Residuals (EST on GT points): reproj L/R=" << rms_l_est_on_gt << "/" << rms_r_est_on_gt
		<< " px, dist L/R=" << dist_l_est_on_gt << "/" << dist_r_est_on_gt << " mm\n";
	Cout() << "Solver residuals (EST fit): reproj L/R=" << diag_est.reproj_rms_l << "/" << diag_est.reproj_rms_r
		<< " px, dist L/R=" << diag_est.dist_rms_l << "/" << diag_est.dist_rms_r << " mm\n";
	Cout() << "Behind camera (EST fit) L/R: " << diag_est.behind_left << "/" << diag_est.behind_right << "\n";
	if (diag_est.dist_rms_l > 10.0 || diag_est.dist_rms_r > 10.0)
		Cout() << "WARN distance RMS from solver fit is high; check dist_weight or local minima.\n";

	bool pass = true;
	if (rms_l_est_on_gt > 1.5 || rms_r_est_on_gt > 1.5) {
		Cout() << "FAIL reprojection RMS too high.\n";
		pass = false;
	}
	if (dist_l_est_on_gt > 10.0 || dist_r_est_on_gt > 10.0) {
		Cout() << "FAIL distance RMS too high.\n";
		pass = false;
	}
	if (behind_l_est_on_gt > 0 || behind_r_est_on_gt > 0) {
		Cout() << "FAIL points behind camera.\n";
		pass = false;
	}
	if (rms_l_gt > 0 && rms_r_gt > 0) {
		double reproj_ratio = max(rms_l_est_on_gt, rms_r_est_on_gt) / max(rms_l_gt, rms_r_gt);
		if (reproj_ratio > 3.0) {
			Cout() << "FAIL reprojection ratio too high: " << reproj_ratio << "\n";
			pass = false;
		}
	}

	if (!pass)
		Cout() << "Solver may be in a local minimum.\n";

	return pass;
}

END_UPP_NAMESPACE
