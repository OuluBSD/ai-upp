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

void FirstAlgo(SearcherExt& searcher) {
	searcher.SetRandomSeed(0x12345);
	for(int i = 0; i < 2; i++) {
		searcher.SetSearchStrategy(
			i == 0 ?
				SEARCHSTRATEGY_MINIMAX :
				SEARCHSTRATEGY_ALPHA_BETA);
		searcher.SetGenerator(GENERATOR_RANDOM);
		searcher.ClearFS();
		if (!i)	searcher.WhenGenerated = [&]{LOG(searcher.GetFS().GetTreeString());};
		else	searcher.WhenGenerated.Clear();
		bool ret = searcher.RunSearch();
		ASSERT(ret);
		const auto& ans = searcher.GetResult();
		LOG(PtrVecStr(ans));
	}
}

void IntegratedTests() {
	using namespace UPP;
	VfsValue& app_root = MetaEnv().root;
	SearcherExt& searcher = app_root.GetAdd<SearcherExt>("searcher");
	searcher.SetVerbose();
	
	ValueMap params;
	params.Set("total", 25);
	params.Set("low", 2);
	params.Set("high", 3);
	
	bool all = 0;
	
	// Simple game algorithms
	if (all) {
		searcher.SetGeneratorParams(params, callback(SetValue1));
		FirstAlgo(searcher);
		
		const auto& ans = searcher.GetResult();
		ASSERT(ans.GetCount() == 4);
		ASSERT(ValueArray(ans[0]->value)[1] == -2);
		ASSERT(ValueArray(ans[1]->value)[1] ==  5);
		ASSERT(ValueArray(ans[2]->value)[1] == -2);
		ASSERT(ValueArray(ans[3]->value)[1] ==  1);
	}
	
	// Simple game algorithms, with runtime node generation.
	if (all) {
		params.Set("initial",0);
		params.Set("runtime",1);
		params.Set("depth_limit",3);
		searcher.SetGeneratorParams(params, callback(SetValue2));
		searcher.SetTerminalTest(TERMTEST_NO_SUB);
		searcher.SetHeuristics(HEURISTIC_SIMPLE);
		FirstAlgo(searcher);
		LOG(searcher.GetFS().GetTreeString());
	}
	
	// Uninformed search strategies, with runtime node generation
	if (all) {
		params.Set("initial",0);
		params.Set("runtime",1);
		params.Set("depth_limit",3);
		params.Set("estimate_to_goal", 20);
		searcher.SetRandomSeed(0x12345);
		searcher.SetSearcherParams(params);
		searcher.SetGeneratorParams(params, Null);
		for(int i = 0; i < 5; i++) {
			SearchStrategyType t;
			switch (i) {
				case 0: t = SEARCHSTRATEGY_BREADTH_FIRST; break;
				case 1: t = SEARCHSTRATEGY_UNIFORM_COST; break;
				case 2: t = SEARCHSTRATEGY_DEPTH_FIRST; break;
				case 3: t = SEARCHSTRATEGY_DEPTH_LIMITED; break;
				case 4: t = SEARCHSTRATEGY_ASTAR; break;
			}
			searcher.SetSearchStrategy(t);
			searcher.SetTerminalTest(TERMTEST_NULL);
			searcher.SetGenerator(GENERATOR_NULL);
			searcher.SetHeuristics(HEURISTIC_NULL);
			searcher.ClearFS();
			searcher.GetFS().CreateExt<RouteGeneratorNode>();
			searcher.WhenGenerated.Clear();
			bool ret = searcher.RunSearch();
			ASSERT(ret);
			const auto& ans = searcher.GetResult();
			LOG(PtrVecStr(ans));
		}
	}
	
	// Action planner
	if (1) {
		ValueArray actions;
		actions.Add("cat");
		actions.Add("approach");
		actions.Add("come down");
		actions.Add("aim");
		actions.Add("attack");
		actions.Add("wait");
		actions.Add("very high jump attack");
		actions.Add("flee");
		
		ValueMap atoms;
		atoms.Add("armed with claws", true);
		atoms.Add("mouse visible",    false);
		atoms.Add("near mouse",       false);
		atoms.Add("at high place",    true);
		atoms.Add("claws extended",   false);
		atoms.Add("ready to attack",  false);
		atoms.Add("mouse alive",      true);
		atoms.Add("alive",            true);
		
		ValueArray events;
		events.Add(ActionEventValue()
			.Pre("cat", "armed with claws", true)
			.Post("cat", "mouse visible", true));
		events.Add(ActionEventValue()
			.Pre("approach", "mouse visible", true)
			.Post("approach", "near mouse", true));
		events.Add(ActionEventValue()
			.Pre("come down", "at high place", true)
			.Post("come down", "at high place", false));
		events.Add(ActionEventValue()
			.Pre("aim", "mouse visible", true)
			.Post("aim", "claws extended", true)
			.Post("aim", "ready to attack", true));
		events.Add(ActionEventValue()
			.Pre("attack", "ready to attack", true)
			.Post("attack", "at high place", false)
			.Post("attack", "mouse alive", false));
		events.Add(ActionEventValue()
			.Pre("wait", "armed with claws", true)
			.Post("wait", "claws extended", true));
		events.Add(ActionEventValue()
			.Pre("very high jump attack", "at high place", true)
			.Post("very high jump attack", "near mouse", true)
			.Post("very high jump attack", "alive", false)
			.Post("very high jump attack", "mouse alive", false));
		events.Add(ActionEventValue()
			.Pre("flee", "mouse visible", true)
			.Post("flee", "near mouse", false));
		params("atom") = atoms;
		params("actions") = actions;
		searcher.SetGeneratorParams(params, Null);
		searcher.SetSearchStrategy(SEARCHSTRATEGY_ASTAR);
		searcher.SetTerminalTest(TERMTEST_ACTION_PLANNER);
		searcher.SetGenerator(GENERATOR_ACTION_PLANNER);
		searcher.SetHeuristics(HEURISTIC_ACTION_PLANNER);
		searcher.ClearFS();
		searcher.WhenGenerated.Clear();
		bool ret = searcher.RunSearch();
		ASSERT(ret);
		const auto& ans = searcher.GetResult();
		LOG(PtrVecStr(ans));
	}
	
	// Single-agent decider: (Classic) Decision Tree
	if (0) {
		
	}
	
	// Multi-agent co-operative decider
	// - has fixed predictor hash as in decider tree
	if (0) {
		
	}
	
	// Multi-agent zero-sum game: (Classic) CFR: Counter-factual regret minimization
	// - will have dynamically changing predictor hash
	if (0) {
		// Kuhn Poker
	}
	
	if (0) {
		// Texas Hold 'em
		// - generate data (super high error rate)
		
	}
}


END_UPP_NAMESPACE
