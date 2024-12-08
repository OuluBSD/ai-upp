#ifndef _AI_Ctrl_Profile_h_
#define _AI_Ctrl_Profile_h_

NAMESPACE_UPP


class LeadsCtrl;

class ProfileInfoCtrl : public WithProfileInfo<ToolAppCtrl> {
	
	
public:
	typedef ProfileInfoCtrl CLASSNAME;
	ProfileInfoCtrl();
	
	void Data();
	void Clear();
	void OnValueChange();
	
	LeadsCtrl* editor = 0;
	
};


END_UPP_NAMESPACE

#endif
