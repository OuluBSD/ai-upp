#ifndef _StereoCalibrationTool_StereoCalibrationTool_h_
#define _StereoCalibrationTool_StereoCalibrationTool_h_

#include <CtrlLib/CtrlLib.h>
#include <Geometry/Geometry.h>
#include <ComputerVision/ComputerVision.h>

NAMESPACE_UPP

struct StereoCalibrationTool : TopWindow {
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
	TabCtrl tabs;
	ParentCtrl source_tab;
	ParentCtrl calibration_tab;
	Label source_info;
	Label calibration_info;
	Label calibration_schema;
	Button export_calibration;
	DropList source_list;
	Button start_source;
	Button stop_source;
	Label source_status;
	Vector<One<StereoSource>> sources;
	StereoCalibrationData last_calibration;

	typedef StereoCalibrationTool CLASSNAME;
	StereoCalibrationTool();
	
	void BuildSourceTab();
	void BuildCalibrationTab();
	void OnSourceChanged();
	void StartSource();
	void StopSource();
	void ExportCalibration();
	bool SaveCalibrationFile(const String& path, const StereoCalibrationData& data);

	void MainMenu(Bar& bar);
	void AppMenu(Bar& bar);
	void ViewMenu(Bar& bar);
	void HelpMenu(Bar& bar);
};

END_UPP_NAMESPACE

#endif
