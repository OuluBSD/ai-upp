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

public:
	typedef SourceDataImporter CLASSNAME;
	SourceDataImporter();

	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;

	static SourceDataImporter& Get(DatasetPtrs p);
	static DbField GetFieldType() { return DBFIELD_SRCTEXT; }

private:
	void Tokenize();
	void Structurize();
};

class SourceAnalysisProcess : public SolverBase {

public:
	enum {
		PHASE_ANALYZE_ARTISTS,
		PHASE_ANALYZE_ELEMENTS,

		PHASE_COUNT,

		PHASE_SUMMARIZE_CONTENT,
	};

	bool filter_foreign = true;
	Atomic actual = 0, total = 0;
	TimeStop ts;
	SourceDataAnalysisArgs args;

public:
	typedef SourceAnalysisProcess CLASSNAME;
	SourceAnalysisProcess();

	int GetPhaseCount() const override;
	int GetBatchCount(int phase) const override;
	int GetSubBatchCount(int phase, int batch) const override;
	void DoPhase() override;

	static SourceAnalysisProcess& Get(DatasetPtrs p);

private:
	void AnalyzeArtists();
	void AnalyzeElements();
	void SummarizeContent();
};

class TokenDataProcess : public SolverBase {

public:
	enum {
		PHASE_GET,

		PHASE_COUNT
	};

	int per_action_task = 100;
	int actual = 0, total = 0;
	TokenArgs token_args;

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
	static DbField GetFieldType() { return DBFIELD_WORDS; }
};

END_UPP_NAMESPACE

#endif
