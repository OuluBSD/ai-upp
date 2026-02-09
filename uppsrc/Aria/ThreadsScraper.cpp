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
		// Wait for either the feed or the login screen
		navigator.WaitForElement("svg[aria-label='Threads'], div[aria-label='Log in'], a[href='/login/']", "css selector", 45);
		
		// Check for login indicators
		Value login_check = navigator.Eval(R"(
			(function() {
				if (document.querySelector('div[aria-label="Log in"]') || document.querySelector('a[href="/login/"]')) return "login_screen";
				if (document.querySelector('svg[aria-label="Threads"]')) return "logged_in";
				return "unknown";
			})()
		)");
		
		if (login_check == "login_screen") {
			GetAriaLogger("threads").Error("Not logged in to Threads. Please log in manually with the profile first.");
			return false;
		}
		
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

bool ThreadsScraper::RefreshFeed() {
	if (!Navigate()) return false;
	ValueArray feed = ScrapeFeed();
	sm.SetSiteData(site_name, "feed", feed);
	return true;
}

ValueArray ThreadsScraper::ScrapeFeed() {
	try {
		GetAriaLogger("threads").Info("Scraping feed...");
		
		// Wait for at least one post to appear
		bool found = false;
		for (int i = 0; i < 10; i++) {
			Value count = navigator.Eval("return document.querySelectorAll('div[data-pressable-container=\"true\"]').length;");
			if (count.Is<double>() && (double)count > 0) {
				found = true;
				break;
			}
			GetAriaLogger("threads").Info("Waiting for posts to load...");
			Sleep(1000);
		}
		
		if (!found) {
			GetAriaLogger("threads").Warning("No posts appeared after waiting.");
		}

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
