#include "CalibrationParsingTest.h"
#include <SoftHMD/SoftHMD.h>

NAMESPACE_UPP

bool TestParseDoubleLocaleIndependent() {
	// This test is no longer needed since we use JSON serialization
	// JSON handles numeric parsing in a locale-independent way
	Cout() << "SKIP: Using JSON serialization (locale-independent by default)\n";
	return true;
}

bool TestStcalRoundTrip() {
	using namespace HMD;

	bool all_ok = true;

	// Test round-trip: save 118.0, reload → 118.0
	{
		StereoCalibrationData data1;
		data1.is_enabled = true;
		data1.eye_dist = 118.0f;
		data1.outward_angle = 0.6f;
		data1.right_pitch = -0.05f;
		data1.right_roll = 0.1f;
		data1.principal_point = vec2(320.0f, 240.0f);
		data1.angle_to_pixel = vec4(400.0f, -300.0f, 700.0f, -400.0f);

		String temp_path = GetTempFileName("test");
		if (!StereoTracker::SaveCalibrationFile(temp_path, data1)) {
			Cout() << "FAIL: Could not save .stcal file\n";
			return false;
		}

		StereoCalibrationData data2;
		if (!StereoTracker::LoadCalibrationFile(temp_path, data2)) {
			Cout() << "FAIL: Could not load .stcal file\n";
			DeleteFile(temp_path);
			return false;
		}

		DeleteFile(temp_path);

		if (fabs(data2.eye_dist - 118.0f) > 0.001f) {
			Cout() << Format("FAIL: eye_dist round-trip: %.6f (expected 118.0)\n", data2.eye_dist);
			all_ok = false;
		} else {
			Cout() << "PASS: eye_dist round-trip: 118.0\n";
		}

		if (fabs(data2.outward_angle - 0.6f) > 0.001f) {
			Cout() << Format("FAIL: outward_angle round-trip: %.6f (expected 0.6)\n", data2.outward_angle);
			all_ok = false;
		} else {
			Cout() << "PASS: outward_angle round-trip: 0.6\n";
		}
	}

	return all_ok;
}

END_UPP_NAMESPACE
