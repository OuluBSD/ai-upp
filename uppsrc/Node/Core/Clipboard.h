#ifndef _Node_Core_Clipboard_h_
#define _Node_Core_Clipboard_h_

#include "Core.h"

namespace Upp {

namespace Node {

struct ClipboardPayload : public Moveable<ClipboardPayload> {
	GraphDoc subgraph;
	int      version = 1;
	
	void Jsonize(JsonIO& jio) {
		jio("subgraph", subgraph)("version", version);
	}
};

String StoreClipboard(const Graph& graph, const Index<EntityId>& ids);
bool   LoadClipboard(Graph& graph, const String& data, Pointf paste_pos, Vector<ValidationMessage>& errors);

} // namespace Node

} // namespace Upp

#endif
