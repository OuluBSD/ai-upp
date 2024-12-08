#ifndef _AI_Ctrl_Release_h_
#define _AI_Ctrl_Release_h_

NAMESPACE_UPP


class SnapInfoCtrl : public WithSnapshotInfo<ToolAppCtrl> {
	
	
public:
	typedef SnapInfoCtrl CLASSNAME;
	SnapInfoCtrl();
	
	void Data();
	void Clear();
	void OnValueChange();
	
};


END_UPP_NAMESPACE

#endif
