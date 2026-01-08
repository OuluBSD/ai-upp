#include "EonApiEditor.h"

NAMESPACE_UPP


void InterfaceBuilder::AddScreen() {
	Package("Screen", "Scr");
	SetColor(0, 128, 0);
	Dependency("api/Graphics");
	Library("X11 Xrandr Xxf86vm", "POSIX & !EMSCRIPTEN");
	Library("GLX GL GLU GLEW glut", "POSIX & OGL & !EMSCRIPTEN");
	Library("D3D11 D3DCompiler dxgi", "WIN32 & DX11");
	Library("opengl32 glu32", "WIN32 & OGL");
	HaveRecvFinalize();
	HaveNegotiateFormat();
	HaveIsReady();
	
	Interface("SinkDevice", "SCREEN");
	Interface("Context", "SCREEN");
	Interface("EventsBase", "SCREEN");
	
	Vendor("X11",		"X11");
	Vendor("X11Sw",		"X11");
	Vendor("X11Ogl",	"X11&OGL");
	Vendor("Win",		"WIN32&!UWP");
	//Vendor("WinSw",	"WIN32");
	Vendor("WinD11",	"WIN32&DX11");
	
	
}


END_UPP_NAMESPACE
