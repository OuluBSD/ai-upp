#include "FormEditor.h"

NAMESPACE_UPP

void FormView::CreateObject(Point p, const char* type)
{
	if (!IsLayout())
		return;

	p = DeZoom(p);

	Rect pageRect = GetPageRect();
	Rect objRect;

	if (GetBool("Grid.Binding", true))
		PointToGrid(p);

	if (AsString(type) == "Button")
		objRect = Rect(Point(p.x - pageRect.left, p.y - pageRect.top),
			Size(GetNumber("Grid.CX", 10) * 10, GetNumber("Grid.CY", 10) * 3)
		);
	else
		objRect = Rect(Point(p.x - pageRect.left, p.y - pageRect.top),
			Size(GetNumber("Grid.CX", 10) * 10, GetNumber("Grid.CY", 10) * 2)
		);

	pageRect = Deoffseted(GetPageRect());
	if (objRect.right  > pageRect.right)  objRect.Offset(- objRect.right + pageRect.right, 0);
	if (objRect.bottom > pageRect.bottom) objRect.Offset(0, - objRect.bottom + pageRect.bottom);

	FormObject obj(objRect);
	obj.Set("Type", type);

	if (AsString(type) == "EditField")
		obj.Set("TextAlign", "Left");

	GetObjects()->Add(obj);

	ClearSelection();
	AddToSelection(GetObjectCount() - 1);

	Refresh();
	WhenUpdate();
	WhenChildSelected(GetSelected());
	WhenChildCount(GetObjectCount());
}

static void AlignFromAnchor(const String& anchor, dword& h, dword& v)
{
	if(anchor == "CENTER") {
		h = Ctrl::CENTER;
		v = Ctrl::CENTER;
	}
	else if(anchor == "TOP_LEFT") {
		h = Ctrl::LEFT;
		v = Ctrl::TOP;
	}
	else if(anchor == "TOP_CENTER") {
		h = Ctrl::CENTER;
		v = Ctrl::TOP;
	}
	else if(anchor == "TOP_RIGHT") {
		h = Ctrl::RIGHT;
		v = Ctrl::TOP;
	}
	else if(anchor == "CENTER_LEFT") {
		h = Ctrl::LEFT;
		v = Ctrl::CENTER;
	}
	else if(anchor == "CENTER_RIGHT") {
		h = Ctrl::RIGHT;
		v = Ctrl::CENTER;
	}
	else if(anchor == "BOTTOM_LEFT") {
		h = Ctrl::LEFT;
		v = Ctrl::BOTTOM;
	}
	else if(anchor == "BOTTOM_CENTER") {
		h = Ctrl::CENTER;
		v = Ctrl::BOTTOM;
	}
	else if(anchor == "BOTTOM_RIGHT") {
		h = Ctrl::RIGHT;
		v = Ctrl::BOTTOM;
	}
	else if(anchor == "TOP_HSIZE") {
		h = Ctrl::SIZE;
		v = Ctrl::TOP;
	}
	else if(anchor == "CENTER_HSIZE") {
		h = Ctrl::SIZE;
		v = Ctrl::CENTER;
	}
	else if(anchor == "BOTTOM_HSIZE") {
		h = Ctrl::SIZE;
		v = Ctrl::BOTTOM;
	}
	else if(anchor == "LEFT_VSIZE") {
		h = Ctrl::LEFT;
		v = Ctrl::SIZE;
	}
	else if(anchor == "CENTER_VSIZE") {
		h = Ctrl::CENTER;
		v = Ctrl::SIZE;
	}
	else if(anchor == "RIGHT_VSIZE") {
		h = Ctrl::RIGHT;
		v = Ctrl::SIZE;
	}
	else if(anchor == "SIZE") {
		h = Ctrl::SIZE;
		v = Ctrl::SIZE;
	}
}

static Rect AbsoluteRectFromStored(const Rect& r, dword h, dword v, const Size& base_sz)
{
	int x = r.left;
	int y = r.top;
	int cx = r.Width();
	int cy = r.Height();

	if(h == Ctrl::RIGHT)
		x = base_sz.cx - r.left - cx;
	else if(h == Ctrl::CENTER)
		x = (base_sz.cx - cx) / 2 + r.left;

	if(v == Ctrl::BOTTOM)
		y = base_sz.cy - r.top - cy;
	else if(v == Ctrl::CENTER)
		y = (base_sz.cy - cy) / 2 + r.top;

	return RectC(x, y, cx, cy);
}

static Rect StoredRectFromAbsolute(const Rect& abs, dword h, dword v, const Size& base_sz)
{
	int x = abs.left;
	int y = abs.top;
	int cx = abs.Width();
	int cy = abs.Height();

	if(h == Ctrl::RIGHT)
		x = base_sz.cx - abs.right;
	else if(h == Ctrl::CENTER)
		x = abs.left - (base_sz.cx - cx) / 2;

	if(v == Ctrl::BOTTOM)
		y = base_sz.cy - abs.bottom;
	else if(v == Ctrl::CENTER)
		y = abs.top - (base_sz.cy - cy) / 2;

	return RectC(x, y, cx, cy);
}

static String AnchorFromAlign(dword h, dword v)
{
	if(h == Ctrl::CENTER && v == Ctrl::CENTER) return "CENTER";
	if(h == Ctrl::LEFT   && v == Ctrl::TOP)    return "TOP_LEFT";
	if(h == Ctrl::CENTER && v == Ctrl::TOP)    return "TOP_CENTER";
	if(h == Ctrl::RIGHT  && v == Ctrl::TOP)    return "TOP_RIGHT";
	if(h == Ctrl::LEFT   && v == Ctrl::CENTER) return "CENTER_LEFT";
	if(h == Ctrl::RIGHT  && v == Ctrl::CENTER) return "CENTER_RIGHT";
	if(h == Ctrl::LEFT   && v == Ctrl::BOTTOM) return "BOTTOM_LEFT";
	if(h == Ctrl::CENTER && v == Ctrl::BOTTOM) return "BOTTOM_CENTER";
	if(h == Ctrl::RIGHT  && v == Ctrl::BOTTOM) return "BOTTOM_RIGHT";
	if(h == Ctrl::SIZE   && v == Ctrl::TOP)    return "TOP_HSIZE";
	if(h == Ctrl::SIZE   && v == Ctrl::CENTER) return "CENTER_HSIZE";
	if(h == Ctrl::SIZE   && v == Ctrl::BOTTOM) return "BOTTOM_HSIZE";
	if(h == Ctrl::LEFT   && v == Ctrl::SIZE)   return "LEFT_VSIZE";
	if(h == Ctrl::CENTER && v == Ctrl::SIZE)   return "CENTER_VSIZE";
	if(h == Ctrl::RIGHT  && v == Ctrl::SIZE)   return "RIGHT_VSIZE";
	if(h == Ctrl::SIZE   && v == Ctrl::SIZE)   return "SIZE";
	return Null;
}

void FormView::SetSprings(dword hAlign, dword vAlign)
{
	if (!IsLayout())
		return;

	Vector<int> sel = GetSelected();
	Size base_sz = GetCurrentLayout()->GetFormSize();
	for (int i = 0; i < sel.GetCount(); ++i)
	{
		FormObject* pI = GetObject(sel[i]);
		if (!pI) continue;

		dword curH = pI->GetHAlign();
		dword curV = pI->GetVAlign();
		AlignFromAnchor(pI->Get("Anchor"), curH, curV);
		Rect abs = AbsoluteRectFromStored(pI->GetRect(), curH, curV, base_sz);

		dword newH = (hAlign != (dword)-1) ? hAlign : curH;
		dword newV = (vAlign != (dword)-1) ? vAlign : curV;
		Rect stored = StoredRectFromAbsolute(abs, newH, newV, base_sz);

		pI->SetHAlign(newH);
		pI->SetVAlign(newV);
		pI->SetRect(stored);

		// Update Anchor string to match new alignment combination
		String anchor = AnchorFromAlign(newH, newV);
		if(!IsNull(anchor))
			pI->Set("Anchor", anchor);
	}

	if(hAlign != (dword)-1) _HAlign = hAlign;
	if(vAlign != (dword)-1) _VAlign = vAlign;

	WhenUpdate();
	Refresh();
}

void FormView::RemoveSelection()
{
	if (!IsLayout())
		return;

	Vector<int> sel = GetSelected();
	Sort(sel);

	for (int i = sel.GetCount() - 1; i >= 0; --i)
		GetObjects()->Remove(sel[i]);

	Refresh();
	WhenChildCount(GetObjectCount());
}

void FormView::MoveUpObject(int id)
{
	if (!IsLayout())
		return;

	if (id < 0 || id >= GetObjectCount() - 1)
		return;

	GetObjects()->Swap(id, id + 1);
	Refresh();
	WhenChildZ();
}

void FormView::MoveDownObject(int id)
{
	if (!IsLayout())
		return;

	if (id <= 0 || id >= GetObjectCount())
		return;

	GetObjects()->Swap(id, id - 1);
	Refresh();
	WhenChildZ();
}

void FormView::MoveToTopObject(int id)
{
	if (!IsLayout())
		return;

	if (id < 0 || id >= GetObjectCount() - 1)
		return;

	for (int i = id; i < GetObjectCount() - 1; ++i)
		GetObjects()->Swap(i, i + 1);
	Refresh();
	WhenChildZ();
}

void FormView::MoveToBottomObject(int id)
{
	if (!IsLayout())
		return;

	if (id <= 0 || id >= GetObjectCount())
		return;

	for (int i = id; i > 0; --i)
		GetObjects()->Swap(i, i - 1);
	Refresh();
	WhenChildZ();
}

void FormView::InvertSelection()
{
	if (!IsLayout())
		return;

	for (int i = 0; i < GetObjectCount(); ++i)
	{
		FormObject* p = &(*GetObjects())[i];
		p->GetState() == FormObject::SELECTED
		   ? p->SetState(FormObject::NONE)
		   : p->SetState(FormObject::SELECTED);
	}
	Refresh();
}

void FormView::AlignTopSelection()
{
	if (!IsLayout())
		return;

	int min = Deoffseted(GetSelectionRect()).top;
	Vector<int> sel = GetSelected();

	for (int i = 0; i < sel.GetCount(); ++i)
	{
		FormObject* p = GetObject(sel[i]);
		if (!p) continue;
		p->SetRect( Rect(Point(p->GetRect().left, min), p->GetSize()) );
	}

	WhenChildPos(sel);
}

void FormView::AlignLeftSelection()
{
	if (!IsLayout())
		return;

	int min = Deoffseted(GetSelectionRect()).left;
	Vector<int> sel = GetSelected();

	for (int i = 0; i < sel.GetCount(); ++i)
	{
		FormObject* p = GetObject(sel[i]);
		if (!p) continue;
		p->SetRect( Rect(Point(min, p->GetRect().top), p->GetSize()) );
	}

	WhenChildPos(sel);
}

void FormView::AlignRightSelection()
{
	if (!IsLayout())
		return;

	int min = Deoffseted(GetSelectionRect()).right;
	Vector<int> sel = GetSelected();

	for (int i = 0; i < sel.GetCount(); ++i)
	{
		FormObject* p = GetObject(sel[i]);
		if (!p) continue;
		p->SetRect( Rect(Point(min - p->GetRect().Width(), p->GetRect().top), p->GetSize()) );
	}

	WhenChildPos(sel);
}

void FormView::AlignBottomSelection()
{
	if (!IsLayout())
		return;

	int min = Deoffseted(GetSelectionRect()).bottom;
	Vector<int> sel = GetSelected();

	for (int i = 0; i < sel.GetCount(); ++i)
	{
		FormObject* p = GetObject(sel[i]);
		if (!p) continue;
		p->SetRect( Rect(Point(p->GetRect().left, min - p->GetRect().Height()), p->GetSize()) );
	}

	WhenChildPos(sel);
}

void FormView::ToggleOutlineDraw(int obj)
{
	if (!IsLayout())
		return;

	FormObject* pI = GetObject(obj);
	if (!pI) return;

	pI->SetBool("OutlineDraw", !pI->GetBool("OutlineDraw", false));
	WhenChildZ();
}

bool FormView::IsOutlineDraw(int obj)
{
	if (!IsLayout())
		return false;

	FormObject* pI = GetObject(obj);
	if (!pI) return false;

	return pI->GetBool("OutlineDraw", false);
}

END_UPP_NAMESPACE
