#ifndef _AI_Ctrl_Voiceover_h_
#define _AI_Ctrl_Voiceover_h_

NAMESPACE_UPP


class VoiceoverTextCtrl : public ValueVFSComponentCtrl {
	
	struct InputTab : WithVoiceoverInput<VNodeComponentCtrl> {
		VoiceoverTextCtrl& owner;
		InputTab(VoiceoverTextCtrl&);
		void Data() override;
	};
	
	struct PartTab : VNodeComponentCtrl {
		VoiceoverTextCtrl& owner;
		PartTab(VoiceoverTextCtrl&);
		void Data() override;
	};
	
	struct GenerateTab : VNodeComponentCtrl {
		VoiceoverTextCtrl& owner;
		Splitter vsplit;
		ArrayCtrl params;
		DocEdit output;
		GenerateTab(VoiceoverTextCtrl&);
		void Data() override;
	};
	
public:
	typedef VoiceoverTextCtrl CLASSNAME;
	VoiceoverTextCtrl();
	
	void ToolMenu(Bar& bar) override;
	void Init() override;
	void RealizeData();
	String GetTitle() const override;
	VNodeComponentCtrl* CreateCtrl(const VirtualNode& vnode) override;
};

INITIALIZE(VoiceoverTextCtrl)


END_UPP_NAMESPACE

#endif
