#ifndef _AI_TextCore_TokenData_h_
#define _AI_TextCore_TokenData_h_

NAMESPACE_UPP


class TokenDataProcess : public SolverBase {
	
public:
	enum {
		PHASE_GET,
		
		PHASE_COUNT
	};
	
	int per_action_task = 100;
	int actual = 0, total = 0;
	TokenArgs token_args;
	DatasetPtrs p;
	
	void GetUsingExisting();
	void Get();

public:
	typedef TokenDataProcess CLASSNAME;
	TokenDataProcess();
	
	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;
	
	static TokenDataProcess& Get(DatasetPtrs p);
	static DbField GetFieldType() {return DBFIELD_WORDS;}
	
};


END_UPP_NAMESPACE


#endif
