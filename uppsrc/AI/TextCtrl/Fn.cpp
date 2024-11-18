#include "TextCtrl.h"


NAMESPACE_UPP


void SetColoredListValue(ArrayCtrl& list, int row, int col, const String& txt, Color clr, bool blend) {
	if (blend) {
		list.Set(row, col, AttrText(txt)
			.NormalPaper(Blend(clr, White(), 128+64)).NormalInk(Black())
			.Paper(Blend(clr, GrayColor())).Ink(White())
		);
	}
	else {
		list.Set(row, col, AttrText(txt)
			.NormalPaper(clr).NormalInk(Black())
			.Paper(Blend(clr, GrayColor(64))).Ink(White())
		);
	}
}


END_UPP_NAMESPACE
