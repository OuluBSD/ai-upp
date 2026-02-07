#include "Aria.h"

NAMESPACE_UPP

const char* ThreadsScraper::URL = "https://www.threads.net/";

ThreadsScraper::ThreadsScraper(AriaNavigator& navigator, SiteManager& sm)
	: navigator(navigator)
	, sm(sm)
{
	site_name = "threads";
}

bool ThreadsScraper::Navigate() {
	GetAriaLogger("threads").Info("Navigating to Threads...");
	try {
		navigator.Navigate(URL);
		navigator.WaitForElement("svg[aria-label='Threads']", "css selector", 45);
		return true;
	} catch (const Exc& e) {
		GetAriaLogger("threads").Error("Failed to load Threads: " + e);
		return false;
	}
}

bool ThreadsScraper::Refresh(bool deep) {
	if (!Navigate()) return false;
	// Scrape logic
	return true;
}

END_UPP_NAMESPACE
