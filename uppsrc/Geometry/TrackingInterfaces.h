#ifndef _Geometry_TrackingInterfaces_h_
#define _Geometry_TrackingInterfaces_h_


struct VisualFrame {
	int64 timestamp_us = 0;
	int format = 0;
	int width = 0;
	int height = 0;
	int stride = 0;
	int eye = -1; // 0=left, 1=right, -1=mono
	const byte* data = 0;
	int data_bytes = 0;
};

struct ImuSample {
	int64 timestamp_us = 0;
	vec3 accel = vec3(0,0,0);
	vec3 gyro = vec3(0,0,0);
	vec3 mag = vec3(0,0,0);
	vec3 gravity = vec3(0,0,0);
	float temperature = 0;
};

struct FusionState {
	int64 timestamp_us = 0;
	vec3 position = vec3(0,0,0);
	quat orientation;
	vec3 velocity = vec3(0,0,0);
	vec3 angular_velocity = vec3(0,0,0);
	float quality = 0;
};

struct StereoCalibrationResult {
	int width = 0;
	int height = 0;
	float fx[2] = {0,0};
	float fy[2] = {0,0};
	float cx[2] = {0,0};
	float cy[2] = {0,0};
	float k1[2] = {0,0};
	float k2[2] = {0,0};
	float k3[2] = {0,0};
	float p1[2] = {0,0};
	float p2[2] = {0,0};
	float fov_x[2] = {0,0};
	float fov_y[2] = {0,0};
	float baseline = 0;
	mat4 left_to_right = Identity<mat4>();
};


class VisualTracker {
public:
	virtual ~VisualTracker() {}
	virtual void Reset() = 0;
	virtual void PutFrame(const VisualFrame& frame) = 0;
	virtual bool GetState(FusionState& out) const = 0;
};

class ImuTracker {
public:
	virtual ~ImuTracker() {}
	virtual void Reset() = 0;
	virtual void PutSample(const ImuSample& sample) = 0;
};

class Fusion {
public:
	virtual ~Fusion() {}
	virtual void Reset() = 0;
	virtual void PutVisual(const VisualFrame& frame) = 0;
	virtual void PutImu(const ImuSample& sample) = 0;
	virtual bool GetState(FusionState& out) const = 0;
};

class Relocalizer {
public:
	virtual ~Relocalizer() {}
	virtual void Reset() = 0;
	virtual bool Relocalize(const VisualFrame& frame, FusionState& out) = 0;
};

class StereoCalibration {
public:
	virtual ~StereoCalibration() {}
	virtual void Reset() = 0;
	virtual bool Solve(StereoCalibrationResult& out) = 0;
};


#endif
