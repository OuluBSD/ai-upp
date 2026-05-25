#ifndef _ide_About_h_
#define _ide_About_h_

#ifdef flagGUI
#include <CtrlLib/CtrlLib.h>
#else
#include <Core/Core.h>
#endif

namespace Upp {

#ifdef flagGUI
class SplashCtrl final : public Ctrl
{
public:
	static String GenerateVersionInfo(bool qtf = false, bool about = false);
	static String GenerateVersionNumber();
	static Size   MakeLogo(Ctrl& parent, Array<Ctrl>& ctrl, bool about = false);
	
public:
	SplashCtrl();
	
private:
	Array<Ctrl> ctrl;
};
#else
class SplashCtrl final
{
public:
	static String GenerateVersionInfo(bool qtf = false, bool about = false);
	static String GenerateVersionNumber();
};
#endif

}

#endif
