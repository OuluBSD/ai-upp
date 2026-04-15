#include <Node/Core/Core.h>

using namespace Upp;
using namespace Upp::Node;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	
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
