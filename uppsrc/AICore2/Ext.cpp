#include "AICore.h"

NAMESPACE_UPP


CommitTreeExt::CommitTreeExt(VfsValue& val) : VfsValueExt(val) {
	
}
	
CommitDiffListExt::CommitDiffListExt(VfsValue& val) : VfsValueExt(val) {
	
}

SolverExt::SolverExt(VfsValue& val) : VfsValueExt(val) {
	
}

void SolverExt::Update(double ts) {
	if (flag.IsRunning()) {
		if (!SearchIteration()) {
			SearchEnd();
			flag.SetStopped();
		}
	}
}

void SolverExt::ClearFS() {
	auto& fs = val.GetAdd("fs");
	fs.sub.Clear();
	fs.type_hash = 0;
	fs.ext.Clear();
}

VfsValue& SolverExt::GetFS() {
	return val.GetAdd("fs");
}

CommitTreeExt& SolverExt::GetCommitTree() {
	return val.GetAdd<CommitTreeExt>("commits");
}

CommitDiffListExt& SolverExt::GetCommitDiffList() {
	return val.GetAdd<CommitDiffListExt>("diffs");
}

void SolverExt::SetSearchStrategy(SearchStrategyType t) {
	searchstrategy = t;
}

void SolverExt::SetHeuristics(HeuristicsType t) {
	heuristics = t;
}

void SolverExt::SetGenerator(GeneratorType t) {
	generator_type = t;
}

void SolverExt::SetRandomSeed(dword seed) {
	random_seed = seed;
}

void SolverExt::SetGeneratorParams(Value args, Event<Val&> set_value) {
	this->args = args;
	this->set_value = set_value;
}

void SolverExt::CreateSearcher() {
	searcher.Clear();
	switch (searchstrategy) {
		case SEARCHSTRATEGY_MINIMAX:		searcher = new MiniMax; break;
		case SEARCHSTRATEGY_ALPHA_BETA:		searcher = new AlphaBeta; break;
		case SEARCHSTRATEGY_BREADTH_FIRST:	searcher = new BreadthFirst; break;
		case SEARCHSTRATEGY_UNIFORM_COST:	searcher = new UniformCost; break;
		case SEARCHSTRATEGY_DEPTH_FIRST:	searcher = new DepthFirst; break;
		case SEARCHSTRATEGY_DEPTH_LIMITED:	searcher = new DepthLimited; break;
		case SEARCHSTRATEGY_BEST_FIRST:		searcher = new BestFirst; break;
		case SEARCHSTRATEGY_ASTAR:			searcher = new AStar; break;
		default: break;
	}
}

void SolverExt::CreateGenerator() {
	switch (generator_type) {
		case GENERATOR_NULL: break;
		case GENERATOR_RANDOM: generator = new GeneratorRandom(); break;
		case GENERATOR_CUSTOM: ASSERT(generator); break;
		default: TODO break;
	}
}

bool SolverExt::RunGenerator() {
	ASSERT(generator);
	
	generator->SetParams(args);
	generator->SetValueFunction(set_value);
	
	if (random_seed)
		SeedRandom(random_seed);
	
	bool succ = generator->Run(GetFS());
	if (succ)
		WhenGenerated();
	
	return succ;
}

bool SolverExt::SearchBegin() {
	flag.Stop();
	result.Clear();
	
	// Create sub paths
	VfsValue& fs			= GetFS();
	CommitTreeExt& tree		= GetCommitTree();
	CommitDiffListExt& list	= GetCommitDiffList();
	
	CreateSearcher();
	if (!searcher)
		return false;
	
	CreateGenerator();
	if (generator && !RunGenerator())
		return false;
	
	if (!searcher->SearchBegin(GetFS()))
		return false;
	
	flag.Start();
	
	return true;
}

bool SolverExt::SearchIteration() {
	if (!flag.IsRunning())
		return false;
	
	return searcher->SearchIteration();
}

bool SolverExt::SearchEnd() {
	result = searcher->SearchEnd();
	return !result.IsEmpty();
}

bool SolverExt::RunSearch()
{
	if (!SearchBegin())
		return false;
	
	while (1) {
		if (!SearchIteration())
			break;
	}
	
	bool ret = SearchEnd();
	
	flag.SetStopped();
	return ret;
}

const Vector<Val*>& SolverExt::GetResult() {
	return result;
}

String SolverExt::PtrVecStr(Vector<Val*>& vec) {
	String out;
	for(int i = 0; i < vec.GetCount(); i++) {
		if (i) out << "\n";
		out << i << ": ";
		if (vec[i]->ext)
			out << vec[i]->ext->ToString();
	}
	return out;
}


	


END_UPP_NAMESPACE
