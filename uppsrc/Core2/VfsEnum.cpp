#include "Core.h"

NAMESPACE_UPP


bool IsMergeable(int kind)
{
	switch(kind) {
	// case Cursor_StructDecl:
	// case Cursor_ClassDecl:
	case Cursor_Namespace:
	case Cursor_LinkageSpec:
	case 0:
		return true;
	default:
		return false;
	}
}


END_UPP_NAMESPACE
