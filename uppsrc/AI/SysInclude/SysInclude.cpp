#include "SysInclude.h"
#undef main

#if HAVE_WIN32 && (HAVE_GCC || HAVE_CLANG)
int WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	return 0;
}
#else
int main(int argc, char** argv){
	return 0;
}
#endif
