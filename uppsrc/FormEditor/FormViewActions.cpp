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

static String AnchorFromAlign(dword h, dword v)
{
	// SIZE has no named anchor; return Null to leave Anchor unchanged
	if(h == Ctrl::SIZE || v == Ctrl::SIZE)
		return Null;

	// Ctrl constants: CENTER=0, LEFT=1, RIGHT=2, TOP=1, BOTTOM=2
	static const char* hname[] = { "CENTER", "LEFT",   "RIGHT"  };
	static const char* vname[] = { "CENTER", "TOP",    "BOTTOM" };

	if(h == Ctrl::CENTER && v == Ctrl::CENTER) return "CENTER";
	if(h == Ctrl::CENTER) return String(vname[v]) + "_CENTER";  // TOP_CENTER, BOTTOM_CENTER
	if(v == Ctrl::CENTER) return String("CENTER_") + hname[h];  // CENTER_LEFT, CENTER_RIGHT
	return String(vname[v]) + "_" + hname[h];                   // TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT
}

void FormView::SetSprings(dword hAlign, dword vAlign)
{
	if (!IsLayout())
		return;

	Vector<int> sel = GetSelected();
	for (int i = 0; i < sel.GetCount(); ++i)
	{
		FormObject* pI = GetObject(sel[i]);
		if (!pI) continue;

		dword newH = (hAlign != (dword)-1) ? hAlign : pI->GetHAlign();
		dword newV = (vAlign != (dword)-1) ? vAlign : pI->GetVAlign();

		pI->SetHAlign(newH);
		pI->SetVAlign(newV);

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
