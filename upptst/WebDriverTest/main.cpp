#include <Core/Core.h>
#include <WebDriver/WebDriver.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	
	try {
		LOG("Starting WebDriver Test...");
		
		Firefox caps;
		caps.ApplyStealthSettings();
		caps.options.AddArgument("--headless");
		
		LOG("Patching Firefox binary...");
		String patched_exe = detail::PatchFirefoxBinary();
		if (!patched_exe.IsEmpty()) {
			LOG("Using patched binary: " << patched_exe);
			caps.options.binary = patched_exe;
		} else {
			LOG("Warning: Could not patch Firefox binary, using default.");
		}
		
		LOG("Starting geckodriver...");
		LocalProcess geckodriver;
		if (!geckodriver.Start2("/usr/bin/geckodriver --port=4444")) {
			LOG("Error starting geckodriver");
			return;
		}
		
		auto log_gecko = [&]() {
			String o, e;
			if (geckodriver.Read2(o, e)) {
				if (!o.IsEmpty()) LOG("GECKO OUT: " << o);
				if (!e.IsEmpty()) LOG("GECKO ERR: " << e);
			}
		};
		
		for (int i = 0; i < 30; i++) {
			log_gecko();
			Sleep(100);
		}
		
		LOG("Creating session...");
		try {
			WebDriver::Web_driver driver(caps);
			
			LOG("Client status: " << AsJSON(driver.GetStatus()));
			
			Vector<Session> sessions = driver.GetSessions();
			LOG("Active sessions: " << sessions.GetCount());
			
			LOG("Applying Stealth JS...");
			driver.ApplyStealthJS();
			
			LOG("Navigating to DuckDuckGo...");
			driver.Navigate("https://duckduckgo.com");
			
			LOG("Page title: " << driver.GetTitle());
			LOG("Page URL: " << driver.GetUrl());
			
			LOG("Searching for 'Aria automation'...");
			Element search_input = driver.FindElement(By::Name("q"));
			if (search_input.GetRef().IsEmpty()) {
				search_input = driver.FindElement(By::Id("search_form_input_homepage"));
			}
			
			if (!search_input.GetRef().IsEmpty()) {
				LOG("Found search input.");
				search_input.SendKeys("Aria automation\n");
				Sleep(3000);
				LOG("Search page title: " << driver.GetTitle());
			} else {
				LOG("Warning: Could not find search input.");
			}
			
			LOG("Navigating to bot detection test site...");
			driver.Navigate("https://bot.sannysoft.com/");
			
			LOG("Waiting for page to load...");
			Sleep(5000);
			
			String source = driver.GetSource();
			LOG("Page source length: " << source.GetLength());
			
			if (source.Find("present (failed)") >= 0) {
				LOG("DETECTION: navigator.webdriver is present!");
			} else if (source.Find("missing (passed)") >= 0) {
				LOG("SUCCESS: navigator.webdriver is hidden.");
			}
			
			LOG("Test completed successfully.");
		} catch (const std::exception& e) {
			LOG("STD ERROR: " << e.what());
			for (int i = 0; i < 50; i++) {
				log_gecko();
				Sleep(100);
			}
		}
		
		geckodriver.Kill();
		
	} catch (const Exc& e) {
		LOG("ERROR: " << e);
	} catch (const std::exception& e) {
		LOG("STD ERROR: " << e.what());
	}
}
