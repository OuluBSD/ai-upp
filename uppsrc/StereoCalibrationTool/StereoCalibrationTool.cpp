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
	if (!IsNull(frame.img))
		return frame.img;
	if (frame.format != GEOM_EVENT_CAM_RGBA8 || frame.width <= 0 || frame.height <= 0 || !frame.data)
		return Image();
	ImageBuffer ib(frame.width, frame.height);
	const RGBA* src = (const RGBA*)frame.data;
	for (int y = 0; y < frame.height; y++)
		memcpy(ib[y], src + y * frame.width, frame.width * sizeof(RGBA));
	return ib;
}

static bool IsFrameNonBlack(const Image& img) {
	if (img.IsEmpty())
		return false;
	const RGBA* src = ~img;
	if (!src)
		return false;
	int count = img.GetLength();
	if (count <= 0)
		return false;
	int step = max(1, count / 2048);
	for (int i = 0; i < count; i += step) {
		const RGBA& c = src[i];
		if (c.r > 8 || c.g > 8 || c.b > 8)
			return true;
	}
	return false;
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

static Image ConvertMjpegToImage(const byte* data, int bytes) {
	if (!data || bytes <= 0)
		return Image();
	String s((const char*)data, bytes);
	return StreamRaster::LoadStringAny(s);
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
	left.img = last_left;
	left.flags = last_is_bright ? VIS_FRAME_BRIGHT : VIS_FRAME_DARK;

	right.timestamp_us = left.timestamp_us;
	right.format = GEOM_EVENT_CAM_RGBA8;
	right.width = last_right.GetWidth();
	right.height = last_right.GetHeight();
	right.stride = right.width * (int)sizeof(RGBA);
	right.eye = 1;
	right.data = (const byte*)~last_right;
	right.data_bytes = last_right.GetLength() * (int)sizeof(RGBA);
	right.img = last_right;
	right.flags = left.flags;

	return true;
}

#ifdef flagLINUX
bool StereoCalibrationTool::UsbStereoSource::Start() {
	if (running)
		return true;
	if (capture)
		capture.Clear();
	if (device_path.IsEmpty())
		device_path = "/dev/video0";
	std::list<unsigned int> formats;
	formats.push_back(V4L2_PIX_FMT_RGB24);
	formats.push_back(V4L2_PIX_FMT_YUYV);
	formats.push_back(V4L2_PIX_FMT_MJPEG);
	V4L2DeviceParameters params(device_path.Begin(), formats, 2560, 720, 30, 0);
	capture = V4l2Capture::create(params, V4l2Access::IOTYPE_MMAP);
	if (!capture) {
		V4L2DeviceParameters fallback(device_path.Begin(), formats, 1280, 720, 30, 0);
		capture = V4l2Capture::create(fallback, V4l2Access::IOTYPE_MMAP);
	}
	if (!capture)
		return false;
	if (!capture->start()) {
		capture.Clear();
		return false;
	}

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
	tv.tv_sec = 0;
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
	else if (pixfmt == V4L2_PIX_FMT_MJPEG)
		frame = ConvertMjpegToImage(raw.Begin(), (int)read_bytes);

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
	left.img = last_left;
	left.flags = VIS_FRAME_BRIGHT;

	right.timestamp_us = left.timestamp_us;
	right.format = GEOM_EVENT_CAM_RGBA8;
	right.width = last_right.GetWidth();
	right.height = last_right.GetHeight();
	right.stride = right.width * (int)sizeof(RGBA);
	right.eye = 1;
	right.data = (const byte*)~last_right;
	right.data_bytes = last_right.GetLength() * (int)sizeof(RGBA);
	right.img = last_right;
	right.flags = VIS_FRAME_BRIGHT;

	return true;
}
#else
bool StereoCalibrationTool::UsbStereoSource::Start() { return false; }
void StereoCalibrationTool::UsbStereoSource::Stop() { running = false; }
bool StereoCalibrationTool::UsbStereoSource::ReadFrame(VisualFrame&, VisualFrame&) { return false; }
#endif

void StereoCalibrationTool::PreviewCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	w.DrawRect(sz, Black());
	if (has_images) {
		int half = max(1, sz.cx / 2);
		if (!IsNull(left_img))
			w.DrawImage(0, 0, half, sz.cy, left_img);
		if (!IsNull(right_img))
			w.DrawImage(half, 0, sz.cx - half, sz.cy, right_img);
		
		// Draw matches
		for (int i = 0; i < matches.GetCount(); i++) {
			const MatchPair& m = matches[i];
			if (!IsNull(m.left)) {
				Point p(int(m.left.x * half), int(m.left.y * sz.cy));
				w.DrawEllipse(p.x - 3, p.y - 3, 6, 6, Green());
				w.DrawText(p.x + 5, p.y + 5, AsString(i), Arial(12), Green());
			}
			if (!IsNull(m.right)) {
				Point p(int(half + m.right.x * (sz.cx - half)), int(m.right.y * sz.cy));
				w.DrawEllipse(p.x - 3, p.y - 3, 6, 6, Green());
				w.DrawText(p.x + 5, p.y + 5, AsString(i), Arial(12), Green());
			}
		}
		
		// Draw pending point
		if (!IsNull(pending_left)) {
			Point p(int(pending_left.x * half), int(pending_left.y * sz.cy));
			w.DrawEllipse(p.x - 3, p.y - 3, 6, 6, Yellow());
		}
	}
	String title = live ? "Live Preview" : "Captured Snapshot";
	w.DrawText(10, 10, title, Arial(18).Bold(), White());
	if (!overlay.IsEmpty())
		w.DrawText(10, 34, overlay, Arial(12), White());
}

void StereoCalibrationTool::PreviewCtrl::LeftDown(Point p, dword flags) {
	Size sz = GetSize();
	if (sz.cx <= 0 || sz.cy <= 0) return;
	int half = sz.cx / 2;
	if (p.x < half) {
		Pointf img_p(float(p.x) / half, float(p.y) / sz.cy);
		WhenClick(img_p, 0);
	}
	else {
		Pointf img_p(float(p.x - half) / (sz.cx - half), float(p.y) / sz.cy);
		WhenClick(img_p, 1);
	}
}

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

	preview.WhenClick = [=](Pointf p, int eye) {
		if (preview.live) return;
		int row = captures_list.GetCursor();
		if (row < 0 || row >= captured_frames.GetCount()) return;
		CapturedFrame& frame = captured_frames[row];
		
		if (eye == 0) {
			preview.SetPendingLeft(p);
			status.Set(Format("Left point selected at %.3f, %.3f. Select matching right point.", p.x, p.y));
		}
		else if (eye == 1) {
			if (IsNull(preview.pending_left)) {
				status.Set("Select left point first.");
				return;
			}
			MatchPair& m = frame.matches.Add();
			m.left = preview.pending_left;
			m.right = p;
			m.left_text = Format("%.3f, %.3f", m.left.x, m.left.y);
			m.right_text = Format("%.3f, %.3f", m.right.x, m.right.y);
			preview.SetPendingLeft(Null);
			preview.SetMatches(frame.matches);
			DataCapturedFrame();
			status.Set("Match pair added.");
		}
	};

	BuildLayout();
	LoadLastCalibration();
	LoadState();
	SyncEditsFromCalibration();
	Data();
	
	tc.Set(-1000/60, [=] { Sync(); });
}

void StereoCalibrationTool::Sync() {
	if (!preview.live) return;
	
	int idx = source_list.GetIndex();
	if (idx >= 0 && idx < sources.GetCount()) {
		if (sources[idx]->IsRunning()) {
			VisualFrame lf, rf;
			if (sources[idx]->ReadFrame(lf, rf)) {
				Image left_img = CopyFrameImage(lf);
				Image right_img = CopyFrameImage(rf);
				if (!IsNull(left_img) || !IsNull(right_img)) {
					preview.SetImages(left_img, right_img);
					preview.SetOverlay("Live view");
				}
			}
		}
	}
}

StereoCalibrationTool::~StereoCalibrationTool() {
	usb_test_cb.Kill();
	hmd_test_cb.Kill();
	StopSource();
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
	clear_matches.SetLabel("Clear matches");
	start_source <<= THISBACK(StartSource);
	stop_source <<= THISBACK(StopSource);
	live_view <<= THISBACK(LiveView);
	capture_frame <<= THISBACK(CaptureFrame);
	clear_matches.WhenAction = [=] {
		int row = captures_list.GetCursor();
		if (row >= 0 && row < captured_frames.GetCount()) {
			captured_frames[row].matches.Clear();
			DataCapturedFrame();
			status.Set("Matches cleared.");
		}
	};

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
	y += 28;
	left.Add(clear_matches.TopPos(y, 24).LeftPos(272, 100));
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
		matches_list.Add(pair.left_text, pair.right_text);
	preview.SetImages(frame.left_img, frame.right_img);
	preview.SetMatches(frame.matches);
	String time = AsString(captures_list.Get(row, 0));
	String source = AsString(captures_list.Get(row, 1));
	int samples = frame.matches.GetCount();
	captures_list.Set(row, 2, samples);
	preview.SetOverlay(Format("Capture %s (%s), samples %d", time, source, samples));
	status.Set(Format("Selected capture %s from %s", time, source));
}

void StereoCalibrationTool::EnableUsbTest(const String& dev, int timeout_ms) {
	usb_test_enabled = true;
	usb_test_device = dev;
	if (timeout_ms > 0)
		usb_test_timeout_ms = timeout_ms;
	PostCallback(THISBACK(StartUsbTest));
}

void StereoCalibrationTool::StartUsbTest() {
	if (!usb_test_enabled || usb_test_active)
		return;
	usb_test_active = true;
	usb_test_start_us = usecs();
	usb_test_last_start_us = 0;
	usb_test_attempts = 0;
	usb_test_cb.Set(-100, THISBACK(RunUsbTest));
	status.Set("USB test: starting...");
}

void StereoCalibrationTool::RunUsbTest() {
	if (!usb_test_active)
		return;
	int64 elapsed_ms = (usecs() - usb_test_start_us) / 1000;
	if (elapsed_ms > usb_test_timeout_ms) {
		usb_test_active = false;
		usb_test_cb.Kill();
		status.Set("USB test: timeout waiting for frame.");
		StopSource();
		Exit(1);
		return;
	}

	if (source_list.GetIndex() != 1) {
		source_list.SetIndex(1);
		OnSourceChanged();
	}
	if (sources.GetCount() <= 1)
		return;
	UsbStereoSource* usb = dynamic_cast<UsbStereoSource*>(~sources[1]);
	if (!usb)
		return;
	if (usb && !usb_test_device.IsEmpty())
		usb->device_path = usb_test_device;
	if (!usb->IsRunning()) {
		int64 now_us = usecs();
		if (usb_test_last_start_us == 0 || now_us - usb_test_last_start_us > 500000) {
			usb_test_last_start_us = now_us;
			StartSource();
			status.Set(Format("USB test: opening (%d)...", ++usb_test_attempts));
		}
		return;
	}

	VisualFrame lf, rf;
	if (!usb->ReadFrame(lf, rf)) {
		status.Set(Format("USB test: waiting for frame (%d)...", ++usb_test_attempts));
		return;
	}

	Image left_img = CopyFrameImage(lf);
	Image right_img = CopyFrameImage(rf);
	if (!IsFrameNonBlack(left_img) && !IsFrameNonBlack(right_img)) {
		status.Set(Format("USB test: black frame (%d), retrying...", ++usb_test_attempts));
		return;
	}

	Time now = GetSysTime();
	captures_list.Add(Format("%02d:%02d:%02d", now.hour, now.minute, now.second), usb->GetName(), 1);
	captures_list.SetCursor(captures_list.GetCount() - 1);
	CapturedFrame frame;
	frame.time = now;
	frame.source = usb->GetName();
	frame.samples = 1;
	frame.left_img = left_img;
	frame.right_img = right_img;
	captured_frames.Add(pick(frame));
	preview.SetLive(false);
	bottom_tabs.Set(0);
	DataCapturedFrame();
	status.Set("USB test: captured frame OK.");
	usb_test_active = false;
	usb_test_cb.Kill();
	StopSource();
	Exit(0);
}

void StereoCalibrationTool::EnableHmdTest(int timeout_ms) {
	hmd_test_enabled = true;
	if (timeout_ms > 0)
		hmd_test_timeout_ms = timeout_ms;
	PostCallback(THISBACK(StartHmdTest));
}

void StereoCalibrationTool::StartHmdTest() {
	if (!hmd_test_enabled || hmd_test_active)
		return;
	hmd_test_active = true;
	hmd_test_start_us = usecs();
	hmd_test_last_start_us = 0;
	hmd_test_attempts = 0;
	hmd_test_cb.Set(-100, THISBACK(RunHmdTest));
	status.Set("HMD test: starting...");
}

void StereoCalibrationTool::RunHmdTest() {
	if (!hmd_test_active)
		return;
	int64 elapsed_ms = (usecs() - hmd_test_start_us) / 1000;
	if (elapsed_ms > hmd_test_timeout_ms) {
		hmd_test_active = false;
		hmd_test_cb.Kill();
		status.Set("HMD test: timeout waiting for frame.");
		StopSource();
		Exit(1);
		return;
	}

	if (source_list.GetIndex() != 0) {
		source_list.SetIndex(0);
		OnSourceChanged();
	}
	if (sources.GetCount() <= 0)
		return;
	HmdStereoSource* hmd = dynamic_cast<HmdStereoSource*>(~sources[0]);
	if (!hmd)
		return;
	if (!hmd->IsRunning()) {
		int64 now_us = usecs();
		if (hmd_test_last_start_us == 0 || now_us - hmd_test_last_start_us > 500000) {
			hmd_test_last_start_us = now_us;
			StartSource();
			status.Set(Format("HMD test: opening (%d)...", ++hmd_test_attempts));
		}
		return;
	}

	VisualFrame lf, rf;
	if (!hmd->ReadFrame(lf, rf)) {
		status.Set(Format("HMD test: waiting for frame (%d)...", ++hmd_test_attempts));
		return;
	}

	Image left_img = CopyFrameImage(lf);
	Image right_img = CopyFrameImage(rf);
	if (!IsFrameNonBlack(left_img) && !IsFrameNonBlack(right_img)) {
		status.Set(Format("HMD test: black frame (%d), retrying...", ++hmd_test_attempts));
		return;
	}

	Time now = GetSysTime();
	captures_list.Add(Format("%02d:%02d:%02d", now.hour, now.minute, now.second), hmd->GetName(), 1);
	captures_list.SetCursor(captures_list.GetCount() - 1);
	CapturedFrame frame;
	frame.time = now;
	frame.source = hmd->GetName();
	frame.samples = 1;
	frame.left_img = left_img;
	frame.right_img = right_img;
	captured_frames.Add(pick(frame));
	preview.SetLive(false);
	bottom_tabs.Set(0);
	DataCapturedFrame();
	status.Set("HMD test: captured frame OK.");
	hmd_test_active = false;
	hmd_test_cb.Kill();
	StopSource();
	Exit(0);
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
	preview.SetMatches(Vector<MatchPair>());
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
