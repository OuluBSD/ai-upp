#ifndef _StereoCalibrationTool_StereoCalibrationTool_h_
#define _StereoCalibrationTool_StereoCalibrationTool_h_

#include <CtrlLib/CtrlLib.h>
#include <Geometry/Geometry.h>
#include <ComputerVision/ComputerVision.h>
#include <SoftHMD/SoftHMD.h>
#ifdef flagLINUX
#include <plugin/libv4l2/libv4l2.h>
#endif

// Include OpenCV headers for StereoRectificationCache
#undef CPU_SSE2
#include <opencv2/core.hpp>

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
  TopWindow modules (Menu/Camera/Calibration/StageB/StageC/LiveResult).

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
	Vector<Vector<Pointf>> annotation_lines_left; // Line chains for left eye (rectified pixel coords)
	Vector<Vector<Pointf>> annotation_lines_right; // Line chains for right eye
	
	// Board Detection (Pixel coords)
	Vector<Pointf> corners_l;
	Vector<Pointf> corners_r;
	bool detected_l = false;
	bool detected_r = false;
	bool used = true; // Include in solve
	
	// Diagnostics
	String reject_reason;
	double reproj_rms_l = 0;
	double reproj_rms_r = 0;

	void Jsonize(JsonIO& jio) {
		jio("time", time)("source", source)("samples", samples)("matches", matches);
		jio("lines_l", annotation_lines_left)("lines_r", annotation_lines_right);
		jio("corners_l", corners_l)("corners_r", corners_r)
		   ("detected_l", detected_l)("detected_r", detected_r)("used", used)
		   ("reject_reason", reject_reason)
		   ("rms_l", reproj_rms_l)("rms_r", reproj_rms_r);
		// Images are intentionally not jsonized (stored as PNGs on disk).
	}
};

enum CalibrationState {
	CALIB_RAW = 0,
	CALIB_MANUAL = 1,
	CALIB_SOLVED = 4
};

// User-editable state that persists to project.json.
struct ProjectState {
	int schema_version = 1;

	// Calibration (basic alignment)
	double eye_dist = 64.0;          // mm
	double yaw_l = 0, pitch_l = 0, roll_l = 0; // deg
	double yaw_r = 0, pitch_r = 0, roll_r = 0; // deg
	double fov_deg = 90.0;           // deg
	double barrel_strength = 0;      // percent-ish scaler
	double lens_f = 0;               // focal length (derived if 0)
	double lens_cx = 0, lens_cy = 0; // principal point (derived if 0)
	double lens_k1 = 0, lens_k2 = 0; // distortion coefficients
	bool preview_extrinsics = true;  // preview uses extrinsics if true
	bool preview_intrinsics = false; // preview uses intrinsics (lens distortion/FOV) if true

	// Board Settings
	int board_x = 9;
	int board_y = 6;
	double square_size_mm = 20.0;
	bool use_charuco = false;
	bool lock_intrinsics = false;
	bool lock_baseline = false;
	bool lock_yaw_symmetry = false;

	// Viewer
	bool overlay_eyes = false;
	int alpha = 50;      // 0..100
	bool overlay_swap = false;
	bool show_difference = false;
	bool show_epipolar = false;
	bool tint_overlay = false;     // Tint left=green, right=violet in overlay mode
	bool show_crosshair = false;   // Show red center crosshair lines
	int tool_mode = 0;             // 0=None, 1=Center yaw, 2=Center pitch, 3=Center both
	bool show_corners = true;      // show detected board corners
	bool show_reprojection = true; // show re-projected points

	// Rectified Overlay (OpenCV stereoRectify)
	bool rectified_overlay = false;  // Show rectified overlay instead of raw overlay
	double rectify_alpha = 0.0;      // stereoRectify alpha parameter [0..1]
	                                 // 0=crop all invalid pixels, 1=retain all pixels

	// Stereo Calibration R matrix (rotation from stereoCalibrate)
	// Stored as 9 doubles to preserve exact matrix from OpenCV
	double stereo_R[9] = {1,0,0, 0,1,0, 0,0,1};  // Identity by default
	bool stereo_R_valid = false;                 // True if R was computed by stereoCalibrate

	// Separate K and D matrices for left and right eyes (from stereoCalibrate)
	// These are needed to reconstruct EXACT rectification (averaging loses precision)
	double K_L_f = 0, K_L_cx = 0, K_L_cy = 0;
	double K_R_f = 0, K_R_cx = 0, K_R_cy = 0;
	double D_L_k1 = 0, D_L_k2 = 0;
	double D_R_k1 = 0, D_R_k2 = 0;

	// Exact T vector from stereoCalibrate (translation between cameras)
	double stereo_T[3] = {0, 0, 0};

	bool stereo_KD_valid = false;  // True if per-eye K/D from stereoCalibrate are saved

	// Pipeline State
	int calibration_state = CALIB_RAW;

	void Jsonize(JsonIO& jio) {
		jio("schema_version", schema_version);
		jio("eye_dist", eye_dist)("yaw_l", yaw_l)("pitch_l", pitch_l)("roll_l", roll_l);
		jio("yaw_r", yaw_r)("pitch_r", pitch_r)("roll_r", roll_r);
		jio("fov_deg", fov_deg)("barrel_strength", barrel_strength)
		   ("lens_f", lens_f)("lens_cx", lens_cx)("lens_cy", lens_cy)
		              ("lens_k1", lens_k1)("lens_k2", lens_k2)
		              ("preview_extrinsics", preview_extrinsics)("preview_intrinsics", preview_intrinsics)
		              ("board_x", board_x)("board_y", board_y)
		              ("square_size_mm", square_size_mm)("use_charuco", use_charuco)
		              ("lock_intrinsics", lock_intrinsics)
		              ("lock_baseline", lock_baseline)
		              ("lock_yaw_symmetry", lock_yaw_symmetry);
		   
		jio("overlay_eyes", overlay_eyes)("alpha", alpha);
		jio("overlay_swap", overlay_swap)("show_difference", show_difference)
		   ("show_epipolar", show_epipolar);
		jio("tint_overlay", tint_overlay)("show_crosshair", show_crosshair)("tool_mode", tool_mode)
		   ("show_corners", show_corners)("show_reprojection", show_reprojection);
		jio("rectified_overlay", rectified_overlay)("rectify_alpha", rectify_alpha);

		// Stereo R matrix (9 elements)
		jio("stereo_R_valid", stereo_R_valid);
		if (stereo_R_valid || jio.IsLoading()) {
			jio("stereo_R_00", stereo_R[0])("stereo_R_01", stereo_R[1])("stereo_R_02", stereo_R[2])
			   ("stereo_R_10", stereo_R[3])("stereo_R_11", stereo_R[4])("stereo_R_12", stereo_R[5])
			   ("stereo_R_20", stereo_R[6])("stereo_R_21", stereo_R[7])("stereo_R_22", stereo_R[8]);
		}

		// Per-eye K and D matrices from stereoCalibrate (exact, not averaged)
		jio("stereo_KD_valid", stereo_KD_valid);
		if (stereo_KD_valid || jio.IsLoading()) {
			jio("K_L_f", K_L_f)("K_L_cx", K_L_cx)("K_L_cy", K_L_cy);
			jio("K_R_f", K_R_f)("K_R_cx", K_R_cx)("K_R_cy", K_R_cy);
			jio("D_L_k1", D_L_k1)("D_L_k2", D_L_k2);
			jio("D_R_k1", D_R_k1)("D_R_k2", D_R_k2);
			jio("stereo_T_x", stereo_T[0])("stereo_T_y", stereo_T[1])("stereo_T_z", stereo_T[2]);
		}

		jio("calibration_state", calibration_state);
	}
};

// Stereo rectification cache (computed from stereo calibration results).
// Stores outputs of cv::stereoRectify and cv::initUndistortRectifyMap.
struct StereoRectificationCache {
	// Input parameters (used to detect when cache is invalid)
	cv::Mat K1, D1, K2, D2;  // Intrinsics from calibration
	cv::Mat R, T;             // Extrinsics from stereo calibration
	cv::Size image_size;      // Image resolution
	double alpha = -1.0;      // stereoRectify alpha parameter

	// Rectification outputs (from cv::stereoRectify)
	cv::Mat R1, R2;           // Rectification rotations for each camera
	cv::Mat P1, P2;           // Projection matrices in rectified coords
	cv::Mat Q;                // Disparity-to-depth mapping matrix
	cv::Rect roi1, roi2;      // Regions of interest (valid pixels)

	// Remap maps (from cv::initUndistortRectifyMap)
	cv::Mat map1x, map1y;     // Left eye remap maps
	cv::Mat map2x, map2y;     // Right eye remap maps

	bool valid = false;       // True if cache contains valid data

	// Check if current inputs match cached parameters
	bool IsValid(const cv::Mat& k1, const cv::Mat& d1,
	             const cv::Mat& k2, const cv::Mat& d2,
	             const cv::Mat& r, const cv::Mat& t,
	             const cv::Size& sz, double a) const;

	// Invalidate cache
	void Invalidate() { valid = false; }
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

	// Stereo rectification cache (OpenCV stereoRectify results)
	StereoRectificationCache rectification_cache;

	// Epipolar alignment metrics (computed after stereo solve)
	double epipolar_median_dy = -1.0;  // Median |yL - yR| in rectified space (px)
	double epipolar_p95_dy = -1.0;     // 95th percentile |Δy| (px)
	int epipolar_num_points = 0;       // Number of points used for metric

	// Viewer caches (per capture and live).
	int64 last_serial = -1;
	int64 live_undist_serial = -1;
	Size live_undist_size = Size(0, 0);
	vec4 live_undist_poly = vec4(0,0,0,0);
	bool live_undist_valid = false;
	Image live_undist_left;
	Image live_undist_right;

	// Global flags
	bool verbose = false;
};

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
	typedef HmdStereoSource CLASSNAME;
	bool running = false;
	bool verbose = false;
	HMD::System sys;
	One<HMD::Camera> cam;
	Image last_left;
	Image last_right;
	bool last_is_bright = false;
	int64 last_serial = -1;

	Upp::Thread background_thread;
	std::atomic<bool> quit;
	Upp::Mutex mutex;
	Image bg_left_bright, bg_right_bright;
	Image bg_left_dark, bg_right_dark;
	int64 bg_serial_bright = -1;
	int64 bg_serial_dark = -1;

	void BackgroundProcess();

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

// Preview/undistort helpers used by Calibration and LiveResult.
struct LensParams {
	float f = 0;
	float cx = 0, cy = 0;
	float k1 = 0, k2 = 0;
};

String GetCalibrationStateText(int state);

RGBA SampleBilinear(const Image& img, float x, float y);
Image UndistortImage(const Image& src, const LensPoly& lens, float linear_scale);
Image DistortImage(const Image& src, const LensPoly& lens, float linear_scale);
Image ApplyExtrinsicsOnly(const Image& src, float yaw, float pitch, float roll, const vec2& pp);
Image ApplyIntrinsicsOnly(const Image& src, const LensPoly& lens, float linear_scale, bool undistort);
Image RectifyAndRotateOnePass(const Image& src, const LensParams& lp, float yaw, float pitch, float roll, Size out_sz);
Pointf ProjectPointOnePass(Pointf src_norm, Size src_sz, const LensParams& lp, float yaw, float pitch, float roll);
Pointf UnprojectPointOnePass(Pointf rect_px, Size rect_sz, const LensParams& lp, float yaw, float pitch, float roll);

// Persistence helpers (project.json + calibration file).
String GetPersistPath(const AppModel& model);
String GetStatePath(const AppModel& model);
String GetReportPath(const AppModel& model);
void LoadLastCalibration(AppModel& model);
void SaveLastCalibration(AppModel& model);
void LoadState(AppModel& model);
void SaveState(const AppModel& model);

void ShowInstructions();
}

// ------------------------------------------------------------
// Forward declarations for window modules
// ------------------------------------------------------------

class MenuWindow;
class CameraWindow;
class PreviewCtrl;
class CalibrationWindow;
class MainCalibWindow;

// Identity test helper (headless diagnostic).
int TestCalibrationIdentity(AppModel& model, const String& project_dir, const String& image_path = String());

// Regression suite for Calibration viewer invariants (headless, no GUI/camera required).
int RunCalibrationRegression(const String& project_dir, bool verbose = false);

// Self-check for Calibration distortion monotonic improvement.
int RunCalibrationDistortionSelfCheck(bool verbose = false);

END_UPP_NAMESPACE

// ------------------------------------------------------------
// Sub-headers (TopWindow modules)
// ------------------------------------------------------------

#include "Menu.h"
#include "Camera.h"
#include "PreviewCtrl.h"
#include "Calibration.h"
#include "MainCalibWindow.h"

#endif
