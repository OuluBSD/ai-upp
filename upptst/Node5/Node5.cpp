#include <Node/Core/Migration.h>

using namespace Upp;
using namespace Upp::Node;

#define TCHECK(cond) \
	if(!(cond)) { LOG("FAIL: " #cond " (" __FILE__ ":" + AsString(__LINE__) + ")"); failures++; }

static int failures = 0;

// Build minimal valid legacy JSON
static String MakeLegacyJson(bool include_pins = false)
{
	String s = "{\"nodes\":[";
	s += "{\"id\":\"n1\",\"label\":\"Alpha\","
	     "\"pos\":{\"x\":10,\"y\":20},\"sz\":{\"cx\":80,\"cy\":40},"
	     "\"fill_clr\":{\"red\":255,\"green\":255,\"blue\":255}";
	if(include_pins)
		s += ",\"pins\":[{\"id\":\"out\",\"label\":\"Out\",\"kind\":1,\"pos\":{\"x\":80,\"y\":20}}]";
	s += "},";
	s += "{\"id\":\"n2\",\"label\":\"Beta\","
	     "\"pos\":{\"x\":200,\"y\":20},\"sz\":{\"cx\":80,\"cy\":40},"
	     "\"fill_clr\":{\"red\":200,\"green\":200,\"blue\":200}";
	if(include_pins)
		s += ",\"pins\":[{\"id\":\"in\",\"label\":\"In\",\"kind\":0,\"pos\":{\"x\":0,\"y\":20}}]";
	s += "}";
	s += "],\"edges\":[";
	s += "{\"id\":\"e1\",\"source_node\":\"n1\",\"source_pin\":\"out\","
	     "\"target_node\":\"n2\",\"target_pin\":\"in\","
	     "\"label\":\"\",\"weight\":1.5,"
	     "\"stroke_clr\":{\"red\":0,\"green\":0,\"blue\":0}}";
	s += "]}";
	return s;
}

void TestImportBasic()
{
	Graph g;
	ImportResult res = ImportGraphLib(g, MakeLegacyJson(true));
	TCHECK(res.success);
	TCHECK(g.FindNode("n1") != nullptr);
	TCHECK(g.FindNode("n1")->label == "Alpha");
	TCHECK(g.FindNode("n1")->pos.x == 10.0);
	TCHECK(g.FindNode("n2") != nullptr);
	TCHECK(g.FindEdge("e1") != nullptr);
	TCHECK(g.FindEdge("e1")->weight == 1.5);
}

void TestImportMissingFields()
{
	// Node missing pos, sz, fill_clr — should not crash, use defaults
	String json = "{\"nodes\":[{\"id\":\"x1\"}],\"edges\":[]}";
	Graph g;
	ImportResult res = ImportGraphLib(g, json);
	TCHECK(res.success);
	TCHECK(g.FindNode("x1") != nullptr);
	TCHECK(g.FindNode("x1")->pos.x == 0.0); // default
}

void TestImportMissingNodeId()
{
	// Node without id gets auto-assigned id
	String json = "{\"nodes\":[{\"label\":\"NoId\"}],\"edges\":[]}";
	Graph g;
	ImportResult res = ImportGraphLib(g, json);
	TCHECK(res.success);
	TCHECK(g.GetDoc().nodes.GetCount() == 1); // Still imported
	bool has_warning = false;
	for(const auto& w : res.warnings)
		if(w.severity == ValidationMessage::WARNING) { has_warning = true; break; }
	TCHECK(has_warning);
}

void TestImportDanglingEdge()
{
	// Edge pointing to non-existent node should be skipped with warning
	String json = "{\"nodes\":[{\"id\":\"a\"}],"
	              "\"edges\":[{\"id\":\"e1\",\"source_node\":\"a\","
	              "\"source_pin\":\"\",\"target_node\":\"ghost\",\"target_pin\":\"\","
	              "\"weight\":1.0}]}";
	Graph g;
	ImportResult res = ImportGraphLib(g, json);
	TCHECK(res.success);
	TCHECK(g.FindEdge("e1") == nullptr); // skipped
	bool has_warning = false;
	for(const auto& w : res.warnings)
		if(w.entity_id == "e1") { has_warning = true; break; }
	TCHECK(has_warning);
}

void TestImportDuplicateNodeId()
{
	// Duplicate node id: second occurrence should be skipped with warning
	String json = "{\"nodes\":[{\"id\":\"dup\"},{\"id\":\"dup\"}],\"edges\":[]}";
	Graph g;
	ImportResult res = ImportGraphLib(g, json);
	TCHECK(res.success);
	TCHECK(g.GetDoc().nodes.GetCount() == 1); // only first
}

void TestImportBadJson()
{
	Graph g;
	ImportResult res = ImportGraphLib(g, "not json {{{");
	TCHECK(!res.success);
}

void TestLegacyFacadeBasic()
{
	Graph g;
	LegacyFacade f(g);
	EntityId n1 = f.AddNode("Hello", Pointf(0, 0));
	EntityId n2 = f.AddNode("World", Pointf(100, 0));
	TCHECK(!n1.IsEmpty());
	TCHECK(!n2.IsEmpty());
	TCHECK(g.FindNode(n1) != nullptr);
	TCHECK(g.FindNode(n1)->label == "Hello");
}

void TestLegacyFacadeEdgeNoPins()
{
	// Nodes with no pins: AddEdge should use "" for both pin IDs, not crash
	Graph g;
	LegacyFacade f(g);
	EntityId n1 = f.AddNode("A", Pointf(0, 0));
	EntityId n2 = f.AddNode("B", Pointf(0, 0));
	EntityId eid = f.AddEdge(n1, n2);
	TCHECK(!eid.IsEmpty());
	TCHECK(g.FindEdge(eid) != nullptr);
	TCHECK(g.FindEdge(eid)->source_pin == ""); // no pins → empty
}

void TestLegacyFacadeEdgeWithPins()
{
	// Nodes with pins: AddEdge should pick the first pin
	Graph g;
	LegacyFacade f(g);
	EntityId n1 = f.AddNode("A", Pointf(0, 0));
	EntityId n2 = f.AddNode("B", Pointf(0, 0));

	// Add pins directly to the nodes
	NodeDoc* nd1 = g.FindNode(n1);
	NodeDoc* nd2 = g.FindNode(n2);
	PinDoc& p1 = nd1->pins.Add(); p1.id = "out1"; p1.kind = PinKind::Output;
	PinDoc& p2 = nd2->pins.Add(); p2.id = "in1";  p2.kind = PinKind::Input;

	EntityId eid = f.AddEdge(n1, n2);
	TCHECK(!eid.IsEmpty());
	TCHECK(g.FindEdge(eid)->source_pin == "out1");
	TCHECK(g.FindEdge(eid)->target_pin == "in1");
}

void TestLegacyFacadeClear()
{
	Graph g;
	LegacyFacade f(g);
	f.AddNode("X", Pointf(0, 0));
	f.Clear();
	TCHECK(g.GetDoc().nodes.GetCount() == 0);
}

CONSOLE_APP_MAIN
{
	TestImportBasic();
	TestImportMissingFields();
	TestImportMissingNodeId();
	TestImportDanglingEdge();
	TestImportDuplicateNodeId();
	TestImportBadJson();
	TestLegacyFacadeBasic();
	TestLegacyFacadeEdgeNoPins();
	TestLegacyFacadeEdgeWithPins();
	TestLegacyFacadeClear();

	if(failures)
		LOG("Node5: " + AsString(failures) + " FAILURE(S)");
	else
		LOG("Node5: all tests passed");

	SetExitCode(failures ? 1 : 0);
}
