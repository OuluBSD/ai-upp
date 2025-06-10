#ifndef _AICore2_Ext_h_
#define _AICore2_Ext_h_


class CommitTreeExt : public VfsValueExt {
	
public:
	CLASSTYPE(CommitTreeExt)
	CommitTreeExt(VfsValue& val);
	void Visit(Vis& v) override {}
	
	
};



class CommitDiffListExt : public VfsValueExt {
	
public:
	CLASSTYPE(CommitDiffListExt)
	CommitDiffListExt(VfsValue& val);
	void Visit(Vis& v) override {}
	
	
};



class SolverExt : public VfsValueExt {
	SearchStrategyType	searchstrategy = SEARCHSTRATEGY_NULL;
	HeuristicsType		heuristics = HEURISTICS_NULL;
	GeneratorType		generator_type = GENERATOR_NULL;
	One<Searcher>		searcher;
	One<Generator>		generator;
	RunningFlagSingle	flag;
	bool				verbose = false;
	dword				random_seed = 0;
	
	Value				args;
	Event<Val&>			set_value;
	Vector<Val*>		result;
	
	void				CreateSearcher();
	void				CreateGenerator();
	static String		PtrVecStr(Vector<Val*>& vec);
public:
	CLASSTYPE(SolverExt)
	SolverExt(VfsValue& val);
	
	void Visit(Vis& v) override {}
	void Update(double ts) override;
	
	void				ClearFS();
	VfsValue&			GetFS();
	CommitTreeExt&		GetCommitTree();
	CommitDiffListExt&	GetCommitDiffList();
	void				SetSearchStrategy(SearchStrategyType t);
	void				SetHeuristics(HeuristicsType t);
	void				SetGenerator(GeneratorType t);
	void				SetGeneratorParams(Value args, Event<Val&> set_value);
	void				SetVerbose(bool b=true) {verbose = b;}
	void				SetRandomSeed(dword seed);
	bool				RunSearch();
	bool				SearchBegin();
	bool				SearchIteration();
	bool				SearchEnd();
	const Vector<Val*>&	GetResult();
	bool				RunGenerator();

	template <class T>
	void SetGenerator() {
		generator_type = GENERATOR_CUSTOM;
		generator = new T;
	}
	
	Event<>				WhenGenerated;
};


#endif
