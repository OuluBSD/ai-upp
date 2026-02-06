#ifndef _WebDriver_Phantom_h_
#define _WebDriver_Phantom_h_

#include <Core/Core.h>

NAMESPACE_UPP

struct Phantom : Capabilities { // copyable
	String executable;
	Vector<String> args;
	Vector<String> ghost_args;
	
	Phantom(const Capabilities& defaults = Capabilities())
		: Capabilities(defaults) {
		browser_name = browser::K_PHANTOM;
		version = String();
		platform = platform::K_ANY;
	}

	// PhantomJS-specific capabilities
	void Jsonize(JsonIO& json) {
		json("phantomjs.binary.path", executable)
			("phantomjs.cli.args", args)
			("phantomjs.page.settings.args", ghost_args);
	}
};

END_UPP_NAMESPACE

#endif