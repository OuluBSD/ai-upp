#include "StereoCalibrationTool.h"

NAMESPACE_UPP

/*
Camera.cpp
==========
Purpose:
- Camera capture UI and capture list management.
- Owns persistence of captures on disk (project.json + PNGs).
 - Does NOT solve calibration or run alignment logic.

Key classes:
- CameraWindow

Data flow:
- Writes captured frames into AppModel.
- Reads AppModel to populate capture list UI.

Gotchas / invariants:
- Capture list is the authoritative list; keep AppModel in sync.
- Live preview should not mutate calibration or match lists.
*/

CameraPane::CameraPane() {
	BuildLayout();
}

// Binds AppModel and initializes source list + UI wiring.
// Assumes AppModel is already loaded by the controller (if applicable).
void CameraPane::Init(AppModel& m) {
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

	board_group.SetLabel("Calibration Board");
	board_info_lbl.SetLabel("Generate printable checkerboard for calibration.");
	generate_board_btn.SetLabel("Generate Board...");
	generate_board_btn <<= THISBACK(OnGenerateBoard);

	BuildCapturesList();

	sources.Clear();
	sources.Add(MakeOne<HmdStereoSource>());
	sources.Add(MakeOne<UsbStereoSource>());
	sources.Add(MakeOne<VideoStereoSource>());

	RefreshFromModel();
}

// Updates verbose flag and forwards it to all sources.
void CameraPane::SetVerbose(bool v) {
	verbose = v;
	for (auto& src : sources)
		src->SetVerbose(v);
}

// Overrides the USB device path used by the USB stereo source.
// Safe to call before starting the source.
void CameraPane::SetUsbDevicePath(const String& path) {
	if (sources.GetCount() > 1) {
		if (UsbStereoSource* usb = dynamic_cast<UsbStereoSource*>(~sources[1])) {
			usb->device_path = path;
		}
	}
}

// Refreshes UI lists based on AppModel state (no disk IO).
void CameraPane::RefreshFromModel() {
	RefreshCapturesList();
	SyncSelectionFromModel();
}

// Builds the camera layout: top control row + preview/list split.
void CameraPane::BuildLayout() {
	int y = 6;
	controls.Add(source_list.TopPos(y, 24).HSizePos(6, 6));
	y += 28;
	controls.Add(start_source.TopPos(y, 24).LeftPos(6, 70));
	controls.Add(stop_source.TopPos(y, 24).LeftPos(82, 70));
	controls.Add(live_view.TopPos(y, 24).LeftPos(158, 80));
	controls.Add(capture_frame.TopPos(y, 24).LeftPos(244, 80));
	y += 28;
	controls.Add(source_status.TopPos(y, 20).HSizePos(6, 6));
	
	y += 28;
	controls.Add(board_group.TopPos(y, 60).HSizePos(6, 6));
	controls.Add(board_info_lbl.TopPos(y + 20, 20).LeftPos(12, 200));
	controls.Add(generate_board_btn.TopPos(y + 20, 24).RightPos(12, 120));
	y += 60;

	controls.SetRect(0, 0, 10, y + 30);
	Add(controls.TopPos(0, y + 30).HSizePos());

	preview_split.Horz(left_view, right_view);
	preview_split.SetPos(5000);
	main_split.Horz(preview_split, captures_list);
	main_split.SetPos(6500);
	Add(main_split.VSizePos(y + 30, 0).HSizePos());
}

// Configures the capture list columns and selection callback.
void CameraPane::BuildCapturesList() {
	captures_list.AddColumn("Time");
	captures_list.AddColumn("Source");
	captures_list.AddColumn("Samples");
	captures_list.WhenCursor = THISBACK(OnCaptureSelection);
	captures_list.WhenBar = THISBACK(OnCapturesBar);
}

void CameraPane::OnCapturesBar(Bar& bar) {
	bar.Add("Delete selected", THISBACK(OnDeleteCapture))
	   .Enable(captures_list.IsCursor());
}

void CameraPane::OnDeleteCapture() {
	int row = captures_list.GetCursor();
	if (row < 0 || row >= model->captured_frames.GetCount())
		return;
	
	if (!PromptOKCancel("Delete selected capture frame? This will rewrite indices on disk."))
		return;
	
	model->captured_frames.Remove(row);
	if (model->selected_capture >= model->captured_frames.GetCount())
		model->selected_capture = model->captured_frames.GetCount() - 1;
	
	StereoCalibrationHelpers::SaveState(*model);
	RefreshCapturesList();
	SyncSelectionFromModel();
	WhenChange();
}

// Updates the status label when the user selects a different source.
void CameraPane::OnSourceChanged() {
	int idx = source_list.GetIndex();
	if (idx >= 0 && idx < sources.GetCount())
		source_status.SetLabel("Selected: " + sources[idx]->GetName());
}

struct BoardGenDialog : TopWindow {
	EditInt squares_x, squares_y;
	EditDouble square_size_mm;
	Button ok, cancel;
	Label lbl_x, lbl_y, lbl_sz;
	
	bool run = false;
	
	BoardGenDialog() {
		Title("Generate Checkerboard");
		SetRect(0, 0, 300, 160);
		
		lbl_x.SetLabel("Squares X:");
		Add(lbl_x.TopPos(10, 20).LeftPos(10, 80));
		squares_x.MinMax(3, 25) <<= 9;
		Add(squares_x.TopPos(10, 20).LeftPos(100, 50));
		
		lbl_y.SetLabel("Squares Y:");
		Add(lbl_y.TopPos(40, 20).LeftPos(10, 80));
		squares_y.MinMax(3, 25) <<= 6;
		Add(squares_y.TopPos(40, 20).LeftPos(100, 50));
		
		lbl_sz.SetLabel("Size (mm):");
		Add(lbl_sz.TopPos(70, 20).LeftPos(10, 80));
		square_size_mm.MinMax(2.0, 100.0) <<= 20.0;
		Add(square_size_mm.TopPos(70, 20).LeftPos(100, 50));
		
		ok.SetLabel("Generate");
		ok.WhenAction = [=] { run = true; Close(); };
		Add(ok.BottomPos(10, 24).RightPos(80, 80));
		
		cancel.SetLabel("Cancel");
		cancel.WhenAction = [=] { Close(); };
		Add(cancel.BottomPos(10, 24).RightPos(10, 60));
	}
};

void CameraPane::OnGenerateBoard() {
	BoardGenDialog dlg;
	dlg.RunAppModal();
	if (!dlg.run) return;
	
	int nx_squares = (int)dlg.squares_x;
	int ny_squares = (int)dlg.squares_y;
	double sz_mm = (double)dlg.square_size_mm;
	
	// A4 Dimensions: 210mm x 297mm
	const double A4_W = 210.0;
	const double A4_H = 297.0;
	
	double board_w_mm = nx_squares * sz_mm;
	double board_h_mm = ny_squares * sz_mm;
	
	// Quiet zone: OpenCV needs at least 1 square size of white border
	if (board_w_mm > (A4_W - sz_mm * 2) || board_h_mm > (A4_H - sz_mm * 2)) {
		if (!PromptOKCancel(Format("Board (%.0fx%.0f mm) is tight for A4.\nContinue?", board_w_mm, board_h_mm)))
			return;
	}
	
	double dpi = 300.0;
	double px_per_mm = dpi / 25.4;
	int w_px = (int)(A4_W * px_per_mm);
	int h_px = (int)(A4_H * px_per_mm);
	
	ImageBuffer ib(w_px, h_px);
	RGBA white = White();
	RGBA black = Black();
	RGBA gray = GrayColor(220); // Very light gray boundary
	
	Fill(ib, white, w_px * h_px);
	
	int margin_x = (int)((A4_W - board_w_mm) * 0.5 * px_per_mm);
	int margin_y = (int)((A4_H - board_h_mm) * 0.5 * px_per_mm);
	int sq_px = (int)(sz_mm * px_per_mm);
	
	// Draw boundary
	for (int i = 0; i <= nx_squares * sq_px; i++) {
		int px = margin_x + i;
		if (px >= 0 && px < w_px) {
			if (margin_y >= 0) ib[margin_y][px] = gray;
			if (margin_y + ny_squares * sq_px < h_px) ib[margin_y + ny_squares * sq_px][px] = gray;
		}
	}
	for (int i = 0; i <= ny_squares * sq_px; i++) {
		int py = margin_y + i;
		if (py >= 0 && py < h_px) {
			if (margin_x >= 0) ib[py][margin_x] = gray;
			if (margin_x + nx_squares * sq_px < w_px) ib[py][margin_x + nx_squares * sq_px] = gray;
		}
	}

	for (int y = 0; y < ny_squares; y++) {
		for (int x = 0; x < nx_squares; x++) {
			if ((x + y) % 2 == 0) {
				int px = margin_x + x * sq_px;
				int py = margin_y + y * sq_px;
				for (int dy = 0; dy < sq_px; dy++) {
					for (int dx = 0; dx < sq_px; dx++) {
						if (px+dx < w_px && py+dy < h_px)
							ib[py+dy][px+dx] = black;
					}
				}
			}
		}
	}
	
	String out_dir = model->project_dir.IsEmpty() ? GetCurrentDirectory() : model->project_dir;
	String png_path = AppendFileName(out_dir, "calibration_board.png");
	String txt_path = AppendFileName(out_dir, "calibration_board.txt");
	
	PNGEncoder().SaveFile(png_path, ib);
	
	FileOut out(txt_path);
	out << "squares_x=" << nx_squares << "\n";
	out << "squares_y=" << ny_squares << "\n";
	out << "corners_x=" << (nx_squares - 1) << "\n";
	out << "corners_y=" << (ny_squares - 1) << "\n";
	out << "square_size_mm=" << sz_mm << "\n";
	out.Close();
	
	PromptOK("Board generated successfully.\n\n"
	         "Term: Specifying 9x6 SQUARES results in 8x5 INNER CORNERS.\n"
	         "Print at 'Actual Size' (100% scale).");
}

// Starts the selected source index. Returns false on failure.
// Assumes the source is constructed in Init().
bool CameraPane::StartSourceByIndex(int idx) {
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
void CameraPane::StartSource() {
	int idx = source_list.GetIndex();
	if (!StartSourceByIndex(idx)) {
		// status.Set("Failed to start source."); // CameraPane doesn't have status bar
	}
}

// Stops the current source and disables live view timers.
void CameraPane::StopSource() {
	Mutex::Lock __(source_mutex);
	int idx = source_list.GetIndex();
	if (idx >= 0 && idx < sources.GetCount())
		sources[idx]->Stop();
	live_active = false;
	live_cb.Kill();
	source_status.SetLabel("Status: stopped");
}

// Toggles continuous live preview. Does not write any captures.
void CameraPane::ToggleLiveView() {
	live_active = !live_active;
	if (live_active)
		live_cb.Set(-16, THISBACK(UpdateLivePreview));
	else
		live_cb.Kill();
}

// Polls the active source and updates the preview control.
// Assumes source is already running.
void CameraPane::UpdateLivePreview() {
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
bool CameraPane::PeekFrame(Image& left, Image& right, bool prefer_bright) {
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
bool CameraPane::CaptureFrameOnce() {
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
	WhenChange();
	return true;
}

// UI wrapper for CaptureFrameOnce with status messaging.
void CameraPane::CaptureFrame() {
	CaptureFrameOnce();
}

// Rebuilds the capture list from AppModel.captured_frames.
void CameraPane::RefreshCapturesList() {
	captures_list.Clear();
	for (int i = 0; i < model->captured_frames.GetCount(); i++) {
		auto& f = model->captured_frames[i];
		captures_list.Add(Format("%02d:%02d:%02d", f.time.hour, f.time.minute, f.time.second), f.source, f.matches.GetCount());
	}
}

// Syncs selection based on AppModel.selected_capture.
void CameraPane::SyncSelectionFromModel() {
	if (model->selected_capture >= 0 && model->selected_capture < captures_list.GetCount())
		captures_list.SetCursor(model->selected_capture);
}

// Handles capture list selection changes and updates preview.
void CameraPane::OnCaptureSelection() {
	int row = captures_list.GetCursor();
	model->selected_capture = row;
	if (row < 0 || row >= model->captured_frames.GetCount())
		return;
	CapturedFrame& frame = model->captured_frames[row];
	left_view.SetImage(frame.left_img);
	right_view.SetImage(frame.right_img);
}

// ------------------------------------------------------------

void CameraPane::OnDeleteAll() {
	if (model->captured_frames.IsEmpty()) return;
	if (!PromptOKCancel("Delete ALL captured frames? This cannot be undone.")) return;
	
	model->captured_frames.Clear();
	model->selected_capture = -1;
	StereoCalibrationHelpers::SaveState(*model);
	RefreshCapturesList();
	left_view.SetImage(Image());
	right_view.SetImage(Image());
	WhenChange();
}

// ------------------------------------------------------------

CameraWindow::CameraWindow() {
	Title("Stereo Calibration Tool - Camera");
	Sizeable().Zoomable();
	AddFrame(menu);
	AddFrame(status);
	menu.Set(THISBACK(MainMenu));
	Add(pane.SizePos());
}

void CameraWindow::MainMenu(Bar& bar) {
	bar.Add("File", THISBACK(SubMenuFile));
	bar.Add("Edit", THISBACK(SubMenuEdit));
	bar.Add("Help", THISBACK(SubMenuHelp));
}

void CameraWindow::SubMenuHelp(Bar& bar) {
	bar.Add("Instructions", [] { StereoCalibrationHelpers::ShowInstructions(); });
}

void CameraWindow::SubMenuFile(Bar& bar) {
	bar.Add("Save project", [=] {
		if (pane.model) StereoCalibrationHelpers::SaveState(*pane.model);
	});
}

void CameraWindow::SubMenuEdit(Bar& bar) {
	bar.Add("Delete selected frame", [=] { pane.OnDeleteCapture(); })
	   .Key(K_DELETE)
	   .Enable(pane.captures_list.IsCursor());
	bar.Add("Delete all frames...", [=] { pane.OnDeleteAll(); });
}

// Ensures camera source is stopped when the window closes.
void CameraWindow::Close() {
	pane.StopSource();
	TopWindow::Close();
}

END_UPP_NAMESPACE
