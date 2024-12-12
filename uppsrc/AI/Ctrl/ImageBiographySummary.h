#ifndef _AI_Ctrl_ImageBiographySummary_h_
#define _AI_Ctrl_ImageBiographySummary_h_

NAMESPACE_UPP


struct ImageBiographySummary : Component
{
	
	COMPONENT_CONSTRUCTOR(ImageBiographySummary)
	void Serialize(Stream& s) override {TODO}
	void Jsonize(JsonIO& json) override {TODO}
	hash_t GetHashValue() const override {TODO; return 0;}
	static int GetKind() {return METAKIND_ECS_COMPONENT_IMAGE_BIOGRAPHY_SUMMARY;}
	
};

INITIALIZE(ImageBiographySummary)

class ImageBiographySummaryCtrl : public ComponentCtrl {
	Splitter hsplit, vsplit;
	ArrayCtrl categories, blocks;
	WithBiography<Ctrl> block;
	
public:
	typedef ImageBiographySummaryCtrl CLASSNAME;
	ImageBiographySummaryCtrl();
	
	void Data() override;
	void DataCategory();
	void DataYear();
	void OnValueChange();
	void Translate();
	void MakeKeywords();
	void Do(int fn);
	void OnTranslate(String s);
	void OnKeywords(String s);
	void ToolMenu(Bar& bar) override;
	void EntryListMenu(Bar& bar);
	
	
};

INITIALIZE(ImageBiographySummaryCtrl)

class ImageBiographySummaryProcess : public SolverBase {
	
public:
	enum {
		PHASE_SUMMARIZE_IMAGE_CATEGORY_YEAR,
		PHASE_SUMMARIZE_IMAGE_BIOGRAPHY,
		
		PHASE_COUNT,
	};
	
	//Profile* prof = 0;
	//BiographySnapshot* snap = 0;
	
	
	
	struct ImageSummaryTask : Moveable<ImageSummaryTask> {
		BiographyCategory* bcat = 0;
		BioYear* by = 0;
		BioImage* summary = 0;
		BioRange range;
		int bcat_i = -1;
	};
	Vector<ImageSummaryTask> imgsum_tasks;
	void TraverseImageSummaryTasks();
	
	
public:
	typedef ImageBiographySummaryProcess CLASSNAME;
	ImageBiographySummaryProcess();
	
	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;
	
	static ImageBiographySummaryProcess& Get(Profile& p, BiographySnapshot& snap);
	
private:
	
	void ProcessSummarizeImageCategoryYear();
	void ProcessSummarizeImageBiography();
	void OnProcessSummarizeImageCategoryYear(String res);
	void OnProcessSummarizeImageBiography(String res);
	
};


END_UPP_NAMESPACE

#endif
