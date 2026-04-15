#ifndef _Node_Core_Migration_h_
#define _Node_Core_Migration_h_

#include "Core.h"

namespace Upp {

namespace Node {

struct ImportResult {
	bool success = false;
	Vector<ValidationMessage> warnings;
};

ImportResult ImportGraphLib(Graph& target, const String& legacy_json);

class LegacyFacade {
	Graph& target;
public:
	LegacyFacade(Graph& g) : target(g) {}
	
	// Legacy-like API
	EntityId AddNode(const String& label, Pointf pos);
	// src_pin / tgt_pin default to the first available pin on each node, or "" if none
	EntityId AddEdge(const EntityId& src, const EntityId& tgt,
	                 const EntityId& src_pin = String(), const EntityId& tgt_pin = String());
	void     Clear() { target.Clear(); }
};

} // namespace Node

} // namespace Upp

#endif
