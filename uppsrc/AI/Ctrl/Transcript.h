#ifndef _AI_Ctrl_Transcript_h_
#define _AI_Ctrl_Transcript_h_

NAMESPACE_UPP

class TranscriptProofreadCtrl : public WithTranscriptProofread<ComponentCtrl> {
	typedef TranscriptProofread COMPNAME;
	RangeFinder<COMPNAME, AudioTranscript> finder;
	TimeStop ts;
	
public:
	typedef TranscriptProofreadCtrl CLASSNAME;
	TranscriptProofreadCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override;
	void DataFile();
	void Start();
	
};

INITIALIZE(TranscriptProofreadCtrl)



class ProofreadStorylineCtrl : public WithProofreadStoryline<ComponentCtrl> {
	
public:
	typedef ProofreadStorylineCtrl CLASSNAME;
	ProofreadStorylineCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override;
	
	
};

INITIALIZE(ProofreadStorylineCtrl)



class StorylineConversionCtrl : public WithStorylineConversion<ComponentCtrl> {
	
public:
	typedef StorylineConversionCtrl CLASSNAME;
	StorylineConversionCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override;
	
	
};

INITIALIZE(StorylineConversionCtrl)



class StorylineScriptCtrl : public WithStorylineScript<ComponentCtrl> {
	
public:
	typedef StorylineScriptCtrl CLASSNAME;
	StorylineScriptCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override;
	
	
};

INITIALIZE(StorylineScriptCtrl)



class ScriptConversionCtrl : public WithScriptConversion<ComponentCtrl> {
	
public:
	typedef ScriptConversionCtrl CLASSNAME;
	ScriptConversionCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override;
	
	
};

INITIALIZE(ScriptConversionCtrl)

END_UPP_NAMESPACE

#endif
