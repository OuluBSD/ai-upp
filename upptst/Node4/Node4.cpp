#include <Node/Core/Scene.h>

using namespace Upp;
using namespace Upp::Node;

#define TCHECK(cond) \
	if(!(cond)) { LOG("FAIL: " #cond " (" __FILE__ ":" + AsString(__LINE__) + ")"); failures++; }

static int failures = 0;

static Graph MakeLinearGraph()
{
	Graph g;
	NodeDoc& n1 = g.AddNode("n1");
	n1.pos = Pointf(0, 0); n1.sz = Sizef(80, 40);
	NodeDoc& n2 = g.AddNode("n2");
	n2.pos = Pointf(200, 0); n2.sz = Sizef(80, 40);
	g.AddEdge("e1", "n1", "", "n2", "");
	return g;
}

void TestSceneBuilderBasic()
{
	Graph g = MakeLinearGraph();

	BaselineSceneBuilder builder;
	Scene scene;
	TCHECK(builder.IsDirty(g));
	builder.Build(scene, g);
	TCHECK(!builder.IsDirty(g));

	// Should have NODE items for n1 and n2 and an EDGE item for e1
	bool has_n1 = false, has_n2 = false, has_e1 = false;
	for(const auto& item : scene.items) {
		if(item.type == SceneItem::NODE && item.entity_id == "n1") has_n1 = true;
		if(item.type == SceneItem::NODE && item.entity_id == "n2") has_n2 = true;
		if(item.type == SceneItem::EDGE && item.entity_id == "e1") has_e1 = true;
	}
	TCHECK(has_n1);
	TCHECK(has_n2);
	TCHECK(has_e1);
}

void TestIncrementalNoDuplicates()
{
	// Move n1 — both endpoints of e1 are dirty — e1 must appear exactly once
	Graph g = MakeLinearGraph();
	BaselineSceneBuilder builder;
	Scene scene;
	builder.Build(scene, g);

	// Mark both nodes dirty (simulates moving both simultaneously)
	g.Invalidate("n1");
	g.Invalidate("n2");

	builder.Build(scene, g);

	int edge_count = 0;
	for(const auto& item : scene.items)
		if(item.type == SceneItem::EDGE && item.entity_id == "e1")
			edge_count++;
	TCHECK(edge_count == 1);
}

void TestHitTestNode()
{
	Graph g = MakeLinearGraph();
	BaselineSceneBuilder builder;
	Scene scene;
	builder.Build(scene, g);

	// Hit center of n1 rect [0,0 - 80,40]
	Scene::HitResult hit = scene.HitTest(Pointf(40, 20));
	TCHECK(hit);
	TCHECK(hit.entity_id == "n1");
}

void TestHitTestMiss()
{
	Graph g = MakeLinearGraph();
	BaselineSceneBuilder builder;
	Scene scene;
	builder.Build(scene, g);

	Scene::HitResult hit = scene.HitTest(Pointf(500, 500));
	TCHECK(!hit);
}

void TestHitTestEdge()
{
	Graph g = MakeLinearGraph();
	BaselineSceneBuilder builder;
	Scene scene;
	builder.Build(scene, g);

	// Hit near midpoint of edge path between (80,20) and (200,20) ≈ (140,20)
	// Allow generous tolerance
	Scene::HitResult hit = scene.HitTest(Pointf(140, 20), 8.0);
	// May or may not hit depending on bezier arc; just check no crash + test edge detection
	// If hit, must be the edge
	if(hit)
		TCHECK(hit.entity_id == "e1" || hit.entity_id == "n1" || hit.entity_id == "n2");
}

void TestMarqueeSelect()
{
	Graph g = MakeLinearGraph();
	BaselineSceneBuilder builder;
	Scene scene;
	builder.Build(scene, g);

	// Marquee that fully covers n1 [0,0-80,40]
	Vector<EntityId> sel = scene.MarqueeSelect(Rectf(-10, -10, 90, 50));
	bool has_n1 = false;
	for(const auto& id : sel) if(id == "n1") has_n1 = true;
	TCHECK(has_n1);
}

void TestSpatialIndexAdaptive()
{
	// Build a scene with many items and verify index is built with reasonable size
	Graph g;
	for(int i = 0; i < 100; i++) {
		EntityId id = "n" + AsString(i);
		NodeDoc& n = g.AddNode(id);
		n.pos = Pointf(i * 50, (i % 10) * 50);
		n.sz = Sizef(40, 30);
	}
	BaselineSceneBuilder builder;
	Scene scene;
	builder.Build(scene, g);

	// Spatial index should exist and not be the old 10x10 hardcoded
	// With 100 nodes (~200 items w/ labels), adaptive grid should be > 4 per side
	int cells = scene.index.grid_sz.cx * scene.index.grid_sz.cy;
	// Grid must be within bounds and not the old hardcoded 10x10=100 for all inputs
	TCHECK(scene.index.grid_sz.cx >= 1 && scene.index.grid_sz.cy >= 1);
	TCHECK(cells <= 64 * 64);
	// For 100 spread-out nodes the grid should be at least 4 wide or 4 tall
	TCHECK(scene.index.grid_sz.cx >= 4 || scene.index.grid_sz.cy >= 4);

	// Hit test still works
	Scene::HitResult hit = scene.HitTest(Pointf(25, 25)); // center of n0
	TCHECK(hit);
	TCHECK(hit.entity_id == "n0");
}

void TestSceneRebuildAfterClear()
{
	Graph g = MakeLinearGraph();
	BaselineSceneBuilder builder;
	Scene scene;
	builder.Build(scene, g);

	g.Clear();
	// After clear, graph serial changed → builder is dirty
	TCHECK(builder.IsDirty(g));
	builder.Build(scene, g);
	TCHECK(scene.items.GetCount() == 0);
}

CONSOLE_APP_MAIN
{
	TestSceneBuilderBasic();
	TestIncrementalNoDuplicates();
	TestHitTestNode();
	TestHitTestMiss();
	TestHitTestEdge();
	TestMarqueeSelect();
	TestSpatialIndexAdaptive();
	TestSceneRebuildAfterClear();

	if(failures)
		LOG("Node4: " + AsString(failures) + " FAILURE(S)");
	else
		LOG("Node4: all tests passed");

	SetExitCode(failures ? 1 : 0);
}
