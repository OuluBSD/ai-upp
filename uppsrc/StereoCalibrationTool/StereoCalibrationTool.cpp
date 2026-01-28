#include "StereoCalibrationTool.h"

NAMESPACE_UPP

StereoCalibrationTool::StereoCalibrationTool() {
	Title("Stereo Calibration Tool");
	Sizeable().Zoomable();

	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));

	source_info.SetLabel("Source setup goes here (live HMD, USB stereo, or video file).");
	calibration_info.SetLabel("Calibration workflow goes here (checkerboard/aruco capture)." );

	BuildSourceTab();
	source_tab.Add(source_info.BottomPos(8, StdFont().GetHeight() + 8).HSizePos(8, 8));
	calibration_tab.Add(calibration_info.SizePos());

	tabs.Add(source_tab, "Source");
	tabs.Add(calibration_tab, "Calibration");

	Add(tabs.SizePos());
}

void StereoCalibrationTool::BuildSourceTab() {
	source_list.Add(0, "HMD Stereo Camera");
	source_list.Add(1, "USB Stereo (Side-by-side)");
	source_list.Add(2, "Stereo Video File");
	source_list.SetIndex(0);
	source_list.WhenAction = THISBACK(OnSourceChanged);
	
	start_source.SetLabel("Start");
	stop_source.SetLabel("Stop");
	start_source <<= THISBACK(StartSource);
	stop_source <<= THISBACK(StopSource);
	
	source_status.SetLabel("Status: idle");
	
	source_tab.Add(source_list.TopPos(8, 24).HSizePos(8, 8));
	source_tab.Add(start_source.TopPos(40, 24).LeftPos(8, 80));
	source_tab.Add(stop_source.TopPos(40, 24).LeftPos(96, 80));
	source_tab.Add(source_status.TopPos(72, 24).HSizePos(8, 8));
	
	sources.Clear();
	sources.Add(MakeOne<HmdStereoSource>());
	sources.Add(MakeOne<UsbStereoSource>());
	sources.Add(MakeOne<VideoStereoSource>());
}

void StereoCalibrationTool::OnSourceChanged() {
	StopSource();
	source_status.SetLabel(Format("Status: ready (%s)", source_list.GetText()));
}

void StereoCalibrationTool::StartSource() {
	int idx = source_list.GetIndex();
	if (idx < 0 || idx >= sources.GetCount())
		return;
	if (sources[idx]->Start())
		source_status.SetLabel(Format("Status: running (%s)", source_list.GetText()));
	else
		source_status.SetLabel(Format("Status: failed (%s)", source_list.GetText()));
}

void StereoCalibrationTool::StopSource() {
	int idx = source_list.GetIndex();
	if (idx < 0 || idx >= sources.GetCount())
		return;
	sources[idx]->Stop();
	source_status.SetLabel(Format("Status: stopped (%s)", source_list.GetText()));
}

void StereoCalibrationTool::MainMenu(Bar& bar) {
	bar.Sub("App", THISBACK(AppMenu));
	bar.Sub("View", THISBACK(ViewMenu));
	bar.Sub("Help", THISBACK(HelpMenu));
}

void StereoCalibrationTool::AppMenu(Bar& bar) {
	bar.Add("Exit", [=] { Close(); });
}

void StereoCalibrationTool::ViewMenu(Bar& bar) {
	bar.Add("Source", [=] { tabs.SetCursor(0); });
	bar.Add("Calibration", [=] { tabs.SetCursor(1); });
}

void StereoCalibrationTool::HelpMenu(Bar& bar) {
	bar.Add("About", [=] {
		PromptOK("Stereo Calibration Tool\n\nWork-in-progress.");
	});
}

END_UPP_NAMESPACE
