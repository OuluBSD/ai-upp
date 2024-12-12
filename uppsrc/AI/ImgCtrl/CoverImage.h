#ifndef _AI_ImgCtrl_CoverImage_h_
#define _AI_ImgCtrl_CoverImage_h_

NAMESPACE_UPP


struct ReleaseCoverImage : Component
{
	
	COMPONENT_CONSTRUCTOR(ReleaseCoverImage)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_RELEASE_COVER_IMAGE;}
	
};

INITIALIZE(ReleaseCoverImage)

class ReleaseCoverImageCtrl : public ComponentCtrl {
	Splitter vsplit, hsplit[3];
	ArrayCtrl attr_list, sugg_list;
	DocEdit attr_text, sugg_text;
	ImageViewerCtrl suggestion[4];
	
	
public:
	typedef ReleaseCoverImageCtrl CLASSNAME;
	ReleaseCoverImageCtrl();
	
	void ToolMenu(Bar& bar) override;
	void Data() override;
	void CreateSuggestionsForPrompts();
	void DataAttribute();
	void DataSuggestion();
	void DataSuggestionImage();
	void OnAttributeChange();
	void OnSuggestionChange();
	void Do(int fn);
	void SuggestionMenu(Bar& bar);
	
};

INITIALIZE(ReleaseCoverImageCtrl)


END_UPP_NAMESPACE

#endif
