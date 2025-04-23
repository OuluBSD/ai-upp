#ifndef _AI_Ctrl_Transcript_h_
#define _AI_Ctrl_Transcript_h_

NAMESPACE_UPP

class TranscriptProofreadCtrl : public WithTranscriptProofread<ComponentCtrl> {
	typedef TranscriptProofread COMPNAME;
	RangeFinder<COMPNAME, AudioTranscript> finder;
	Array<Option> opt;
	TimeStop ts;
	bool playing = false;
	
public:
	typedef TranscriptProofreadCtrl CLASSNAME;
	TranscriptProofreadCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override;
	void DataFile();
	void Start();
	void PlaySelected();
	void PlaySingle(int idx);
	
};

INITIALIZE(TranscriptProofreadCtrl)



// TODO remove
#if 0
class ProofreadStorylineCtrl : public WithProofreadStoryline<ComponentCtrl> {
	typedef ProofreadStoryline COMPNAME;
	RangeFinder<COMPNAME, TranscriptProofread> finder;
	TimeStop ts;
	
public:
	typedef ProofreadStorylineCtrl CLASSNAME;
	ProofreadStorylineCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override;
	void DataFile();
	void Start();
	
};

INITIALIZE(ProofreadStorylineCtrl)


class StorylineDialogCtrl : public WithStorylineDialog<ComponentCtrl> {
	typedef StorylineDialog COMPNAME;
	RangeFinder<COMPNAME, ProofreadStoryline> finder;
	TimeStop ts;
	
public:
	typedef StorylineDialogCtrl CLASSNAME;
	StorylineDialogCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override;
	void DataFile();
	void Start();
	
};

INITIALIZE(StorylineDialogCtrl)
#endif

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
