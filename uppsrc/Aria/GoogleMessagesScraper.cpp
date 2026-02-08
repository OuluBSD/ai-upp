#include "Aria.h"

NAMESPACE_UPP

const char* GoogleMessagesScraper::URL = "https://messages.google.com/web/conversations";

GoogleMessagesScraper::GoogleMessagesScraper(AriaNavigator& navigator, SiteManager& sm)
	: navigator(navigator)
	, sm(sm)
{
	site_name = "google-messages";
}

bool GoogleMessagesScraper::Navigate() {
	GetAriaLogger("google_messages").Info("Navigating to Google Messages...");
	try {
		navigator.Navigate(URL);
		navigator.WaitForElement("mws-conversations-list", "tag name", 30);
		return true;
	} catch (const Exc& e) {
		GetAriaLogger("google_messages").Error("Failed to load Google Messages: " + e);
		return false;
	}
}

bool GoogleMessagesScraper::Refresh(bool deep) {
	if (!Navigate()) return false;
	ValueArray conversations = ScrapeAllConversations();
	
	Vector<String> names;
	for (int i = 0; i < conversations.GetCount(); i++)
		names.Add(conversations[i]["name"]);
	
	sm.UpdateRegistry(site_name, names);
	return true;
}

ValueArray GoogleMessagesScraper::ScrapeAllConversations() {
	ValueArray all_data;
	try {
		GetAriaLogger("google_messages").Info("Scraping conversations...");
		Vector<Element> convs = navigator.FindElements(By::TagName("mws-conversation-list-item"));
		GetAriaLogger("google_messages").Info(Format("Found %d conversations", convs.GetCount()));
		
		for (int i = 0; i < min(convs.GetCount(), 10); i++) {
			try {
				ValueMap conv;
				String name = convs[i].FindElement(By::CssSelector(".name-container")).GetText();
				conv.Set("name", name);
				
				// Optional: Click to load messages
				// convs[i].Click();
				// conv.Set("messages", ExtractVisibleMessages());
				
				all_data.Add(conv);
			} catch (...) {}
		}
	} catch (const Exc& e) {
		GetAriaLogger("google_messages").Error("Error scraping: " + e);
	}
	return all_data;
}

ValueArray GoogleMessagesScraper::ExtractVisibleMessages() {
	ValueArray messages;
	try {
		Vector<Element> msg_els = navigator.FindElements(By::TagName("mws-message-wrapper"));
		for (const auto& e : msg_els) {
			try {
				ValueMap msg;
				msg.Set("text", e.FindElement(By::CssSelector(".text-msg")).GetText());
				messages.Add(msg);
			} catch (...) {}
		}
	} catch (...) {}
	return messages;
}

END_UPP_NAMESPACE