#ifndef _AI_Ctrl_Owner_h_
#define _AI_Ctrl_Owner_h_

NAMESPACE_UPP


class LeadsCtrl;

class OwnerInfoCtrl : public WithOwnerInfo<ToolAppCtrl> {
	
	
public:
	typedef OwnerInfoCtrl CLASSNAME;
	OwnerInfoCtrl();
	
	void Data();
	void Clear();
	void OnValueChange();
	
	LeadsCtrl* editor = 0;
	
};


END_UPP_NAMESPACE

#endif
