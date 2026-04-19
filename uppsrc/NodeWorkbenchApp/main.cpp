#include <NodeWorkbench/NodeWorkbench.h>
#include <Node/Script/Script.h>

using namespace Upp;

// --dump-graph <path>  — headless: open a graph file, print stats, exit 0 on success
static int DumpGraph(const String& path) {
	if (!FileExists(path)) {
		Cout() << "ERROR: file not found: " << path << "\n";
		return 1;
	}

	Node::Graph g;
	Vector<Node::ValidationMessage> msgs;
	bool ok = Node::LoadEonFile(g, path, msgs);

	// Print any load messages
	for (auto& m : msgs)
		Cout() << (m.severity == Node::ValidationMessage::ERROR ? "ERROR" :
		           m.severity == Node::ValidationMessage::WARNING ? "WARNING" : "INFO")
		       << ": " << m.message << "\n";

	const Node::GraphDoc& doc = g.GetDoc();
	int n_nodes = doc.nodes.GetCount();
	int n_edges = doc.edges.GetCount();

	Cout() << "File    : " << path << "\n";
	Cout() << "Nodes   : " << n_nodes << "\n";
	Cout() << "Edges   : " << n_edges << "\n";

	if (n_nodes > 0) {
		Cout() << "\nNode list:\n";
		for (const Node::NodeDoc& nd : doc.nodes) {
			Cout() << "  [" << nd.id << "]"
			       << "  type=" << nd.node_type_id
			       << "  pins=" << nd.pins.GetCount()
			       << "  slots=" << nd.slots.GetCount()
			       << "\n";
		}
	}

	if (n_edges > 0) {
		Cout() << "\nConnection list:\n";
		for (const Node::EdgeDoc& ed : doc.edges) {
			Cout() << "  " << ed.source_node << "." << ed.source_pin
			       << " -> " << ed.target_node << "." << ed.target_pin
			       << "\n";
		}
	}

	Cout() << "\nResult  : " << (ok ? "OK" : "LOAD FAILED") << "\n";
	return ok && n_nodes > 0 ? 0 : 2;
}

GUI_APP_MAIN {
	// Headless mode: --dump-graph <path>
	const Vector<String>& args = CommandLine();
	for (int i = 0; i < args.GetCount(); i++) {
		if (args[i] == "--dump-graph" && i + 1 < args.GetCount()) {
			SetExitCode(DumpGraph(args[i + 1]));
			return;
		}
	}

	NodeWorkbenchWindow win;
	if (!args.IsEmpty()) {
		String path = args[0];
		if (FileExists(path))
			win.OpenPath(path);
	}
	win.Run();
}
