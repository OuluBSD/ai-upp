#include "StereoCalibrationTool.h"

NAMESPACE_UPP

/*
Camera.cpp
==========
Purpose:
- Camera capture UI and capture list management.
- Owns persistence of captures on disk (project.json + PNGs).
 - Does NOT solve calibration or run Stage A/B/C logic.

Key classes:
- CameraWindow

Data flow:
- Writes captured frames into AppModel.
- Reads AppModel to populate capture list UI.

Gotchas / invariants:
- Capture list is the authoritative list; keep AppModel in sync.
- Live preview should not mutate calibration or match lists.
*/

CameraWindow::CameraWindow() {
	Title("Stereo Calibration Tool - Camera");
	Sizeable().Zoomable();
	AddFrame(status);
	BuildLayout();
}

// Binds AppModel and initializes source list + UI wiring.
// Assumes AppModel is already loaded by the controller (if applicable).
void CameraWindow::Init(AppModel& m) {
	model = &m;

	source_list.Add(0, "HMD Stereo Camera");
	source_list.Add(1, "USB Stereo (Side-by-side)");
	source_list.Add(2, "Stereo Video File");
	source_list.SetIndex(0);
	source_list.WhenAction = THISBACK(OnSourceChanged);

	start_source.SetLabel("Start");
	stop_source.SetLabel("Stop");
	live_view.SetLabel("Live view");
	capture_frame.SetLabel("Capture");

	start_source <<= THISBACK(StartSource);
	stop_source <<= THISBACK(StopSource);
	live_view <<= THISBACK(ToggleLiveView);
	capture_frame <<= THISBACK(CaptureFrame);

	source_status.SetLabel("Status: idle");

	BuildCapturesList();

	sources.Clear();
	sources.Add(MakeOne<HmdStereoSource>());
	sources.Add(MakeOne<UsbStereoSource>());
	sources.Add(MakeOne<VideoStereoSource>());

	RefreshFromModel();
}

// Updates verbose flag and forwards it to all sources.
void CameraWindow::SetVerbose(bool v) {
	verbose = v;
	for (auto& src : sources)
		src->SetVerbose(v);
}

// Overrides the USB device path used by the USB stereo source.
// Safe to call before starting the source.
void CameraWindow::SetUsbDevicePath(const String& path) {
	if (sources.GetCount() > 1) {
		if (UsbStereoSource* usb = dynamic_cast<UsbStereoSource*>(~sources[1])) {
			usb->device_path = path;
		}
	}
}

// Refreshes UI lists based on AppModel state (no disk IO).
void CameraWindow::RefreshFromModel() {
	RefreshCapturesList();
	SyncSelectionFromModel();
}

// Builds the camera layout: top control row + preview/list split.
void CameraWindow::BuildLayout() {
	int y = 6;
	controls.Add(source_list.TopPos(y, 24).HSizePos(6, 6));
	y += 28;
	controls.Add(start_source.TopPos(y, 24).LeftPos(6, 70));
	controls.Add(stop_source.TopPos(y, 24).LeftPos(82, 70));
	controls.Add(live_view.TopPos(y, 24).LeftPos(158, 80));
	controls.Add(capture_frame.TopPos(y, 24).LeftPos(244, 80));
	y += 28;
	controls.Add(source_status.TopPos(y, 20).HSizePos(6, 6));

	controls.SetRect(0, 0, 10, y + 30);
	Add(controls.TopPos(0, y + 30).HSizePos());

	preview_split.Horz(left_view, right_view);
	preview_split.SetPos(5000);
	main_split.Horz(preview_split, captures_list);
	main_split.SetPos(6500);
	Add(main_split.VSizePos(y + 30, 0).HSizePos());
}

// Configures the capture list columns and selection callback.
void CameraWindow::BuildCapturesList() {
	captures_list.AddColumn("Time");
	captures_list.AddColumn("Source");
	captures_list.AddColumn("Samples");
	captures_list.WhenCursor = THISBACK(OnCaptureSelection);
}

// Updates the status label when the user selects a different source.
void CameraWindow::OnSourceChanged() {
	int idx = source_list.GetIndex();
	if (idx >= 0 && idx < sources.GetCount())
		source_status.SetLabel("Selected: " + sources[idx]->GetName());
}

// Starts the selected source index. Returns false on failure.
// Assumes the source is constructed in Init().
bool CameraWindow::StartSourceByIndex(int idx) {
	if (idx < 0 || idx >= sources.GetCount())
		return false;
	Mutex::Lock __(source_mutex);
	sources[idx]->SetVerbose(verbose);
	if (!sources[idx]->Start())
		return false;
	source_list.SetIndex(idx);
	source_status.SetLabel("Running: " + sources[idx]->GetName());
	return true;
}

// Starts the current source selected in the UI.
void CameraWindow::StartSource() {
	int idx = source_list.GetIndex();
	if (!StartSourceByIndex(idx))
		status.Set("Failed to start source.");
}

// Stops the current source and disables live view timers.
void CameraWindow::StopSource() {
	Mutex::Lock __(source_mutex);
	int idx = source_list.GetIndex();
	if (idx >= 0 && idx < sources.GetCount())
		sources[idx]->Stop();
	live_active = false;
	live_cb.Kill();
	source_status.SetLabel("Status: stopped");
}

// Toggles continuous live preview. Does not write any captures.
void CameraWindow::ToggleLiveView() {
	live_active = !live_active;
	if (live_active)
		live_cb.Set(-30, THISBACK(UpdateLivePreview));
	else
		live_cb.Kill();
}

// Polls the active source and updates the preview control.
// Assumes source is already running.
void CameraWindow::UpdateLivePreview() {
	int idx = source_list.GetIndex();
	if (idx < 0 || idx >= sources.GetCount())
		return;
	VisualFrame lf, rf;
	bool ok = false;
	{
		Mutex::Lock __(source_mutex);
		ok = sources[idx]->PeakFrame(lf, rf, false);
	}
	if (!ok)
		return;
	Image left = StereoCalibrationHelpers::CopyFrameImage(lf);
	Image right = StereoCalibrationHelpers::CopyFrameImage(rf);
	if (left.IsEmpty() || right.IsEmpty())
		return;
	left_view.SetImage(left);
	right_view.SetImage(right);
}

// Non-destructive peek for headless tests or preview checks.
bool CameraWindow::PeekFrame(Image& left, Image& right, bool prefer_bright) {
	int idx = source_list.GetIndex();
	if (idx < 0 || idx >= sources.GetCount())
		return false;
	VisualFrame lf, rf;
	bool ok = false;
	{
		Mutex::Lock __(source_mutex);
		ok = sources[idx]->PeakFrame(lf, rf, prefer_bright);
	}
	if (!ok)
		return false;
	left = StereoCalibrationHelpers::CopyFrameImage(lf);
	right = StereoCalibrationHelpers::CopyFrameImage(rf);
	return !left.IsEmpty() && !right.IsEmpty();
}

// Captures a single bright frame (if available) and stores it in AppModel.
// Returns false if no bright frame arrives within the timeout.
bool CameraWindow::CaptureFrameOnce() {
	int idx = source_list.GetIndex();
	if (idx < 0 || idx >= sources.GetCount())
		return false;

	VisualFrame lf, rf;
	bool found = false;
	TimeStop ts;
	while (ts.Elapsed() < 2000) {
		bool read_ok = false;
		{
			if (source_mutex.TryEnter()) {
				read_ok = sources[idx]->PeakFrame(lf, rf, true);
				source_mutex.Leave();
			}
		}
		if (read_ok && (lf.flags & VIS_FRAME_BRIGHT)) {
			found = true;
			break;
		}
		Sleep(5);
	}
	if (!found)
		return false;

	{
		Mutex::Lock __(source_mutex);
		sources[idx]->ReadFrame(lf, rf, true);
	}

	CapturedFrame frame;
	frame.time = GetSysTime();
	frame.source = AsString(source_list.GetValue());
	frame.left_img = StereoCalibrationHelpers::CopyFrameImage(lf);
	frame.right_img = StereoCalibrationHelpers::CopyFrameImage(rf);
	if (frame.left_img.IsEmpty() || frame.right_img.IsEmpty())
		return false;

	model->captured_frames.Add(pick(frame));
	model->selected_capture = model->captured_frames.GetCount() - 1;
	StereoCalibrationHelpers::SaveState(*model);
	RefreshCapturesList();
	captures_list.SetCursor(model->selected_capture);
	return true;
}

// UI wrapper for CaptureFrameOnce with status messaging.
void CameraWindow::CaptureFrame() {
	if (!CaptureFrameOnce()) {
		status.Set("Capture failed (no bright frame).");
		return;
	}
	status.Set("Captured and saved snapshot.");
}

// Rebuilds the capture list from AppModel.captured_frames.
void CameraWindow::RefreshCapturesList() {
	captures_list.Clear();
	for (int i = 0; i < model->captured_frames.GetCount(); i++) {
		auto& f = model->captured_frames[i];
		captures_list.Add(Format("%02d:%02d:%02d", f.time.hour, f.time.minute, f.time.second), f.source, f.matches.GetCount());
	}
}

// Syncs selection based on AppModel.selected_capture.
void CameraWindow::SyncSelectionFromModel() {
	if (model->selected_capture >= 0 && model->selected_capture < captures_list.GetCount())
		captures_list.SetCursor(model->selected_capture);
}

// Handles capture list selection changes and updates preview.
void CameraWindow::OnCaptureSelection() {
	int row = captures_list.GetCursor();
	model->selected_capture = row;
	if (row < 0 || row >= model->captured_frames.GetCount())
		return;
	CapturedFrame& frame = model->captured_frames[row];
	left_view.SetImage(frame.left_img);
	right_view.SetImage(frame.right_img);
}

// Loads project state from disk into AppModel (explicit call).
void CameraWindow::LoadProjectState() {
	if (!model || model->project_dir.IsEmpty())
		return;
	StereoCalibrationHelpers::LoadState(*model);
	RefreshCapturesList();
}

// Writes project state + captures to disk (explicit call).
void CameraWindow::SaveProjectState() {
	if (!model || model->project_dir.IsEmpty())
		return;
	StereoCalibrationHelpers::SaveState(*model);
}

// Ensures camera source is stopped when the window closes.
void CameraWindow::Close() {
	StopSource();
	TopWindow::Close();
}

END_UPP_NAMESPACE
