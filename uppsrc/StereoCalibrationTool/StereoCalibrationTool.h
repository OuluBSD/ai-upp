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

struct StereoCalibrationTool : TopWindow {
	struct PreviewCtrl : Ctrl {
		bool live = true;
		bool has_images = false;
		Image left_img;
		Image right_img;
		String overlay;
		
		void SetLive(bool b) { live = b; Refresh(); }
		void SetImages(const Image& l, const Image& r) { left_img = l; right_img = r; has_images = !IsNull(l) || !IsNull(r); Refresh(); }
		void SetOverlay(const String& s) { overlay = s; Refresh(); }
		
		virtual void Paint(Draw& w) override {
			Size sz = GetSize();
			w.DrawRect(sz, Black());
			if (has_images) {
				int half = max(1, sz.cx / 2);
				if (!IsNull(left_img))
					w.DrawImage(0, 0, half, sz.cy, left_img);
				if (!IsNull(right_img))
					w.DrawImage(half, 0, sz.cx - half, sz.cy, right_img);
			}
			String title = live ? "Live Preview" : "Captured Snapshot";
			w.DrawText(10, 10, title, Arial(18).Bold(), White());
			if (!overlay.IsEmpty())
				w.DrawText(10, 34, overlay, Arial(12), White());
		}
	};
	
	struct StereoSource {
		virtual ~StereoSource() {}
		virtual String GetName() const = 0;
		virtual bool Start() = 0;
		virtual void Stop() = 0;
		virtual bool IsRunning() const = 0;
		virtual bool ReadFrame(VisualFrame& left, VisualFrame& right) = 0;
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
		bool ReadFrame(VisualFrame& left, VisualFrame& right) override;
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
		bool ReadFrame(VisualFrame&, VisualFrame&) override;
	};
	
	struct VideoStereoSource : StereoSource {
		bool running = false;
		String path;
		String GetName() const override { return "Stereo Video File"; }
		bool Start() override { running = true; return true; }
		void Stop() override { running = false; }
		bool IsRunning() const override { return running; }
		bool ReadFrame(VisualFrame&, VisualFrame&) override { return false; }
	};
	
	struct MatchPair : Moveable<MatchPair> {
		String left;
		String right;
	};

	struct CapturedFrame : Moveable<CapturedFrame> {
		Time time;
		String source;
		int samples = 0;
		Image left_img;
		Image right_img;
		Vector<MatchPair> matches;
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
	StereoCalibrationData last_calibration;
	Vector<CapturedFrame> captured_frames;
	int pending_capture_row = -1;

	typedef StereoCalibrationTool CLASSNAME;
	StereoCalibrationTool();
	~StereoCalibrationTool();
	
	void BuildLayout();
	void BuildLeftPanel();
	void BuildBottomTabs();
	void Data();
	void DataCapturedFrame();
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
