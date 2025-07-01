#ifndef _AICtrl_VfsProgram_h_
#define _AICtrl_VfsProgram_h_

NAMESPACE_UPP

struct VfsProgramCtrl : VfsValueExtCtrl {
	Splitter vsplit, hsplit;
	ArrayCtrl prjlist, stagelist;
	ArrayCtrl sessionlist, iterlist, querylist;
	Splitter prjsplit, stagesplit;
	Splitter stagectrl, memoryctrl;
	CodeEditor prj, stage;
	TabCtrl btabs, rtabs;
	DocEdit log;
	Ptr<Agent> agent;
	TreeCtrl memtree;
	
	Vector<VfsValue*> projects, sessions, iterations, stages;
	Ptr<VfsValue> cur_project, cur_session, cur_iter;
	
	/*VectorMap<int,VfsValue*> structure_nodes;
	VectorMap<int,String> structure_values;*/
	
	void PrintLog(Vector<ProcMsg>& msgs);
	void Print(EscEscape& e);
	void PrintString(String s);
	void Input(EscEscape& e);
	
public:
	typedef VfsProgramCtrl CLASSNAME;
	VfsProgramCtrl();
	
	void Data() override;
	void DataProjectList();
	void DataProject();
	void DataSession();
	void DataIteration();
	void DataStageList();
	void DataStage();
	void DataQuery();
	void DataBottom();
	void ToolMenu(Bar& bar) override;
	void DataList(ArrayCtrl& list, VfsValue& parent, Vector<VfsValue*>& nodes, hash_t type_hash, Event<> WhenData);
	bool CompileStages(bool force);
	bool Compile(bool force);
	bool Run();
	
	VfsValue* GetProgram();
	VfsValue* GetStage();
	
	void ProjectMenu(Bar& b);
	void AddProject();
	void RemoveProject();
	void RenameProject();
	void DuplicateProject();
	
	void SessionMenu(Bar& b);
	void AddSession();
	void RemoveSession();
	void RenameSession();
	void DuplicateSession();
	
	void IterationMenu(Bar& b);
	void RenameIteration();
	
	void QueryMenu(Bar& b);
	
	void StageMenu(Bar& b);
	void AddStage();
	void RemoveStage();
	void RenameStage();
	void DuplicateStage();
	
};

INITIALIZE(VfsProgramCtrl)

END_UPP_NAMESPACE

#endif
