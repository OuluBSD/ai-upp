#include <CtrlLib/CtrlLib.h>
#include <AI/Core/Base/Base.h>

using namespace Upp;

struct ActionPlannerGui : public TopWindow {
	ActionPlanner planner;
	ActionPlannerWrapper wrap;
	
	MenuBar menu;
	Splitter split;
	ArrayCtrl actionList;
	DocEdit output;
	
	ActionPlannerGui() : wrap(planner) {
		Title("ActionPlanner GUI Example");
		SetRect(0, 0, 800, 600);
		
		Add(menu.TopPos(0, 30).HSizePos());
		Add(split.VSizePos(30, 0).HSizePos());
		
		actionList.AddColumn("Action Name");
		actionList.AddColumn("Cost");
		
		split << actionList << output;
		split.SetPos(3000);
		
		menu.Set([=] (Bar& bar) {
			bar.Add("Setup Scenario", [=] { SetupScenario(); });
			bar.Add("Plan", [=] { RunPlan(); });
			bar.Add("Clear", [=] { ClearScenario(); });
		});
		
		SetupScenario();
	}
	
	void ClearScenario() {
		planner.Clear();
		actionList.Clear();
		output.Clear();
		SetupScenario();
	}
	
	void SetupScenario() {
		planner.Clear();
		
		// Define some actions for a "Robot in a House" scenario
		// Action: Move to Kitchen
		wrap.SetPostCondition("Move to Kitchen", "at_kitchen", true);
		wrap.SetCost("Move to Kitchen", 5);
		
		// Action: Pick up Apple (requires being in kitchen)
		wrap.SetPreCondition("Pick up Apple", "at_kitchen", true);
		wrap.SetPostCondition("Pick up Apple", "has_apple", true);
		wrap.SetCost("Pick up Apple", 2);
		
		// Action: Move to Table
		wrap.SetPostCondition("Move to Table", "at_table", true);
		wrap.SetCost("Move to Table", 5);
		
		// Action: Place Apple on Table
		wrap.SetPreCondition("Place Apple on Table", "at_table", true);
		wrap.SetPreCondition("Place Apple on Table", "has_apple", true);
		wrap.SetPostCondition("Place Apple on Table", "apple_on_table", true);
		wrap.SetCost("Place Apple on Table", 1);
		
		RefreshActions();
		output.Append("Scenario loaded: Get an apple from the kitchen to the table.\n");
	}
	
	void RefreshActions() {
		actionList.Clear();
		for(int i = 0; i < planner.GetEventCount(); i++) {
			actionList.Add(wrap.GetActionName(i), "??"); 
		}
	}
	
	void RunPlan() {
		output.Append("\n--- Planning ---\n");
		
		// Simple BFS solver for the demo
		struct MyNode : Moveable<MyNode> {
			BinaryWorldState state;
			Vector<int> plan;
			double cost = 0;
			
			MyNode() {}
			void Pick(MyNode&& n) {
				state = pick(n.state);
				plan = pick(n.plan);
				cost = n.cost;
			}
		};
		
		Array<MyNode> queue;
		MyNode& start = queue.Add();
		start.state.mask = wrap.GetMask();
		
		Index<hash_t> visited;
		visited.Add(start.state.GetHashValue());
		
		int goal_atom = wrap.GetAtomIndex("apple_on_table");
		
		while(!queue.IsEmpty()) {
			MyNode current;
			current.Pick(pick(queue[0]));
			queue.Remove(0);
			
			// Goal Check: apple_on_table == true
			if (current.state.atoms.GetCount() > goal_atom && 
			    current.state.atoms[goal_atom].in_use && 
			    current.state.atoms[goal_atom].value) {
				
				output.Append("Found optimal plan (Cost: " + AsString(current.cost) + "):\n");
				for(int act_id : current.plan) {
					output.Append("  -> " + wrap.GetActionName(act_id) + "\n");
				}
				return;
			}
			
			Array<BinaryWorldState*> next_states;
			Vector<int> act_ids;
			Vector<double> action_costs;
			planner.GetPossibleStateTransition(current.state, next_states, act_ids, action_costs);
			
			for(int i = 0; i < next_states.GetCount(); i++) {
				hash_t h = next_states[i]->GetHashValue();
				if(visited.Find(h) < 0) {
					visited.Add(h);
					MyNode& next = queue.Add();
					next.state = *next_states[i];
					next.plan <<= current.plan;
					next.plan.Add(act_ids[i]);
					next.cost = current.cost + action_costs[i];
				}
			}
		}
		
		output.Append("No plan found.\n");
	}
};

GUI_APP_MAIN {
	ActionPlannerGui().Run();
}