#include "Aria.h"

NAMESPACE_UPP

void CreateUndetectedFirefoxSession(bool headless, const String& profile_path) {
	Capabilities caps;
	caps.browser_name = browser::K_FIREFOX;
	if (headless) {
		// Add headless
	}
	// Use UndetectedFirefox
	static One<UndetectedFirefox> driver;
	driver.Create(caps);
}

END_UPP_NAMESPACE
