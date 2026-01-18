#include "Draw.h"


NAMESPACE_UPP


void ToolSelectorKey::Visit(Vis& v) {
	
	TODO //e % type; // TypeCls to string
	
}

void ToolComponent::Visit(Vis& v) {
	v & active_tool & active_hand;
	v && tools;
	// Note: hand_path is only used during Arg() and Initialize(), no need to serialize

	TODO
	#if 0
	e % title
	  % description;
	
	EtherizeRef(e, active_tool);
	EtherizeRefContainer(e, tools);
	EtherizeRef(e, active_hand);
	
	TODO // tool_type TypeCId
	#endif
}


END_UPP_NAMESPACE

