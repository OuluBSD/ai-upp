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
	GetAriaLogger("threads").Info(Format("Starting %s data refresh for Threads...", deep ? "deep" : "shallow"));
	
	ValueArray feed = ScrapeFeed();
	GetAriaLogger("threads").Info(Format("Found %d posts in general feed.", feed.GetCount()));
	
	sm.SetSiteData(site_name, "feed", feed);
	sm.SetSiteData(site_name, "metadata", ValueMap()
		("last_refresh", GetUtcTime())
		("mode", deep ? "deep" : "shallow")
	);
	
	return true;
}

ValueArray ThreadsScraper::ScrapeFeed() {
	try {
		GetAriaLogger("threads").Info("Scraping feed...");
		// Use JS to extract posts
		Value res = navigator.Eval(R"(
			const posts = [];
			const postEls = document.querySelectorAll('div[data-pressable-container="true"]');
			postEls.forEach(el => {
				try {
					const authorEl = el.querySelector('a[href^="/@"]');
					const contentEl = el.querySelector('span');
					if (authorEl && contentEl) {
						posts.push({
							author: authorEl.innerText.trim(),
							content: contentEl.innerText.trim()
						});
					}
				} catch(e) {}
			});
			return posts;
		)");
		
		if (res.Is<ValueArray>()) return res;
	} catch (const Exc& e) {
		GetAriaLogger("threads").Error("Error scraping feed: " + e);
	}
	return ValueArray();
}

END_UPP_NAMESPACE
