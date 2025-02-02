#ifndef _AI_Ctrl_AudioTranscript_h_
#define _AI_Ctrl_AudioTranscript_h_

NAMESPACE_UPP

class AudioTranscriptCtrl : public WithAudioTranscript<ComponentCtrl> {
	
public:
	typedef AudioTranscriptCtrl CLASSNAME;
	AudioTranscriptCtrl();
	
	void Data() override;
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
