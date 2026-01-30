#ifndef _Geometry_StereoCalibrationSolver_h_
#define _Geometry_StereoCalibrationSolver_h_

struct StereoCalibrationMatch : Moveable<StereoCalibrationMatch> {
	vec2 left_px;
	vec2 right_px;
	Size image_size;
	double dist_l = 0;
	double dist_r = 0;
};

struct StereoCalibrationParams {
	double a = 0;
	double b = 0;
	double c = 0;
	double d = 0;
	double cx = 0;
	double cy = 0;
	double yaw = 0;
	double pitch = 0;
	double roll = 0;
};

struct StereoCalibrationResidual : Moveable<StereoCalibrationResidual> {
	int match_index = -1;
	vec2 measured_l;
	vec2 measured_r;
	vec2 reproj_l;
	vec2 reproj_r;
	double err_l_px = 0;
	double err_r_px = 0;
	double dist_l_err = 0;
	double dist_r_err = 0;
	double z_l = 0;
	double z_r = 0;
	double disparity_px = 0;
	vec3 point;
};

struct StereoCalibrationDiagnostics {
	double reproj_rms_l = 0;
	double reproj_rms_r = 0;
	double dist_rms_l = 0;
	double dist_rms_r = 0;
	int reproj_count_l = 0;
	int reproj_count_r = 0;
	int dist_count_l = 0;
	int dist_count_r = 0;
	int behind_left = 0;
	int behind_right = 0;
	Vector<StereoCalibrationResidual> residuals;
};

class StereoCalibrationSolver {
public:
	Vector<StereoCalibrationMatch> matches;
	Vector<vec3> last_points;
	double eye_dist = 0;
	double dist_weight = 0.1;
	double huber_px = 2.0;
	double huber_mm = 30.0;
	int max_fevals = 0;
	String* log = NULL;

	bool Solve(StereoCalibrationParams& params, bool lock_distortion);
	void ComputeDiagnostics(const StereoCalibrationParams& params, StereoCalibrationDiagnostics& out) const;
};

#endif
