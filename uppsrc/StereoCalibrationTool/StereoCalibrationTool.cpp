#include "StereoCalibrationTool.h"

NAMESPACE_UPP

static bool SplitStereoImage(const Image& src, Image& left, Image& right) {
	Size sz = src.GetSize();
	if (sz.cx < 2 || sz.cy <= 0)
		return false;
	int half = sz.cx / 2;
	if (half <= 0)
		return false;
	const RGBA* src_data = ~src;
	if (!src_data)
		return false;
	ImageBuffer lb(half, sz.cy);
	ImageBuffer rb(half, sz.cy);
	for (int y = 0; y < sz.cy; y++) {
		const RGBA* row = src_data + y * sz.cx;
		memcpy(lb[y], row, half * sizeof(RGBA));
		memcpy(rb[y], row + half, half * sizeof(RGBA));
	}
	left = lb;
	right = rb;
	return true;
}

static Image CopyFrameImage(const VisualFrame& frame) {
	if (frame.format != GEOM_EVENT_CAM_RGBA8 || frame.width <= 0 || frame.height <= 0 || !frame.data)
		return Image();
	ImageBuffer ib(frame.width, frame.height);
	const RGBA* src = (const RGBA*)frame.data;
	for (int y = 0; y < frame.height; y++)
		memcpy(ib[y], src + y * frame.width, frame.width * sizeof(RGBA));
	return ib;
}

static Image ConvertRgb24ToImage(const byte* data, int width, int height) {
	if (!data || width <= 0 || height <= 0)
		return Image();
	ImageBuffer ib(width, height);
	const byte* src = data;
	for (int y = 0; y < height; y++) {
		RGBA* dst = ib[y];
		for (int x = 0; x < width; x++) {
			dst[x].r = *src++;
			dst[x].g = *src++;
			dst[x].b = *src++;
			dst[x].a = 255;
		}
	}
	return ib;
}

static Image ConvertYuyvToImage(const byte* data, int width, int height) {
	if (!data || width <= 0 || height <= 0)
		return Image();
	ImageBuffer ib(width, height);
	const byte* src = data;
	for (int y = 0; y < height; y++) {
		RGBA* dst = ib[y];
		for (int x = 0; x < width; x += 2) {
			int y0 = src[0];
			int u = src[1] - 128;
			int y1 = src[2];
			int v = src[3] - 128;
			src += 4;
			auto conv = [&](int yy) -> RGBA {
				int c = yy - 16;
				int d = u;
				int e = v;
				int r = (298 * c + 409 * e + 128) >> 8;
				int g = (298 * c - 100 * d - 208 * e + 128) >> 8;
				int b = (298 * c + 516 * d + 128) >> 8;
				RGBA out;
				out.r = (byte)Clamp(r, 0, 255);
				out.g = (byte)Clamp(g, 0, 255);
				out.b = (byte)Clamp(b, 0, 255);
				out.a = 255;
				return out;
			};
			dst[x] = conv(y0);
			if (x + 1 < width)
				dst[x + 1] = conv(y1);
		}
	}
	return ib;
}
bool StereoCalibrationTool::HmdStereoSource::Start() {
	if (running)
		return true;
	if (!sys.Initialise())
		return false;
	cam.Create();
	if (!cam->Open()) {
		cam.Clear();
		sys.Uninitialise();
		return false;
	}
	running = true;
	return true;
}

void StereoCalibrationTool::HmdStereoSource::Stop() {
	if (cam)
		cam->Close();
	cam.Clear();
	if (running)
		sys.Uninitialise();
	running = false;
	last_left = Image();
	last_right = Image();
}

bool StereoCalibrationTool::HmdStereoSource::ReadFrame(VisualFrame& left, VisualFrame& right) {
	if (!cam || !cam->IsOpen())
		return false;
	Vector<HMD::CameraFrame> frames;
	cam->PopFrames(frames);
	if (frames.IsEmpty())
		return false;
	const HMD::CameraFrame& f = frames.Top();
	if (!SplitStereoImage(f.img, last_left, last_right))
		return false;
	last_is_bright = f.is_bright;

	left.timestamp_us = usecs();
	left.format = GEOM_EVENT_CAM_RGBA8;
	left.width = last_left.GetWidth();
	left.height = last_left.GetHeight();
	left.stride = left.width * (int)sizeof(RGBA);
	left.eye = 0;
	left.data = (const byte*)~last_left;
	left.data_bytes = last_left.GetLength() * (int)sizeof(RGBA);
	left.flags = last_is_bright ? VIS_FRAME_BRIGHT : VIS_FRAME_DARK;

	right.timestamp_us = left.timestamp_us;
	right.format = GEOM_EVENT_CAM_RGBA8;
	right.width = last_right.GetWidth();
	right.height = last_right.GetHeight();
	right.stride = right.width * (int)sizeof(RGBA);
	right.eye = 1;
	right.data = (const byte*)~last_right;
	right.data_bytes = last_right.GetLength() * (int)sizeof(RGBA);
	right.flags = left.flags;

	return true;
}

#ifdef flagLINUX
bool StereoCalibrationTool::UsbStereoSource::Start() {
	if (running)
		return true;
	if (device_path.IsEmpty())
		device_path = "/dev/video0";
	std::list<unsigned int> formats;
	formats.push_back(V4L2_PIX_FMT_RGB24);
	formats.push_back(V4L2_PIX_FMT_YUYV);
	V4L2DeviceParameters params(device_path.Begin(), formats, 2560, 720, 30, 0);
	capture = V4l2Capture::create(params, V4l2Access::IOTYPE_MMAP);
	if (!capture) {
		V4L2DeviceParameters fallback(device_path.Begin(), formats, 1280, 720, 30, 0);
		capture = V4l2Capture::create(fallback, V4l2Access::IOTYPE_MMAP);
	}
	if (!capture)
		return false;
	if (!capture->start())
		return false;

	width = (int)capture->getWidth();
	height = (int)capture->getHeight();
	pixfmt = (int)capture->getFormat();
	raw.SetCount((int)capture->getBufferSize());
	running = true;
	return true;
}

void StereoCalibrationTool::UsbStereoSource::Stop() {
	if (capture) {
		capture->stop();
		capture.Clear();
	}
	raw.Clear();
	running = false;
	last_left = Image();
	last_right = Image();
}

bool StereoCalibrationTool::UsbStereoSource::ReadFrame(VisualFrame& left, VisualFrame& right) {
	if (!capture)
		return false;

	timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if (capture->isReadable(&tv) <= 0)
		return false;

	size_t read_bytes = capture->read((char*)raw.Begin(), raw.GetCount());
	if (read_bytes == 0)
		return false;
	Image frame;
	if (pixfmt == V4L2_PIX_FMT_RGB24)
		frame = ConvertRgb24ToImage(raw.Begin(), width, height);
	else if (pixfmt == V4L2_PIX_FMT_YUYV)
		frame = ConvertYuyvToImage(raw.Begin(), width, height);

	if (frame.IsEmpty())
		return false;
	if (!SplitStereoImage(frame, last_left, last_right))
		return false;

	left.timestamp_us = usecs();
	left.format = GEOM_EVENT_CAM_RGBA8;
	left.width = last_left.GetWidth();
	left.height = last_left.GetHeight();
	left.stride = left.width * (int)sizeof(RGBA);
	left.eye = 0;
	left.data = (const byte*)~last_left;
	left.data_bytes = last_left.GetLength() * (int)sizeof(RGBA);
	left.flags = VIS_FRAME_BRIGHT;

	right.timestamp_us = left.timestamp_us;
	right.format = GEOM_EVENT_CAM_RGBA8;
	right.width = last_right.GetWidth();
	right.height = last_right.GetHeight();
	right.stride = right.width * (int)sizeof(RGBA);
	right.eye = 1;
	right.data = (const byte*)~last_right;
	right.data_bytes = last_right.GetLength() * (int)sizeof(RGBA);
	right.flags = VIS_FRAME_BRIGHT;

	return true;
}
#else
bool StereoCalibrationTool::UsbStereoSource::Start() { return false; }
void StereoCalibrationTool::UsbStereoSource::Stop() { running = false; }
bool StereoCalibrationTool::UsbStereoSource::ReadFrame(VisualFrame&, VisualFrame&) { return false; }
#endif

StereoCalibrationTool::StereoCalibrationTool() {
	Title("Stereo Calibration Tool");
	Sizeable().Zoomable();

	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));
	AddFrame(status);

	source_info.SetLabel("Source setup goes here (live HMD, USB stereo, or video file).");
	calibration_info.SetLabel("Calibration workflow goes here (checkerboard/aruco capture).");
	calibration_schema.SetLabel("Output schema (.stcal):\n"
		"  enabled=0|1\n"
		"  eye_dist=<float>\n"
		"  outward_angle=<float>\n"
		"  angle_poly=a,b,c,d\n");
	calibration_preview.SetLabel("Preview: (no calibration loaded)");

	BuildLayout();
	LoadLastCalibration();
	LoadState();
	SyncEditsFromCalibration();
	Data();
}

StereoCalibrationTool::~StereoCalibrationTool() {
	SaveLastCalibration();
	SaveState();
}

void StereoCalibrationTool::BuildLayout() {
	hsplitter.Horz(left, right);
	hsplitter.SetPos(2000);
	vsplitter.Vert(hsplitter, bottom_tabs);
	vsplitter.SetPos(7000);
	Add(vsplitter.SizePos());

	BuildLeftPanel();
	BuildBottomTabs();
	right.Add(preview.SizePos());
	status.Set("Status: idle");
}

void StereoCalibrationTool::BuildLeftPanel() {
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
	live_view <<= THISBACK(LiveView);
	capture_frame <<= THISBACK(CaptureFrame);

	source_status.SetLabel("Status: idle");
	sep_source.SetLabel("Source");
	sep_mode.SetLabel("Mode");
	sep_calib.SetLabel("Calibration");
	sep_diag.SetLabel("Diagnostics");
	mode_lbl.SetLabel("Match mode");
	mode_list.Add(0, "Point pairs");
	mode_list.Add(1, "Line pairs");
	mode_list.SetIndex(0);

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

	int y = 8;
	left.Add(sep_source.TopPos(y, 18).HSizePos(8, 8));
	y += 24;
	left.Add(source_list.TopPos(y, 24).HSizePos(8, 8));
	y += 32;
	left.Add(start_source.TopPos(y, 24).LeftPos(8, 80));
	left.Add(stop_source.TopPos(y, 24).LeftPos(96, 80));
	left.Add(live_view.TopPos(y, 24).LeftPos(184, 80));
	left.Add(capture_frame.TopPos(y, 24).LeftPos(272, 80));
	y += 32;
	left.Add(source_status.TopPos(y, 20).HSizePos(8, 8));
	y += 28;

	left.Add(sep_mode.TopPos(y, 18).HSizePos(8, 8));
	y += 24;
	left.Add(mode_lbl.TopPos(y, 20).LeftPos(8, 80));
	left.Add(mode_list.TopPos(y, 20).LeftPos(96, 160));
	y += 28;

	left.Add(sep_calib.TopPos(y, 18).HSizePos(8, 8));
	y += 24;
	left.Add(calib_enabled_lbl.TopPos(y, 20).LeftPos(8, 80));
	left.Add(calib_enabled.TopPos(y, 20).LeftPos(96, 20));
	y += 24;
	left.Add(calib_eye_lbl.TopPos(y, 20).LeftPos(8, 80));
	left.Add(calib_eye_dist.TopPos(y, 20).LeftPos(96, 120));
	y += 24;
	left.Add(calib_outward_lbl.TopPos(y, 20).LeftPos(8, 80));
	left.Add(calib_outward_angle.TopPos(y, 20).LeftPos(96, 120));
	y += 24;
	left.Add(calib_poly_lbl.TopPos(y, 20).LeftPos(8, 80));
	left.Add(calib_poly_a.TopPos(y, 20).LeftPos(96, 70));
	left.Add(calib_poly_b.TopPos(y, 20).LeftPos(170, 70));
	left.Add(calib_poly_c.TopPos(y, 20).LeftPos(244, 70));
	left.Add(calib_poly_d.TopPos(y, 20).LeftPos(318, 70));
	y += 28;
	left.Add(load_calibration.TopPos(y, 24).LeftPos(8, 120));
	left.Add(export_calibration.TopPos(y, 24).LeftPos(136, 120));
	y += 32;

	left.Add(sep_diag.TopPos(y, 18).HSizePos(8, 8));
	y += 24;
	left.Add(calibration_schema.TopPos(y, 110).HSizePos(8, 8));
	y += 118;
	left.Add(calibration_preview.TopPos(y, 60).HSizePos(8, 8));

	sources.Clear();
	sources.Add(MakeOne<HmdStereoSource>());
	sources.Add(MakeOne<UsbStereoSource>());
	sources.Add(MakeOne<VideoStereoSource>());
}

void StereoCalibrationTool::BuildBottomTabs() {
	captures_list.AddColumn("Time");
	captures_list.AddColumn("Source");
	captures_list.AddColumn("Samples");
	captures_list.WhenCursor = THISBACK(DataCapturedFrame);
	matches_list.AddColumn("Left");
	matches_list.AddColumn("Right");
	report_text.SetReadOnly();
	report_text <<= "Solve report and .stcal preview will appear here.";

	captures_split.Horz(captures_list, matches_list);
	captures_split.SetPos(4000);

	bottom_tabs.Add(captures_split.SizePos(), "Captured Frames");
	bottom_tabs.Add(report_text.SizePos(), "Report");
}

void StereoCalibrationTool::Data() {
	UpdatePreview();
	DataCapturedFrame();
}

void StereoCalibrationTool::DataCapturedFrame() {
	if (preview.live)
		return;
	if (pending_capture_row >= 0 && captures_list.GetCount() > pending_capture_row) {
		int set_row = pending_capture_row;
		pending_capture_row = -1;
		captures_list.SetCursor(set_row);
		return;
	}
	int row = captures_list.GetCursor();
	if (row < 0) {
		preview.SetOverlay("No capture selected");
		return;
	}
	if (row >= captured_frames.GetCount()) {
		preview.SetOverlay("Capture data unavailable");
		return;
	}
	const CapturedFrame& frame = captured_frames[row];
	matches_list.Clear();
	for (const MatchPair& pair : frame.matches)
		matches_list.Add(pair.left, pair.right);
	preview.SetImages(frame.left_img, frame.right_img);
	String time = AsString(captures_list.Get(row, 0));
	String source = AsString(captures_list.Get(row, 1));
	Value samples = captures_list.Get(row, 2);
	preview.SetOverlay(Format("Capture %s (%s), samples %s", time, source, samples));
	status.Set(Format("Selected capture %s from %s", time, source));
}

void StereoCalibrationTool::OnSourceChanged() {
	StopSource();
	String name = AsString(source_list.GetValue());
	source_status.SetLabel(Format("Status: ready (%s)", name));
	status.Set(Format("Source ready: %s", name));
}

void StereoCalibrationTool::StartSource() {
	int idx = source_list.GetIndex();
	if (idx < 0 || idx >= sources.GetCount())
		return;
	String name = AsString(source_list.GetValue());
	if (sources[idx]->Start()) {
		source_status.SetLabel(Format("Status: running (%s)", name));
		status.Set(Format("Source running: %s", name));
	} else {
		source_status.SetLabel(Format("Status: failed (%s)", name));
		status.Set(Format("Source failed: %s", name));
	}
}

void StereoCalibrationTool::StopSource() {
	int idx = source_list.GetIndex();
	if (idx < 0 || idx >= sources.GetCount())
		return;
	sources[idx]->Stop();
	String name = AsString(source_list.GetValue());
	source_status.SetLabel(Format("Status: stopped (%s)", name));
	status.Set(Format("Source stopped: %s", name));
}

void StereoCalibrationTool::LiveView() {
	preview.SetLive(true);
	preview.SetImages(Image(), Image());
	preview.SetOverlay("Live view");
	status.Set("Live view enabled.");
}

void StereoCalibrationTool::CaptureFrame() {
	preview.SetLive(false);
	String name = AsString(source_list.GetValue());
	Time now = GetSysTime();
	captures_list.Add(Format("%02d:%02d:%02d", now.hour, now.minute, now.second), name, 0);
	captures_list.SetCursor(captures_list.GetCount() - 1);
	CapturedFrame frame;
	frame.time = now;
	frame.source = name;
	int idx = source_list.GetIndex();
	if (idx >= 0 && idx < sources.GetCount()) {
		VisualFrame lf;
		VisualFrame rf;
		if (sources[idx]->ReadFrame(lf, rf)) {
			frame.left_img = CopyFrameImage(lf);
			frame.right_img = CopyFrameImage(rf);
		}
	}
	captured_frames.Add(pick(frame));
	bottom_tabs.Set(0);
	DataCapturedFrame();
	status.Set("Captured snapshot.");
}

void StereoCalibrationTool::ExportCalibration() {
	SyncCalibrationFromEdits();
	Data();
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
	Data();
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
	Data();
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

String StereoCalibrationTool::GetStatePath() const {
	return ConfigFile("StereoCalibrationTool.state");
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

void StereoCalibrationTool::LoadState() {
	String text = LoadFile(GetStatePath());
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
		if (key == "capture_row")
			pending_capture_row = atoi(val);
	}
}

void StereoCalibrationTool::SaveState() {
	int row = captures_list.GetCursor();
	Vector<String> lines;
	lines.Add(Format("capture_row=%d", row));
	SaveFile(GetStatePath(), Join(lines, "\n") + "\n");
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
	bar.Add("Captured Frames", [=] { bottom_tabs.Set(0); });
	bar.Add("Report", [=] { bottom_tabs.Set(1); });
}

void StereoCalibrationTool::HelpMenu(Bar& bar) {
	bar.Add("About", [=] {
		PromptOK("Stereo Calibration Tool\n\nWork-in-progress.");
	});
}

END_UPP_NAMESPACE
