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
	
	// Action planner tests
	if (1 || all) {
		for(int i = 0; i < 4; i++) {
			ValueMap atoms, goal, actions;
			if (i == 0) {
				atoms	.Add("A", false);
				goal	.Add("A", true);
				actions	.Add("write A", ActionEventValue().Pre("A",false).Post("A",true));
			}
			else if (i == 1) {
				atoms	.Add("A", false);
				atoms	.Add("B", false);
				goal	.Add("A", true);
				actions	.Add("write A", ActionEventValue().Pre("A",false).Pre("B",true).Post("A",true));
				actions	.Add("write B", ActionEventValue().Pre("B",false).Post("B",true));
			}
			else if (i == 2) {
				atoms	.Add("A", false);
				atoms	.Add("B", false);
				atoms	.Add("C", false);
				goal	.Add("A", true);
				// avoid 'write A (via B)' by setting cost multiplier to 5
				actions	.Add("write A via B", ActionEventValue().Cost(5).Pre("A",false).Pre("B",true).Post("A",true));
				actions	.Add("write A via C", ActionEventValue().Pre("A",false).Pre("C",true).Post("A",true));
				actions	.Add("write B", ActionEventValue().Pre("B",false).Post("B",true));
				// ... even though writing C is slightly more costly
				actions	.Add("write C", ActionEventValue().Cost(2).Pre("C",false).Post("C",true));
			}
			else if (i == 3) {
				// Same as previous, but B=B(0) and C=(B1)
				// This test is for testing parameter parsing, comparison, etc.
				params("use_params") = true;
				atoms	.Add("A", false);
				atoms	.Add("B(0)", false);
				atoms	.Add("B(1)", false);
				goal	.Add("A", true);
				actions	.Add("write A (via B(0))", ActionEventValue().Cost(5).Pre("A",false).Pre("B(0)",true).Post("A",true));
				actions	.Add("write A (via B(1))", ActionEventValue().Pre("A",false).Pre("B(1)",true).Post("A",true));
				actions	.Add("write B(0)", ActionEventValue().Pre("B(0)",false).Post("B(0)",true));
				actions	.Add("write B(1)", ActionEventValue().Cost(2).Pre("B(1)",false).Post("B(1)",true));
			}
			else if (i == 4) {
				// Same as previous, but the "id" param is added
				params("use_params") = true;
				params("use_resolver") = true;
				atoms	.Add("A(id)", false);
				atoms	.Add("B(id,boolean)", false);
				goal	.Add("A(\"abc\")", true); // the 'id' should be resolved to be 'abc'
				actions	.Add("write A via B(id,0)", ActionEventValue().Cost(5).Pre("A(id)",false).Pre("B(id,0)",true).Post("A(id)",true));
				actions	.Add("write A via B(id,1)", ActionEventValue().Pre("A(id)",false).Pre("B(id,1)",true).Post("A(id)",true));
				actions	.Add("write B(id,0)", ActionEventValue().Pre("B(id,0)",false).Post("B(id,0)",true));
				actions	.Add("write B(id,1)", ActionEventValue().Cost(2).Pre("B(id,1)",false).Post("B(id,1)",true));
			}
			params("atoms") = atoms;
			params("actions") = actions;
			params("goal") = goal;
			params("dump_intermediate_trees") = true;
			searcher.SetSearcherParams(params);
			searcher.SetGeneratorParams(params, Null);
			searcher.SetSearchStrategy(SEARCHSTRATEGY_ASTAR);
			searcher.SetTerminalTest(TERMTEST_ACTION_PLANNER);
			searcher.SetGenerator(GENERATOR_ACTION_PLANNER);
			searcher.SetHeuristics(HEURISTIC_ACTION_PLANNER);
			searcher.ClearFS();
			searcher.WhenGenerated.Clear();
			searcher.WhenError = [](String s) {LOG("ActionPlanner error: " << s);};
			bool ret = searcher.RunSearch();
			if (!ret) {
				LOG(searcher.GetTreeString());
			}
			ASSERT(ret);
			//LOG(PtrVecStr(searcher.GetResult()));
			LOG(searcher.GetResultString());
		}
	}
	
	// Action planner
	if (all) {
		ValueMap atoms;
		atoms.Add("armed with claws", true);
		atoms.Add("mouse visible",    false);
		atoms.Add("near mouse",       false);
		atoms.Add("at high place",    true);
		atoms.Add("claws extended",   false);
		atoms.Add("ready to attack",  false);
		atoms.Add("mouse alive",      true);
		atoms.Add("alive",            true);
		
		ValueMap goal;
		goal.Add("mouse alive",		  false);
		goal.Add("alive",			  true); // add this to avoid hurting by 'fall' action in the plan.
		goal.Add("near mouse",		  false);
		
		ValueMap actions;
		actions.Add("cat", ActionEventValue()
			.Pre("armed with claws", true)
			.Post("mouse visible", true));
		actions.Add("approach", ActionEventValue()
			.Pre("mouse visible", true)
			.Post("near mouse", true));
		actions.Add("come down", ActionEventValue()
			.Pre("at high place", true)
			.Post("at high place", false));
		actions.Add("aim", ActionEventValue()
			.Pre("mouse visible", true)
			.Pre("claws extended", true)
			.Post("ready to attack", true));
		actions.Add("attack", ActionEventValue()
			.Pre("ready to attack", true)
			.Pre("at high place", false)
			.Pre("near mouse", true)
			.Post("mouse alive", false));
		actions.Add("prepare claws", ActionEventValue()
			.Pre("armed with claws", true)
			.Post("claws extended", true));
		actions.Add("very high jump attack", ActionEventValue()
			.Pre("at high place", true)
			.Pre("near mouse", true)
			.Post("alive", false)
			.Post("mouse alive", false)
			.Cost(5));
		actions.Add("flee", ActionEventValue()
			.Pre("mouse visible", true)
			.Post("near mouse", false));
		
		params("atoms") = atoms;
		params("actions") = actions;
		params("goal") = goal;
		params("dump_intermediate_trees") = false;
		searcher.SetSearcherParams(params);
		searcher.SetGeneratorParams(params, Null);
		searcher.SetSearchStrategy(SEARCHSTRATEGY_ASTAR);
		searcher.SetTerminalTest(TERMTEST_ACTION_PLANNER);
		searcher.SetGenerator(GENERATOR_ACTION_PLANNER);
		searcher.SetHeuristics(HEURISTIC_ACTION_PLANNER);
		searcher.ClearFS();
		searcher.WhenGenerated.Clear();
		searcher.WhenError = [](String s) {LOG("ActionPlanner error: " << s);};
		bool ret = searcher.RunSearch();
		if (!ret) {
			LOG(searcher.GetFS().GetTreeString());
		}
		ASSERT(ret);
		LOG(searcher.GetResultString());
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
