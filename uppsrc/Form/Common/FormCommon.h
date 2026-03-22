#ifndef FORM_COMMON_H
#define FORM_COMMON_H

#include <Core/Core.h>

NAMESPACE_UPP

#ifndef flagGUI
struct Ctrl {
	enum PlacementConstants {
		CENTER   = 0,
		MIDDLE   = 0,
		LEFT     = 1,
		RIGHT    = 2,
		TOP      = 1,
		BOTTOM   = 2,
		SIZE     = 3,
	};
};
#endif

void ResolveAnchorLayout(Rect& r, dword& h_align, dword& v_align, const String& anchor, const Size& base_sz);

END_UPP_NAMESPACE

#endif
