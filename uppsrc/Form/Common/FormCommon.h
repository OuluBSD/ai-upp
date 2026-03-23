#ifndef FORM_COMMON_H
#define FORM_COMMON_H

#include <Core/Core.h>

NAMESPACE_UPP

enum FormPlacementConstants {
	FORM_CENTER = 0,
	FORM_MIDDLE = 0,
	FORM_LEFT   = 1,
	FORM_RIGHT  = 2,
	FORM_TOP    = 1,
	FORM_BOTTOM = 2,
	FORM_SIZE   = 3,
};

void ResolveAnchorLayout(Rect& r, dword& h_align, dword& v_align, const String& anchor, const Size& base_sz);

END_UPP_NAMESPACE

#endif
