#ifndef _AI_Ctrl_Owner_h_
#define _AI_Ctrl_Owner_h_

NAMESPACE_UPP


class LeadsCtrl;

class OwnerInfoCtrl : public WithOwnerInfo<ComponentCtrl> {
	
public:
	typedef OwnerInfoCtrl CLASSNAME;
	OwnerInfoCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override {}
	void Clear();
	void OnValueChange();
	
	LeadsCtrl* editor = 0;
	
	static int GetKind() {return METAKIND_ECS_COMPONENT_OWNER;}
	
};

INITIALIZE(OwnerInfoCtrl)


END_UPP_NAMESPACE

#endif
