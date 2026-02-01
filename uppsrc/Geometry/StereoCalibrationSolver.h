#ifndef _Geometry_StereoCalibrationSolver_h_
#define _Geometry_StereoCalibrationSolver_h_

// UNIT CONVENTION:
// All lengths (distances, positions, baseline, etc.) are in METERS inside the solver.
// Conversion to/from millimeters happens only at UI boundary.

struct StereoCalibrationMatch : Moveable<StereoCalibrationMatch> {
	vec2 left_px;
	vec2 right_px;
	Size image_size;
	double dist_l = 0;  // meters (measured distance from left camera to 3D point)
	double dist_r = 0;  // meters (measured distance from right camera to 3D point)
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
	// Left eye rotation (optional, used for per-eye refinement)
	double yaw_l = 0;
	double pitch_l = 0;
	double roll_l = 0;
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

struct StereoCalibrationTrace {
	bool enabled = false;
	int verbosity = 2;
	int max_lines = 20000;
	int64 max_bytes = 5000000;
	Vector<String> lines;
	int64 total_bytes = 0;

	void Add(const String& s);
	void Addf(const char* fmt, ...);
	String GetText() const;
	void Clear();
};

class StereoCalibrationSolver {
public:
	Vector<StereoCalibrationMatch> matches;
	Vector<vec3> last_points;
	double eye_dist = 0;  // meters
	double dist_weight = 0.1;  // relative weight for distance residuals vs pixel residuals
	double huber_px = 2.0;  // Huber threshold for pixel residuals (pixels)
	double huber_m = 0.030;  // Huber threshold for distance residuals (meters, e.g. 30mm)
	int max_fevals = 0;
	String* log = NULL;
	mutable StereoCalibrationTrace trace;
	String last_failure_reason;

	struct GABounds {
		double yaw_min = -15.0; // degrees
		double yaw_max = 15.0;
		double pitch_min = -10.0;
		double pitch_max = 10.0;
		double roll_min = -15.0;
		double roll_max = 15.0;
	};

	struct GABoundsIntrinsics {
		double fov_min = 80.0; // degrees
		double fov_max = 160.0;
		double cx_delta = 40.0; // pixels
		double cy_delta = 40.0;
		double k1_min = -1.0;
		double k1_max = 0.0;
		double k2_min = -0.5;
		double k2_max = 0.5;
	};

	// GA bootstrap parameters
	bool use_ga_init = false;
	int ga_population = 30;
	int ga_generations = 20;
	int ga_top_candidates = 3;  // Number of top GA candidates to refine with LM
	GABounds ga_bounds;
	GABoundsIntrinsics ga_bounds_intr;

	bool Solve(StereoCalibrationParams& params, bool lock_distortion);
	bool SolveIntrinsicsOnly(StereoCalibrationParams& params);
	void GABootstrapIntrinsics(StereoCalibrationParams& params);
	
	// SolveExtrinsicsOnlyMicroRefine
	// Optimizes extrinsics deltas relative to input 'params'.
	// If per_eye_mode is true, optimizes L and R deltas independently (6 DOF).
	// If false, optimizes R relative to L (3 DOF).
	// bounds_deg: {yaw, pitch, roll} limits in DEGREES.
	// lambda: regularization weight for ||delta||^2
	bool SolveExtrinsicsOnlyMicroRefine(StereoCalibrationParams& params, 
	                                    const vec3& bounds_deg, 
	                                    double lambda, 
	                                    bool per_eye_mode);

	void ComputeDiagnostics(const StereoCalibrationParams& params, StereoCalibrationDiagnostics& out) const;
	void EnableTrace(bool on, int verbosity = 2, int max_lines = 20000);
	String GetTraceText() const { return trace.GetText(); }
};

#endif
