#ifndef _AI_Ctrl_ImageBiography_h_
#define _AI_Ctrl_ImageBiography_h_

NAMESPACE_UPP


struct ImageBiography : Component
{
	
	COMPONENT_CONSTRUCTOR(ImageBiography)
	void Serialize(Stream& s) override {TODO}
	void Jsonize(JsonIO& json) override {TODO}
	hash_t GetHashValue() const override {TODO; return 0;}
	static int GetKind() {return METAKIND_ECS_COMPONENT_IMAGE_BIOGRAPHY;}
	
};

INITIALIZE(ImageBiography)

class ImageViewerCtrl : public Ctrl {
	Image img;
	
public:
	typedef ImageViewerCtrl CLASSNAME;
	ImageViewerCtrl();
	
	void Paint(Draw& d) override;
	void SetImage(const Image& i);
	void Clear();
	void Menu(Bar& menu);
	void RightDown(Point p, dword keyflags) override;

};

class ImageBiographyCtrl : public ComponentCtrl {
	Splitter hsplit, vsplit, bsplit;
	ArrayCtrl categories, years, entries;
	WithImageBiography<Ctrl> year;
	ImageViewerCtrl img;
	
public:
	typedef ImageBiographyCtrl CLASSNAME;
	ImageBiographyCtrl();
	
	void Data() override;
	void DataCategory();
	void DataYear();
	void DataEntry();
	void OnCategoryCursor();
	void OnValueChange();
	void Translate();
	void MakeKeywords(int fn);
	void OnTranslate(String s);
	void OnKeywords(int fn, String s);
	void ToolMenu(Bar& bar) override;
	void EntryListMenu(Bar& bar);
	void AddEntry();
	void RemoveEntry();
	void PasteImagePath();
	void AnalyseImage();
	void SetCurrentImage(Image img);
	void Do(int fn);
	
	
};

INITIALIZE(ImageBiographyCtrl)


class ImageBiographyProcess : public SolverBase {
	
public:
	enum {
		PHASE_ANALYZE_IMAGE_BIOGRAPHY,
		
		PHASE_COUNT,
	};
	
	Profile* p = 0;
	BiographySnapshot* snap = 0;
	
	
	struct VisionTask : Moveable<VisionTask> {
		BioImage* bimg = 0;
		String jpeg;
	};
	Vector<VisionTask> vision_tasks;
	void TraverseVisionTasks();
	
	
	
	
public:
	typedef ImageBiographyProcess CLASSNAME;
	ImageBiographyProcess();
	
	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;
	
	static ImageBiographyProcess& Get(Profile& p, BiographySnapshot& snap);
	
private:
	
	void ProcessAnalyzeImageBiography();
	void OnProcessAnalyzeImageBiography(String res);
	
};

END_UPP_NAMESPACE

#endif
