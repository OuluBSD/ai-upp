#ifndef _WebDriver_Capabilities_h_
#define _WebDriver_Capabilities_h_

#include <Core/Core.h>


NAMESPACE_UPP

namespace browser {
typedef String Value;
typedef const char* const ConstValue;
ConstValue K_ANDROID = "android";
ConstValue K_CHROME = "chrome";
ConstValue K_FIREFOX = "firefox";
ConstValue K_HTML_UNIT = "htmlunit";
ConstValue K_INTERNET_EXPLORER = "internet explorer";
ConstValue K_IPHONE = "iPhone";
ConstValue K_IPAD = "iPad";
ConstValue K_MOCK = "mock";
ConstValue K_OPERA = "opera";
ConstValue K_SAFARI = "safari";
ConstValue K_PHANTOM = "phantomjs";
} // namespace browser

namespace platform {
typedef String Value;
typedef const char* const ConstValue;
ConstValue K_ANY = "ANY";
ConstValue K_WINDOWS = "WINDOWS";
ConstValue K_XP = "XP";
ConstValue K_VISTA = "VISTA";
ConstValue K_MAC = "MAC";
ConstValue K_LINUX = "LINUX";
ConstValue K_UNIX = "UNIX";
ConstValue K_ANDROID = "ANDROID";
} // namespace platform

namespace unexpected_alert_behaviour {
typedef String Value;
typedef const char* const ConstValue;
ConstValue K_ACCEPT = "accept";
ConstValue K_DISMISS = "dismiss";
ConstValue K_IGNORE = "ignore";
} // namespace unexpected_alert_behaviour

namespace proxy_type {
typedef String Value;
typedef const char* const ConstValue;
ConstValue K_DIRECT = "direct";
ConstValue K_MANUAL = "manual"; // Manual proxy settings configured, e.g. setting a proxy for HTTP, a proxy for FTP
ConstValue K_PAC = "pac"; // Proxy autoconfiguration from a URL
ConstValue K_AUTODETECT = "autodetect"; // Proxy autodetection, probably with WPAD
ConstValue K_SYSTEM = "system"; // Use system settings
} // namespace proxy_type

struct Proxy { // copyable
	String proxy_type;
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type);
	}
};

struct DirectConnection : Proxy { // copyable
	DirectConnection() { proxy_type = proxy_type::K_DIRECT; }
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type);
	}
};

struct AutodetectProxy : Proxy { // copyable
	AutodetectProxy() { proxy_type = proxy_type::K_AUTODETECT; }
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type);
	}
};

struct SystemProxy : Proxy { // copyable
	SystemProxy() { proxy_type = proxy_type::K_SYSTEM; }
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type);
	}
};

struct AutomaticProxyFromUrl : Proxy { // copyable
	String autoconfig_url;
	
	explicit AutomaticProxyFromUrl(const String& url) {
		proxy_type = proxy_type::K_PAC;
		autoconfig_url = url;
	}
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type)("proxyAutoconfigUrl", autoconfig_url);
	}
};

struct ManualProxy : Proxy { // copyable
	String no_proxy_for;
	
	ManualProxy() { proxy_type = proxy_type::K_MANUAL; }
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type)("noProxy", no_proxy_for);
	}
};

struct FtpProxy : ManualProxy { // copyable
	String proxy_address;
	
	explicit FtpProxy(const String& address) { 
		proxy_address = address; 
	}
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type)("ftpProxy", proxy_address);
	}
};

struct HttpProxy : ManualProxy { // copyable
	String proxy_address;
	
	explicit HttpProxy(const String& address) { 
		proxy_address = address; 
	}
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type)("httpProxy", proxy_address);
	}
};

struct SslProxy : ManualProxy { // copyable
	String proxy_address;
	String username;
	String password;
	
	explicit SslProxy(const String& address) { 
		proxy_address = address; 
	}
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type)("sslProxy", proxy_address)
			("socksUsername", username)("socksPassword", password);
	}
};

struct SocksProxy : ManualProxy { // copyable
	String proxy_address;
	String username;
	String password;
	
	explicit SocksProxy(const String& address) { 
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
ConstValue K_OFF = "OFF";
ConstValue K_SEVERE = "SEVERE";
ConstValue K_WARNING = "WARNING";
ConstValue K_INFO = "INFO";
ConstValue K_CONFIG = "CONFIG";
ConstValue K_FINE = "FINE";
ConstValue K_FINER = "FINER";
ConstValue K_FINEST = "FINEST";
ConstValue K_ALL = "ALL";
} // namespace log_level

struct LoggingPrefs {
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
	LoggingPrefs logging_prefs;
	
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
			("webdriver.remote.quietExceptions", quiet_exceptions)
			("loggingPrefs", logging_prefs);
	}
};

END_UPP_NAMESPACE

#endif