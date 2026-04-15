#include <Node/Core/Core.h>
#include <Node/Core/Viewport.h>
#include <Node/Core/Scene.h>
#include <Node/Core/Layout.h>
#include <Node/Core/Algo.h>
#include <Node/Core/Snap.h>
#include <Node/Core/Editor.h>
#include <Node/Core/Command.h>

using namespace Upp;
using namespace Upp::Node;

void CommandTest()
{
	Graph g;
	EditorState es;
	CommandContext ctx(g, es);
	CommandDispatcher disp;
	disp.RegisterStandardCommands();
	
	ValueMap arg;
	arg.Add("id", "n1");
	arg.Add("x", 100.0);
	arg.Add("y", 200.0);
	
	auto res = disp.Execute("AddNode", ctx, arg);
	ASSERT(res);
	ASSERT(g.FindNode("n1"));
	ASSERT(g.FindNode("n1")->pos.x == 100.0);
	
	ValueMap arg2;
	arg2.Add("id", "n1");
	arg2.Add("x", 150.0);
	arg2.Add("y", 250.0);
	res = disp.Execute("MoveNode", ctx, arg2);
	ASSERT(res);
	ASSERT(g.FindNode("n1")->pos.x == 150.0);
	
	ValueMap arg3;
	arg3.Add("id", "n2");
	res = disp.Execute("RemoveNode", ctx, arg3);
	ASSERT(!res); // n2 doesn't exist
	
	LOG("CommandTest passed.");
}

void EditorTest()
{
	EditorState es;
	ASSERT(es.mode == EditorMode::READY);
	
	es.Select("n1");
	ASSERT(es.IsSelected("n1"));
	
	es.mode = EditorMode::DRAGGING;
	es.drag_start = Pointf(10, 10);
	
	LOG("EditorTest passed.");
}

void IdTest()
{
	ASSERT(IsValidEntityId("node1"));
	ASSERT(IsValidEntityId("NODE-1"));
	ASSERT(IsValidEntityId("node_1"));
	ASSERT(!IsValidEntityId(""));
	ASSERT(!IsValidEntityId(" "));
	ASSERT(!IsValidEntityId("node 1"));
	ASSERT(!IsValidEntityId("node.1"));
	ASSERT(!IsValidEntityId("node@1"));
	
	LOG("IdTest passed.");
}

void DocTest()
{
	GraphDoc doc;
	doc.version = 1;
	
	NodeDoc& n1 = doc.nodes.Add();
	n1.id = "n1";
	n1.label = "Node 1";
	n1.pos = Pointf(10, 20);
	
	PinDoc& p1 = n1.pins.Add();
	p1.id = "p1";
	p1.kind = PinKind::Output;
	
	NodeDoc& n2 = doc.nodes.Add();
	n2.id = "n2";
	n2.label = "Node 2";
	n2.pos = Pointf(100, 200);
	
	PinDoc& p2 = n2.pins.Add();
	p2.id = "p1"; // local ID can be same across nodes
	p2.kind = PinKind::Input;
	
	EdgeDoc& e = doc.edges.Add();
	e.id = "e1";
	e.source_node = "n1";
	e.source_pin = "p1";
	e.target_node = "n2";
	e.target_pin = "p1";
	
	String json = StoreAsJson(doc);
	LOG("JSON: " << json);
	
	GraphDoc doc2;
	LoadFromJson(doc2, json);
	
	ASSERT(doc2.version == 1);
	ASSERT(doc2.nodes.GetCount() == 2);
	ASSERT(doc2.nodes[0].id == "n1");
	ASSERT(doc2.nodes[0].pins.GetCount() == 1);
	ASSERT(doc2.edges.GetCount() == 1);
	ASSERT(doc2.edges[0].source_node == "n1");
	
	LOG("DocTest passed.");
}

void GraphTest()
{
	Graph g;
	g.AddNode("n1");
	g.AddNode("n2");
	
	NodeDoc* n1 = g.FindNode("n1");
	ASSERT(n1);
	n1->pins.Add().id = "p1";
	
	NodeDoc* n2 = g.FindNode("n2");
	ASSERT(n2);
	n2->pins.Add().id = "p1";
	
	g.AddEdge("e1", "n1", "p1", "n2", "p1");
	ASSERT(g.FindEdge("e1"));
	
	auto msgs = g.Validate();
	ASSERT(msgs.IsEmpty());
	
	g.RemoveNode("n1");
	ASSERT(!g.FindNode("n1"));
	ASSERT(!g.FindEdge("e1")); // Edge should be removed automatically
	
	// Test invalid edge
	g.AddNode("n1");
	g.AddEdge("e1", "n1", "p1", "n2", "p1"); // n1 exists but p1 is missing now
	msgs = g.Validate();
	ASSERT(!msgs.IsEmpty());
	ASSERT(msgs[0].severity == ValidationMessage::ERROR);
	
	LOG("GraphTest passed.");
}

void RoundtripTest()
{
	Graph g;
	g.AddNode("n1").label = "Node 1";
	g.AddNode("n2").label = "Node 2";
	g.FindNode("n1")->pins.Add().id = "p1";
	g.FindNode("n2")->pins.Add().id = "p1";
	g.AddEdge("e1", "n1", "p1", "n2", "p1");
	
	String json = g.SaveJson();
	
	Graph g2;
	Vector<ValidationMessage> errors;
	ASSERT(g2.LoadJson(json, errors));
	ASSERT(errors.IsEmpty());
	
	ASSERT(g2.FindNode("n1")->label == "Node 1");
	ASSERT(g2.FindEdge("e1")->source_node == "n1");
	
	String xml = g.SaveXml();
	Graph g3;
	errors.Clear();
	ASSERT(g3.LoadXml(xml, errors));
	ASSERT(errors.IsEmpty());
	ASSERT(g3.FindNode("n1")->label == "Node 1");
	
	LOG("RoundtripTest passed.");
}

void ViewportTest()
{
	Viewport vp;
	vp.SetOffset(Pointf(100, 100));
	vp.SetScale(2.0);
	
	Pointf w(150, 150);
	Pointf v = vp.WorldToView(w);
	ASSERT(v.x == 100 && v.y == 100); // (150-100)*2 = 100
	
	Pointf w2 = vp.ViewToWorld(v);
	ASSERT(w2.x == 150 && w2.y == 150);
	
	// Test Zoom at focus
	vp.Zoom(4.0, Pointf(100, 100)); // Zoom at view point (100,100), which corresponds to world (150,150)
	ASSERT(vp.GetScale() == 4.0);
	Pointf w3 = vp.ViewToWorld(Pointf(100, 100));
	ASSERT(abs(w3.x - 150) < 1e-9 && abs(w3.y - 150) < 1e-9);
	
	LOG("ViewportTest passed.");
}

void SceneTest()
{
	Graph g;
	NodeDoc& n1 = g.AddNode("n1");
	n1.pos = Pointf(10, 10);
	n1.sz = Sizef(100, 50);
	n1.label = "Hello";
	
	BaselineSceneBuilder builder;
	Scene scene;
	builder.Build(scene, g);
	
	ASSERT(scene.items.GetCount() >= 2); // Node + Label
	
	bool found_node = false;
	bool found_label = false;
	for(const auto& item : scene.items) {
		if(item.type == SceneItem::NODE && item.entity_id == "n1") found_node = true;
		if(item.type == SceneItem::LABEL && item.text == "Hello") found_label = true;
	}
	ASSERT(found_node);
	ASSERT(found_label);
	
	LOG("SceneTest passed.");
}

void HitTestTest()
{
	Graph g;
	NodeDoc& n1 = g.AddNode("n1");
	n1.pos = Pointf(10, 10);
	n1.sz = Sizef(100, 50);
	
	PinDoc& p1 = n1.pins.Add();
	p1.id = "p1";
	p1.pos = Pointf(0, 25); // local
	p1.sz = Sizef(10, 10);
	
	BaselineSceneBuilder builder;
	Scene scene;
	builder.Build(scene, g);
	
	// Hit node
	auto res = scene.HitTest(Pointf(50, 30));
	ASSERT(res.entity_id == "n1");
	ASSERT(res.type == SceneItem::NODE);
	
	// Hit pin (precedence)
	res = scene.HitTest(Pointf(10, 35)); // pin is at world (10, 35)
	ASSERT(res.entity_id == "n1:p1");
	ASSERT(res.type == SceneItem::PIN);
	
	// Marquee
	auto ids = scene.MarqueeSelect(Rectf(0, 0, 200, 200));
	ASSERT(ids.GetCount() == 1);
	ASSERT(ids[0] == "n1");
	
	LOG("HitTestTest passed.");
}

void LayoutTest()
{
	Graph g;
	g.AddNode("n1");
	g.AddNode("n2");
	NodeDoc* n1 = g.FindNode("n1");
	NodeDoc* n2 = g.FindNode("n2");
	n1->pins.Add().id = "p1";
	n2->pins.Add().id = "p1";
	g.AddEdge("e1", "n1", "p1", "n2", "p1");
	
	Vector<NodeState> states;
	SpringLayout layout;
	layout.Iterations(100).Seed(1234);
	layout.Run(g, states);
	
	ASSERT(states.GetCount() == 2);
	// Basic check: nodes should have moved from initial (0,0), (10,10)
	ASSERT(states[0].layout_pos != Pointf(0, 0));
	ASSERT(states[1].layout_pos != Pointf(10, 10));
	
	LOG("LayoutTest passed.");
}

void AlgoTest()
{
	Graph g;
	g.AddNode("n1");
	g.AddNode("n2");
	g.AddNode("n3");
	g.AddEdge("e1", "n1", "p1", "n2", "p1");
	g.AddEdge("e2", "n2", "p1", "n3", "p1");
	
	// TopSort
	auto order = TopologicalSort(g);
	ASSERT(order.GetCount() == 3);
	ASSERT(order[0] == "n1" && order[1] == "n2" && order[2] == "n3");
	
	// Dijkstra
	auto paths = Dijkstra(g, "n1");
	ASSERT(paths.GetCount() == 3);
	for(const auto& p : paths) {
		if(p.entity_id == "n3") {
			ASSERT(p.distance == 2.0);
			ASSERT(p.predecessor == "n2");
		}
	}
	
	LOG("AlgoTest passed.");
}

void LayeredLayoutTest()
{
	Graph g;
	g.AddNode("n1");
	g.AddNode("n2");
	g.AddNode("n3");
	g.AddEdge("e1", "n1", "p1", "n2", "p1");
	g.AddEdge("e2", "n1", "p1", "n3", "p1");
	g.AddEdge("e3", "n2", "p1", "n3", "p1");
	
	Vector<NodeState> states;
	LayeredLayout(g, states);
	
	ASSERT(states.GetCount() == 3);
	// n1 (rank 0), n2 (rank 1), n3 (rank 2)
	ASSERT(states[0].layout_pos.x == 0);
	ASSERT(states[1].layout_pos.x == 200);
	ASSERT(states[2].layout_pos.x == 400);
	
	LOG("LayeredLayoutTest passed.");
}

void SnapTest()
{
	// Grid Snap
	SnapRequest req;
	req.point = Pointf(12, 18);
	req.grid_step = 10.0;
	req.tolerance = 5.0;
	
	auto res = SnapToGrid(req);
	ASSERT(res.snapped_point.x == 10.0);
	ASSERT(res.snapped_point.y == 20.0);
	ASSERT(res.snapped_x && res.snapped_y);
	
	// Guides
	Graph g;
	g.AddNode("n1").pos = Pointf(100, 100);
	g.FindNode("n1")->sz = Sizef(100, 100);
	
	GuideRequest greq;
	greq.moving_rect = Rectf(Pointf(202, 100), Sizef(100, 100)); // Near n1.right (200)
	greq.tolerance = 5.0;
	
	auto guides = FindGuides(g, greq);
	ASSERT(guides.GetCount() > 0);
	bool found_v = false;
	for(const auto& gd : guides) {
		if(gd.orientation == Guide::VERTICAL && gd.pos == 200.0) found_v = true;
	}
	ASSERT(found_v);
	
	LOG("SnapTest passed.");
}

void UndoRedoTest()
{
	Graph g;
	EditorState es;
	CommandContext ctx(g, es);
	CommandDispatcher disp;
	disp.RegisterStandardCommands();
	HistoryStack history;
	
	ValueMap arg;
	arg.Add("id", "n1");
	arg.Add("x", 10.0);
	arg.Add("y", 10.0);
	
	history.Execute(ctx, disp.Create("AddNode", arg));
	ASSERT(g.FindNode("n1"));
	
	history.Undo(ctx);
	ASSERT(!g.FindNode("n1"));
	
	history.Redo(ctx);
	ASSERT(g.FindNode("n1"));
	
	// Transaction
	history.Begin();
	ValueMap arg2; arg2.Add("id", "n2"); arg2.Add("x", 50.0); arg2.Add("y", 50.0);
	history.Execute(ctx, disp.Create("AddNode", arg2));
	
	ValueMap arg3; arg3.Add("id", "n1"); arg3.Add("x", 100.0); arg3.Add("y", 100.0);
	history.Execute(ctx, disp.Create("MoveNode", arg3));
	history.Commit();
	
	ASSERT(g.FindNode("n2"));
	ASSERT(g.FindNode("n1")->pos.x == 100.0);
	
	history.Undo(ctx); // Should undo BOTH AddNode(n2) and MoveNode(n1)
	ASSERT(!g.FindNode("n2"));
	ASSERT(g.FindNode("n1")->pos.x == 10.0);
	
	LOG("UndoRedoTest passed.");
}

void SelectionTest()
{
	Graph g;
	EditorState es;
	CommandContext ctx(g, es);
	CommandDispatcher disp;
	disp.RegisterStandardCommands();
	HistoryStack history;
	
	g.AddNode("n1");
	g.AddNode("n2");
	
	ValueMap arg;
	arg.Add("id", "n1");
	arg.Add("exclusive", true);
	history.Execute(ctx, disp.Create("Select", arg));
	ASSERT(es.IsSelected("n1"));
	
	arg.Set("id", "n2");
	arg.Set("exclusive", true);
	history.Execute(ctx, disp.Create("Select", arg));
	ASSERT(es.IsSelected("n2"));
	ASSERT(!es.IsSelected("n1")); // exclusive
	
	history.Undo(ctx);
	ASSERT(es.IsSelected("n1"));
	ASSERT(!es.IsSelected("n2"));
	
	arg.Set("id", "n2");
	arg.Set("exclusive", false);
	history.Execute(ctx, disp.Create("Select", arg));
	ASSERT(es.IsSelected("n1") && es.IsSelected("n2")); // non-exclusive
	
	ValueMap arg2;
	arg2.Add("id", "n1");
	history.Execute(ctx, disp.Create("ToggleSelection", arg2));
	ASSERT(!es.IsSelected("n1"));
	ASSERT(es.IsSelected("n2"));
	
	LOG("SelectionTest passed.");
}

#include <Node/Core/Migration.h>

void MigrationTest()
{
	String legacy = "{\"nodes\":[{\"id\":\"n1\",\"label\":\"Old Node\",\"pos\":{\"x\":10,\"y\":20},\"sz\":{\"cx\":100,\"cy\":50},\"fill_clr\":{\"red\":255,\"green\":0,\"blue\":0},\"pins\":[{\"id\":\"p1\",\"label\":\"In\",\"kind\":0,\"pos\":{\"x\":0,\"y\":25}}]}],\"edges\":[]}";
	
	Graph g;
	ImportResult res = ImportGraphLib(g, legacy);
	ASSERT(res.success);
	
	NodeDoc* n = g.FindNode("n1");
	ASSERT(n);
	ASSERT(n->label == "Old Node");
	LOG("Imported color: " << n->fill_clr << " LtRed(): " << LtRed());
	ASSERT(n->fill_clr == LtRed());
	ASSERT(n->pins.GetCount() == 1);
	ASSERT(n->pins[0].kind == PinKind::Input);
	
	LOG("MigrationTest passed.");
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	
	IdTest();
	DocTest();
	GraphTest();
	RoundtripTest();
	ViewportTest();
	SceneTest();
	HitTestTest();
	LayoutTest();
	AlgoTest();
	LayeredLayoutTest();
	SnapTest();
	EditorTest();
	CommandTest();
	UndoRedoTest();
	SelectionTest();
	MigrationTest();
	
	LOG("All NodeCore tests passed.");
}
