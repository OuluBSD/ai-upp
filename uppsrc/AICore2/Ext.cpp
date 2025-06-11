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

void SolverExt::ClearGeneratorParams() {
	generator_params = Value();
	generator_set_value.Clear();
}

void SolverExt::ClearFS() {
	auto& fs = val.GetAdd("fs");
	fs.sub.Clear();
	fs.type_hash = 0;
	fs.ext.Clear();
	fs.value = Value();
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

void SolverExt::SetHeuristics(HeuristicType t) {
	heuristic_type = t;
}

void SolverExt::SetGenerator(GeneratorType t) {
	generator_type = t;
}

void SolverExt::SetTerminalTest(TerminalTestType t) {
	termtest_type = t;
}

void SolverExt::SetRandomSeed(dword seed) {
	random_seed = seed;
}

void SolverExt::SetSearcherParams(Value params) {
	this->searcher_params = params;
}

void SolverExt::SetGeneratorParams(Value params, Event<Val&> set_value) {
	this->generator_params = params;
	this->generator_set_value = set_value;
}

void SolverExt::CreateSearcher() {
	switch (searchstrategy) {
		case SEARCHSTRATEGY_NULL:			searcher.Clear(); break;
		case SEARCHSTRATEGY_MINIMAX:		searcher = new MiniMax; break;
		case SEARCHSTRATEGY_ALPHA_BETA:		searcher = new AlphaBeta; break;
		case SEARCHSTRATEGY_BREADTH_FIRST:	searcher = new BreadthFirst; break;
		case SEARCHSTRATEGY_UNIFORM_COST:	searcher = new UniformCost; break;
		case SEARCHSTRATEGY_DEPTH_FIRST:	searcher = new DepthFirst; break;
		case SEARCHSTRATEGY_DEPTH_LIMITED:	searcher = new DepthLimited; break;
		case SEARCHSTRATEGY_BEST_FIRST:		searcher = new BestFirst; break;
		case SEARCHSTRATEGY_ASTAR:			searcher = new AStar; break;
		case SEARCHSTRATEGY_CUSTOM:			ASSERT(searcher); break;
		default: TODO searcher.Clear(); break;
	}
	
	if (searcher)
		searcher->SetParams(searcher_params);
}

void SolverExt::CreateGenerator() {
	switch (generator_type) {
		case GENERATOR_NULL: generator.Clear(); break;
		case GENERATOR_RANDOM:				generator = new GeneratorRandom(); break;
		case GENERATOR_CUSTOM:				ASSERT(generator); break;
		default: TODO generator.Clear(); break;
	}
	if (searcher)
		searcher->SetGenerator(generator ? &*generator : 0);
}

void SolverExt::CreateTerminalTester() {
	switch (termtest_type) {
		case TERMTEST_NULL:					termtester.Clear(); break;
		case TERMTEST_NO_SUB:				termtester = new NoSubTerminal(); break;
		case TERMTEST_CUSTOM:				ASSERT(termtester); break;
		default: TODO termtester.Clear(); break;
	}
	if (searcher)
		searcher->SetTerminalTester(termtester ? &*termtester : 0);
}

void SolverExt::CreateHeuristic() {
	switch (heuristic_type) {
		case HEURISTIC_NULL:				heuristic.Clear(); break;
		case HEURISTIC_SIMPLE:				heuristic = new SimpleHeuristic; break;
		case HEURISTIC_HAMMING_DISTANCE_OF_PREDICATES: TODO; break;
		case HEURISTIC_CUSTOM:				ASSERT(heuristic); break;
		default: TODO heuristic.Clear(); break;
	}
	if (searcher)
		searcher->SetHeuristic(heuristic ? &*heuristic : 0);
}

bool SolverExt::RunGenerator() {
	ASSERT(generator);
	
	generator->SetParams(generator_params);
	generator->SetValueFunction(generator_set_value);
	
	bool succ = generator->Run(GetFS());
	if (succ)
		WhenGenerated();
	
	return succ;
}

bool SolverExt::SearchBegin() {
	flag.Stop();
	result.Clear();
	
	if (random_seed) {
		SeedRandom(random_seed);
		// Get few random values to skip first non changing constants
		for(int i = 0; i < 10; i++) Random();
	}
	
	// Create sub paths
	VfsValue& fs			= GetFS();
	CommitTreeExt& tree		= GetCommitTree();
	CommitDiffListExt& list	= GetCommitDiffList();
	
	CreateSearcher();
	if (!searcher)
		return false;
	
	CreateTerminalTester();
	CreateHeuristic();
	CreateGenerator();
	
	if (generator && !RunGenerator())
		return false;
	
	if (!generator && searcher) {
		searcher->generator_params     = generator_params;
		searcher->generator_set_value  = generator_set_value;
	}
	
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
