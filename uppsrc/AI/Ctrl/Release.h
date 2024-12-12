#ifndef _AI_Ctrl_Release_h_
#define _AI_Ctrl_Release_h_

NAMESPACE_UPP


struct ReleaseBriefing : Component
{
	
	COMPONENT_CONSTRUCTOR(ReleaseBriefing)
	void Serialize(Stream& s) override {TODO}
	void Jsonize(JsonIO& json) override {TODO}
	hash_t GetHashValue() const override {TODO; return 0;}
	static int GetKind() {return METAKIND_ECS_COMPONENT_RELEASE_BRIEFING;}
	
};

INITIALIZE(ReleaseBriefing)

class ReleaseInfoCtrl : public WithSnapshotInfo<ComponentCtrl> {
	
	
public:
	typedef ReleaseInfoCtrl CLASSNAME;
	ReleaseInfoCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override {}
	void Clear();
	void OnValueChange();
	
};

INITIALIZE(ReleaseInfoCtrl)


END_UPP_NAMESPACE

#endif
