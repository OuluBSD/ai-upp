#ifndef _StereoCalibrationTool_StereoCalibrationTool_h_
#define _StereoCalibrationTool_StereoCalibrationTool_h_

#include <CtrlLib/CtrlLib.h>
#include <Geometry/Geometry.h>
#include <ComputerVision/ComputerVision.h>

NAMESPACE_UPP

struct StereoCalibrationTool : TopWindow {
	struct PreviewCtrl : Ctrl {
		bool live = true;
		String overlay;
		
		void SetLive(bool b) { live = b; Refresh(); }
		void SetOverlay(const String& s) { overlay = s; Refresh(); }
		
		virtual void Paint(Draw& w) override {
			Size sz = GetSize();
			w.DrawRect(sz, Black());
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
		String GetName() const override { return "HMD Stereo Camera"; }
		bool Start() override { running = true; return true; }
		void Stop() override { running = false; }
		bool IsRunning() const override { return running; }
		bool ReadFrame(VisualFrame&, VisualFrame&) override { return false; }
	};
	
	struct UsbStereoSource : StereoSource {
		bool running = false;
		String GetName() const override { return "USB Stereo (Side-by-side)"; }
		bool Start() override { running = true; return true; }
		void Stop() override { running = false; }
		bool IsRunning() const override { return running; }
		bool ReadFrame(VisualFrame&, VisualFrame&) override { return false; }
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
	
	MenuBar menu;
	StatusBar status;
	Splitter vsplitter;
	Splitter hsplitter;
	ParentCtrl left;
	ParentCtrl right;
	TabCtrl bottom_tabs;
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

	typedef StereoCalibrationTool CLASSNAME;
	StereoCalibrationTool();
	~StereoCalibrationTool();
	
	void BuildLayout();
	void BuildLeftPanel();
	void BuildBottomTabs();
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
	String GetPersistPath() const;
	void LoadLastCalibration();
	void SaveLastCalibration();

	void MainMenu(Bar& bar);
	void AppMenu(Bar& bar);
	void ViewMenu(Bar& bar);
	void HelpMenu(Bar& bar);
};

END_UPP_NAMESPACE

#endif
