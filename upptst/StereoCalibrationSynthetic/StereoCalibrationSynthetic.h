#ifndef _StereoCalibrationSynthetic_StereoCalibrationSynthetic_h_
#define _StereoCalibrationSynthetic_StereoCalibrationSynthetic_h_

#include <Geometry/Geometry.h>

NAMESPACE_UPP

struct WmrCaseMatch : Moveable<WmrCaseMatch> {
	vec2 left_px;
	vec2 right_px;
	double dist_l = 0;  // meters
	double dist_r = 0;  // meters
	vec3 point;  // meters
};

struct WmrCaseData {
	Size image_size;
	double eye_dist = 0;  // meters
	double fov_x_deg = 0;
	double fov_y_deg = 0;
	double left_yaw_deg = 0;
	double left_pitch_deg = 0;
	double left_roll_deg = 0;
	double right_yaw_deg = 0;
	double right_pitch_deg = 0;
	double right_roll_deg = 0;
	StereoCalibrationParams gt;
	Vector<WmrCaseMatch> matches;
};

bool GenerateWmrCase(WmrCaseData& out, int seed);
bool SaveWmrCase(const WmrCaseData& data, const String& path);
bool LoadWmrCase(WmrCaseData& out, const String& path);
bool RunWmrCaseTest(const String& dataset_path);

END_UPP_NAMESPACE

#endif
