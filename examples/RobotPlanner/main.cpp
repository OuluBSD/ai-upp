#include <CtrlLib/CtrlLib.h>
#include <AI/Core/Base/Base.h>
#include <vector>

using namespace Upp;

// --- Low-level A* Navigation ---

struct Point2D : Moveable<Point2D> {
	int x, y;
	Point2D(int x=0, int y=0) : x(x), y(y) {}
	bool operator==(const Point2D& b) const { return x == b.x && y == b.y; }
	bool operator!=(const Point2D& b) const { return !(*this == b); }
	hash_t GetHashValue() const { return CombineHash(x, y); }
	String ToString() const { return Format("(%d, %d)", x, y); }
};

struct GridMap {
	int w = 20, h = 20;
	Index<Point2D> obstacles;
	VectorMap<String, Point2D> locations;
	
	GridMap() {
		// Default map
		for(int i = 0; i < 12; i++) obstacles.Add(Point2D(7, i));
		for(int i = 8; i < 20; i++) obstacles.Add(Point2D(13, i));
		
		locations.Add("Home", Point2D(1, 1));
		locations.Add("Kitchen", Point2D(18, 2));
		locations.Add("Table", Point2D(10, 10));
		locations.Add("Storage", Point2D(2, 18));
	}
	
	bool IsPassable(Point2D p) const {
		return p.x >= 0 && p.x < w && p.y >= 0 && p.y < h && obstacles.Find(p) < 0;
	}
};

struct AStarNode {
	Point2D pt;
	double g = DBL_MAX, f = DBL_MAX;
	Point2D parent = Point2D(-1, -1);
};

std::vector<Point2D> FindPath(const GridMap& map, Point2D start, Point2D goal) {
	if(!map.IsPassable(start) || !map.IsPassable(goal)) return {};
	
	VectorMap<Point2D, AStarNode> nodes;
	auto GetNode = [&](Point2D p) -> AStarNode& {
		int q = nodes.Find(p);
		if(q < 0) {
			AStarNode& n = nodes.Add(p);
			n.pt = p;
			return n;
		}
		return nodes[q];
	};
	
	auto Heuristic = [&](Point2D p) { return (double)abs(p.x - goal.x) + abs(p.y - goal.y); };
	
	Index<Point2D> openSet;
	AStarNode& startNode = GetNode(start);
	startNode.g = 0;
	startNode.f = Heuristic(start);
	openSet.Add(start);
	
	while(openSet.GetCount() > 0) {
		int bestIdx = 0;
		for(int i = 1; i < openSet.GetCount(); i++)
			if(GetNode(openSet[i]).f < GetNode(openSet[bestIdx]).f)
				bestIdx = i;
		
		Point2D current = openSet[bestIdx];
		if(current == goal) {
			std::vector<Point2D> path;
			while(current != Point2D(-1, -1)) {
				path.insert(path.begin(), current);
				current = GetNode(current).parent;
			}
			return path;
		}
		
		openSet.Remove(bestIdx);
		
		static const int dx[] = {0, 0, 1, -1};
		static const int dy[] = {1, -1, 0, 0};
		for(int i = 0; i < 4; i++) {
			Point2D neighbor(current.x + dx[i], current.y + dy[i]);
			if(map.IsPassable(neighbor)) {
				double tg = GetNode(current).g + 1.0;
				AStarNode& nbNode = GetNode(neighbor);
				if(tg < nbNode.g) {
					nbNode.parent = current;
					nbNode.g = tg;
					nbNode.f = tg + Heuristic(neighbor);
					if(openSet.Find(neighbor) < 0)
						openSet.Add(neighbor);
				}
			}
		}
	}
	return {};
}

// --- Visual Grid Component ---

class GridCtrl : public Ctrl {
	GridMap* map = nullptr;
	std::vector<Point2D> path;
	Point2D robotPos;
	
public:
	Event<> WhenMapChanged;
	
	GridCtrl() { robotPos = Point2D(1, 1); }
	
	void SetMap(GridMap& m) { map = &m; Refresh(); }
	void SetPath(const std::vector<Point2D>& p) { path = p; Refresh(); }
	void SetRobot(Point2D p) { robotPos = p; Refresh(); }
	
	virtual void Paint(Draw& w) override {
		Size sz = GetSize();
		if(!map) { w.DrawRect(sz, White()); return; }
		
		int cw = sz.cx / map->w;
		int ch = sz.cy / map->h;
		
		w.DrawRect(sz, White());
		
		// Draw Grid
		for(int i = 0; i <= map->w; i++) w.DrawLine(i * cw, 0, i * cw, sz.cy, 1, GrayColor(220));
		for(int i = 0; i <= map->h; i++) w.DrawLine(0, i * ch, sz.cx, i * ch, 1, GrayColor(220));
		
		// Draw Obstacles
		for(int i = 0; i < map->obstacles.GetCount(); i++)
			w.DrawRect(map->obstacles[i].x * cw, map->obstacles[i].y * ch, cw, ch, GrayColor(100));
		
		// Draw Path
		if(!path.empty()) {
			for(int i = 0; i < (int)path.size() - 1; i++) {
				Point2D p1 = path[i];
				Point2D p2 = path[i+1];
				w.DrawLine(p1.x * cw + cw/2, p1.y * ch + ch/2, p2.x * cw + cw/2, p2.y * ch + ch/2, 3, LtBlue());
			}
		}
		
		// Draw Locations
		for(int i = 0; i < map->locations.GetCount(); i++) {
			Point2D p = map->locations[i];
			w.DrawEllipse(p.x * cw + 2, p.y * ch + 2, cw - 4, ch - 4, Green());
			w.DrawText(p.x * cw + 4, p.y * ch + 4, map->locations.GetKey(i), StdFont().Bold().Italic(), White());
		}
		
		// Draw Robot
		w.DrawEllipse(robotPos.x * cw + 4, robotPos.y * ch + 4, cw - 8, ch - 8, Red());
		w.DrawEllipse(robotPos.x * cw + cw/2 - 2, robotPos.y * ch + ch/2 - 2, 4, 4, White());
	}
	
	virtual void LeftDown(Point p, dword flags) override {
		if(!map) return;
		int cw = GetSize().cx / map->w;
		int ch = GetSize().cy / map->h;
		if (cw == 0 || ch == 0) return;
		int cx = p.x / cw;
		int cy = p.y / ch;
		Point2D pt(cx, cy);
		
		int q = map->obstacles.Find(pt);
		if(q >= 0) map->obstacles.Remove(q);
		else map->obstacles.Add(pt);
		
		WhenMapChanged();
		Refresh();
	}
};

// --- Main Application ---

struct RobotPlannerGui : public TopWindow {
	GridMap map;
	GridCtrl grid;
	DocEdit log;
	Splitter split;
	StaticRect leftPane;
	Button runBtn, resetBtn;
	
	Point2D robotPos;
	bool hasApple = false;
	bool isBusy = false;
	
	RobotPlannerGui() {
		Title("Robot A* + Action Planner Demo");
		SetRect(0, 0, 1000, 700);
		
		Add(split.SizePos());
		
		leftPane.Add(grid.VSizePos(0, 40).HSizePos());
		leftPane.Add(runBtn.BottomPos(5, 30).LeftPos(5, 100));
		leftPane.Add(resetBtn.BottomPos(5, 30).LeftPos(110, 100));
		
		split << leftPane << log;
		split.SetPos(7000);
		
		runBtn.SetLabel("Run Plan");
		runBtn << [=] { RunIntegratedPlan(); };
		
		resetBtn.SetLabel("Reset");
		resetBtn << [=] { Reset(); };
		
		grid.SetMap(map);
		grid.WhenMapChanged = [=] { RefreshCosts(); };
		
		Reset();
	}
	
	void Reset() {
		robotPos = map.locations.Get("Home");
		hasApple = false;
		grid.SetRobot(robotPos);
		grid.SetPath({});
		log.Clear();
		log.Append("Robot initialized at Home.\n");
		log.Append("Click on grid to toggle obstacles.\n");
	}
	
	void RefreshCosts() {
		// In a real app, we'd update high-level costs here
	}
	
	void RunIntegratedPlan() {
		if(isBusy) return;
		isBusy = true;
		runBtn.Disable();
		
		log.Append("\nCalculating High-level Action Plan...\n");
		
		// Define High-level state and actions for ActionPlanner
		static ActionPlanner planner;
		planner.Clear();
		ActionPlannerWrapper wrap(planner);
		
		// Locations: Home, Kitchen, Table, Storage
		for(int i = 0; i < map.locations.GetCount(); i++) {
			String loc = map.locations.GetKey(i);
			String act = "Goto " + loc;
			wrap.SetPostCondition(act, "at_" + ToLower(loc), true);
			for(int j = 0; j < map.locations.GetCount(); j++)
				if(i != j) wrap.SetPostCondition(act, "at_" + ToLower(map.locations.GetKey(j)), false);
			
			// Cost is dynamic via A*
			std::vector<Point2D> path = FindPath(map, robotPos, map.locations[i]);
			wrap.SetCost(act, !path.empty() ? (int)path.size() : 1000);
		}
		
		wrap.SetPreCondition("Pick Apple", "at_kitchen", true);
		wrap.SetPostCondition("Pick Apple", "has_apple", true);
		wrap.SetCost("Pick Apple", 2);
		
		wrap.SetPreCondition("Deliver Apple", "at_table", true);
		wrap.SetPreCondition("Deliver Apple", "has_apple", true);
		wrap.SetPostCondition("Deliver Apple", "apple_delivered", true);
		wrap.SetPostCondition("Deliver Apple", "has_apple", false);
		wrap.SetCost("Deliver Apple", 2);
		
		// Initial State
		BinaryWorldState start;
		start.mask = wrap.GetMask();
		start.SetAtomIndex(wrap.GetAtomIndex("at_home"), true);
		
		// Search for plan to "apple_delivered"
		BinaryWorldState goal = start;
		goal.SetAtomIndex(wrap.GetAtomIndex("apple_delivered"), true);
		
		// We use a simple BFS for the action sequence in this demo
		struct Step : Moveable<Step> {
			BinaryWorldState state;
			Vector<int> actions;
		};
		Array<Step> queue;
		Step& first = queue.Add();
		first.state = start;
		
		Index<hash_t> visited;
		visited.Add(start.GetHashValue());
		
		Vector<int> finalPlan;
		int appleDeliveredIdx = wrap.GetAtomIndex("apple_delivered");
		
		while(!queue.IsEmpty()) {
			Step curr = pick(queue[0]); queue.Remove(0);
			if(appleDeliveredIdx >= 0 && appleDeliveredIdx < curr.state.atoms.GetCount() && curr.state.atoms[appleDeliveredIdx].value) {
				finalPlan = pick(curr.actions);
				break;
			}
			for(int i = 0; i < planner.GetEventCount(); i++) {
				BinaryWorldState next;
				planner.DoAction(i, curr.state, next);
				if(visited.Find(next.GetHashValue()) < 0) {
					visited.Add(next.GetHashValue());
					Step& ns = queue.Add();
					ns.state = next;
					ns.actions <<= curr.actions;
					ns.actions.Add(i);
				}
			}
		}
		
		if(finalPlan.IsEmpty()) {
			log.Append("No plan found!\n");
			isBusy = false;
			runBtn.Enable();
			return;
		}
		
		log.Append("Action Plan Found: " + AsString(finalPlan.GetCount()) + " steps.\n");
		
		// Execute Plan with Animations
		Thread().Run([=, plan = pick(finalPlan)]() mutable {
			for(int actId : plan) {
				String name = (actId < 4) ? String("Goto ") + map.locations.GetKey(actId) : (actId == 4 ? String("Pick Apple") : String("Deliver Apple"));
				PostCallback([=]{ log.Append("Executing: " + name + "\n"); });
				
				if(name.StartsWith("Goto ")) {
					String loc = name.Mid(5);
					Point2D target = map.locations.Get(loc);
					std::vector<Point2D> path = FindPath(map, robotPos, target);
					
					for(const auto& pt : path) {
						robotPos = pt;
						PostCallback([=]{ grid.SetRobot(pt); grid.SetPath(path); });
						Sleep(100);
					}
				} else {
					if(name == "Pick Apple") hasApple = true;
					Sleep(500); // Simulate action
				}
			}
			PostCallback([=]{ 
				log.Append("Plan Completed Successfully!\n");
				isBusy = false;
				runBtn.Enable();
			});
		});
	}
};

GUI_APP_MAIN {

	RobotPlannerGui().Run();

}
