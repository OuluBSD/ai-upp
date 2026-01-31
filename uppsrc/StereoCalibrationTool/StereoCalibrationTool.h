#ifndef _StereoCalibrationTool_StereoCalibrationTool_h_
#define _StereoCalibrationTool_StereoCalibrationTool_h_

#include <CtrlLib/CtrlLib.h>
#include <Geometry/Geometry.h>
#include <ComputerVision/ComputerVision.h>
#include <SoftHMD/SoftHMD.h>
#ifdef flagLINUX
#include <plugin/libv4l2/libv4l2.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

NAMESPACE_UPP

/*
StereoCalibrationTool.h
=======================
Purpose:
- Master header for the StereoCalibrationTool package.
- Owns shared data types, AppModel, and shared UI helpers used by all
  TopWindow modules (Menu/Camera/StageA/StageB/StageC/LiveResult).

How to run (smoke test):
- build: python3 script/build.py -j 12 -mc 1 StereoCalibrationTool
- run:   ./bin/StereoCalibrationTool
*/

// ------------------------------------------------------------
// Shared data model
// ------------------------------------------------------------

// Match pair between left/right views (normalized 0..1 coordinates).
struct MatchPair : Moveable<MatchPair> {
	Pointf left = Null;   // Left eye normalized coord (x,y in [0,1])
	Pointf right = Null;  // Right eye normalized coord (x,y in [0,1])
	String left_text;     // Display text for the left point (UI label)
	String right_text;    // Display text for the right point (UI label)
	double dist_l = 0;    // Optional distance hint for left ray (mm)
	double dist_r = 0;    // Optional distance hint for right ray (mm)

	void Jsonize(JsonIO& jio) {
		jio("left", left)("right", right)
		   ("left_text", left_text)("right_text", right_text)
		   ("dist_l", dist_l)("dist_r", dist_r);
	}
};

// Shared view modes for preview/undistort handling.
enum ViewMode {
	VIEW_RAW = 0,
	VIEW_BASIC = 1,
	VIEW_SOLVED = 2
};

// One captured stereo frame and its stored match points.
struct CapturedFrame : Moveable<CapturedFrame> {
	Time time;                 // Capture timestamp
	String source;             // Human-readable source label
	int samples = 0;           // Total matches at capture time
	Image left_img;            // Captured left image
	Image right_img;           // Captured right image
	Image undist_left;         // Cached undistorted left image (view mode)
	Image undist_right;        // Cached undistorted right image (view mode)
	vec4 undist_poly = vec4(0,0,0,0);
	Size undist_size = Size(0,0);
	bool undist_valid = false; // Cache validity flag
	Vector<MatchPair> matches; // Stored match pairs

	void Jsonize(JsonIO& jio) {
		jio("time", time)("source", source)("samples", samples)("matches", matches);
		// Images are intentionally not jsonized (stored as PNGs on disk).
	}
};

// User-editable state that persists to project.json.
struct ProjectState {
	int schema_version = 1;

	// Stage A (basic alignment)
	double eye_dist = 64.0;          // mm
	double yaw_l = 0, pitch_l = 0, roll_l = 0; // deg
	double yaw_r = 0, pitch_r = 0, roll_r = 0; // deg
	double fov_deg = 90.0;           // deg
	double barrel_strength = 0;      // percent-ish scaler
	bool preview_extrinsics = true;  // preview uses extrinsics if true
	bool preview_intrinsics = false; // preview uses intrinsics (lens distortion/FOV) if true

	// Stage B (solve)
	double distance_weight = 0.1;
	double huber_px = 2.0;
	double huber_m = 0.030;
	bool lock_distortion = false;
	bool verbose_math_log = false;
	bool compare_basic_params = false;

	// Stage C (micro-refine)
	bool stage_c_enabled = false;
	int stage_c_mode = 0; // 0=Relative, 1=Per-eye
	double max_dyaw = 3.0;
	double max_dpitch = 2.0;
	double max_droll = 3.0;
	double lambda = 0.1;

	// Viewer
	int view_mode = 0;   // 0=Raw, 1=Basic, 2=Solved
	bool overlay_eyes = false;
	int alpha = 50;      // 0..100
	bool overlay_swap = false;
	bool show_difference = false;
	bool show_epipolar = false;
	bool tint_overlay = false;     // Tint left=blue, right=red in overlay mode
	bool show_crosshair = false;   // Show red center crosshair lines
	int tool_mode = 0;             // 0=None, 1=Center yaw, 2=Center pitch, 3=Center both

	void Jsonize(JsonIO& jio) {
		jio("schema_version", schema_version);
		jio("eye_dist", eye_dist)("yaw_l", yaw_l)("pitch_l", pitch_l)("roll_l", roll_l);
		jio("yaw_r", yaw_r)("pitch_r", pitch_r)("roll_r", roll_r);
		jio("fov_deg", fov_deg)("barrel_strength", barrel_strength)
		   ("preview_extrinsics", preview_extrinsics)("preview_intrinsics", preview_intrinsics);

		jio("distance_weight", distance_weight)("huber_px", huber_px)("huber_m", huber_m);
		jio("lock_distortion", lock_distortion)("verbose_math_log", verbose_math_log)
		   ("compare_basic_params", compare_basic_params);

		jio("stage_c_enabled", stage_c_enabled)("stage_c_mode", stage_c_mode);
		jio("max_dyaw", max_dyaw)("max_dpitch", max_dpitch)("max_droll", max_droll)
		   ("lambda", lambda);

		jio("view_mode", view_mode)("overlay_eyes", overlay_eyes)("alpha", alpha);
		jio("overlay_swap", overlay_swap)("show_difference", show_difference)
		   ("show_epipolar", show_epipolar);
		jio("tint_overlay", tint_overlay)("show_crosshair", show_crosshair)("tool_mode", tool_mode);
	}
};

// Shared application state used by all windows.
struct AppModel {
	// Core project path and persistence data.
	String project_dir;               // Root directory for the project
	ProjectState project_state;       // User-editable controls that persist

	// Captured data set (image files are stored by index in project_dir/captures).
	Vector<CapturedFrame> captured_frames; // Persistent capture list
	int selected_capture = -1;            // Selected capture index
	int pending_capture_row = -1;         // UI convenience for list selection

	// Calibration data.
	StereoCalibrationData last_calibration; // Latest solved calibration

	// Viewer caches (per capture and live).
	int64 last_serial = -1;
	int64 live_undist_serial = -1;
	Size live_undist_size = Size(0, 0);
	vec4 live_undist_poly = vec4(0,0,0,0);
	bool live_undist_valid = false;
	Image live_undist_left;
	Image live_undist_right;

	// Stage C deltas (diagnostic only).
	float dyaw_c = 0;
	float dpitch_c = 0;
	float droll_c = 0;

	// Output buffers (used by Stage B/Stage C UI).
	String report_text;
	String math_text;

	// Global flags
	bool verbose = false;

	// GA bootstrap settings (headless or advanced use).
	bool use_ga_bootstrap = false;
	int ga_population = 30;
	int ga_generations = 20;
};

// ------------------------------------------------------------
// Shared UI helpers
// ------------------------------------------------------------

// Preview widget used by Camera, Stage A, and LiveResult.
// Disabled per request (see module windows for simplified views).
#if 0
struct PreviewCtrl : public Ctrl {
	struct ResidualSample : Moveable<ResidualSample> {
		Pointf measured = Null;
		Pointf reproj = Null;
		int eye = 0;
		double err_px = 0;
	};

	bool live = true;
	bool has_images = false;
	bool show_epipolar = false;
	bool show_residuals = false;
	bool overlay_mode = false;
	bool show_difference = false;
	float overlay_alpha = 0.5f;
	int overlay_base_eye = 0; // 0 = Left is base, 1 = Right is base
	Image left_img;
	Image right_img;
	String overlay;
	Pointf pending_left = Null;
	Vector<MatchPair> matches;
	Vector<ResidualSample> residuals;
	double residual_rms = 0;

	Event<Pointf, int> WhenClick;

	void SetLive(bool b) { live = b; Refresh(); }
	void SetImages(const Image& l, const Image& r) { left_img = l; right_img = r; has_images = !IsNull(l) || !IsNull(r); Refresh(); }
	void SetOverlay(const String& s) { overlay = s; Refresh(); }
	void SetPendingLeft(Pointf p) { pending_left = p; Refresh(); }
	void SetMatches(const Vector<MatchPair>& m) { matches <<= m; Refresh(); }
	void SetEpipolar(bool b) { show_epipolar = b; Refresh(); }
	void SetResiduals(const Vector<ResidualSample>& r, double rms, bool show) { residuals <<= r; residual_rms = rms; show_residuals = show; Refresh(); }
	void SetOverlayMode(bool m, float alpha, int base_eye) { overlay_mode = m; overlay_alpha = alpha; overlay_base_eye = base_eye; Refresh(); }
	void SetDifference(bool b) { show_difference = b; Refresh(); }

	virtual void Paint(Draw& w) override;
	virtual void LeftDown(Point p, dword flags) override;
};
#endif

// ------------------------------------------------------------
// Shared stereo camera sources
// ------------------------------------------------------------

struct StereoSource {
	virtual ~StereoSource() {}
	virtual String GetName() const = 0;
	virtual bool Start() = 0;
	virtual void Stop() = 0;
	virtual bool IsRunning() const = 0;
	virtual bool ReadFrame(VisualFrame& left, VisualFrame& right, bool prefer_bright = false) = 0;
	virtual bool PeakFrame(VisualFrame& left, VisualFrame& right, bool prefer_bright = false) = 0;
	virtual void SetVerbose(bool v) {}
};

struct HmdStereoSource : StereoSource {
	bool running = false;
	bool verbose = false;
	HMD::System sys;
	One<HMD::Camera> cam;
	Image last_left;
	Image last_right;
	bool last_is_bright = false;

	String GetName() const override { return "HMD Stereo Camera"; }
	bool Start() override;
	void Stop() override;
	bool IsRunning() const override { return running; }
	bool ReadFrame(VisualFrame& left, VisualFrame& right, bool prefer_bright = false) override;
	bool PeakFrame(VisualFrame& left, VisualFrame& right, bool prefer_bright = false) override;
	void SetVerbose(bool v) override { verbose = v; }
};

struct UsbStereoSource : StereoSource {
	bool running = false;
	String device_path;
	Image last_left;
	Image last_right;
	bool last_is_bright = false;
#ifdef flagLINUX
	One<V4l2Capture> capture;
	Vector<byte> raw;
	int width = 0;
	int height = 0;
	int pixfmt = 0;
#endif
	String GetName() const override { return "USB Stereo (Side-by-side)"; }
	bool Start() override;
	void Stop() override;
	bool IsRunning() const override { return running; }
	bool ReadFrame(VisualFrame&, VisualFrame&, bool prefer_bright = false) override;
	bool PeakFrame(VisualFrame&, VisualFrame&, bool prefer_bright = false) override;
};

struct VideoStereoSource : StereoSource {
	bool running = false;
	String path;
	String GetName() const override { return "Stereo Video File"; }
	bool Start() override { running = true; return true; }
	void Stop() override { running = false; }
	bool IsRunning() const override { return running; }
	bool ReadFrame(VisualFrame&, VisualFrame&, bool prefer_bright = false) override { return false; }
	bool PeakFrame(VisualFrame&, VisualFrame&, bool prefer_bright = false) override { return false; }
};

// ------------------------------------------------------------
// Shared helper functions (implemented in StereoCalibrationTool.cpp)
// ------------------------------------------------------------

namespace StereoCalibrationHelpers {
// Image/conversion helpers used by Camera and LiveResult.
bool SplitStereoImage(const Image& src, Image& left, Image& right);
Image CopyFrameImage(const VisualFrame& frame);
bool IsFrameNonBlack(const Image& img);
Image ConvertRgb24ToImage(const byte* data, int width, int height);
Image ConvertYuyvToImage(const byte* data, int width, int height);
Image ConvertMjpegToImage(const byte* data, int bytes);

// Preview/undistort helpers used by StageA and LiveResult.
RGBA SampleBilinear(const Image& img, float x, float y);
Image UndistortImage(const Image& src, const LensPoly& lens, float linear_scale);
Image DistortImage(const Image& src, const LensPoly& lens, float linear_scale);
Image ApplyExtrinsicsOnly(const Image& src, float yaw, float pitch, float roll, const vec2& pp);
Image ApplyIntrinsicsOnly(const Image& src, const LensPoly& lens, float linear_scale, bool undistort);

// Persistence helpers (project.json + calibration file).
String GetPersistPath(const AppModel& model);
String GetStatePath(const AppModel& model);
String GetReportPath(const AppModel& model);
void LoadLastCalibration(AppModel& model);
void SaveLastCalibration(AppModel& model);
void LoadState(AppModel& model);
void SaveState(const AppModel& model);
}

// ------------------------------------------------------------
// Forward declarations for window modules
// ------------------------------------------------------------

class MenuWindow;
class CameraWindow;
class StageAWindow;
class StageBWindow;
class StageCWindow;
class LiveResultWindow;

// Identity test helper (headless diagnostic).
int TestStageAIdentity(AppModel& model, const String& project_dir, const String& image_path = String());

// ------------------------------------------------------------
// Legacy controller (kept for reference, currently disabled).
// ------------------------------------------------------------

#if 0
class StereoCalibrationTool {
public:
	StereoCalibrationTool();
	~StereoCalibrationTool();

	void Run();
	void SetVerbose(bool v);
	void SetProjectDir(const String& dir);
	void EnableGABootstrap(bool enable, int population = 30, int generations = 20);

	// Headless tools
	int SolveHeadless(const String& project_dir);
	int TestStageAIdentity(const String& project_dir, const String& image_path = String());

	// Capture test hooks (used by CLI switches)
	void EnableUsbTest(const String& dev, int timeout_ms);
	void EnableHmdTest(int timeout_ms);
	void EnableLiveTest(int timeout_ms);

	AppModel& Model() { return model; }

private:
	AppModel model;

	// Windows live for the lifetime of the app (Menu owns no logic).
	MenuWindow* menu = nullptr;
	CameraWindow* camera = nullptr;
	StageAWindow* stage_a = nullptr;
	StageBWindow* stage_b = nullptr;
	StageCWindow* stage_c = nullptr;
	LiveResultWindow* live = nullptr;

	// Test / timer infrastructure (headless capture checks).
	TimeCallback usb_test_cb;
	TimeCallback hmd_test_cb;
	TimeCallback live_test_cb;
	bool usb_test_enabled = false;
	bool usb_test_active = false;
	int usb_test_timeout_ms = 4000;
	String usb_test_device;
	int64 usb_test_start_us = 0;
	int64 usb_test_last_start_us = 0;
	int usb_test_attempts = 0;
	bool hmd_test_enabled = false;
	bool hmd_test_active = false;
	int hmd_test_timeout_ms = 4000;
	int64 hmd_test_start_us = 0;
	int64 hmd_test_last_start_us = 0;
	int hmd_test_attempts = 0;
	bool live_test_active = false;
	int live_test_timeout_ms = 5000;
	int64 live_test_start_us = 0;

	void StartUsbTest();
	void RunUsbTest();
	void StartHmdTest();
	void RunHmdTest();
	void StartLiveTest();
	void RunLiveTest();
};
#endif

// ------------------------------------------------------------
// Sub-headers (TopWindow modules)
// ------------------------------------------------------------

#include "Menu.h"
#include "Camera.h"
#include "StageA.h"
#include "StageB.h"
#include "StageC.h"
#include "LiveResult.h"

END_UPP_NAMESPACE

#endif
