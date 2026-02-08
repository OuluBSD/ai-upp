#include "Aria.h"

NAMESPACE_UPP

const char* CalendarScraper::URL = "https://calendar.google.com/calendar/u/0/r";

CalendarScraper::CalendarScraper(AriaNavigator& navigator, SiteManager& sm)
	: navigator(navigator)
	, sm(sm)
{
	site_name = "calendar";
}

bool CalendarScraper::Navigate() {
	GetAriaLogger("calendar").Info("Navigating to Google Calendar...");
	try {
		navigator.Navigate(URL);
		navigator.WaitForElement("div[role='main']", "css selector", 45);
		return true;
	} catch (const Exc& e) {
		GetAriaLogger("calendar").Error("Failed to load Calendar: " + e);
		return false;
	}
}

bool CalendarScraper::Refresh(bool deep) {
	if (!Navigate()) return false;
	GetAriaLogger("calendar").Info("Refreshing Google Calendar data...");
	
	ValueArray events = ScrapeEvents();
	GetAriaLogger("calendar").Info(Format("Found %d events in current view.", events.GetCount()));
	
	sm.SetSiteData(site_name, "events", events);
	sm.SetSiteData(site_name, "metadata", ValueMap()
		("last_refresh", GetUtcTime())
		("event_count", events.GetCount())
	);
	
	return true;
}

ValueArray CalendarScraper::ScrapeEvents() {
	try {
		GetAriaLogger("calendar").Info("Scraping events...");
		// JS logic to extract events from aria-labels
		Value res = navigator.Eval(R"(
			const main = document.querySelector('div[role="main"]') || 
			             document.querySelector('div[role="grid"]') ||
			             document.body;
			const els = main.querySelectorAll('[aria-label]');
			const events = [];
			els.forEach(el => {
				const label = el.getAttribute('aria-label');
				// Heuristic for events: contains comma and time-like patterns
				if (label && label.length > 10 && label.includes(',')) {
					if (/\d+[:.]\d+/.test(label)) {
						events.append({ label: label });
					}
				}
			});
			return events;
		)");
		
		if (res.Is<ValueArray>()) return res;
	} catch (const Exc& e) {
		GetAriaLogger("calendar").Error("Error scraping events: " + e);
	}
	return ValueArray();
}

END_UPP_NAMESPACE
