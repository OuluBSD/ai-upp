#ifndef _WebDriver_Chrome_h_
#define _WebDriver_Chrome_h_

#include <Core/Core.h>

NAMESPACE_UPP

namespace chrome {

struct PerfLoggingPrefs {
	bool enable_network = false;
	bool enable_page = false;
	bool enable_timeline = false;
	String tracing_categories;
	int buffer_usage_reporting_interval = 0;
	
	void Jsonize(JsonIO& json) {
		json("enableNetwork", enable_network)
			("enablePage", enable_page)
			("enableTimeline", enable_timeline)
			("tracingCategories", tracing_categories)
			("bufferUsageReportingInterval", buffer_usage_reporting_interval);
	}
};

} // namespace chrome

struct Chrome : Capabilities { // copyable
	Vector<String> args;
	String binary;
	Vector<String> extensions;
	Value local_state;
	Value prefs;
	bool detach = false;
	String debugger_address;
	Vector<String> exclude_switches;
	String minidump_path;
	Value mobile_emulation;
	chrome::PerfLoggingPrefs perf_logging_prefs;
	
	Chrome(const Capabilities& defaults = Capabilities())
		: Capabilities(defaults) {
		browser_name = browser::K_CHROME;
		version = String();
		platform = platform::K_ANY;
	}

	// See https://sites.google.com/a/chromium.org/chromedriver/capabilities for details
	void Jsonize(JsonIO& json) {
		json("loggingPrefs", logging_prefs)
			("args", args)
			("binary", binary)
			("extensions", extensions)
			("localState", local_state)
			("prefs", prefs)
			("detach", detach)
			("debuggerAddress", debugger_address)
			("excludeSwitches", exclude_switches)
			("minidumpPath", minidump_path)
			("mobileEmulation", mobile_emulation)
			("perfLoggingPrefs", perf_logging_prefs);
	}
};

END_UPP_NAMESPACE

#endif