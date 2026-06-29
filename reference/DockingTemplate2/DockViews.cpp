#include "MainWindow.h"

// ---------------------------------------------------------------------------
// DockViewA

DockViewA::DockViewA()
{
	Add(label_.HSizePos(8, 8).TopPos(8, 20));
	Add(cursor_label_.HSizePos(8, 8).TopPos(36, 20));
	label_.SetLabel("Dock A — sample view");
	cursor_label_.SetLabel("cursor: —");
}

void DockViewA::OnCursorChanged(Point p)
{
	if(updating_) return;
	updating_ = true;
	cursor_label_.SetLabel(Format("cursor: (%d, %d)", p.x, p.y));
	updating_ = false;
}

// ---------------------------------------------------------------------------
// DockViewB

DockViewB::DockViewB()
{
	Add(label_.HSizePos(8, 8).TopPos(8, 20));
	Add(cursor_label_.HSizePos(8, 8).TopPos(36, 20));
	label_.SetLabel("Dock B — sample view");
	cursor_label_.SetLabel("cursor: —");
}

void DockViewB::OnCursorChanged(Point p)
{
	if(updating_) return;
	updating_ = true;
	cursor_label_.SetLabel(Format("cursor: (%d, %d)", p.x, p.y));
	updating_ = false;
}

// ---------------------------------------------------------------------------
// DockViewC

DockViewC::DockViewC()
{
	Add(label_.HSizePos(8, 8).TopPos(8, 20));
	Add(cursor_label_.HSizePos(8, 8).TopPos(36, 20));
	label_.SetLabel("Dock C — sample view");
	cursor_label_.SetLabel("cursor: —");
}

void DockViewC::OnCursorChanged(Point p)
{
	if(updating_) return;
	updating_ = true;
	cursor_label_.SetLabel(Format("cursor: (%d, %d)", p.x, p.y));
	updating_ = false;
}
