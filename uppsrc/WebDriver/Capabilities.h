#ifndef _WebDriver_Capabilities_h_
#define _WebDriver_Capabilities_h_

#include <Core/Core.h>


NAMESPACE_UPP

namespace browser {
typedef String Value;
typedef const char* const ConstValue;
ConstValue Android = "android";
ConstValue Chrome = "chrome";
ConstValue Firefox = "firefox";
ConstValue HtmlUnit = "htmlunit";
ConstValue InternetExplorer = "internet explorer";
ConstValue IPhone = "iPhone";
ConstValue IPad = "iPad";
ConstValue Mock = "mock";
ConstValue Opera = "opera";
ConstValue Safari = "safari";
ConstValue Phantom = "phantomjs";
} // namespace browser

namespace platform {
typedef String Value;
typedef const char* const ConstValue;
ConstValue Any = "ANY";
ConstValue Windows = "WINDOWS";
ConstValue Xp = "XP";
ConstValue Vista = "VISTA";
ConstValue Mac = "MAC";
ConstValue Linux = "LINUX";
ConstValue Unix = "UNIX";
ConstValue Android = "ANDROID";
} // namespace platform

namespace unexpected_alert_behaviour {
typedef String Value;
typedef const char* const ConstValue;
ConstValue Accept = "accept";
ConstValue Dismiss = "dismiss";
ConstValue Ignore = "ignore";
} // namespace unexpected_alert_behaviour

namespace proxy_type {
typedef String Value;
typedef const char* const ConstValue;
ConstValue Direct = "direct";
ConstValue Manual = "manual"; // Manual proxy settings configured, e.g. setting a proxy for HTTP, a proxy for FTP
ConstValue Pac = "pac"; // Proxy autoconfiguration from a URL
ConstValue Autodetect = "autodetect"; // Proxy autodetection, probably with WPAD
ConstValue System = "system"; // Use system settings
} // namespace proxy_type

struct Proxy { // copyable
	String proxy_type;
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type);
	}
};

struct Direct_connection : Proxy { // copyable
	Direct_connection() { proxy_type = proxy_type::Direct; }
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type);
	}
};

struct Autodetect_proxy : Proxy { // copyable
	Autodetect_proxy() { proxy_type = proxy_type::Autodetect; }
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type);
	}
};

struct System_proxy : Proxy { // copyable
	System_proxy() { proxy_type = proxy_type::System; }
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type);
	}
};

struct Automatic_proxy_from_url : Proxy { // copyable
	String autoconfig_url;
	
	explicit Automatic_proxy_from_url(const String& url) {
		proxy_type = proxy_type::Pac;
		autoconfig_url = url;
	}
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type)("proxyAutoconfigUrl", autoconfig_url);
	}
};

struct Manual_proxy : Proxy { // copyable
	String no_proxy_for;
	
	Manual_proxy() { proxy_type = proxy_type::Manual; }
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type)("noProxy", no_proxy_for);
	}
};

struct Ftp_proxy : Manual_proxy { // copyable
	String proxy_address;
	
	explicit Ftp_proxy(const String& address) { 
		proxy_address = address; 
	}
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type)("ftpProxy", proxy_address);
	}
};

struct Http_proxy : Manual_proxy { // copyable
	String proxy_address;
	
	explicit Http_proxy(const String& address) { 
		proxy_address = address; 
	}
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type)("httpProxy", proxy_address);
	}
};

struct Ssl_proxy : Manual_proxy { // copyable
	String proxy_address;
	String username;
	String password;
	
	explicit Ssl_proxy(const String& address) { 
		proxy_address = address; 
	}
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type)("sslProxy", proxy_address)
			("socksUsername", username)("socksPassword", password);
	}
};

struct Socks_proxy : Manual_proxy { // copyable
	String proxy_address;
	String username;
	String password;
	
	explicit Socks_proxy(const String& address) { 
		proxy_address = address; 
	}
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type)("socksProxy", proxy_address)
			("socksUsername", username)("socksPassword", password);
	}
};

namespace log_level {
typedef String Value;
typedef const char* const ConstValue;
ConstValue Off = "OFF";
ConstValue Severe = "SEVERE";
ConstValue Warning = "WARNING";
ConstValue Info = "INFO";
ConstValue Config = "CONFIG";
ConstValue Fine = "FINE";
ConstValue Finer = "FINER";
ConstValue Finest = "FINEST";
ConstValue All = "ALL";
} // namespace log_level

struct Logging_prefs {
	String level;
	
	void Jsonize(JsonIO& json) {
		json("driver", level);
	}
};

// List of keys and values indicating features that server can or should provide.
struct Capabilities { // copyable
	String browser_name;
	String version;
	String platform;
	bool takes_screenshot = false;
	bool handles_alerts = false;
	bool css_selectors_enabled = false;
	bool javascript_enabled = false;
	bool database_enabled = false;
	bool location_context_enabled = false;
	bool application_cache_enabled = false;
	bool browser_connection_enabled = false;
	bool web_storage_enabled = false;
	bool accept_ssl_certs = false;
	bool rotatable = false;
	bool native_events = false;
	Proxy proxy;
	String unexpected_alert_behaviour;
	int element_scroll_behavior = 0;
	String session_id;
	bool quiet_exceptions = false;
	
	Capabilities() {}
	
	void Jsonize(JsonIO& json) {
		json("browserName", browser_name)
			("version", version)
			("platform", platform)
			("takesScreenshot", takes_screenshot)
			("handlesAlerts", handles_alerts)
			("cssSelectorsEnabled", css_selectors_enabled)
			("javascriptEnabled", javascript_enabled)
			("databaseEnabled", database_enabled)
			("locationContextEnabled", location_context_enabled)
			("applicationCacheEnabled", application_cache_enabled)
			("browserConnectionEnabled", browser_connection_enabled)
			("webStorageEnabled", web_storage_enabled)
			("acceptSslCerts", accept_ssl_certs)
			("rotatable", rotatable)
			("nativeEvents", native_events)
			("proxy", proxy)
			("unexpectedAlertBehaviour", unexpected_alert_behaviour)
			("elementScrollBehavior", element_scroll_behavior)
			("webdriver.remote.sessionid", session_id)
			("webdriver.remote.quietExceptions", quiet_exceptions);
	}
};

END_UPP_NAMESPACE

#endif