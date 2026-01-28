#include "StereoCalibrationTool.h"

NAMESPACE_UPP

StereoCalibrationTool::StereoCalibrationTool() {
	Title("Stereo Calibration Tool");
	Sizeable().Zoomable();

	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));

	source_info.SetLabel("Source setup goes here (live HMD, USB stereo, or video file).");
	calibration_info.SetLabel("Calibration workflow goes here (checkerboard/aruco capture).");
	calibration_schema.SetLabel("Output schema (.stcal):\n"
		"  enabled=0|1\n"
		"  eye_dist=<float>\n"
		"  outward_angle=<float>\n"
		"  angle_poly=a,b,c,d\n");
	calibration_preview.SetLabel("Preview: (no calibration loaded)");

	BuildSourceTab();
	BuildCalibrationTab();
	source_tab.Add(source_info.BottomPos(8, StdFont().GetHeight() + 8).HSizePos(8, 8));

	tabs.Add(source_tab, "Source");
	tabs.Add(calibration_tab, "Calibration");

	Add(tabs.SizePos());
	LoadLastCalibration();
	SyncEditsFromCalibration();
	UpdatePreview();
}

StereoCalibrationTool::~StereoCalibrationTool() {
	SaveLastCalibration();
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

void StereoCalibrationTool::BuildCalibrationTab() {
	export_calibration.SetLabel("Export .stcal");
	export_calibration <<= THISBACK(ExportCalibration);
	load_calibration.SetLabel("Load .stcal");
	load_calibration <<= THISBACK(LoadCalibration);
	calib_enabled_lbl.SetLabel("Enabled");
	calib_eye_lbl.SetLabel("Eye dist");
	calib_outward_lbl.SetLabel("Outward angle");
	calib_poly_lbl.SetLabel("Angle poly");
	calib_enabled.WhenAction = THISBACK(SyncCalibrationFromEdits);
	calib_eye_dist.WhenAction = THISBACK(SyncCalibrationFromEdits);
	calib_outward_angle.WhenAction = THISBACK(SyncCalibrationFromEdits);
	calib_poly_a.WhenAction = THISBACK(SyncCalibrationFromEdits);
	calib_poly_b.WhenAction = THISBACK(SyncCalibrationFromEdits);
	calib_poly_c.WhenAction = THISBACK(SyncCalibrationFromEdits);
	calib_poly_d.WhenAction = THISBACK(SyncCalibrationFromEdits);

	calibration_tab.Add(calibration_info.TopPos(8, 24).HSizePos(8, 8));
	calibration_tab.Add(calibration_schema.TopPos(40, 120).HSizePos(8, 8));
	calibration_tab.Add(calib_enabled_lbl.TopPos(170, 20).LeftPos(8, 80));
	calibration_tab.Add(calib_enabled.TopPos(168, 20).LeftPos(96, 20));
	calibration_tab.Add(calib_eye_lbl.TopPos(196, 20).LeftPos(8, 80));
	calibration_tab.Add(calib_eye_dist.TopPos(194, 20).LeftPos(96, 120));
	calibration_tab.Add(calib_outward_lbl.TopPos(222, 20).LeftPos(8, 80));
	calibration_tab.Add(calib_outward_angle.TopPos(220, 20).LeftPos(96, 120));
	calibration_tab.Add(calib_poly_lbl.TopPos(248, 20).LeftPos(8, 80));
	calibration_tab.Add(calib_poly_a.TopPos(246, 20).LeftPos(96, 80));
	calibration_tab.Add(calib_poly_b.TopPos(246, 20).LeftPos(182, 80));
	calibration_tab.Add(calib_poly_c.TopPos(246, 20).LeftPos(268, 80));
	calibration_tab.Add(calib_poly_d.TopPos(246, 20).LeftPos(354, 80));
	calibration_tab.Add(calibration_preview.TopPos(276, 40).HSizePos(8, 8));
	calibration_tab.Add(load_calibration.BottomPos(8, 24).LeftPos(8, 120));
	calibration_tab.Add(export_calibration.BottomPos(8, 24).LeftPos(136, 120));

	SyncEditsFromCalibration();
	UpdatePreview();
}

void StereoCalibrationTool::OnSourceChanged() {
	StopSource();
	source_status.SetLabel(Format("Status: ready (%s)", AsString(source_list.GetValue())));
}

void StereoCalibrationTool::StartSource() {
	int idx = source_list.GetIndex();
	if (idx < 0 || idx >= sources.GetCount())
		return;
	if (sources[idx]->Start())
		source_status.SetLabel(Format("Status: running (%s)", AsString(source_list.GetValue())));
	else
		source_status.SetLabel(Format("Status: failed (%s)", AsString(source_list.GetValue())));
}

void StereoCalibrationTool::StopSource() {
	int idx = source_list.GetIndex();
	if (idx < 0 || idx >= sources.GetCount())
		return;
	sources[idx]->Stop();
	source_status.SetLabel(Format("Status: stopped (%s)", AsString(source_list.GetValue())));
}

void StereoCalibrationTool::ExportCalibration() {
	SyncCalibrationFromEdits();
	UpdatePreview();
	FileSel fs;
	fs.Type("Stereo Calibration", "*.stcal");
	fs.AllFilesType();
	if (!fs.ExecuteSaveAs("Export Stereo Calibration"))
		return;
	StereoCalibrationData data = last_calibration;
	if (!data.is_enabled) {
		data.is_enabled = false;
		data.eye_dist = 0;
		data.outward_angle = 0;
		data.angle_to_pixel = vec4(0,0,0,0);
	}
	if (!SaveCalibrationFile(fs, data)) {
		PromptOK("Failed to export calibration.");
		return;
	}
	PromptOK("Calibration exported.");
}

void StereoCalibrationTool::LoadCalibration() {
	FileSel fs;
	fs.Type("Stereo Calibration", "*.stcal");
	fs.AllFilesType();
	if (!fs.ExecuteOpen("Load Stereo Calibration"))
		return;
	StereoCalibrationData data;
	if (!LoadCalibrationFile(fs, data)) {
		PromptOK("Failed to load calibration.");
		return;
	}
	last_calibration = data;
	SyncEditsFromCalibration();
	UpdatePreview();
	PromptOK("Calibration loaded.");
}

bool StereoCalibrationTool::SaveCalibrationFile(const String& path, const StereoCalibrationData& data) {
	Vector<String> lines;
	lines.Add("enabled=" + String(data.is_enabled ? "1" : "0"));
	lines.Add(Format("eye_dist=%g", (double)data.eye_dist));
	lines.Add(Format("outward_angle=%g", (double)data.outward_angle));
	lines.Add(Format("angle_poly=%g,%g,%g,%g",
		(double)data.angle_to_pixel[0], (double)data.angle_to_pixel[1],
		(double)data.angle_to_pixel[2], (double)data.angle_to_pixel[3]));
	String text = Join(lines, "\n") + "\n";
	return SaveFile(path, text);
}

bool StereoCalibrationTool::LoadCalibrationFile(const String& path, StereoCalibrationData& out) {
	String text = LoadFile(path);
	if (text.IsEmpty())
		return false;
	StereoCalibrationData data;
	Vector<String> lines = Split(text, '\n');
	for (String line : lines) {
		line = TrimBoth(line);
		if (line.IsEmpty() || line[0] == '#')
			continue;
		int eq = line.Find('=');
		if (eq < 0)
			continue;
		String key = TrimBoth(line.Left(eq));
		String val = TrimBoth(line.Mid(eq + 1));
		if (key == "enabled")
			data.is_enabled = atoi(val) != 0;
		else if (key == "eye_dist")
			data.eye_dist = (float)atof(val);
		else if (key == "outward_angle")
			data.outward_angle = (float)atof(val);
		else if (key == "angle_poly") {
			Vector<String> parts = Split(val, ',');
			if (parts.GetCount() >= 4) {
				data.angle_to_pixel[0] = (float)atof(parts[0]);
				data.angle_to_pixel[1] = (float)atof(parts[1]);
				data.angle_to_pixel[2] = (float)atof(parts[2]);
				data.angle_to_pixel[3] = (float)atof(parts[3]);
			}
		}
	}
	out = data;
	return true;
}

void StereoCalibrationTool::SyncCalibrationFromEdits() {
	last_calibration.is_enabled = calib_enabled;
	last_calibration.eye_dist = (float)~calib_eye_dist;
	last_calibration.outward_angle = (float)~calib_outward_angle;
	last_calibration.angle_to_pixel[0] = (float)~calib_poly_a;
	last_calibration.angle_to_pixel[1] = (float)~calib_poly_b;
	last_calibration.angle_to_pixel[2] = (float)~calib_poly_c;
	last_calibration.angle_to_pixel[3] = (float)~calib_poly_d;
	UpdatePreview();
}

void StereoCalibrationTool::SyncEditsFromCalibration() {
	calib_enabled = last_calibration.is_enabled;
	calib_eye_dist <<= (double)last_calibration.eye_dist;
	calib_outward_angle <<= (double)last_calibration.outward_angle;
	calib_poly_a <<= (double)last_calibration.angle_to_pixel[0];
	calib_poly_b <<= (double)last_calibration.angle_to_pixel[1];
	calib_poly_c <<= (double)last_calibration.angle_to_pixel[2];
	calib_poly_d <<= (double)last_calibration.angle_to_pixel[3];
}

void StereoCalibrationTool::UpdatePreview() {
	String s;
	s << "Preview:\n";
	s << "  enabled=" << (last_calibration.is_enabled ? "1" : "0") << "\n";
	s << "  eye_dist=" << last_calibration.eye_dist << "\n";
	s << "  outward_angle=" << last_calibration.outward_angle << "\n";
	s << "  angle_poly=" << last_calibration.angle_to_pixel[0] << ", "
	  << last_calibration.angle_to_pixel[1] << ", "
	  << last_calibration.angle_to_pixel[2] << ", "
	  << last_calibration.angle_to_pixel[3] << "\n";
	calibration_preview.SetLabel(s);
}

String StereoCalibrationTool::GetPersistPath() const {
	return ConfigFile("StereoCalibrationTool.stcal");
}

void StereoCalibrationTool::LoadLastCalibration() {
	StereoCalibrationData data;
	String path = GetPersistPath();
	if (FileExists(path) && LoadCalibrationFile(path, data))
		last_calibration = data;
}

void StereoCalibrationTool::SaveLastCalibration() {
	SyncCalibrationFromEdits();
	SaveCalibrationFile(GetPersistPath(), last_calibration);
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
	bar.Add("Source", [=] { tabs.Set(0); });
	bar.Add("Calibration", [=] { tabs.Set(1); });
}

void StereoCalibrationTool::HelpMenu(Bar& bar) {
	bar.Add("About", [=] {
		PromptOK("Stereo Calibration Tool\n\nWork-in-progress.");
	});
}

END_UPP_NAMESPACE
