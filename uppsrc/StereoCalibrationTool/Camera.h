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

Data flow:
- Reads/writes AppModel (captures, selection, project_state, calibration).
- Delegates image conversion + capture logic to shared helpers.

Gotchas / invariants:
- Capture list is the authoritative list of frames.
- Live preview should never mutate calibration state.
*/

class CameraWindow : public TopWindow {
public:
	typedef CameraWindow CLASSNAME;
	CameraWindow();

	void Init(AppModel& model);
	void SetVerbose(bool v);
	void RefreshFromModel();
	void SetUsbDevicePath(const String& path);

	// External triggers used by the controller for headless tests.
	bool StartSourceByIndex(int idx);
	void StopSource();
	bool CaptureFrameOnce();
	bool PeekFrame(Image& left, Image& right, bool prefer_bright);

private:
	AppModel* model = nullptr;

	// Camera sources and synchronization.
	Vector<One<StereoSource>> sources;
	Mutex source_mutex;
	bool verbose = false;

	// UI controls.
	MenuBar menu;
	DropList source_list;
	Button start_source;
	Button stop_source;
	Button live_view;
	Button capture_frame;
	Label source_status;
	
	LabelBox board_group;
	Label board_info_lbl;
	Button generate_board_btn;
	
	ArrayCtrl captures_list;
	ImageCtrl left_view;
	ImageCtrl right_view;
	StatusBar status;
	ParentCtrl controls;
	Splitter main_split;
	Splitter preview_split;

	// Internal state.
	bool live_active = false;
	int64 live_serial = -1;
	TimeCallback live_cb;
	
	virtual void Close() override;
	virtual void MainMenu(Bar& bar);

	void SubMenuFile(Bar& bar);
	void SubMenuEdit(Bar& bar);
	void OnDeleteAll();
	void OnGenerateBoard();

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
	void OnDeleteCapture();
	void OnCaptureSelection();
	void LoadProjectState();
	void SaveProjectState();
};

END_UPP_NAMESPACE

#endif
