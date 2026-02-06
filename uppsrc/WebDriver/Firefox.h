#ifndef _WebDriver_Firefox_h_
#define _WebDriver_Firefox_h_

#include <Core/Core.h>

NAMESPACE_UPP

struct FirefoxProfile { // copyable
	bool accept_untrusted_certs = false;
	bool assume_untrusted_issuer = false;
	String binary;
	bool disable_plugin_whitelist = false;
	bool use_legacy_firefox = false;
	
	void Jsonize(JsonIO& json) {
		json("acceptUntrustedCertificates", accept_untrusted_certs)
			("assumeUntrustedCertificateIssuer", assume_untrusted_issuer)
			("firefox_binary", binary)
			("disablePluginWhitelist", disable_plugin_whitelist)
			("useLegacyFirefox", use_legacy_firefox);
	}
};

struct Firefox : Capabilities { // copyable
	FirefoxProfile profile;
	
	Firefox(const Capabilities& defaults = Capabilities())
		: Capabilities(defaults) {
		browser_name = browser::K_FIREFOX;
		version = String();
		platform = platform::K_ANY;
	}

	// See https://github.com/SeleniumHQ/selenium/wiki/DesiredCapabilities for details
	void Jsonize(JsonIO& json) {
		json("firefox_profile", profile)
			("loggingPrefs", logging_prefs);
	}
};

END_UPP_NAMESPACE

#endif