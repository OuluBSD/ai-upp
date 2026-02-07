#include "Aria.h"

NAMESPACE_UPP

const char* WhatsAppScraper::URL = "https://web.whatsapp.com/";

WhatsAppScraper::WhatsAppScraper(AriaNavigator& navigator, SiteManager& sm)
	: navigator(navigator)
	, sm(sm)
{
	site_name = "whatsapp";
}

bool WhatsAppScraper::Navigate() {
	GetAriaLogger("whatsapp").Info("Navigating to WhatsApp Web...");
	try {
		navigator.Navigate(URL);
		navigator.WaitForElement("div[contenteditable='true']", "css selector", 45);
		return true;
	} catch (const Exc& e) {
		GetAriaLogger("whatsapp").Error("Failed to load WhatsApp: " + e);
		return false;
	}
}

bool WhatsAppScraper::Refresh(bool deep) {
	if (!Navigate()) return false;
	// Scrape logic
	return true;
}

END_UPP_NAMESPACE
