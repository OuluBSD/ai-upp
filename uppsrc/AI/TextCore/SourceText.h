#ifndef _AI_TextCore_SourceText_h_
#define _AI_TextCore_SourceText_h_

NAMESPACE_UPP


class SourceDataImporter : public SolverBase {
	
public:
	enum {
		PHASE_TOKENIZE,
		
		PHASE_COUNT
	};
	
	bool filter_foreign = true;
	Atomic actual = 0, total = 0;
	TimeStop ts;
	DatasetPtrs p;
	
public:
	typedef SourceDataImporter CLASSNAME;
	SourceDataImporter();
	
	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;
	
	static SourceDataImporter& Get(DatasetPtrs& p);
	static DbField GetFieldType() {return DBFIELD_SRCTEXT;}
	
private:
	void Tokenize();
	void Structurize();
};


END_UPP_NAMESPACE

#endif
