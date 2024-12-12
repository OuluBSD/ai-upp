#ifndef _AI_Ctrl_BiographyConcepts_h_
#define _AI_Ctrl_BiographyConcepts_h_

NAMESPACE_UPP


struct BiographyConcepts : Component
{
	
	COMPONENT_CONSTRUCTOR(BiographyConcepts)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_BIOGRAPHY_CONCEPTS;}
	
};

INITIALIZE(BiographyConcepts)

class ConceptualFrameworkNavigator : public ComponentCtrl {
	Splitter cfsplit, vsplit, tsplit, bsplit;
	ArrayCtrl cfs;
	ArrayCtrl stories;
	WithConceptualFramework<Ctrl> cf;
	DocEdit story_struct, story_improved;
	int story_sort_column = 0;
	
public:
	WithConceptualFrameworkStory<Ctrl> story;
	
public:
	typedef ConceptualFrameworkNavigator CLASSNAME;
	ConceptualFrameworkNavigator();
	
	void Data() override;
	void ToolMenu(Bar& bar) override;
	void DataAll(bool forced);
	void DataFramework();
	void DataStory();
	void OnValueChange();
	void Do(int fn);
	void MainLayout();
	void SideLayout();
	void LockForm();
	void MoveSortColumn(int fn);
	
	
	void GetElements(ConceptualFrameworkArgs& args);
	int64 GetBeliefUniq() const;
	
};

class ConceptualFrameworkCtrl : public ConceptualFrameworkNavigator {
	
public:
	typedef ConceptualFrameworkCtrl CLASSNAME;
	ConceptualFrameworkCtrl();
	
	void ToolMenu(Bar& bar) override;
	
	
};

INITIALIZE(ConceptualFrameworkCtrl)


END_UPP_NAMESPACE

#endif
