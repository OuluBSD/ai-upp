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
		
		// Force visibility and focus events
		navigator.Eval(R"(
			document.dispatchEvent(new Event('visibilitychange'));
			window.dispatchEvent(new Event('focus'));
		)");

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

bool ThreadsScraper::RefreshPublic() {
	if (!Navigate()) return false;
	GetAriaLogger("threads").Info("Navigating to user profile...");
	try {
		// Click profile link/icon
		navigator.FindElement(By::CssSelector("a[href^='/@']")).Click();
		Sleep(2000);
		ValueArray posts = ScrapePublic();
		sm.SetSiteData(site_name, "public", posts);
		return true;
	} catch (const Exc& e) {
		GetAriaLogger("threads").Error("Failed to refresh public messages: " + e);
		return false;
	}
}

bool ThreadsScraper::RefreshPrivate() {
	if (!Navigate()) return false;
	GetAriaLogger("threads").Info("Navigating to activity/messages...");
	try {
		// Navigate to activity as a proxy for private interactions
		navigator.Navigate("https://www.threads.net/notifications");
		Sleep(2000);
		ValueArray items = ScrapePrivate();
		sm.SetSiteData(site_name, "private", items);
		return true;
	} catch (const Exc& e) {
		GetAriaLogger("threads").Error("Failed to refresh private messages: " + e);
		return false;
	}
}

ValueArray ThreadsScraper::ScrapeFeed() {
	try {
		GetAriaLogger("threads").Info("Scraping feed...");
		
		// Wait for at least one post to appear
		bool found = false;
		for (int i = 0; i < 10; i++) {
			Value count = navigator.Eval("return document.querySelectorAll('article, [role=\"article\"], [data-pressable-container=\"true\"]').length;");
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
			return (function() {
				const posts = [];
				const containers = document.querySelectorAll('article, [role="article\"], [data-pressable-container="true"]');
				for (let i = 0; i < containers.length; i++) {
					const art = containers[i];
					try {
						// 1. Find author
						const links = art.querySelectorAll('a');
						let author = "";
						for (let j = 0; j < links.length; j++) {
							const href = links[j].getAttribute('href') || "";
							if (href.includes('@')) {
								author = links[j].innerText.trim();
								if (author) break;
							}
						}
						if (!author) continue;

						// 2. Find content
						let content = "";
						const candidates = art.querySelectorAll('[dir="auto"]');
						for (let k = 0; k < candidates.length; k++) {
							const node = candidates[k];
							if (node.querySelector('[dir="auto"]')) continue;
							
							const txt = (node.innerText || node.textContent || "").trim();
							if (!txt || txt === author || author.includes(txt)) continue;
							if (txt.match(/^\d+\s*[hdmsw]$/) || txt === "•" || txt === "Reply" || txt === "Threads") continue;
							
							if (txt.length > content.length) content = txt;
						}

						if (author && content && content.length > 2) {
							if (!posts.some(p => p.author === author && p.content === content)) {
								posts.push({author: author, content: content});
							}
						}
					} catch(e) {}
				}
				return { count: containers.length, posts: posts };
			})()
		)");
		
		if (res.Is<ValueMap>()) {
			ValueArray posts = res["posts"];
			GetAriaLogger("threads").Info(Format("Scraper saw %d containers, extracted %d posts.", (int)res["count"], posts.GetCount()));
			return posts;
		}
	} catch (const Exc& e) {
		GetAriaLogger("threads").Error("Error scraping feed: " + e);
	}
	return ValueArray();
}

ValueArray ThreadsScraper::ScrapePublic() {
	GetAriaLogger("threads").Info("Scraping public profile...");
	return ScrapeFeed(); // Reuse feed scraping logic for profile posts
}

ValueArray ThreadsScraper::ScrapePrivate() {
	GetAriaLogger("threads").Info("Scraping activity/notifications...");
	try {
		Value res = navigator.Eval(R"(
			const items = [];
			const els = document.querySelectorAll('div[role="listitem"]');
			els.forEach(el => {
				try {
					const textEl = el.innerText;
					if (textEl) {
						items.push({
							author: "System",
							content: textEl.trim()
						});
					}
				} catch(e) {}
			});
			return items;
		)");
		if (res.Is<ValueArray>()) return res;
	} catch (const Exc& e) {
		GetAriaLogger("threads").Error("Error scraping activity: " + e);
	}
	return ValueArray();
}

END_UPP_NAMESPACE