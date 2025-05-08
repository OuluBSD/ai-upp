#include "EonDraw.h"


NAMESPACE_UPP namespace Ecs {


void ToolSelectorKey::Visit(Vis& v) {
	
	TODO //e % type; // TypeCls to string
	
}

void ToolComponent::Visit(Vis& v) {
	v & active_tool & active_hand;
	v && tools;
}

} END_UPP_NAMESPACE

