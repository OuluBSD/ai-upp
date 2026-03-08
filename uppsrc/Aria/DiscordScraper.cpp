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
	GetAriaLogger("discord").Info("Refreshing Discord data...");
	
	try {
		// Wait for the message list to be visible
		navigator.WaitForElement("ol[class*='messageLog']", "css selector", 20);
		
		Vector<ValueMap> messages;
		
		// Basic message extraction logic
		// In Discord, messages are often in 'li' elements inside an 'ol'
		Vector<Element> msg_elements = navigator.WaitForElement("ol[class*='messageLog']", "css selector").FindElements(By::TagName("li"));
		
		GetAriaLogger("discord").Info(Format("Found %d message elements", msg_elements.GetCount()));
		
		for (int i = 0; i < msg_elements.GetCount(); i++) {
			try {
				Element& e = msg_elements[i];
				// Skip if not a real message (e.g. date separators)
				if (e.GetAttribute("class").Find("message") < 0) continue;
				
				ValueMap msg;
				
				// Extract author
				try {
					Element author_el = e.FindElement(By::CssSelector("span[class*='username']"));
					msg.Set("author", author_el.GetText());
				} catch (...) { msg.Set("author", "Unknown"); }
				
				// Extract content
				try {
					Element content_el = e.FindElement(By::CssSelector("div[class*='messageContent']"));
					msg.Set("content", content_el.GetText());
				} catch (...) { msg.Set("content", ""); }
				
				// Extract timestamp
				try {
					Element time_el = e.FindElement(By::CssSelector("time"));
					msg.Set("timestamp", time_el.GetAttribute("datetime"));
				} catch (...) { msg.Set("timestamp", ""); }
				
				if (!IsNull(msg["content"]))
					messages.Add(msg);
					
			} catch (...) {}
		}
		
		GetAriaLogger("discord").Info(Format("Extracted %d valid messages", messages.GetCount()));
		
		// Store in site manager
		ValueArray va;
		for (const auto& m : messages) va.Add(m);
		sm.SetSiteData(site_name, "messages", va);
		
		return true;
	} catch (const Exc& e) {
		GetAriaLogger("discord").Error("Error during Discord refresh: " + e);
		return false;
	}
}

END_UPP_NAMESPACE
