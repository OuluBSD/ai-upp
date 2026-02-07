#include "Aria.h"

NAMESPACE_UPP

const char* DiscordScraper::URL = "https://discord.com/app";

DiscordScraper::DiscordScraper(AriaNavigator& navigator, SiteManager& sm)
	: navigator(navigator)
	, sm(sm)
{
	site_name = "discord";
}

bool DiscordScraper::Navigate() {
	if (navigator.ListTabs().GetCount() > 0) { // Check current URL
		// Should check if URL contains /channels/
	}
	
	GetAriaLogger("discord").Info("Navigating to Discord...");
	try {
		navigator.Navigate(URL);
		navigator.WaitForElement("div[id='app-mount']", "css selector", 45);
		return true;
	} catch (const Exc& e) {
		GetAriaLogger("discord").Error("Failed to load Discord: " + e);
		return false;
	}
}

bool DiscordScraper::Refresh(bool deep) {
	if (!Navigate()) return false;
	// Scrape logic
	return true;
}

END_UPP_NAMESPACE
