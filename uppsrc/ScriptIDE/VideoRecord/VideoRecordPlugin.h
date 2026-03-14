#ifndef _VideoRecord_VideoRecordPlugin_h_
#define _VideoRecord_VideoRecordPlugin_h_

struct VideoRecordSettings {
	enum AlignmentMode {
		ALIGN_CENTER,
		ALIGN_FIT,
		ALIGN_STRETCH,
		ALIGN_ZOOM,
	};

	String ffmpeg_path = "ffmpeg";
	String frames_dir;
	String output_file = "render.mp4";
	int    fps = 30;
	int    width = 1280;
	int    height = 720;
	int    alignment = ALIGN_FIT;
	bool   auto_start_on_run = false;
	bool   keep_frames = false;
	bool   overwrite = true;

	void Serialize(Stream& s) {
		s % ffmpeg_path % frames_dir % output_file % fps % width % height % alignment % auto_start_on_run % keep_frames % overwrite;
	}
};

class VideoRecordPreferencesPage : public ParentCtrl, public IPluginPreferencesPage {
public:
	typedef VideoRecordPreferencesPage CLASSNAME;

	VideoRecordPreferencesPage();

	virtual Ctrl& GetCtrl() override { return *this; }
	virtual void  LoadConfig() override;
	virtual void  SaveConfig() override;
	virtual void  ApplyConfig(IDEContext& ctx) override;
	virtual void  SetDefaults() override;
	virtual bool  IsModified() const override;

private:
	VideoRecordSettings ReadSettings() const;
	void                WriteSettings(const VideoRecordSettings& cfg);
	static String       ConfigPath();

	VideoRecordSettings loaded;

	Label      ffmpeg_lbl;
	EditString ffmpeg_path;
	Label      frames_lbl;
	EditString frames_dir;
	Button     browse_frames;
	Label      output_lbl;
	EditString output_file;
	Button     browse_output;
	Label      fps_lbl;
	EditIntSpin fps;
	Label      width_lbl;
	EditIntSpin width;
	Label      height_lbl;
	EditIntSpin height;
	Label      alignment_lbl;
	DropList   alignment;
	Option     keep_frames;
	Option     overwrite;
	Label      note;
};

class VideoRecordPane : public ParentCtrl {
public:
	typedef VideoRecordPane CLASSNAME;

	VideoRecordPane();
	void SetRunState(const ScriptRunState& state, bool recordable);
	void SetRecordingState(bool recording, int frame_count, const String& output_path, const String& status);
	void SetAutoStart(bool enabled);

	Event<> WhenStart;
	Event<> WhenStop;
	Event<bool> WhenAutoStart;

private:
	bool recordable = false;
	bool recording = false;

	Label title_lbl;
	Label state_lbl;
	Label mode_lbl;
	Label path_lbl;
	Label rec_lbl;
	Option auto_start;
	Button start_btn;
	Button stop_btn;
	Label note_lbl;
};

class VideoRecordPlugin : public IPlugin, public IPluginPreferencesProvider, public IRunStateListener {
public:
	VideoRecordPlugin();

	virtual String GetID() const override { return "scriptide.video_record"; }
	virtual String GetName() const override { return "Video Record"; }
	virtual String GetDescription() const override { return "Records .gamestate playback to a video file."; }
	virtual void   Init(IPluginContext& context) override;
	virtual void   Shutdown() override;

	virtual int    GetPreferencesPageCount() const override { return 1; }
	virtual String GetPreferencesPageCategory(int index) const override;
	virtual String GetPreferencesPageID(int index) const override;
	virtual String GetPreferencesPageTitle(int index) const override;
	virtual Image  GetPreferencesPageIcon(int index) const override;
	virtual IPluginPreferencesPage& GetPreferencesPage(int index) override;

	virtual void OnRunStateChanged(PythonIDE& ide, const ScriptRunState& state) override;

private:
	IPluginContextGUI* ctx = nullptr;
	IPluginRegistryGUI* registry = nullptr;
	VideoRecordPreferencesPage prefs_page;
	VideoRecordPane pane;
	ScriptRunState last_state;
	bool recordable_active = false;
	bool recording = false;
	int frame_count = 0;
	int capture_interval_ms = 33;
	String output_path;
	String session_dir;

	void StartRecording();
	void StopRecording(bool finalize);
	void CaptureFrame();
	void ScheduleCapture();
	void UpdatePane();
	void SetAutoStartOnRun(bool enabled);
	VideoRecordSettings LoadSettings() const;
	Image ComposeFrame(const Image& src, Size target, int alignment) const;
};

#endif
