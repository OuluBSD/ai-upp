#include <Node/Core/Core.h>

using namespace Upp;
using namespace Upp::Node;

#define TCHECK(cond) \
	if(!(cond)) { LOG("FAIL: " #cond " (" __FILE__ ":" + AsString(__LINE__) + ")"); failures++; }

static int failures = 0;

void TestEntityId()
{
	TCHECK(IsValidEntityId("abc"));
	TCHECK(IsValidEntityId("n_123"));
	TCHECK(IsValidEntityId("a-b-c"));
	TCHECK(!IsValidEntityId(""));
	TCHECK(!IsValidEntityId("has space"));
	TCHECK(!IsValidEntityId("has.dot"));
}

void TestNodeCRUD()
{
	Graph g;
	NodeDoc& n = g.AddNode("n1");
	n.label = "Hello";
	n.pos = Pointf(10, 20);

	TCHECK(g.FindNode("n1") != nullptr);
	TCHECK(g.FindNode("n1")->label == "Hello");
	TCHECK(g.FindNode("n1")->pos.x == 10);
	TCHECK(g.FindNode("missing") == nullptr);

	g.RemoveNode("n1");
	TCHECK(g.FindNode("n1") == nullptr);
}

void TestEdgeCRUD()
{
	Graph g;
	g.AddNode("a");
	g.AddNode("b");
	g.AddEdge("e1", "a", "p1", "b", "p2");

	TCHECK(g.FindEdge("e1") != nullptr);
	TCHECK(g.FindEdge("e1")->source_node == "a");
	TCHECK(g.FindEdge("e1")->target_node == "b");

	// Removing source node cascades to edge
	g.RemoveNode("a");
	TCHECK(g.FindEdge("e1") == nullptr);
}

void TestGroupCRUD()
{
	Graph g;
	g.AddNode("n1");
	GroupDoc& gr = g.AddGroup("g1");
	gr.nodes.Add("n1");

	TCHECK(g.FindGroup("g1") != nullptr);
	g.RemoveGroup("g1");
	TCHECK(g.FindGroup("g1") == nullptr);
}

void TestValidation()
{
	Graph g;
	g.AddNode("n1");
	g.AddNode("n2");
	g.AddEdge("e1", "n1", "", "n2", "");

	// Valid graph → no errors
	Vector<ValidationMessage> msgs = g.Validate();
	bool has_error = false;
	for(const auto& m : msgs)
		if(m.severity == ValidationMessage::ERROR) has_error = true;
	TCHECK(!has_error);

	// Dangling edge check: add edge pointing to nonexistent node via doc directly
	EdgeDoc& bad = g.GetDoc().edges.Add();
	bad.id = "bad";
	bad.source_node = "n1";
	bad.source_pin = "";
	bad.target_node = "ghost";
	bad.target_pin = "";

	msgs = g.Validate();
	has_error = false;
	for(const auto& m : msgs)
		if(m.severity == ValidationMessage::ERROR && m.entity_id == "bad") has_error = true;
	TCHECK(has_error);
}

void TestSerial()
{
	Graph g;
	uint64 s0 = g.GetSerial();
	g.AddNode("n1");
	TCHECK(g.GetSerial() > s0);
	uint64 s1 = g.GetSerial();
	g.Invalidate("n1");
	TCHECK(g.GetSerial() > s1);
	TCHECK(g.GetDirtyEntities().Find("n1") >= 0);
}

void TestJsonRoundtrip()
{
	Graph g;
	NodeDoc& n = g.AddNode("n1");
	n.label = "Test";
	n.pos = Pointf(5, 7);
	PinDoc& p = n.pins.Add();
	p.id = "p1";
	p.kind = PinKind::Output;

	g.AddNode("n2");
	g.AddEdge("e1", "n1", "p1", "n2", "");

	String json = g.SaveJson();
	TCHECK(!json.IsEmpty());

	Graph g2;
	Vector<ValidationMessage> errs;
	TCHECK(g2.LoadJson(json, errs));
	TCHECK(g2.FindNode("n1") != nullptr);
	TCHECK(g2.FindNode("n1")->label == "Test");
	TCHECK(g2.FindEdge("e1") != nullptr);
	TCHECK(g2.FindEdge("e1")->source_node == "n1");
}

void TestXmlRoundtrip()
{
	Graph g;
	g.AddNode("x1").label = "XNode";
	g.AddNode("x2");
	g.AddEdge("xe1", "x1", "", "x2", "");

	String xml = g.SaveXml();
	TCHECK(!xml.IsEmpty());

	Graph g2;
	Vector<ValidationMessage> errs;
	TCHECK(g2.LoadXml(xml, errs));
	TCHECK(g2.FindNode("x1") != nullptr);
	TCHECK(g2.FindNode("x1")->label == "XNode");
	TCHECK(g2.FindEdge("xe1") != nullptr);
}

CONSOLE_APP_MAIN
{
	TestEntityId();
	TestNodeCRUD();
	TestEdgeCRUD();
	TestGroupCRUD();
	TestValidation();
	TestSerial();
	TestJsonRoundtrip();
	TestXmlRoundtrip();

	if(failures)
		LOG("Node1: " + AsString(failures) + " FAILURE(S)");
	else
		LOG("Node1: all tests passed");

	SetExitCode(failures ? 1 : 0);
}
