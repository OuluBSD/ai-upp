#ifndef _AI_Ctrl_Voiceover_h_
#define _AI_Ctrl_Voiceover_h_

NAMESPACE_UPP


class VoiceoverTextCtrl : public ValueVFSComponentCtrl {
	
	struct AnalyzeTab : VNodeComponentCtrl {
		Splitter hsplit, vsplit;
		DocEdit input;
		ArrayCtrl cat, values;
		AnalyzeTab();
	};
	
	struct GenerateTab : VNodeComponentCtrl {
		Splitter vsplit;
		ArrayCtrl params;
		DocEdit output;
		GenerateTab();
	};
	
public:
	typedef VoiceoverTextCtrl CLASSNAME;
	VoiceoverTextCtrl();
	
	void ToolMenu(Bar& bar) override;
	void Init() override;
	void VirtualData() override;
	void RealizeData();
	String GetTitle() const override;
};

INITIALIZE(VoiceoverTextCtrl)


END_UPP_NAMESPACE

#endif
