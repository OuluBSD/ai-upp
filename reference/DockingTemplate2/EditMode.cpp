#include "MainWindow.h"

void ModeManager::SetMode(Mode m)
{
	if(m == current_) return;
	current_ = m;
	WhenModeChanged((int)current_);
}

void ModeManager::FireCursor(Point p)
{
	WhenCursorChanged(p);
}
