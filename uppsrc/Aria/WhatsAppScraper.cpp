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
	
	GetAriaLogger("whatsapp").Info("Refreshing WhatsApp data...");
	
	try {
		// Wait for message container
		navigator.WaitForElement("div[role='application']", "css selector", 20);
		
		Vector<ValueMap> messages;
		
		// In WhatsApp Web, messages are often in 'div' with class containing 'message-'
		Vector<Element> msg_elements = navigator.FindElements(By::CssSelector("div[class*='message-']"));
		
		GetAriaLogger("whatsapp").Info(Format("Found %d potential message elements", msg_elements.GetCount()));
		
		for (int i = 0; i < msg_elements.GetCount(); i++) {
			try {
				Element& e = msg_elements[i];
				String text = e.GetText();
				if (text.IsEmpty()) continue;
				
				ValueMap msg;
				
				// WhatsApp structure is complex, often author is in a separate div or we're in a group
				// For now, simplified extraction
				msg.Set("content", text);
				
				// Try to find metadata (timestamp etc)
				try {
					Element info = e.FindElement(By::CssSelector("div[class*='copyable-text']"));
					String data = info.GetAttribute("data-pre-plain-text");
					if (!data.IsEmpty()) {
						// Format: "[HH:MM, YYYY-MM-DD] Author: "
						msg.Set("raw_metadata", data);
					}
				} catch (...) {}
				
				messages.Add(msg);
			} catch (...) {}
		}
		
		GetAriaLogger("whatsapp").Info(Format("Extracted %d valid messages", messages.GetCount()));
		
		ValueArray va;
		for (const auto& m : messages) va.Add(m);
		sm.SetSiteData(site_name, "messages", va);
		
		return true;
	} catch (const Exc& e) {
		GetAriaLogger("whatsapp").Error("Error during WhatsApp refresh: " + e);
		return false;
	}
}

END_UPP_NAMESPACE