#ifndef _StereoCalibrationTool_LiveResult_h_
#define _StereoCalibrationTool_LiveResult_h_

/*
LiveResult.h
------------
Purpose:
- Live camera view with calibration applied in real-time.

Key classes:
- LiveResultWindow: TopWindow that reads live frames and applies calibration.

Data flow:
- Uses camera pipeline to read frames.
- Uses AppModel.last_calibration + Stage C deltas for live rendering.

Gotchas / invariants:
- Live preview must not update capture list.
- If no calibration is loaded, view should fall back to raw.
*/

class LiveResultWindow : public TopWindow {
public:
	typedef LiveResultWindow CLASSNAME;
	LiveResultWindow();

	void Init(AppModel& model);
	void SetVerbose(bool v);

	// External triggers used by controller tests.
	bool StartSourceByIndex(int idx);
	void StopSource();
	bool PeekFrame(Image& left, Image& right, bool prefer_bright);

private:
	AppModel* model = nullptr;
	Vector<One<StereoSource>> sources;
	Mutex source_mutex;
	bool verbose = false;

	DropList source_list;
	Button start_source;
	Button stop_source;
	Button live_view;
	Label source_status;
	ImageCtrl left_view;
	ImageCtrl right_view;
	StatusBar status;
	Splitter preview_split;

	bool live_active = false;
	int64 live_serial = -1;
	TimeCallback live_cb;

	void BuildLayout();
	void OnSourceChanged();
	void StartSource();
	void ToggleLiveView();
	void UpdateLivePreview();
	bool BuildLiveUndistortCache(const Image& left, const Image& right, int64 serial);
};

#endif
