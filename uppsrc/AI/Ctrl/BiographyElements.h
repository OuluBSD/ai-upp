#ifndef _AI_Ctrl_BiographyElements_h_
#define _AI_Ctrl_BiographyElements_h_

NAMESPACE_UPP


struct BiographyElements : Component
{
	
	COMPONENT_CONSTRUCTOR(BiographyElements)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_BIOGRAPHY_ELEMENTS;}
	
};

INITIALIZE(BiographyElements)

class BiographyElementsCtrl : public ComponentCtrl {
	Splitter hsplit, vsplit;
	ArrayCtrl categories, elements;
	WithBiography<Ctrl> block;
	int sort_column = 0;
public:
	typedef BiographyElementsCtrl CLASSNAME;
	BiographyElementsCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override;
	void Do(int fn);
	void DataCategory();
	void DataElement();
	void DataYear();
	void OnValueChange();
	
	
};

INITIALIZE(BiographyElementsCtrl)


END_UPP_NAMESPACE

#endif
