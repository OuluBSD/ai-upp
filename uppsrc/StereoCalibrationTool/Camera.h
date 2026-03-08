#ifndef _StereoCalibrationTool_Camera_h_
#define _StereoCalibrationTool_Camera_h_

NAMESPACE_UPP

/*
Camera.h
--------
Purpose:
- Owns camera start/stop/capture and the captured frame list UI.
- Handles persistence of project.json and capture images on disk.

Key classes:
- CameraWindow: TopWindow for source selection, live preview, capture list.
- CameraPane: Embedded control for unified window.
*/

class CameraPane : public ParentCtrl {
public:
	typedef CameraPane CLASSNAME;
	CameraPane();

	void Init(AppModel& model);
	void SetVerbose(bool v);
	void RefreshFromModel();
	void SetUsbDevicePath(const String& path);

	// External triggers used by the controller for headless tests.
	bool StartSourceByIndex(int idx);
	void StopSource();
	bool CaptureFrameOnce();
	bool PeekFrame(Image& left, Image& right, bool prefer_bright);

	void OnDeleteCapture();
	void OnDeleteAll();

	Event<> WhenChange;

	AppModel* model = nullptr;
	ArrayCtrl captures_list;

private:
	// Camera sources and synchronization.
	Vector<One<StereoSource>> sources;
	Vector<String> source_ids;
	Mutex source_mutex;
	bool verbose = false;
	String usb_device_path;

	// UI controls.
	DropList source_list;
	Button start_source;
	Button stop_source;
	Button live_view;
	Button capture_frame;
	Label source_status;
	
	LabelBox board_group;
	Label board_info_lbl;
	Button generate_board_btn;
	
	StereoPreviewCtrl preview;
	ParentCtrl controls;
	Splitter main_split;

	// Internal state.
	bool live_active = false;
	int64 live_serial = -1;
	TimeCallback live_cb;

	void BuildLayout();
	void BuildCapturesList();
	void OnSourceChanged();
	void StartSource();
	void ToggleLiveView();
	void CaptureFrame();
	void UpdateLivePreview();
	void RefreshCapturesList();
	void SyncSelectionFromModel();
	void OnCapturesBar(Bar& bar);
	void OnCaptureSelection();
	void OnGenerateBoard();
	void ApplyCalibrationForSource(int idx);
};

class CameraWindow : public TopWindow {
public:
	typedef CameraWindow CLASSNAME;
	CameraWindow();

	void Init(AppModel& model) { pane.Init(model); }
	void SetVerbose(bool v) { pane.SetVerbose(v); }
	void RefreshFromModel() { pane.RefreshFromModel(); }
	void SetUsbDevicePath(const String& path) { pane.SetUsbDevicePath(path); }

	// External triggers used by the controller for headless tests.
	bool StartSourceByIndex(int idx) { return pane.StartSourceByIndex(idx); }
	void StopSource() { pane.StopSource(); }
	bool CaptureFrameOnce() { return pane.CaptureFrameOnce(); }
	bool PeekFrame(Image& left, Image& right, bool prefer_bright) { return pane.PeekFrame(left, right, prefer_bright); }

private:
	CameraPane pane;
	MenuBar menu;
	StatusBar status;
	
	virtual void Close() override;
	virtual void MainMenu(Bar& bar);

	void SubMenuFile(Bar& bar);
	void SubMenuEdit(Bar& bar);
	void SubMenuHelp(Bar& bar);
};

END_UPP_NAMESPACE

#endif
