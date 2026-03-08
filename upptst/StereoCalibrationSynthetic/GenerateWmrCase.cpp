#include "StereoCalibrationSynthetic.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

NAMESPACE_UPP

static double DegToRad(double deg) {
	return deg * M_PI / 180.0;
}

static double RadToDeg(double rad) {
	return rad * 180.0 / M_PI;
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

static double AngleToPixel(const StereoCalibrationParams& p, double theta) {
	double t2 = theta * theta;
	double t3 = t2 * theta;
	double t4 = t3 * theta;
	return p.a * theta + p.b * t2 + p.c * t3 + p.d * t4;
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

static double GaussianNoise(double sigma) {
	if (sigma <= 0)
		return 0.0;
	double u1 = max(1e-9, Randomf());
	double u2 = max(1e-9, Randomf());
	double mag = sigma * sqrt(-2.0 * log(u1));
	double z0 = mag * cos(2.0 * M_PI * u2);
	return z0;
}

static void WriteParams(String& out, const char* name, double v) {
	out << name << "=" << Format("%.8f", v) << "\n";
}

bool GenerateWmrCase(WmrCaseData& out, int seed) {
	out = WmrCaseData();
	out.image_size = Size(640, 480);
	out.eye_dist = 0.063;  // 63mm in meters
	out.fov_x_deg = 110.0;
	out.fov_y_deg = out.fov_x_deg * out.image_size.cy / out.image_size.cx;

	double f = (out.image_size.cx * 0.5) / tan(DegToRad(out.fov_x_deg * 0.5));
	StereoCalibrationParams gt;
	gt.a = f;
	gt.b = -12.0;
	gt.c = -2.0;
	gt.d = 0.4;
	gt.cx = 322.0;
	gt.cy = 238.0;

	out.left_yaw_deg = -20.0;
	out.left_pitch_deg = -5.0;
	out.left_roll_deg = 30.0;
	out.right_yaw_deg = 20.0;
	out.right_pitch_deg = -10.0;
	out.right_roll_deg = -30.0;

	gt.yaw = DegToRad(out.right_yaw_deg - out.left_yaw_deg);
	gt.pitch = DegToRad(out.right_pitch_deg - out.left_pitch_deg);
	gt.roll = DegToRad(out.right_roll_deg - out.left_roll_deg);
	out.gt = gt;

	SeedRandom((dword)seed);
	int target = 9;
	int attempts = 0;
	while (out.matches.GetCount() < target && attempts < target * 200) {
		attempts++;
		double x = (Random(400) - 200) * 0.001;  // -200 to 200 mm -> -0.2 to 0.2 m
		double y = (Random(300) - 150) * 0.001;  // -150 to 150 mm -> -0.15 to 0.15 m
		double z = (Random(800) + 400.0) * 0.001;  // 400 to 1200 mm -> 0.4 to 1.2 m
		if (IS_NEGATIVE_Z)
			z = -z;
		vec3 pt((float)x, (float)y, (float)z);

		vec2 lp, rp;
		double zf_l = 0, zf_r = 0;
		if (!ProjectPoint(gt, pt, 0, out.eye_dist, lp, zf_l))
			continue;
		if (!ProjectPoint(gt, pt, 1, out.eye_dist, rp, zf_r))
			continue;

		lp[0] += (float)GaussianNoise(0.3);
		lp[1] += (float)GaussianNoise(0.3);
		rp[0] += (float)GaussianNoise(0.3);
		rp[1] += (float)GaussianNoise(0.3);

		if (lp[0] < 0 || lp[0] >= out.image_size.cx || lp[1] < 0 || lp[1] >= out.image_size.cy)
			continue;
		if (rp[0] < 0 || rp[0] >= out.image_size.cx || rp[1] < 0 || rp[1] >= out.image_size.cy)
			continue;
		if (zf_l <= 0 || zf_r <= 0)
			continue;

		WmrCaseMatch m;
		m.point = pt;
		m.left_px = lp;
		m.right_px = rp;
		m.dist_l = (pt - vec3((float)-out.eye_dist / 2.0f, 0, 0)).GetLength();
		m.dist_r = (pt - vec3((float)out.eye_dist / 2.0f, 0, 0)).GetLength();
		out.matches.Add(m);
	}
	return out.matches.GetCount() == target;
}

bool SaveWmrCase(const WmrCaseData& data, const String& path) {
	String out;
	out << "# WMR synthetic stereo calibration dataset\n";
	out << "image_size=" << data.image_size.cx << "x" << data.image_size.cy << "\n";
	WriteParams(out, "eye_dist_m", data.eye_dist);
	WriteParams(out, "fov_x_deg", data.fov_x_deg);
	WriteParams(out, "fov_y_deg", data.fov_y_deg);
	WriteParams(out, "left_yaw_deg", data.left_yaw_deg);
	WriteParams(out, "left_pitch_deg", data.left_pitch_deg);
	WriteParams(out, "left_roll_deg", data.left_roll_deg);
	WriteParams(out, "right_yaw_deg", data.right_yaw_deg);
	WriteParams(out, "right_pitch_deg", data.right_pitch_deg);
	WriteParams(out, "right_roll_deg", data.right_roll_deg);
	WriteParams(out, "a", data.gt.a);
	WriteParams(out, "b", data.gt.b);
	WriteParams(out, "c", data.gt.c);
	WriteParams(out, "d", data.gt.d);
	WriteParams(out, "cx", data.gt.cx);
	WriteParams(out, "cy", data.gt.cy);
	WriteParams(out, "yaw_rad", data.gt.yaw);
	WriteParams(out, "pitch_rad", data.gt.pitch);
	WriteParams(out, "roll_rad", data.gt.roll);
	out << "# derived angles in degrees\n";
	WriteParams(out, "yaw_deg", RadToDeg(data.gt.yaw));
	WriteParams(out, "pitch_deg", RadToDeg(data.gt.pitch));
	WriteParams(out, "roll_deg", RadToDeg(data.gt.roll));
	out << "matches=" << data.matches.GetCount() << "\n";
	out << "# uL vL uR vR rL(m) rR(m) X(m) Y(m) Z(m)\n";
	for (const auto& m : data.matches) {
		out << Format("%.5f %.5f %.5f %.5f %.5f %.5f %.5f %.5f %.5f\n",
			(double)m.left_px[0], (double)m.left_px[1],
			(double)m.right_px[0], (double)m.right_px[1],
			m.dist_l, m.dist_r,
			(double)m.point[0], (double)m.point[1], (double)m.point[2]);
	}
	return SaveFile(path, out);
}

bool LoadWmrCase(WmrCaseData& out, const String& path) {
	String text = LoadFile(path);
	if (text.IsEmpty())
		return false;
	out = WmrCaseData();
	Vector<String> lines = Split(text, '\n');
	int expected = 0;
	for (String line : lines) {
		line = TrimBoth(line);
		if (line.IsEmpty() || line[0] == '#')
			continue;
		if (line.Find('=') >= 0) {
			int eq = line.Find('=');
			String key = TrimBoth(line.Left(eq));
			String val = TrimBoth(line.Mid(eq + 1));
			if (key == "image_size") {
				Vector<String> parts = Split(val, 'x');
				if (parts.GetCount() >= 2) {
					out.image_size.cx = atoi(parts[0]);
					out.image_size.cy = atoi(parts[1]);
				}
			}
			else if (key == "eye_dist_m") out.eye_dist = atof(val);
			else if (key == "fov_x_deg") out.fov_x_deg = atof(val);
			else if (key == "fov_y_deg") out.fov_y_deg = atof(val);
			else if (key == "left_yaw_deg") out.left_yaw_deg = atof(val);
			else if (key == "left_pitch_deg") out.left_pitch_deg = atof(val);
			else if (key == "left_roll_deg") out.left_roll_deg = atof(val);
			else if (key == "right_yaw_deg") out.right_yaw_deg = atof(val);
			else if (key == "right_pitch_deg") out.right_pitch_deg = atof(val);
			else if (key == "right_roll_deg") out.right_roll_deg = atof(val);
			else if (key == "a") out.gt.a = atof(val);
			else if (key == "b") out.gt.b = atof(val);
			else if (key == "c") out.gt.c = atof(val);
			else if (key == "d") out.gt.d = atof(val);
			else if (key == "cx") out.gt.cx = atof(val);
			else if (key == "cy") out.gt.cy = atof(val);
			else if (key == "yaw_rad") out.gt.yaw = atof(val);
			else if (key == "pitch_rad") out.gt.pitch = atof(val);
			else if (key == "roll_rad") out.gt.roll = atof(val);
			else if (key == "matches") expected = atoi(val);
			continue;
		}
		Vector<String> parts = Split(line, ' ');
		Vector<String> filtered;
		filtered.Reserve(parts.GetCount());
		for (const String& s : parts)
			if (!s.IsEmpty())
				filtered.Add(s);
		parts = pick(filtered);
		if (parts.GetCount() >= 9) {
			WmrCaseMatch m;
			m.left_px[0] = (float)atof(parts[0]);
			m.left_px[1] = (float)atof(parts[1]);
			m.right_px[0] = (float)atof(parts[2]);
			m.right_px[1] = (float)atof(parts[3]);
			m.dist_l = atof(parts[4]);
			m.dist_r = atof(parts[5]);
			m.point[0] = (float)atof(parts[6]);
			m.point[1] = (float)atof(parts[7]);
			m.point[2] = (float)atof(parts[8]);
			out.matches.Add(m);
		}
	}
	return expected == 0 ? out.matches.GetCount() > 0 : out.matches.GetCount() == expected;
}

END_UPP_NAMESPACE
