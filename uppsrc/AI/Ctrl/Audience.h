#ifndef _AI_Ctrl_Audience_h_
#define _AI_Ctrl_Audience_h_

NAMESPACE_UPP


class AudienceCtrl : public ToolAppCtrl {
	Splitter menusplit, hsplit, vsplit, bsplit;
	ArrayCtrl roles, profiles, responses, entries;
	WithAudience<Ctrl> entry;
	ImageViewerCtrl img;
	
public:
	typedef AudienceCtrl CLASSNAME;
	AudienceCtrl();
	
	void Data() override;
	void DataRole();
	void DataProfile();
	void DataResponse();
	void ToolMenu(Bar& bar) override;
	void EntryListMenu(Bar& bar);
	void Do(int fn);
	
	
};

class AudienceProcess : public SolverBase {
	
public:
	enum {
		PHASE_AUDIENCE_PROFILE_CATEGORIES,
		
		PHASE_COUNT,
	};
	
	Profile* p = 0;
	BiographySnapshot* snap = 0;
	
	
public:
	typedef AudienceProcess CLASSNAME;
	AudienceProcess();
	
	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;
	
	static AudienceProcess& Get(Profile& p, BiographySnapshot& snap);
	
private:
	
	void ProcessAudienceProfileCategories();
	void OnProcessAudienceProfileCategories(String res);
	
};


END_UPP_NAMESPACE

#endif
