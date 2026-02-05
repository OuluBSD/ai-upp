#include "CtrlLib.h"

NAMESPACE_UPP

TreeArrayCtrl::Node::Node() {
	Init();
}

TreeArrayCtrl::Node::Node(const Image& img, const Value& v) {
	Init();
	image = img;
	key = value = v;
}

TreeArrayCtrl::Node::Node(const Image& img, const Value& v, const Value& t) {
	Init();
	image = img;
	key = v;
	value = t;
}

TreeArrayCtrl::Node::Node(const Value& v) {
	Init();
	key = value = v;
}

TreeArrayCtrl::Node::Node(const Value& v, const Value& t) {
	Init();
	key = v;
	value = t;
}

TreeArrayCtrl::Node::Node(Ctrl& ctrl) {
	Init();
	this->ctrl = &ctrl;
}

TreeArrayCtrl::Node::Node(const Image& img, Ctrl& ctrl, int cx, int cy) {
	Init();
	image = img;
	this->ctrl = &ctrl;
	size = Size(cx, cy);
}

void TreeArrayCtrl::TreeDisplay::Paint(Draw& w, const Rect& r, const Value& v, Color ink, Color paper, dword style) const {
	if (!owner) {
		StdDisplay().Paint(w, r, v, ink, paper, style);
		return;
	}
	w.DrawRect(r, paper);
	if (IsNull(v))
		return;
	int id = -1;
	if (v.Is<ValueArray>()) {
		ValueArray va = v;
		if (va.GetCount() > 0)
			id = va[0];
	}
	else
		id = v;
	if (!owner->IsValid(id))
		return;
	const Item& m = owner->item[id];

	int level = m.level;
	int x = r.left + level * owner->levelcx;
	int cy = r.Height();
	bool has_children = m.canopen || m.child.GetCount();
	Image openimg = m.isopen ? CtrlImg::treeminus() : CtrlImg::treeplus();
	int box = openimg.GetWidth();
	if (has_children) {
		int yy = r.top + (cy - openimg.GetHeight()) / 2;
		w.DrawImage(x, yy, openimg);
		x += box + 2;
	}

	if (!IsNull(m.image)) {
		int yy = r.top + (cy - m.image.GetHeight()) / 2;
		w.DrawImage(x, yy, m.image);
		x += m.image.GetWidth() + 2;
	}

	Rect tr = r;
	tr.left = x;
	if (m.display)
		m.display->Paint(w, tr, m.value, ink, paper, style);
	else {
		Font fnt = StdFont();
		int ty = r.top + (cy - fnt.GetHeight()) / 2;
		w.DrawText(x, ty, AsString(m.value), fnt, ink);
	}
}

Size TreeArrayCtrl::TreeDisplay::GetStdSize(const Value& v) const {
	if (!owner || IsNull(v))
		return StdDisplay().GetStdSize(v);
	int id = -1;
	if (v.Is<ValueArray>()) {
		ValueArray va = v;
		if (va.GetCount() > 0)
			id = va[0];
	}
	else
		id = v;
	if (!owner->IsValid(id))
		return StdDisplay().GetStdSize(v);
	const Item& m = owner->item[id];
	Size sz = GetTextSize(AsString(m.value), StdFont());
	sz.cx += m.level * owner->levelcx;
	if (m.canopen || m.child.GetCount())
		sz.cx += CtrlImg::treeplus().GetWidth() + 2;
	if (!IsNull(m.image))
		sz.cx += m.image.GetWidth() + 2;
	sz.cy = max(sz.cy, CtrlImg::treeplus().GetHeight());
	return sz;
}

TreeArrayCtrl::TreeArrayCtrl() {
	treedisplay.SetOwner(this);
	AddColumn("", 200).SetDisplay(treedisplay).NoEdit();
	NoHeader();
	EnsureRoot();
	WhenCursor << [=] {
		int linei = ArrayCtrl::GetCursor();
		cursor_id = (linei >= 0 && linei < line.GetCount()) ? GetItemAtLine(linei) : -1;
	};
}

TreeArrayCtrl::~TreeArrayCtrl() {}

TreeArrayCtrl::Column& TreeArrayCtrl::AddColumn(const char *text, int w) {
	Column& c = ArrayCtrl::AddColumn(text, w);
	EnsureColumnCount();
	dirty = true;
	SyncTree();
	return c;
}

TreeArrayCtrl::Column& TreeArrayCtrl::AddColumn(const Id& id, const char *text, int w) {
	Column& c = ArrayCtrl::AddColumn(id, text, w);
	EnsureColumnCount();
	dirty = true;
	SyncTree();
	return c;
}

void TreeArrayCtrl::EnsureRoot() {
	if (item.GetCount() == 0) {
		item.Add();
		item[0].free = false;
		item[0].parent = -1;
		item[0].isopen = true;
		item[0].canopen = true;
		item[0].level = 0;
	}
}

int TreeArrayCtrl::NewItem() {
	int id;
	if (freelist >= 0) {
		id = freelist;
		freelist = item[id].freelink;
		item[id] = Item();
		item[id].free = false;
	}
	else {
		id = item.GetCount();
		item.Add();
		item[id].free = false;
	}
	return id;
}

void TreeArrayCtrl::EnsureColumnCount() {
	int cols = max(0, GetColumnCount() - 1);
	for (int i = 0; i < item.GetCount(); i++) {
		if (item[i].free)
			continue;
		item[i].cols.SetCount(cols);
	}
}

void TreeArrayCtrl::SetRoot(const Node& n) {
	EnsureRoot();
	(Item&)item[0] = Item();
	(Node&)item[0] = n;
	item[0].isopen = true;
	item[0].parent = -1;
	EnsureColumnCount();
	if (item[0].canopen && item[0].child.IsEmpty())
		WhenOpen(0);
	dirty = true;
	SyncTree();
}

void TreeArrayCtrl::SetRoot(const Image& img, Value v) {
	SetRoot(Node(img, v).CanOpen());
}

void TreeArrayCtrl::SetRoot(const Image& img, Value v, Value t) {
	SetRoot(Node(img, v, t).CanOpen());
}

void TreeArrayCtrl::SetRoot(const Image& img, Ctrl& ctrl, int cx, int cy) {
	SetRoot(Node(img, ctrl, cx, cy).CanOpen());
}

int TreeArrayCtrl::Insert(int parentid, int i, const Node& n) {
	EnsureRoot();
	if (!IsValid(parentid))
		parentid = 0;
	int id = NewItem();
	Item& m = item[id];
	(Node&)m = n;
	m.parent = parentid;
	m.isopen = false;
	m.line = -1;
	m.level = 0;
	if (m.canopen && m.child.GetCount())
		m.isopen = true;
	Item& p = item[parentid];
	if (i < 0 || i > p.child.GetCount())
		i = p.child.GetCount();
	p.child.Insert(i, id);
	EnsureColumnCount();
	dirty = true;
	SyncTree();
	return id;
}

int TreeArrayCtrl::Add(int parentid, const Node& n) {
	if (!IsValid(parentid))
		parentid = 0;
	return Insert(parentid, item[parentid].child.GetCount(), n);
}

int TreeArrayCtrl::Insert(int parentid, int i) {
	return Insert(parentid, i, Node());
}

int TreeArrayCtrl::Add(int parentid) {
	return Add(parentid, Node());
}

int TreeArrayCtrl::Insert(int parentid, int i, const Image& img, Value v, bool withopen) {
	return Insert(parentid, i, Node(img, v).CanOpen(withopen));
}

int TreeArrayCtrl::Insert(int parentid, int i, const Image& img, Value v, Value t, bool withopen) {
	return Insert(parentid, i, Node(img, v, t).CanOpen(withopen));
}

int TreeArrayCtrl::Insert(int parentid, int i, const Image& img, Value key, const String& value, bool withopen) {
	return Insert(parentid, i, Node(img, key, value).CanOpen(withopen));
}

int TreeArrayCtrl::Insert(int parentid, int i, const Image& img, Value key, const char *value, bool withopen) {
	return Insert(parentid, i, Node(img, key, value).CanOpen(withopen));
}

int TreeArrayCtrl::Add(int parentid, const Image& img, Value v, bool withopen) {
	return Add(parentid, Node(img, v).CanOpen(withopen));
}

int TreeArrayCtrl::Add(int parentid, const Image& img, Value v, Value t, bool withopen) {
	return Add(parentid, Node(img, v, t).CanOpen(withopen));
}

int TreeArrayCtrl::Add(int parentid, const Image& img, Value key, const String& value, bool withopen) {
	return Add(parentid, Node(img, key, value).CanOpen(withopen));
}

int TreeArrayCtrl::Add(int parentid, const Image& img, Value key, const char *value, bool withopen) {
	return Add(parentid, Node(img, key, value).CanOpen(withopen));
}

int TreeArrayCtrl::Insert(int parentid, int i, const Image& img, Ctrl& ctrl, int cx, int cy, bool withopen) {
	return Insert(parentid, i, Node(img, ctrl, cx, cy).CanOpen(withopen));
}

int TreeArrayCtrl::Add(int parentid, const Image& img, Ctrl& ctrl, int cx, int cy, bool withopen) {
	return Add(parentid, Node(img, ctrl, cx, cy).CanOpen(withopen));
}

void TreeArrayCtrl::RemoveChildren(int id) {
	if (!IsValid(id))
		return;
	Item& m = item[id];
	for (int cid : m.child)
		Remove(cid);
	m.child.Clear();
	dirty = true;
	SyncTree();
}

void TreeArrayCtrl::Remove(int id) {
	if (!IsValid(id) || id == 0)
		return;
	Item& m = item[id];
	RemoveChildren(id);
	int parentid = m.parent;
	if (IsValid(parentid)) {
		Item& p = item[parentid];
		int idx = -1;
		for (int i = 0; i < p.child.GetCount(); i++) {
			if (p.child[i] == id) {
				idx = i;
				break;
			}
		}
		if (idx >= 0)
			p.child.Remove(idx);
	}
	m.free = true;
	m.freelink = freelist;
	freelist = id;
	dirty = true;
	SyncTree();
}

void TreeArrayCtrl::Clear() {
	item.Clear();
	freelist = -1;
	line.Clear();
	ArrayCtrl::Clear();
	cursor_id = -1;
	EnsureRoot();
	dirty = true;
}

bool TreeArrayCtrl::IsValid(int id) const {
	return id >= 0 && id < item.GetCount() && !item[id].free;
}

int TreeArrayCtrl::GetChildCount(int id) const {
	return IsValid(id) ? item[id].child.GetCount() : 0;
}

int TreeArrayCtrl::GetChild(int id, int i) const {
	return IsValid(id) ? item[id].child[i] : -1;
}

int TreeArrayCtrl::GetChildIndex(int id, int childid) const {
	if (!IsValid(id))
		return -1;
	for (int i = 0; i < item[id].child.GetCount(); i++)
		if (item[id].child[i] == childid)
			return i;
	return -1;
}

int TreeArrayCtrl::GetParent(int id) const {
	return IsValid(id) ? item[id].parent : -1;
}

Value TreeArrayCtrl::Get(int id) const {
	return IsValid(id) ? item[id].key : Value();
}

Value TreeArrayCtrl::GetValue(int id) const {
	return IsValid(id) ? item[id].value : Value();
}

void TreeArrayCtrl::Set(int id, Value v) {
	if (!IsValid(id))
		return;
	item[id].key = item[id].value = v;
	if (item[id].line >= 0)
		SetRowValue(id, 0, v);
}

void TreeArrayCtrl::Set(int id, Value k, Value v) {
	if (!IsValid(id))
		return;
	item[id].key = k;
	item[id].value = v;
	if (item[id].line >= 0)
		SetRowValue(id, 0, v);
}

void TreeArrayCtrl::SetValue(const Value& v) {
	Set(0, v);
}

void TreeArrayCtrl::SetDisplay(int id, const Display& display) {
	if (!IsValid(id))
		return;
	item[id].display = &display;
	RefreshRow(item[id].line);
}

const Display& TreeArrayCtrl::GetDisplay(int id) const {
	ASSERT(IsValid(id));
	return item[id].display ? *item[id].display : treedisplay;
}

bool TreeArrayCtrl::IsOpen(int id) const {
	return IsValid(id) ? item[id].isopen : false;
}

void TreeArrayCtrl::Open(int id, bool open) {
	if (!IsValid(id))
		return;
	Item& m = item[id];
	if (m.isopen == open)
		return;
	m.isopen = open;
	if (open)
		WhenOpen(id);
	else
		WhenClose(id);
	dirty = true;
	SyncTree();
}

void TreeArrayCtrl::OpenDeep(int id, bool open) {
	if (!IsValid(id))
		return;
	Open(id, open);
	for (int cid : item[id].child)
		OpenDeep(cid, open);
}

int TreeArrayCtrl::GetLineCount() {
	if (!syncing)
		SyncTree();
	return line.GetCount();
}

int TreeArrayCtrl::GetItemAtLine(int i) {
	if (!syncing)
		SyncTree();
	return i >= 0 && i < line.GetCount() ? line[i] : -1;
}

int TreeArrayCtrl::GetLineAtItem(int id) {
	if (!syncing)
		SyncTree();
	return IsValid(id) ? item[id].line : -1;
}

void TreeArrayCtrl::SetCursor(int id) {
	if (!IsValid(id))
		return;
	SyncTree();
	int linei = item[id].line;
	if (linei >= 0) {
		ArrayCtrl::SetCursor(linei);
		cursor_id = id;
	}
}

void TreeArrayCtrl::KillCursor() {
	ArrayCtrl::KillCursor();
	cursor_id = -1;
}

void TreeArrayCtrl::MakeVisible(int id) {
	SyncTree();
	int linei = IsValid(id) ? item[id].line : -1;
	if (linei >= 0)
		ScrollInto(linei);
}

TreeArrayCtrl& TreeArrayCtrl::NoCursor(bool b) {
	if (b)
		KillCursor();
	return *this;
}

void TreeArrayCtrl::SetRowValue(int id, int col, const Value& v) {
	if (!IsValid(id))
		return;
	if (col == 0) {
		item[id].value = v;
		if (item[id].line >= 0) {
			ValueArray va;
			va.Add(id);
			va.Add(item[id].value);
			ArrayCtrl::Set(item[id].line, 0, va);
			RefreshRow(item[id].line);
		}
		return;
	}
	int ci = col - 1;
	if (ci < 0)
		return;
	if (ci >= item[id].cols.GetCount())
		item[id].cols.SetCount(ci + 1);
	item[id].cols[ci] = v;
	if (item[id].line >= 0)
		ArrayCtrl::Set(item[id].line, col, v);
}

Value TreeArrayCtrl::GetRowValue(int id, int col) const {
	if (!IsValid(id))
		return Value();
	if (col == 0)
		return item[id].value;
	int ci = col - 1;
	if (ci < 0 || ci >= item[id].cols.GetCount())
		return Value();
	return item[id].cols[ci];
}

Vector<Value> TreeArrayCtrl::GetRowValues(int id) const {
	Vector<Value> row;
	if (!IsValid(id))
		return row;
	row.Add(item[id].value);
	for (const Value& v : item[id].cols)
		row.Add(v);
	return row;
}

bool TreeArrayCtrl::IsVisible(int id) const {
	if (!IsValid(id))
		return false;
	int pid = item[id].parent;
	while (pid >= 0) {
		const Item& p = item[pid];
		if (!p.isopen && pid != 0)
			return false;
		pid = p.parent;
	}
	return true;
}

void TreeArrayCtrl::BuildLine(int id, int level) {
	if (!IsValid(id))
		return;
	Item& m = item[id];
	m.level = level;
	m.line = -1;
	if (!noroot || id != 0) {
		m.line = line.GetCount();
		line.Add(id);
	}
	if (m.isopen || id == 0) {
		for (int cid : m.child)
			BuildLine(cid, level + 1);
	}
}

void TreeArrayCtrl::SyncTree() {
	if (!dirty)
		return;
	if (syncing)
		return;
	syncing = true;
	EnsureRoot();
	EnsureColumnCount();
	line.Clear();
	if (noroot) {
		for (int cid : item[0].child)
			BuildLine(cid, 0);
	}
	else {
		BuildLine(0, 0);
	}

	ArrayCtrl::Clear();
	for (int i = 0; i < line.GetCount(); i++) {
		int id = line[i];
		if (!IsValid(id))
			continue;
		Item& m = item[id];
		Vector<Value> row;
		row.SetCount(GetColumnCount());
		ValueArray va;
		va.Add(id);
		va.Add(m.value);
		row[0] = va;
		for (int col = 1; col < GetColumnCount(); col++) {
			int ci = col - 1;
			if (ci < m.cols.GetCount())
				row[col] = m.cols[ci];
		}
		ArrayCtrl::Add(row);
	}

	if (cursor_id >= 0) {
		int linei = IsValid(cursor_id) ? item[cursor_id].line : -1;
		if (linei >= 0)
			ArrayCtrl::SetCursor(linei);
	}
	dirty = false;
	syncing = false;
}

Rect TreeArrayCtrl::GetToggleRect(int linei, const Rect& cell) const {
	int id = (linei >= 0 && linei < line.GetCount()) ? line[linei] : -1;
	if (!IsValid(id))
		return Rect();
	const Item& m = item[id];
	int x = cell.left + m.level * levelcx;
	Image img = m.isopen ? CtrlImg::treeminus() : CtrlImg::treeplus();
	int w = img.GetWidth();
	int h = img.GetHeight();
	int y = cell.top + (cell.Height() - h) / 2;
	return RectC(x, y, w, h);
}

Rect TreeArrayCtrl::GetIconRect(int linei, const Rect& cell) const {
	int id = (linei >= 0 && linei < line.GetCount()) ? line[linei] : -1;
	if (!IsValid(id))
		return Rect();
	const Item& m = item[id];
	int x = cell.left + m.level * levelcx;
	if (m.canopen || m.child.GetCount())
		x += CtrlImg::treeplus().GetWidth() + 2;
	if (IsNull(m.image))
		return Rect();
	int w = m.image.GetWidth();
	int h = m.image.GetHeight();
	int y = cell.top + (cell.Height() - h) / 2;
	return RectC(x, y, w, h);
}

void TreeArrayCtrl::LeftDown(Point p, dword flags) {
	int linei = GetLineAt(p.y + GetScroll());
	if (linei >= 0 && linei < line.GetCount()) {
		Rect cell = GetCellRect(linei, 0);
		if (cell.Contains(p)) {
			Rect toggle = GetToggleRect(linei, cell);
			int id = line[linei];
			if (toggle.Contains(p) && IsValid(id)) {
				Item& m = item[id];
				if (m.canopen || m.child.GetCount()) {
					Open(id, !m.isopen);
					return;
				}
			}
			Rect icon = GetIconRect(linei, cell);
			if (icon.Contains(p) && IsValid(id)) {
				Item& m = item[id];
				if (m.canopen || m.child.GetCount()) {
					Open(id, !m.isopen);
					return;
				}
			}
		}
	}
	ArrayCtrl::LeftDown(p, flags);
}

void TreeArrayCtrl::LeftDouble(Point p, dword flags) {
	int linei = GetLineAt(p.y + GetScroll());
	if (linei >= 0 && linei < line.GetCount()) {
		int id = line[linei];
		if (IsValid(id)) {
			Item& m = item[id];
			if (m.canopen || m.child.GetCount()) {
				Open(id, !m.isopen);
				return;
			}
		}
	}
	ArrayCtrl::LeftDouble(p, flags);
}

bool TreeArrayCtrl::Key(dword key, int count) {
	if (key == K_LEFT || key == K_RIGHT) {
		int linei = ArrayCtrl::GetCursor();
		int id = (linei >= 0 && linei < line.GetCount()) ? GetItemAtLine(linei) : -1;
		if (IsValid(id)) {
			Item& m = item[id];
			if (m.canopen || m.child.GetCount()) {
				if (key == K_LEFT && m.isopen) {
					Open(id, false);
					SetCursor(id);
					return true;
				}
				if (key == K_RIGHT && !m.isopen) {
					Open(id, true);
					SetCursor(id);
					return true;
				}
			}
			if (key == K_LEFT && !m.isopen && m.parent >= 0) {
				SetCursor(m.parent);
				return true;
			}
			if (key == K_RIGHT && m.isopen && m.child.GetCount()) {
				SetCursor(m.child[0]);
				return true;
			}
		}
	}
	return ArrayCtrl::Key(key, count);
}

END_UPP_NAMESPACE
