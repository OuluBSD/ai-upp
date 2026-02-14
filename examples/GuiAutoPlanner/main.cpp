#include <CtrlLib/CtrlLib.h>
#include <AI/Core/Base/Base.h>

using namespace Upp;

// This example demonstrates an Action Planner controlling GUI widgets directly.
// The planner treats widget states as atoms in its world state.

struct GuiAutoPlanner : public TopWindow {
	// Widgets
	MenuBar   menu;
	StatusBar status;
	DocEdit   editor;
	SliderCtrl slider;
	Switch    toggle;
	Button    runBtn, resetBtn;
	DropList  goalSelector;
	
	// Speed control
	SliderCtrl speedSlider;
	Label      speedLabel;
	
	// Layout containers
	ParentCtrl mainArea;
	StaticRect sideBar;
	Splitter   split;
	
	// App State
	ActionPlanner planner;
	
	GuiAutoPlanner() {
		Title("GUI Automation Action Planner");
		SetRect(0, 0, 900, 600);
		
		// Setup Frame
		AddFrame(menu);
		AddFrame(status);
		
		menu.Set([=](Bar& bar) {
			bar.Sub("File", [=](Bar& bar) {
				bar.Add("Exit", [=] { Break(); });
			});
		});
		
		// Layout
		Add(split.SizePos());
		split << mainArea << sideBar;
		split.SetPos(7000);
		
		// Main Area Widgets
		mainArea.Add(editor.VSizePos(10, 100).HSizePos(10, 10));
		mainArea.Add(slider.BottomPos(50, 30).HSizePos(10, 10));
		mainArea.Add(toggle.BottomPos(10, 30).LeftPos(10, 200));
		
		slider.Range(100);
		slider.SetData(0);
		toggle.SetLabel("Turbo Mode");
		
		// Sidebar Controls
		sideBar.Add(goalSelector.TopPos(10, 30).HSizePos(10, 10));
		sideBar.Add(runBtn.TopPos(50, 30).HSizePos(10, 10));
		sideBar.Add(resetBtn.TopPos(90, 30).HSizePos(10, 10));
		
		sideBar.Add(speedLabel.BottomPos(50, 20).HSizePos(10, 10));
		sideBar.Add(speedSlider.BottomPos(20, 30).HSizePos(10, 10));
		
		speedLabel.SetLabel("Execution Delay (ms):");
		speedSlider.Range(2000);
		speedSlider.SetData(600);
		
		goalSelector.Add("Text: 'HELLO', Slider: 50");
		goalSelector.Add("Turbo ON, Slider: 100");
		goalSelector.Add("Text: 'RESET', Turbo OFF, Slider: 0");
		goalSelector.SetIndex(0);
		
		runBtn.SetLabel("Execute Goal");
		runBtn << [=] { ExecuteGoal(); };
		
		resetBtn.SetLabel("Manual Reset");
		resetBtn << [=] {
			editor.Clear();
			slider.SetData(0);
			toggle.SetData(0);
			status.Set("");
		};
	}
	
	void ExecuteGoal() {
		runBtn.Disable();
		status.Set("Planning...");
		
		static ActionPlanner planner;
		planner.Clear();
		ActionPlannerWrapper wrap(planner);
		
		// Define Atoms (Widget States)
		// We discretize the slider and text for the planner
		wrap.SetAtom(0, "text_hello");
		wrap.SetAtom(1, "text_reset");
		wrap.SetAtom(2, "slider_mid");
		wrap.SetAtom(3, "slider_high");
		wrap.SetAtom(4, "slider_low");
		wrap.SetAtom(5, "turbo_on");
		
		// Actions: GUI Interactions
		// Text actions
		wrap.SetPostCondition("Type 'HELLO'", "text_hello", true);
		wrap.SetPostCondition("Type 'HELLO'", "text_reset", false);
		wrap.SetCost("Type 'HELLO'", 5);
		
		wrap.SetPostCondition("Clear & Type 'RESET'", "text_reset", true);
		wrap.SetPostCondition("Clear & Type 'RESET'", "text_hello", false);
		wrap.SetCost("Clear & Type 'RESET'", 5);
		
		// Slider actions
		wrap.SetPostCondition("Move Slider to 50", "slider_mid", true);
		wrap.SetPostCondition("Move Slider to 50", "slider_high", false);
		wrap.SetPostCondition("Move Slider to 50", "slider_low", false);
		wrap.SetCost("Move Slider to 50", 2);
		
		wrap.SetPostCondition("Move Slider to 100", "slider_high", true);
		wrap.SetPostCondition("Move Slider to 100", "slider_mid", false);
		wrap.SetPostCondition("Move Slider to 100", "slider_low", false);
		wrap.SetCost("Move Slider to 100", 3);
		
		wrap.SetPostCondition("Move Slider to 0", "slider_low", true);
		wrap.SetPostCondition("Move Slider to 0", "slider_mid", false);
		wrap.SetPostCondition("Move Slider to 0", "slider_high", false);
		wrap.SetCost("Move Slider to 0", 1);
		
		// Turbo actions
		wrap.SetPostCondition("Enable Turbo", "turbo_on", true);
		wrap.SetCost("Enable Turbo", 1);
		wrap.SetPostCondition("Disable Turbo", "turbo_on", false);
		wrap.SetCost("Disable Turbo", 1);
		
		// Map current GUI state to BinaryWorldState
		BinaryWorldState start;
		start.mask = wrap.GetMask();
		
		String currentText = editor.Get();
		if (currentText == "HELLO") start.SetAtomIndex(wrap.GetAtomIndex("text_hello"), true);
		if (currentText == "RESET") start.SetAtomIndex(wrap.GetAtomIndex("text_reset"), true);
		
		int val = (int)slider.GetData();
		if (val >= 90) start.SetAtomIndex(wrap.GetAtomIndex("slider_high"), true);
		else if (val >= 40) start.SetAtomIndex(wrap.GetAtomIndex("slider_mid"), true);
		else start.SetAtomIndex(wrap.GetAtomIndex("slider_low"), true);
		
		if ((int)toggle.GetData() == 1) start.SetAtomIndex(wrap.GetAtomIndex("turbo_on"), true);
		
		// Goal Selection
		BinaryWorldState goal = start;
		int goalIdx = goalSelector.GetIndex();
		if (goalIdx == 0) {
			goal.SetAtomIndex(wrap.GetAtomIndex("text_hello"), true);
			goal.SetAtomIndex(wrap.GetAtomIndex("slider_mid"), true);
		} else if (goalIdx == 1) {
			goal.SetAtomIndex(wrap.GetAtomIndex("turbo_on"), true);
			goal.SetAtomIndex(wrap.GetAtomIndex("slider_high"), true);
		} else {
			goal.SetAtomIndex(wrap.GetAtomIndex("text_reset"), true);
			goal.SetAtomIndex(wrap.GetAtomIndex("turbo_on"), false);
			goal.SetAtomIndex(wrap.GetAtomIndex("slider_low"), true);
		}
		
		// Simple BFS for Action Sequence
		struct Step : Moveable<Step> {
			BinaryWorldState state;
			Vector<int> actions;
		};
		Array<Step> queue;
		queue.Add().state = start;
		Index<hash_t> visited;
		visited.Add(start.GetHashValue());
		
		Vector<int> finalPlan;
		while(!queue.IsEmpty()) {
			Step curr = pick(queue[0]); queue.Remove(0);
			if (curr.state == goal) {
				finalPlan = pick(curr.actions);
				break;
			}
			for(int i = 0; i < planner.GetEventCount(); i++) {
				BinaryWorldState next;
				planner.DoAction(i, curr.state, next);
				if (visited.Find(next.GetHashValue()) < 0) {
					visited.Add(next.GetHashValue());
					Step& ns = queue.Add();
					ns.state = next;
					ns.actions <<= curr.actions;
					ns.actions.Add(i);
				}
			}
		}
		
		if (finalPlan.IsEmpty()) {
			Exclamation("No plan found to reach this GUI state!");
			runBtn.Enable();
			return;
		}
		
		// Map IDs to Names for the thread (as wrapper is non-copyable)
		std::vector<String> actionNames;
		for(int id : finalPlan) actionNames.push_back(wrap.GetActionName(id));
		
		int delay = (int)speedSlider.GetData();
		
		// Execution Loop (Async to avoid blocking UI)
		Thread().Run([=, plan = std::move(actionNames)]() mutable {
			for(int i = 0; i < plan.size(); i++) {
				const String& name = plan[i];
				PostCallback([=] {
					status.Set(Format("Step %d/%d: %s", i + 1, (int)plan.size(), name));
					if (name == "Type 'HELLO'") editor.Set(WString("HELLO"));
					else if (name == "Clear & Type 'RESET'") editor.Set(WString("RESET"));
					else if (name == "Move Slider to 50") slider.SetData(50);
					else if (name == "Move Slider to 100") slider.SetData(100);
					else if (name == "Move Slider to 0") slider.SetData(0);
					else if (name == "Enable Turbo") toggle.SetData(1);
					else if (name == "Disable Turbo") toggle.SetData(0);
				});
				
				Sleep(delay); // Visual delay to see the "automation"
			}
			PostCallback([=] { 
				runBtn.Enable(); 
				status.Set("Execution completed.");
			});
		});
	}
};

GUI_APP_MAIN {
	GuiAutoPlanner().Run();
}
