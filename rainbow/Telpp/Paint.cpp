#include "Local.h"

#ifdef GUI_TELPP

NAMESPACE_UPP

void Ctrl::DragRectDraw(const Rect& rect1, const Rect& rect2, const Rect& clip, int n,
                        Color color, int type, int animation)
{
	static byte solid[] =  { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	static byte normal[] = { 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00 };
	static byte dashed[] = { 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00 };
	const byte *pattern = type == DRAWDRAGRECT_DASHED ? dashed :
	                      type == DRAWDRAGRECT_NORMAL ? normal : solid;
	RemoveCursor();
	RemoveCaret();
	DragRectDraw0(GetPaintRects(), rect1, n, pattern, animation);
	DragRectDraw0(GetPaintRects(), rect2, n, pattern, animation);
}

END_UPP_NAMESPACE

#endif
