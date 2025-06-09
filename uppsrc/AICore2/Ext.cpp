#include "AICore.h"

NAMESPACE_UPP


CommitTreeExt::CommitTreeExt(VfsValue& val) : VfsValueExt(val) {
	
}
	
CommitDiffListExt::CommitDiffListExt(VfsValue& val) : VfsValueExt(val) {
	
}

SolverExt::SolverExt(VfsValue& val) : VfsValueExt(val) {
	
}

VfsValue& SolverExt::GetFS() {
	return val.GetAdd("fs",0);
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
		case GENERATOR_RANDOM: generator = new GeneratorRandom(); break;
		case GENERATOR_CUSTOM: ASSERT(generator); break;
		default: TODO break;
	}
}

bool SolverExt::RunSearch() {
	// Create sub paths
	VfsValue& fs			= GetFS();
	CommitTreeExt& tree		= GetCommitTree();
	CommitDiffListExt& list	= GetCommitDiffList();
	
	CreateSearcher();
	CreateGenerator();
	
	if (!searcher)
		return false;
	
	Vector<Val*> ans = searcher->Search(GetFS());
	if (verbose) {LOG(PtrVecStr(ans));}
	
	return true;
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
