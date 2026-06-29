#include "MainWindow.h"

GUI_APP_MAIN
{
	MainWindow wnd;
	wnd.OpenMain();
	wnd.DockInit();
	Ctrl::EventLoop(&wnd);
}
