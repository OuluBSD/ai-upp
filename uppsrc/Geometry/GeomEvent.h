#ifndef _Geometry_GeomEvent_h_
#define _Geometry_GeomEvent_h_


enum {
	HOLO_CALIB_FOV,
	HOLO_CALIB_SCALE,
	HOLO_CALIB_EYE_DIST,
	HOLO_CALIB_X,
	HOLO_CALIB_Y,
	HOLO_CALIB_Z,
	HOLO_CALIB_YAW,
	HOLO_CALIB_PITCH,
	HOLO_CALIB_ROLL,
};

typedef enum {
	EVENT_INVALID,
	EVENT_WINDOW_RESIZE,
	EVENT_SHUTDOWN,
	EVENT_KEYDOWN,
	EVENT_KEYUP,
	EVENT_MOUSEMOVE,
	//EVENT_MOUSEDRAG,
	EVENT_MOUSEWHEEL,
	EVENT_MOUSE_EVENT,
	
	
	EVENT_HOLO_STATE,
	EVENT_HMD_POSE,
	EVENT_HMD_CONTROLLER,
	EVENT_HMD_IMU,
	EVENT_HMD_CAMERA,
	EVENT_HMD_FUSION,
	EVENT_HOLO_LOOK,
	EVENT_HOLO_CALIB,
	
	EVENT_HOLO_CONTROLLER_DETECTED,
	EVENT_HOLO_CONTROLLER_LOST,
	EVENT_HOLO_MOVE_FAR_RELATIVE,
	EVENT_HOLO_MOVE_NEAR,
	EVENT_HOLO_MOVE_CONTROLLER,
	EVENT_HOLO_PRESSED,
	EVENT_HOLO_RELEASED,
	EVENT_HOLO_UPDATED,
	
	EVENT_TYPE_COUNT
} GeomEventType;


inline String GetEventTypeString(int event) {
	switch (event) {
		case EVENT_WINDOW_RESIZE:	return "Window Resize";
		case EVENT_SHUTDOWN:		return "Shutdown";
		case EVENT_KEYDOWN:			return "Key Down";
		case EVENT_KEYUP:			return "Key Up";
		case EVENT_MOUSEMOVE:		return "Mouse Move";
		//case EVENT_MOUSEDRAG:		return "Mouse Drag";
		case EVENT_MOUSEWHEEL:		return "Mouse Wheel";
		case EVENT_MOUSE_EVENT:		return "Mouse Event";
		
		case EVENT_HOLO_STATE:					return "Holographic full state";
		case EVENT_HMD_POSE:					return "HMD pose";
		case EVENT_HMD_CONTROLLER:				return "HMD controller state";
		case EVENT_HMD_IMU:						return "HMD IMU sample";
		case EVENT_HMD_CAMERA:					return "HMD camera frame";
		case EVENT_HMD_FUSION:					return "HMD fusion state";
		case EVENT_HOLO_LOOK:					return "Holographic Look";
		case EVENT_HOLO_CALIB:					return "Holographic Calibration";
		
		case EVENT_HOLO_CONTROLLER_DETECTED:	return "Holographic controller detected";
		case EVENT_HOLO_CONTROLLER_LOST:		return "Holographic controller lost";
		case EVENT_HOLO_MOVE_FAR_RELATIVE:		return "Holographic Move Far (relative)";
		case EVENT_HOLO_MOVE_NEAR:				return "Holographic Move Near";
		case EVENT_HOLO_PRESSED:				return "Holographic controller button pressed";
		case EVENT_HOLO_RELEASED:				return "Holographic controller button released";
		
		case EVENT_INVALID:
		case EVENT_TYPE_COUNT:
		default:					return "<invalid>";
	}
}

enum {
	GEOM_EVENT_PAYLOAD_BYTES = 128,
	GEOM_EVENT_MAX_CONTROLLERS = 2,
	GEOM_EVENT_MAX_ANALOG = 2,
};

enum GeomEventFlags {
	GEOM_EVENT_FLAG_HAS_HMD_POSE   = 1 << 0,
	GEOM_EVENT_FLAG_HAS_CONTROLLER = 1 << 1,
	GEOM_EVENT_FLAG_HAS_IMU        = 1 << 2,
	GEOM_EVENT_FLAG_HAS_CAMERA     = 1 << 3,
	GEOM_EVENT_FLAG_HAS_FUSION     = 1 << 4,
	GEOM_EVENT_FLAG_LEFT           = 1 << 5,
	GEOM_EVENT_FLAG_RIGHT          = 1 << 6,
	GEOM_EVENT_FLAG_STEREO         = 1 << 7,
};

enum GeomEventCameraFormat {
	GEOM_EVENT_CAM_UNKNOWN = 0,
	GEOM_EVENT_CAM_LUMA8,
	GEOM_EVENT_CAM_RGB8,
	GEOM_EVENT_CAM_BGR8,
	GEOM_EVENT_CAM_RGBA8,
	GEOM_EVENT_CAM_BGRA8,
};

struct HoloStatePayload {
	int64 timestamp_ms;
	int flags;
	int controller_count;
	float hmd_position[3];
	float hmd_orientation[4];
	float ctrl_position[GEOM_EVENT_MAX_CONTROLLERS][3];
	float ctrl_orientation[GEOM_EVENT_MAX_CONTROLLERS][4];
	uint32 button_mask[GEOM_EVENT_MAX_CONTROLLERS];
	float analog[GEOM_EVENT_MAX_CONTROLLERS][GEOM_EVENT_MAX_ANALOG];
	byte _pad[GEOM_EVENT_PAYLOAD_BYTES - 124];
};

struct HoloStatePtrPayload {
	TransformMatrix* trans;
	ControllerMatrix* ctrl;
	ControllerState* state;
	byte _pad[GEOM_EVENT_PAYLOAD_BYTES - 24];
};

struct HmdPosePayload {
	int64 timestamp_ms;
	int flags;
	int reserved0;
	float position[3];
	float orientation[4];
	float axes[3];
	float fov[2];
	float eye_dist;
	float quality;
	byte _pad[GEOM_EVENT_PAYLOAD_BYTES - 72];
};

struct ControllerPayload {
	int64 timestamp_ms;
	int flags;
	int controller_count;
	float position[GEOM_EVENT_MAX_CONTROLLERS][3];
	float orientation[GEOM_EVENT_MAX_CONTROLLERS][4];
	uint32 button_mask[GEOM_EVENT_MAX_CONTROLLERS];
	float analog[GEOM_EVENT_MAX_CONTROLLERS][GEOM_EVENT_MAX_ANALOG];
	byte _pad[GEOM_EVENT_PAYLOAD_BYTES - 96];
};

struct ImuSamplePayload {
	int64 timestamp_ms;
	int flags;
	int reserved0;
	float accel[3];
	float gyro[3];
	float mag[3];
	float gravity[3];
	float temperature;
	float sample_rate;
	byte _pad[GEOM_EVENT_PAYLOAD_BYTES - 72];
};

struct CameraFramePayload {
	int64 timestamp_ms;
	int flags;
	int format;
	int width;
	int height;
	int stride;
	int eye;
	int reserved0;
	uint64 data_ptr;
	int data_bytes;
	int frame_id;
	float exposure;
	float gain;
	float gamma;
	byte _pad[GEOM_EVENT_PAYLOAD_BYTES - 68];
};

struct FusionPayload {
	int64 timestamp_ms;
	int flags;
	int reserved0;
	float position[3];
	float orientation[4];
	float velocity[3];
	float angular_velocity[3];
	float covariance[6];
	float quality;
	byte _pad[GEOM_EVENT_PAYLOAD_BYTES - 96];
};

union GeomEventPayload {
	byte raw[GEOM_EVENT_PAYLOAD_BYTES];
	HoloStatePayload holo_state;
	HoloStatePtrPayload holo_state_ptr;
	HmdPosePayload hmd_pose;
	ControllerPayload controller;
	ImuSamplePayload imu;
	CameraFramePayload camera;
	FusionPayload fusion;
};

static_assert(sizeof(HoloStatePayload) == GEOM_EVENT_PAYLOAD_BYTES, "HoloStatePayload size mismatch");
static_assert(sizeof(HoloStatePtrPayload) == GEOM_EVENT_PAYLOAD_BYTES, "HoloStatePtrPayload size mismatch");
static_assert(sizeof(HmdPosePayload) == GEOM_EVENT_PAYLOAD_BYTES, "HmdPosePayload size mismatch");
static_assert(sizeof(ControllerPayload) == GEOM_EVENT_PAYLOAD_BYTES, "ControllerPayload size mismatch");
static_assert(sizeof(ImuSamplePayload) == GEOM_EVENT_PAYLOAD_BYTES, "ImuSamplePayload size mismatch");
static_assert(sizeof(CameraFramePayload) == GEOM_EVENT_PAYLOAD_BYTES, "CameraFramePayload size mismatch");
static_assert(sizeof(FusionPayload) == GEOM_EVENT_PAYLOAD_BYTES, "FusionPayload size mismatch");

struct ControllerProperties {
	typedef enum {
		LEFT,
		UP,
		RIGHT,
		DOWN,
		
		A0, // cross
		A1, // rectangle
		A2, // circle
		A3, // triangle
		
		L1, // lower shoulder
		L2, // higher shoulder
		R1, // lower shoulder
		R2, // higher shoulder
		
		SELECT,
		START,
		HOME,
		
		BUTTON_COUNT,
		
		CROSS		= A0,
		RECTANGLE	= A1,
		CIRCLE		= A2,
		TRIANGLE	= A3,
	} Button;
	
	bool pressed[BUTTON_COUNT] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0};
	float touchpad[2][2] = {{0,0},{0,0}};
	float thumbstick[2][2] = {{0,0},{0,0}};
	float touchpad_travel[2] = {0,0};
	float touchpad_pressure[2] = {0,0};
	float shoulder_high[2] = {0,0};
	float shoulder_low[2] = {0,0};
	
	float GetTouchpadX(int i=0) const {return touchpad[i][0];}
	float GetTouchpadY(int i=0) const {return touchpad[i][1];}
	float GetThumbstickX(int i=0) const {return thumbstick[i][0];}
	float GetThumbstickY(int i=0) const {return thumbstick[i][1];}
	bool IsGrasped(int i=0) const {return shoulder_low[i] != 0.0f;}
	bool IsTouchpadTouched(int i=0) const {return touchpad_pressure[i] != 0.0f;}
	bool IsTouchpadPressed(int i=0) const {return touchpad_travel[i] != 0.0f;}
	bool IsSelectPressed() const {return pressed[SELECT];}
	
};

class TransformMatrix;
struct ControllerState;
struct ControllerMatrix;

struct GeomEvent : Moveable<GeomEvent> {
	GeomEventType type = EVENT_INVALID;
	union {
		dword value = 0;
		float fvalue;
	};
	int n = 0;
	Point pt;
	float pt3[3];
	Size sz;
	GeomEventPayload payload;
	
	// Device extension
	TransformMatrix* trans = 0;
	ControllerMatrix* ctrl = 0;
	ControllerState* state = 0;
	
	
	const ControllerState& GetState() const {ASSERT(state); return *state;}
	const ControllerMatrix& GetControllerMatrix() const {ASSERT(ctrl); return *ctrl;}
	
	void operator=(const GeomEvent& e) {
		type = e.type;
		value = e.value;
		n = e.n;
		pt.x = e.pt.x;
		pt.y = e.pt.y;
		sz.cx = e.sz.cx;
		sz.cy = e.sz.cy;
		trans = e.trans;
		ctrl = e.ctrl;
		state = e.state;
		for(int i = 0; i < 3; i++) pt3[i] = e.pt3[i];
		memcpy(payload.raw, e.payload.raw, GEOM_EVENT_PAYLOAD_BYTES);
	}
	
	void Clear() {
		type = EVENT_INVALID;
		value = 0;
		n = 0;
		pt = Point(0,0);
		sz = Size(0,0);
		trans = 0;
		ctrl = 0;
		state = 0;
		for(int i = 0; i < 3; i++) pt3[i] = 0;
		memset(payload.raw, 0, GEOM_EVENT_PAYLOAD_BYTES);
	}
	
	String ToString() const {
		String s;
		s << GetEventTypeString(type) << ", " << (int)value << ", " << n << ", (" << pt.x << "," << pt.y << "), [" << sz.cx << "," << sz.cy << "]";
		return s;
	}
};

struct GeomEventCollection : Vector<GeomEvent> {
	
};

void RandomizeEvent(GeomEvent& ev);

class EventFrame {
	
public:
	GeomEventCollection ctrl;
	
	void Reset() {ctrl.SetCount(0);}
};




inline double ResetSeconds(TimeStop& ts) {double s = ts.Seconds(); ts.Reset(); return s;}


struct MidiEvent {
	int pad;
	
};


//#include "Keycodes.inl"



#endif
