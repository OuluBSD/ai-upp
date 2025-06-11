#include "AgentDemo.h"

NAMESPACE_UPP

/*
struct RouteSearcherUtil :
	Generator,
	TerminalTester,
	HeuristicEval
{
	RouteSearcherUtil();
	void SetParams(Value val) override;
	bool Run(Val& fs) override;
	void GenerateSubValues(Val& val) override;
	bool TerminalTest(Val& v) override;
	double Utility(Val& val) override;
	double Estimate(Val& n) override;
	double Distance(Val& n, Val& dest) override;
};
*/

void SetValue2(Val& i) {
	i.value = -2 + (int)Random(100);
}

void FirstAlgo(SolverExt& modules) {
	modules.SetRandomSeed(0x12345);
	for(int i = 0; i < 2; i++) {
		modules.SetSearchStrategy(
			i == 0 ?
				SEARCHSTRATEGY_MINIMAX :
				SEARCHSTRATEGY_ALPHA_BETA);
		modules.SetGenerator(GENERATOR_RANDOM);
		modules.ClearFS();
		if (!i)	modules.WhenGenerated = [&]{LOG(modules.GetFS().GetTreeString());};
		else	modules.WhenGenerated.Clear();
		bool ret = modules.RunSearch();
		ASSERT(ret);
		const auto& ans = modules.GetResult();
		LOG(PtrVecStr(ans));
	}
}

void IntegratedTests() {
	using namespace UPP;
	VfsValue& app_root = MetaEnv().root;
	SolverExt& modules = app_root.GetAdd<SolverExt>("solver");
	modules.SetVerbose();
	
	ValueMap params;
	params.Set("total", 25);
	params.Set("low", 2);
	params.Set("high", 3);
	
	// Simple game algorithms
	if (1) {
		modules.SetGeneratorParams(params, callback(SetValue1));
		FirstAlgo(modules);
		
		const auto& ans = modules.GetResult();
		ASSERT(ans.GetCount() == 4);
		ASSERT(ValueArray(ans[0]->value)[1] == -2);
		ASSERT(ValueArray(ans[1]->value)[1] ==  5);
		ASSERT(ValueArray(ans[2]->value)[1] == -2);
		ASSERT(ValueArray(ans[3]->value)[1] ==  1);
	}
	
	// Simple game algorithms, with runtime node generation.
	if (1) {
		params.Set("initial",0);
		params.Set("runtime",1);
		params.Set("depth_limit",3);
		modules.SetGeneratorParams(params, callback(SetValue2));
		modules.SetTerminalTest(TERMTEST_NO_SUB);
		modules.SetHeuristics(HEURISTIC_SIMPLE);
		FirstAlgo(modules);
		LOG(modules.GetFS().GetTreeString());
	}
	
	// Uninformed search strategies, with runtime node generation
	if (1) {
		params.Set("initial",0);
		params.Set("runtime",1);
		params.Set("depth_limit",3);
		params.Set("estimate_to_goal", 20);
		modules.SetRandomSeed(0x12345);
		modules.SetSearcherParams(params);
		modules.SetGeneratorParams(params, Null);
		for(int i = 0; i < 4; i++) {
			SearchStrategyType t;
			switch (i) {
				case 0: t = SEARCHSTRATEGY_BREADTH_FIRST; break;
				case 1: t = SEARCHSTRATEGY_UNIFORM_COST; break;
				case 2: t = SEARCHSTRATEGY_DEPTH_FIRST; break;
				case 3: t = SEARCHSTRATEGY_DEPTH_LIMITED; break;
			}
			modules.SetSearchStrategy(t);
			modules.SetTerminalTest(TERMTEST_NULL);
			modules.SetGenerator(GENERATOR_NULL);
			modules.ClearFS();
			modules.GetFS().CreateExt<RouteGeneratorNode>();
			modules.WhenGenerated.Clear();
			bool ret = modules.RunSearch();
			ASSERT(ret);
			const auto& ans = modules.GetResult();
			LOG(PtrVecStr(ans));
		}
	}
	
}


END_UPP_NAMESPACE
