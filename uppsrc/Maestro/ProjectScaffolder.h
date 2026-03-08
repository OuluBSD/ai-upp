#ifndef _Maestro_ProjectScaffolder_h_
#define _Maestro_ProjectScaffolder_h_

#include <Core/Core.h>

namespace Upp {

struct ProjectScaffolder {
	static bool Scaffold(const String& dir, const String& name, const String& template_name = "gui");
	static bool InitMaestro(const String& dir);
};

}

#endif
