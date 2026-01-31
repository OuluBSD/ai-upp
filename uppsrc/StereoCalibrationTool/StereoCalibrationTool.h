#ifndef _StereoCalibrationTool_StereoCalibrationTool_h_
#define _StereoCalibrationTool_StereoCalibrationTool_h_

#include <CtrlLib/CtrlLib.h>
#include <Geometry/Geometry.h>
#include <ComputerVision/ComputerVision.h>
#include <SoftHMD/SoftHMD.h>
#ifdef flagLINUX
#include <plugin/libv4l2/libv4l2.h>
#endif

NAMESPACE_UPP

struct StereoCalibrationTool : public Upp::TopWindow {
	struct MatchPair : Moveable<MatchPair> {
		Pointf left = Null;
		Pointf right = Null;
		String left_text;
		String right_text;
		double dist_l = 0;
		double dist_r = 0;

		void Jsonize(JsonIO& jio) {
			jio("left", left)("right", right)("left_text", left_text)("right_text", right_text)("dist_l", dist_l)("dist_r", dist_r);
		}
	};

	struct CapturedFrame : Moveable<CapturedFrame> {
		Time time;
		String source;
		int samples = 0;
		Image left_img;
		Image right_img;
		Image undist_left;
		Image undist_right;
		vec4 undist_poly = vec4(0,0,0,0);
		Size undist_size = Size(0,0);
		bool undist_valid = false;
		Vector<MatchPair> matches;

		void Jsonize(JsonIO& jio) {
			jio("time", time)("source", source)("samples", samples)("matches", matches);
			// Images are not jsonized for simplicity now, or could be base64. 
			// For now we persist coordinates mainly.
		}
	};

	struct PreviewCtrl : public Upp::Ctrl {
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

	MenuBar menu;
	StatusBar status;
	Splitter vsplitter;
	Splitter hsplitter;
	ParentCtrl left_panel;
	ParentCtrl right_panel;
	TabCtrl bottom_tabs;
	Splitter captures_split;
	PreviewCtrl preview;
	
	Label source_info;
	Label calibration_info;
	Label calibration_schema;
	Label calibration_preview;
	Button export_calibration;
	Button deploy_calibration;
	Button load_calibration;
	Button live_view;
	Button capture_frame;
	Button solve_calibration;
	Button clear_matches;
	Label calib_enabled_lbl;
	Option calib_enabled;
	Label calib_eye_lbl;
	EditDouble calib_eye_dist;
	Label calib_outward_lbl;
	EditDouble calib_outward_angle;
	Label calib_poly_lbl;
	EditDouble calib_poly_a;
	EditDouble calib_poly_b;
	EditDouble calib_poly_c;
	EditDouble calib_poly_d;
	Label mode_lbl;
	DropList mode_list;
	DropList source_list;
	Button start_source;
	Button stop_source;
	Label source_status;
	LabelBox sep_source;
	LabelBox sep_mode;
	LabelBox sep_calib;
	LabelBox sep_review;
	LabelBox sep_diag;
	Option show_epipolar;
	// Option undistort_view; // Replaced by View Mode
	Option verbose_math_log;

	// New Layout Elements
	TabCtrl stage_tabs;
	ParentCtrl pinned_camera_controls;
	
	Label view_mode_lbl;
	DropList view_mode_list;
	Option overlay_eyes;
	Label alpha_lbl;
	SliderCtrl alpha_slider;
	Option overlay_swap; 
	Option show_difference;

	// Stage A controls
	ParentCtrl stage_a_ctrl;
	LabelBox eye_l_group, eye_r_group;
	Label yaw_l_lbl, pitch_l_lbl, roll_l_lbl;
	Label yaw_r_lbl, pitch_r_lbl, roll_r_lbl;
	EditDoubleSpin yaw_l, pitch_l, roll_l;
	EditDoubleSpin yaw_r, pitch_r, roll_r;
	Option preview_extrinsics;
	Label barrel_lbl, fov_lbl;
	EditDoubleSpin barrel_strength, fov_deg;
	// Option stage_a_undistort; // Removed/Integrated
	DocEdit basic_params_doc;
	// Option overlay_eyes; // Moved to pinned
	// Label alpha_lbl;
	// SliderCtrl alpha_slider;
	// DropList overlay_base_eye; // Replaced/Moved
	Button yaw_center_btn, pitch_center_btn;

	// Stage B controls
	ParentCtrl stage_b_ctrl;
	// Option stage_b_undistort; // Removed/Integrated
	Option stage_b_compare_basic;
	// solve_calibration and verbose_math_log moved here logically

	// Stage C controls
	ParentCtrl stage_c_ctrl;
	Option enable_stage_c;
	Label stage_c_mode_lbl;
	Switch stage_c_mode;
	Label max_dyaw_lbl, max_dpitch_lbl, max_droll_lbl;
	EditDoubleSpin max_dyaw, max_dpitch, max_droll;
	Label lambda_lbl;
	EditDoubleSpin lambda_edit;
	Button refine_btn;
	
	ArrayCtrl captures_list;
	ArrayCtrl matches_list;
	EditDouble dist_l_editor;
	EditDouble dist_r_editor;
	DocEdit report_text;
	DocEdit math_text;
	Vector<One<StereoSource>> sources;
	Upp::Mutex source_mutex;
	String project_dir;
	StereoCalibrationData last_calibration;
	LensPoly preview_lens;
	Size preview_lens_size = Size(0,0);
	vec4 preview_lens_poly = vec4(0,0,0,0);
	float preview_lens_outward = 0;
	vec2 preview_lens_pp = vec2(0,0);
	vec2 preview_lens_tilt = vec2(0,0);
	struct ProjectState {
		int schema_version = 1;
		
		// Stage A
		double eye_dist = 64.0;
		double yaw_l = 0, pitch_l = 0, roll_l = 0;
		double yaw_r = 0, pitch_r = 0, roll_r = 0;
		double fov_deg = 90.0;
		double barrel_strength = 0;
		bool preview_extrinsics = true;
		
		// Stage B
		double distance_weight = 0.1;
		double huber_px = 2.0;
		double huber_m = 0.030;
		bool lock_distortion = false;
		bool verbose_math_log = false;
		bool compare_basic_params = false;
		
		// Stage C
		bool stage_c_enabled = false;
		int stage_c_mode = 0; // 0=Relative, 1=Per-eye
		double max_dyaw = 3.0;
		double max_dpitch = 2.0;
		double max_droll = 3.0;
		double lambda = 0.1;
		
		// Viewer
		int view_mode = 0; // 0=Raw, 1=Basic, 2=Solved
		bool overlay_eyes = false;
		int alpha = 50;
		bool overlay_swap = false;
		bool show_difference = false;
		bool show_epipolar = false;

		void Jsonize(JsonIO& jio) {
			jio("schema_version", schema_version);
			jio("eye_dist", eye_dist)("yaw_l", yaw_l)("pitch_l", pitch_l)("roll_l", roll_l);
			jio("yaw_r", yaw_r)("pitch_r", pitch_r)("roll_r", roll_r);
			jio("fov_deg", fov_deg)("barrel_strength", barrel_strength)("preview_extrinsics", preview_extrinsics);
			
			jio("distance_weight", distance_weight)("huber_px", huber_px)("huber_m", huber_m);
			jio("lock_distortion", lock_distortion)("verbose_math_log", verbose_math_log)("compare_basic_params", compare_basic_params);
			
			jio("stage_c_enabled", stage_c_enabled)("stage_c_mode", stage_c_mode);
			jio("max_dyaw", max_dyaw)("max_dpitch", max_dpitch)("max_droll", max_droll)("lambda", lambda);
			
			jio("view_mode", view_mode)("overlay_eyes", overlay_eyes)("alpha", alpha);
			jio("overlay_swap", overlay_swap)("show_difference", show_difference)("show_epipolar", show_epipolar);
		}
	};
	
	ProjectState project_state;
	Vector<CapturedFrame> captured_frames;
	int pending_capture_row = -1;
	int64 last_serial = -1;
	int64 live_undist_serial = -1;
	Size live_undist_size = Size(0,0);
	vec4 live_undist_poly = vec4(0,0,0,0);
	bool live_undist_valid = false;
	Image live_undist_left;
	Image live_undist_right;

	// Stage C refined deltas
	float dyaw_c = 0, dpitch_c = 0, droll_c = 0;

	bool verbose = false;
	TimeCallback tc;
	TimeCallback usb_test_cb;
	bool usb_test_enabled = false;
	bool usb_test_active = false;
	int usb_test_timeout_ms = 4000;
	String usb_test_device;
	int64 usb_test_start_us = 0;
	int64 usb_test_last_start_us = 0;
	int usb_test_attempts = 0;
	TimeCallback hmd_test_cb;
	bool hmd_test_enabled = false;
	bool hmd_test_active = false;
	int hmd_test_timeout_ms = 4000;
	int64 hmd_test_start_us = 0;
	int64 hmd_test_last_start_us = 0;
	int hmd_test_attempts = 0;
	
	TimeCallback live_test_cb;
	bool live_test_active = false;
	int live_test_timeout_ms = 5000;
	int64 live_test_start_us = 0;

	// GA bootstrap settings
	bool use_ga_bootstrap = false;
	int ga_population = 30;
	int ga_generations = 20;

	enum ViewMode { VIEW_RAW = 0, VIEW_BASIC, VIEW_SOLVED };

	typedef StereoCalibrationTool CLASSNAME;
	StereoCalibrationTool();
	~StereoCalibrationTool();
	
	void BuildLayout();
	void BuildLeftPanel();
	void BuildBottomTabs();
	void BuildStageA();
	void BuildStageB();
	void BuildStageC();
	void Data();
	void Sync();
	void DataCapturedFrame();
	void EnableUsbTest(const String& dev, int timeout_ms);
	void StartUsbTest();
	void RunUsbTest();
	void EnableHmdTest(int timeout_ms);
	void StartHmdTest();
	void RunHmdTest();
	void EnableLiveTest(int timeout_ms);
	void StartLiveTest();
	void RunLiveTest();
	void SetVerbose(bool v);
	void EnableGABootstrap(bool enable, int population = 30, int generations = 20);
	void SetProjectDir(const String& dir);
	int SolveHeadless(const String& project_dir);
	void OnSourceChanged();
	void StartSource();
	void StopSource();
	void LiveView();
	void CaptureFrame();
	void SolveCalibration();
	void RefineExtrinsics();
	void ClearMatches();
	void RemoveSnapshot();
	void RemoveMatchPair();
	void ExportCalibration();
	void DeployCalibration();
	void LoadCalibration();
	void SyncCalibrationFromEdits();
	void SyncEditsFromCalibration();
	void SyncStageA();
	void UpdatePreview();
	void UpdateReviewOverlay();
	void UpdateReviewEnablement();
	bool PreparePreviewLens(const Size& sz);
	bool BuildUndistortCache(CapturedFrame& frame);
	bool BuildLiveUndistortCache(const Image& left, const Image& right, int64 serial);
	void ApplyPreviewImages(CapturedFrame& frame);
	Pointf MapClickToRaw(Pointf p);
	void OnReviewChanged();
	void OnYawCenter();
	void OnPitchCenter();
	String GetStatePath() const;
	String GetPersistPath() const;
	String GetReportPath() const;
	void LoadLastCalibration();
	void SaveLastCalibration();
	void LoadState();
	void SaveState();

	void MainMenu(Bar& bar);
	void AppMenu(Bar& bar);
	void ViewMenu(Bar& bar);
	void EditMenu(Bar& bar);
	void HelpMenu(Bar& bar);
};

END_UPP_NAMESPACE

#endif
