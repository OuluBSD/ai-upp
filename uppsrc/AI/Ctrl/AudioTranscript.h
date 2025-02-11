#ifndef _AI_Ctrl_AudioTranscript_h_
#define _AI_Ctrl_AudioTranscript_h_

NAMESPACE_UPP

class AudioTranscriptCtrl : public WithAudioTranscript<ComponentCtrl> {
	Vector<Ptr<VideoSourceFileRange>> file_ptrs;
	Vector<String> file_paths;
	double duration = 0;
	double frame_rate = 1;
	double range_begin = 0;
	double range_end = 0;
	String vidpath;
	String mp3path;
	TimeStop ts;
	
	void MakeAudio(Event<> cb_ready);
	void Start();
	bool UpdateSources();
public:
	typedef AudioTranscriptCtrl CLASSNAME;
	AudioTranscriptCtrl();
	
	void Data() override;
	void DataFile();
	void ToolMenu(Bar& bar) override;
	
};

INITIALIZE(AudioTranscriptCtrl)

class ScriptSpeechCtrl : public WithScriptSpeech<ComponentCtrl> {
	
public:
	typedef ScriptSpeechCtrl CLASSNAME;
	ScriptSpeechCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override;
	
	
};

INITIALIZE(ScriptSpeechCtrl)

END_UPP_NAMESPACE

#endif
