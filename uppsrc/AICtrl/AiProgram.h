#ifndef _AICtrl_AiProgram_h_
#define _AICtrl_AiProgram_h_

NAMESPACE_UPP

struct AiProgramCtrl : VfsValueExtCtrl {
	Splitter vsplit, hsplit;
	ArrayCtrl proglist, stagelist;
	ArrayCtrl sessionlist, iterlist, querylist;
	Splitter progsplit, stagesplit;
	CodeEditor prog, stage;
	TabCtrl btabs;
	DocEdit log;
	Ptr<Agent> agent;
	
	Vector<VfsValue*> programs, stages;
	/*VectorMap<int,VfsValue*> structure_nodes;
	VectorMap<int,String> structure_values;*/
	
	void PrintLog(Vector<ProcMsg>& msgs);
	void Print(EscEscape& e);
	void PrintString(String s);
	void Input(EscEscape& e);
	
public:
	typedef AiProgramCtrl CLASSNAME;
	AiProgramCtrl();
	
	void Data() override;
	void DataProgramList();
	void DataProgram();
	void DataSession();
	void DataIteration();
	void DataStageList();
	void DataStage();
	void DataQuery();
	void DataBottom();
	void ToolMenu(Bar& bar) override;
	void DataList(ArrayCtrl& list, Vector<VfsValue*>& nodes, hash_t type_hash);
	bool CompileStages(bool force);
	bool Compile(bool force);
	bool Run();
	
	VfsValue* GetProgram();
	VfsValue* GetStage();
	
	void ProgramMenu(Bar& b);
	void AddProgram();
	void RemoveProgram();
	void RenameProgram();
	void DuplicateProgram();
	
	void SessionMenu(Bar& b);
	
	void IterationMenu(Bar& b);
	
	void QueryMenu(Bar& b);
	
	void StageMenu(Bar& b);
	void AddStage();
	void RemoveStage();
	void RenameStage();
	void DuplicateStage();
	
};

INITIALIZE(AiProgramCtrl)

END_UPP_NAMESPACE

#endif
