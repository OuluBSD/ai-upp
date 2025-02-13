#ifndef _AI_Ctrl_Script_h_
#define _AI_Ctrl_Script_h_

NAMESPACE_UPP


class ScriptTextCtrl : public ValueVFSComponentCtrl {
	
	struct InputTab : WithScriptTextInput<VNodeComponentCtrl> {
		ScriptTextCtrl& owner;
		InputTab(ScriptTextCtrl&);
		void Data() override;
	};
	
	struct PartTab : VNodeComponentCtrl {
		ScriptTextCtrl& owner;
		PartTab(ScriptTextCtrl&);
		void Data() override;
	};
	
	struct GenerateTab : VNodeComponentCtrl {
		ScriptTextCtrl& owner;
		Splitter vsplit;
		ArrayCtrl params;
		DocEdit output;
		GenerateTab(ScriptTextCtrl&);
		void Data() override;
	};
	
	Value params;
	void AddPart();
	void RemovePart();
	void RefreshParams();
	void ImportProofread(VirtualNode new_node, TranscriptProofread& proofread);
	ScriptTextProcess* active_process = 0;
public:
	typedef ScriptTextCtrl CLASSNAME;
	ScriptTextCtrl();
	
	void DataTree(TreeCtrl& tree) override;
	void ToolMenu(Bar& bar) override;
	void Init() override;
	void RealizeData();
	String GetTitle() const override;
	VNodeComponentCtrl* CreateCtrl(const VirtualNode& vnode) override;
};

INITIALIZE(ScriptTextCtrl)


END_UPP_NAMESPACE

#endif
