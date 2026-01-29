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
	};

	struct CapturedFrame : Moveable<CapturedFrame> {
		Time time;
		String source;
		int samples = 0;
		Image left_img;
		Image right_img;
		Vector<MatchPair> matches;
	};

	struct PreviewCtrl : public Upp::Ctrl {
		bool live = true;
		bool has_images = false;
		Image left_img;
		Image right_img;
		String overlay;
		Pointf pending_left = Null;
		Vector<MatchPair> matches;
		
		Event<Pointf, int> WhenClick;
		
		void SetLive(bool b) { live = b; Refresh(); }
		void SetImages(const Image& l, const Image& r) { left_img = l; right_img = r; has_images = !IsNull(l) || !IsNull(r); Refresh(); }
		void SetOverlay(const String& s) { overlay = s; Refresh(); }
		void SetPendingLeft(Pointf p) { pending_left = p; Refresh(); }
		void SetMatches(const Vector<MatchPair>& m) { matches <<= m; Refresh(); }
		
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
	};
	
	struct HmdStereoSource : StereoSource {
		bool running = false;
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
	ParentCtrl left;
	ParentCtrl right;
	TabCtrl bottom_tabs;
	Splitter captures_split;
	PreviewCtrl preview;
	
	Label source_info;
	Label calibration_info;
	Label calibration_schema;
	Label calibration_preview;
	Button export_calibration;
	Button load_calibration;
	Button live_view;
	Button capture_frame;
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
	LabelBox sep_diag;
	
	ArrayCtrl captures_list;
	ArrayCtrl matches_list;
	DocEdit report_text;
	Vector<One<StereoSource>> sources;
	Upp::Mutex source_mutex;
	StereoCalibrationData last_calibration;
	Vector<CapturedFrame> captured_frames;
	int pending_capture_row = -1;
	int64 last_serial = -1;
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

	typedef StereoCalibrationTool CLASSNAME;
	StereoCalibrationTool();
	~StereoCalibrationTool();
	
	void BuildLayout();
	void BuildLeftPanel();
	void BuildBottomTabs();
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
	void SetVerbose(bool v) { verbose = v; }
	void OnSourceChanged();
	void StartSource();
	void StopSource();
	void LiveView();
	void CaptureFrame();
	void ExportCalibration();
	void LoadCalibration();
	bool SaveCalibrationFile(const String& path, const StereoCalibrationData& data);
	bool LoadCalibrationFile(const String& path, StereoCalibrationData& out);
	void SyncCalibrationFromEdits();
	void SyncEditsFromCalibration();
	void UpdatePreview();
	String GetStatePath() const;
	String GetPersistPath() const;
	void LoadLastCalibration();
	void SaveLastCalibration();
	void LoadState();
	void SaveState();

	void MainMenu(Bar& bar);
	void AppMenu(Bar& bar);
	void ViewMenu(Bar& bar);
	void HelpMenu(Bar& bar);
};

END_UPP_NAMESPACE

#endif