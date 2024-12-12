#ifndef _AI_Ctrl_BiographySummary_h_
#define _AI_Ctrl_BiographySummary_h_

NAMESPACE_UPP


struct BiographySummary : Component
{
	
	COMPONENT_CONSTRUCTOR(BiographySummary)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_BIOGRAPHY_SUMMARY;}
	
};

INITIALIZE(BiographySummary)

class BiographySummaryCtrl : public ComponentCtrl {
	Splitter hsplit, vsplit;
	ArrayCtrl categories, blocks;
	WithBiography<Ctrl> block;
	
public:
	typedef BiographySummaryCtrl CLASSNAME;
	BiographySummaryCtrl();
	
	void Data() override;
	void DataCategory();
	void DataYear();
	void OnValueChange();
	void Translate();
	void MakeKeywords();
	void UpdateElements();
	void Do(int fn);
	void OnTranslate(String s);
	void OnKeywords(String s);
	void ToolMenu(Bar& bar) override;
	void EntryListMenu(Bar& bar);
	
	
};

INITIALIZE(BiographySummaryCtrl)


class BiographySummaryProcess : public SolverBase {
	
public:
	enum {
		PHASE_FIX_SUMMARY_HASHES,
		PHASE_SUMMARIZE_USING_EXISTING,
		PHASE_SUMMARIZE,
		PHASE_SUMMARIZE_ELEMENTS_USING_EXISTING,
		PHASE_SUMMARIZE_ELEMENTS,
		
		PHASE_COUNT,
	};
	
	Ptr<Profile> profile;
	Ptr<BiographySnapshot> snap;
	
public:
	typedef BiographySummaryProcess CLASSNAME;
	BiographySummaryProcess();
	
	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;
	
	static BiographySummaryProcess& Get(Profile& p, BiographySnapshot& snap);
	
private:
	
	void FixSummaryHashes();
	void SummarizeUsingExisting();
	bool SummarizeBase(int fn, BiographySummaryProcessArgs& args);
	void Summarize();
	void SummarizeElementsUsingExisting();
	void SummarizeElements();
	void OnProcessSummarize(String res);
	void OnProcessSummarizeElements(String res);
	
};

END_UPP_NAMESPACE

#endif
