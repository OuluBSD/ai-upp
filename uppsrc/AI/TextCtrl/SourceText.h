#ifndef _AI_TextCtrl_SourceText_h_
#define _AI_TextCtrl_SourceText_h_

NAMESPACE_UPP


class SourceDataCtrl : public ToolAppCtrl {
	Splitter vsplit, hsplit;
	ArrayCtrl entities, components;//, active_components;
	DocEdit scripts, analysis;
	
public:
	typedef SourceDataCtrl CLASSNAME;
	SourceDataCtrl();
	
	void Data() override;
	void DataEntity();
	void DataComponent();
	void ToolMenu(Bar& bar) override;
	void Do(int fn);
	
	static DbField GetFieldType() {return DBFIELD_SRCTEXT;}
	
};


END_UPP_NAMESPACE

#endif
