#include "VideoRecord.h"

NAMESPACE_UPP

namespace {

bool IsVideoRecordableHost(IDocumentHost* host)
{
	IVideoRenderSource* src = dynamic_cast<IVideoRenderSource*>(host);
	return src && src->CanRecordVideo();
}

IVideoRenderSource* GetVideoRenderSource(IDocumentHost* host)
{
	return dynamic_cast<IVideoRenderSource*>(host);
}

}

String VideoRecordPreferencesPage::ConfigPath()
{
	return ConfigFile("videorecord.bin");
}

static String VideoAlignmentName(int alignment)
{
	switch(alignment) {
	case VideoRecordSettings::ALIGN_CENTER:  return "Center";
	case VideoRecordSettings::ALIGN_FIT:     return "Fit to screen";
	case VideoRecordSettings::ALIGN_STRETCH: return "Stretch to screen";
	case VideoRecordSettings::ALIGN_ZOOM:    return "Zoom to screen";
	default:                                 return "Fit to screen";
	}
}

VideoRecordPane::VideoRecordPane()
{
	Add(title_lbl.LeftPosZ(8, 180).TopPosZ(8, 20));
	title_lbl.SetLabel("Video recording");
	title_lbl.SetFont(StdFont().Bold());

	Add(state_lbl.HSizePosZ(8, 8).TopPosZ(36, 20));
	Add(mode_lbl.HSizePosZ(8, 8).TopPosZ(62, 20));
	Add(path_lbl.HSizePosZ(8, 8).TopPosZ(88, 44));
	Add(rec_lbl.HSizePosZ(8, 8).TopPosZ(136, 20));
	Add(auto_start.LeftPosZ(8, 220).TopPosZ(164, 20));
	auto_start.SetLabel("Start record when running starts");
	auto_start.WhenAction = [this] { WhenAutoStart((bool)auto_start); };
	Add(start_btn.LeftPosZ(8, 100).TopPosZ(190, 24));
	start_btn.SetLabel("Start");
	start_btn.WhenAction = [this] { WhenStart(); };
	Add(stop_btn.LeftPosZ(116, 100).TopPosZ(190, 24));
	stop_btn.SetLabel("Stop");
	stop_btn.WhenAction = [this] { WhenStop(); };
	Add(note_lbl.HSizePosZ(8, 8).VSizePosZ(222, 8));
	note_lbl.SetFrame(InsetFrame());
	note_lbl.SetLabel("Enable the plugin to expose this pane.\n"
	                  "Recording stores PNG frames, then assembles them with ffmpeg on stop.");
}

void VideoRecordPane::SetRunState(const ScriptRunState& state, bool recordable)
{
	this->recordable = recordable;
	String run_state = "Idle";
	if(state.running)
		run_state = state.paused ? "Paused" : "Running";
	state_lbl.SetLabel("State: " + run_state);

	String mode = "None";
	switch(state.mode) {
	case IDocumentHost::RUNMODE_DEBUG:   mode = "Debug";   break;
	case IDocumentHost::RUNMODE_PROFILE: mode = "Profile"; break;
	case IDocumentHost::RUNMODE_RUN:     mode = "Run";     break;
	default: break;
	}
	mode_lbl.SetLabel("Mode: " + mode + (recordable ? "  |  Recordable: yes" : "  |  Recordable: no"));
	path_lbl.SetLabel("Path: " + (state.path.IsEmpty() ? String("<none>") : state.path));
	start_btn.Enable(recordable && !this->recording);
}

void VideoRecordPane::SetRecordingState(bool recording, int frame_count, const String& output_path, const String& status)
{
	this->recording = recording;
	rec_lbl.SetLabel(Format("Recording: %s  |  Frames: %d", recording ? "yes" : "no", frame_count));
	start_btn.Enable(!recording && recordable);
	stop_btn.Enable(recording);
	note_lbl.SetLabel(status + (output_path.IsEmpty() ? String() : "\nOutput: " + output_path));
}

void VideoRecordPane::SetAutoStart(bool enabled)
{
	auto_start = enabled;
}

VideoRecordPreferencesPage::VideoRecordPreferencesPage()
{
	Add(ffmpeg_lbl.LeftPosZ(8, 120).TopPosZ(8, 20));
	ffmpeg_lbl.SetLabel("ffmpeg path:");
	Add(ffmpeg_path.HSizePosZ(136, 8).TopPosZ(8, 20));

	Add(frames_lbl.LeftPosZ(8, 120).TopPosZ(36, 20));
	frames_lbl.SetLabel("Frames dir:");
	Add(frames_dir.HSizePosZ(136, 36).TopPosZ(36, 20));
	Add(browse_frames.RightPosZ(8, 24).TopPosZ(36, 20));
	browse_frames.SetLabel("...");
	browse_frames.WhenAction = [=] {
		FileSel fs;
		String dir = frames_dir.GetData();
		if(!dir.IsEmpty())
			fs.ActiveDir(dir);
		if(fs.ExecuteSelectDir("Select Frames Directory"))
			frames_dir.SetData(fs.Get());
	};

	Add(output_lbl.LeftPosZ(8, 120).TopPosZ(64, 20));
	output_lbl.SetLabel("Output file:");
	Add(output_file.HSizePosZ(136, 36).TopPosZ(64, 20));
	Add(browse_output.RightPosZ(8, 24).TopPosZ(64, 20));
	browse_output.SetLabel("...");
	browse_output.WhenAction = [=] {
		FileSel fs;
		fs.Type("MP4 video", "*.mp4");
		fs.AllFilesType();
		String path = output_file.GetData();
		if(!path.IsEmpty())
			fs.Set(path);
		if(fs.ExecuteSaveAs("Select Output Video File"))
			output_file.SetData(fs.Get());
	};

	Add(fps_lbl.LeftPosZ(8, 120).TopPosZ(92, 20));
	fps_lbl.SetLabel("FPS:");
	Add(fps.LeftPosZ(136, 80).TopPosZ(92, 20));
	fps.MinMax(1, 240);

	Add(width_lbl.LeftPosZ(8, 120).TopPosZ(120, 20));
	width_lbl.SetLabel("Width:");
	Add(width.LeftPosZ(136, 80).TopPosZ(120, 20));
	width.MinMax(64, 8192);

	Add(height_lbl.LeftPosZ(8, 120).TopPosZ(148, 20));
	height_lbl.SetLabel("Height:");
	Add(height.LeftPosZ(136, 80).TopPosZ(148, 20));
	height.MinMax(64, 8192);

	Add(alignment_lbl.LeftPosZ(8, 120).TopPosZ(176, 20));
	alignment_lbl.SetLabel("Alignment:");
	Add(alignment.LeftPosZ(136, 180).TopPosZ(176, 20));
	alignment.Add(VideoRecordSettings::ALIGN_CENTER, "Center");
	alignment.Add(VideoRecordSettings::ALIGN_FIT, "Fit to screen");
	alignment.Add(VideoRecordSettings::ALIGN_STRETCH, "Stretch to screen");
	alignment.Add(VideoRecordSettings::ALIGN_ZOOM, "Zoom to screen");

	Add(keep_frames.LeftPosZ(8, 220).TopPosZ(206, 20));
	keep_frames.SetLabel("Keep frame sequence");
	Add(overwrite.LeftPosZ(8, 220).TopPosZ(232, 20));
	overwrite.SetLabel("Overwrite output");

	Add(note.HSizePosZ(8, 8).VSizePosZ(264, 8));
	note.SetLabel("Video export plugin scaffold.\n"
	              "Settings are stored in a plugin-specific ConfigFile and applied through Preferences.");
	note.SetFrame(InsetFrame());
}

VideoRecordSettings VideoRecordPreferencesPage::ReadSettings() const
{
	VideoRecordSettings cfg;
	cfg.ffmpeg_path = ffmpeg_path.GetData();
	cfg.frames_dir = frames_dir.GetData();
	cfg.output_file = output_file.GetData();
	cfg.fps = max(1, (int)~fps);
	cfg.width = max(64, (int)~width);
	cfg.height = max(64, (int)~height);
	int alignment_index = alignment.GetIndex();
	cfg.alignment = alignment_index >= 0 ? (int)alignment.GetKey(alignment_index) : VideoRecordSettings::ALIGN_FIT;
	if(cfg.alignment < VideoRecordSettings::ALIGN_CENTER || cfg.alignment > VideoRecordSettings::ALIGN_ZOOM)
		cfg.alignment = VideoRecordSettings::ALIGN_FIT;
	cfg.keep_frames = keep_frames;
	cfg.overwrite = overwrite;
	return cfg;
}

void VideoRecordPreferencesPage::WriteSettings(const VideoRecordSettings& cfg)
{
	ffmpeg_path.SetData(cfg.ffmpeg_path);
	frames_dir.SetData(cfg.frames_dir);
	output_file.SetData(cfg.output_file);
	fps <<= cfg.fps;
	width <<= cfg.width;
	height <<= cfg.height;
	int q = alignment.FindKey(cfg.alignment);
	if(q >= 0)
		alignment.SetIndex(q);
	else
		alignment.SetIndex(alignment.FindKey(VideoRecordSettings::ALIGN_FIT));
	keep_frames = cfg.keep_frames;
	overwrite = cfg.overwrite;
}

void VideoRecordPreferencesPage::LoadConfig()
{
	loaded = VideoRecordSettings();
	LoadFromFile(loaded, ConfigPath());
	WriteSettings(loaded);
}

void VideoRecordPreferencesPage::SaveConfig()
{
	loaded = ReadSettings();
	StoreToFile(loaded, ConfigPath());
}

void VideoRecordPreferencesPage::ApplyConfig(IDEContext& ctx)
{
	if(ctx.main_window)
		ctx.main_window->Log("VideoRecord preferences applied.");
}

void VideoRecordPreferencesPage::SetDefaults()
{
	WriteSettings(VideoRecordSettings());
}

bool VideoRecordPreferencesPage::IsModified() const
{
	VideoRecordSettings cfg = ReadSettings();
	StringStream a, b;
	cfg.Serialize(a);
	const_cast<VideoRecordSettings&>(loaded).Serialize(b);
	return a.GetResult() != b.GetResult();
}

VideoRecordSettings VideoRecordPlugin::LoadSettings() const
{
	VideoRecordSettings cfg;
	LoadFromFile(cfg, ConfigFile("videorecord.bin"));
	return cfg;
}

VideoRecordPlugin::VideoRecordPlugin()
{
	pane.WhenStart = [this] { StartRecording(); };
	pane.WhenStop = [this] { StopRecording(true); };
	pane.WhenAutoStart = [this](bool enabled) { SetAutoStartOnRun(enabled); };
}

void VideoRecordPlugin::Init(IPluginContext& context)
{
	ctx = dynamic_cast<IPluginContextGUI*>(&context);
	registry = dynamic_cast<IPluginRegistryGUI*>(&context);
	if(!ctx || !registry)
		return;
	registry->RegisterPreferencesProvider(*this);
	registry->RegisterRunStateListener(*this);
	ctx->RegisterDockPane("plugin.video_record", "Video Record", pane);
	UpdatePane();
}

void VideoRecordPlugin::Shutdown()
{
	StopRecording(false);
	if(ctx)
		ctx->UnregisterDockPane("plugin.video_record");
	ctx = nullptr;
	registry = nullptr;
	last_state = ScriptRunState();
	recordable_active = false;
	output_path.Clear();
	session_dir.Clear();
}

String VideoRecordPlugin::GetPreferencesPageID(int index) const
{
	ASSERT(index == 0);
	return "video_record";
}

String VideoRecordPlugin::GetPreferencesPageCategory(int index) const
{
	ASSERT(index == 0);
	return "Plugins";
}

String VideoRecordPlugin::GetPreferencesPageTitle(int index) const
{
	ASSERT(index == 0);
	return "Video recording";
}

Image VideoRecordPlugin::GetPreferencesPageIcon(int index) const
{
	ASSERT(index == 0);
	return Icons::Profiler();
}

IPluginPreferencesPage& VideoRecordPlugin::GetPreferencesPage(int index)
{
	ASSERT(index == 0);
	return prefs_page;
}

void VideoRecordPlugin::OnRunStateChanged(PythonIDE& ide, const ScriptRunState& state)
{
	IDocumentHost* previous_host = last_state.host;
	bool previous_running = last_state.running;
	last_state = state;
	recordable_active = state.running
		&& !state.paused
		&& (state.mode == IDocumentHost::RUNMODE_RUN || state.mode == IDocumentHost::RUNMODE_DEBUG)
		&& IsVideoRecordableHost(state.host);
	if(recording && (!recordable_active || state.host != previous_host || (!state.running && previous_running)))
		StopRecording(true);
	if(!recording && recordable_active && !previous_running && LoadSettings().auto_start_on_run)
		StartRecording();
	UpdatePane();
	if(recording)
		ide.Log(Format("VideoRecord: state update frames=%d", frame_count));
}

void VideoRecordPlugin::StartRecording()
{
	if(recording || !recordable_active || !last_state.host || !ctx)
		return;
	IVideoRenderSource* src = GetVideoRenderSource(last_state.host);
	if(!src)
		return;

	VideoRecordSettings cfg = LoadSettings();
	capture_interval_ms = max(1, 1000 / max(1, cfg.fps));
	frame_count = 0;

	String root = cfg.frames_dir;
	if(root.IsEmpty())
		root = AppendFileName(GetTempPath(), "scriptide_videorecord");
	Time tm = GetSysTime();
	session_dir = AppendFileName(root, Format("%04d%02d%02d_%02d%02d%02d",
		(int)tm.year, (int)tm.month, (int)tm.day, (int)tm.hour, (int)tm.minute, (int)tm.second));
	RealizeDirectory(session_dir);

	output_path = cfg.output_file;
	if(!IsFullPath(output_path))
		output_path = AppendFileName(last_state.path.IsEmpty() ? GetCurrentDirectory() : GetFileDirectory(last_state.path), output_path);

	recording = true;
	CaptureFrame();
	ScheduleCapture();
	if(ctx)
		ctx->GetIDE().Log("VideoRecord: recording started.");
	UpdatePane();
}

void VideoRecordPlugin::ScheduleCapture()
{
	Upp::KillTimeCallback(this);
	if(recording)
		Upp::SetTimeCallback(-capture_interval_ms, [this] { CaptureFrame(); ScheduleCapture(); }, this);
}

void VideoRecordPlugin::CaptureFrame()
{
	if(!recording || !last_state.host)
		return;
	IVideoRenderSource* src = GetVideoRenderSource(last_state.host);
	if(!src || !src->CanRecordVideo())
		return;

	VideoRecordSettings cfg = LoadSettings();
	Size target(cfg.width, cfg.height);
	Image img = src->CaptureRecordFrame();
	img = ComposeFrame(img, target, cfg.alignment);
	if(img.IsEmpty())
		return;
	String frame_path = AppendFileName(session_dir, Format("frame_%06d.png", frame_count++));
	PNGEncoder().SaveFile(frame_path, img);
	UpdatePane();
}

void VideoRecordPlugin::StopRecording(bool finalize)
{
	Upp::KillTimeCallback(this);
	if(!recording && !finalize)
		return;

	bool had_recording = recording;
	recording = false;

	if(finalize && had_recording && frame_count > 0) {
		VideoRecordSettings cfg = LoadSettings();
		Vector<String> args;
		if(cfg.overwrite)
			args.Add("-y");
		args << "-framerate" << AsString(max(1, cfg.fps))
		     << "-i" << AppendFileName(session_dir, "frame_%06d.png")
		     << "-pix_fmt" << "yuv420p"
		     << output_path;
		String out;
		int code = Sys(~cfg.ffmpeg_path, args, out);
		if(ctx)
			ctx->GetIDE().Log(code == 0 ? "VideoRecord: ffmpeg export finished." : "VideoRecord: ffmpeg export failed.");
		if(ctx && !out.IsEmpty())
			ctx->GetIDE().Log(out);
		if(!cfg.keep_frames)
			DeleteFolderDeep(session_dir);
	}

	if(!had_recording) {
		output_path.Clear();
		session_dir.Clear();
		frame_count = 0;
	}
	UpdatePane();
}

void VideoRecordPlugin::UpdatePane()
{
	pane.SetRunState(last_state, recordable_active);
	String status = recording ? "Recording active." : "Recording idle.";
	if(!session_dir.IsEmpty())
		status << "\nFrames: " << session_dir;
	VideoRecordSettings cfg = LoadSettings();
	pane.SetAutoStart(cfg.auto_start_on_run);
	status << "\nAlignment: " << VideoAlignmentName(cfg.alignment);
	pane.SetRecordingState(recording, frame_count, output_path, status);
}

void VideoRecordPlugin::SetAutoStartOnRun(bool enabled)
{
	VideoRecordSettings cfg = LoadSettings();
	if(cfg.auto_start_on_run == enabled)
		return;
	cfg.auto_start_on_run = enabled;
	StoreToFile(cfg, ConfigFile("videorecord.bin"));
	UpdatePane();
	if(ctx)
		ctx->GetIDE().Log(String("VideoRecord: auto-start on run ") + (enabled ? "enabled." : "disabled."));
}

Image VideoRecordPlugin::ComposeFrame(const Image& src, Size target, int alignment) const
{
	if(src.IsEmpty())
		return Image();
	if(target.cx <= 0 || target.cy <= 0)
		return src;
	if(alignment == VideoRecordSettings::ALIGN_STRETCH)
		return Rescale(src, target);

	Size src_sz = src.GetSize();
	ImageDraw iw(target);
	iw.DrawRect(target, Black());

	if(alignment == VideoRecordSettings::ALIGN_CENTER) {
		Point p((target.cx - src_sz.cx) / 2, (target.cy - src_sz.cy) / 2);
		iw.DrawImage(p.x, p.y, src);
		return iw;
	}

	if(src_sz.cx <= 0 || src_sz.cy <= 0)
		return iw;

	double sx = (double)target.cx / (double)src_sz.cx;
	double sy = (double)target.cy / (double)src_sz.cy;
	double scale = alignment == VideoRecordSettings::ALIGN_ZOOM ? max(sx, sy) : min(sx, sy);
	int scaled_cx = max(1, (int)floor(src_sz.cx * scale + 0.5));
	int scaled_cy = max(1, (int)floor(src_sz.cy * scale + 0.5));
	Image scaled = Rescale(src, Size(scaled_cx, scaled_cy));
	Point p((target.cx - scaled_cx) / 2, (target.cy - scaled_cy) / 2);
	iw.DrawImage(p.x, p.y, scaled);
	return iw;
}

END_UPP_NAMESPACE
