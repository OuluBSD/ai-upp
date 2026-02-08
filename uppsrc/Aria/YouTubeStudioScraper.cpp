#include "Aria.h"

NAMESPACE_UPP

const char* YouTubeStudioScraper::URL = "https://studio.youtube.com/";

YouTubeStudioScraper::YouTubeStudioScraper(AriaNavigator& navigator, SiteManager& sm)
	: navigator(navigator)
	, sm(sm)
{
	site_name = "youtube-studio";
}

bool YouTubeStudioScraper::Navigate() {
	GetAriaLogger("youtube_studio").Info("Navigating to YouTube Studio...");
	try {
		navigator.Navigate(URL);
		navigator.WaitForElement("ytcp-app", "tag name", 45);
		return true;
	} catch (const Exc& e) {
		GetAriaLogger("youtube_studio").Error("Failed to load YouTube Studio: " + e);
		return false;
	}
}

bool YouTubeStudioScraper::Refresh(bool deep) {
	if (!Navigate()) return false;
	GetAriaLogger("youtube_studio").Info("Refreshing YouTube Studio data...");
	
	ValueMap analytics = ScrapeDashboard();
	ValueArray videos = ScrapeVideos();
	
	sm.SetSiteData(site_name, "analytics", analytics);
	sm.SetSiteData(site_name, "videos", videos);
	sm.SetSiteData(site_name, "metadata", ValueMap()
		("last_refresh", GetUtcTime())
		("video_count", videos.GetCount())
	);
	
	return true;
}

ValueMap YouTubeStudioScraper::ScrapeDashboard() {
	try {
		GetAriaLogger("youtube_studio").Info("Scraping dashboard...");
		// Use JS to extract summary text from snapshot cards
		Value res = navigator.Eval(R"(
			const cards = document.querySelectorAll('ytcp-video-snapshot, ytcp-channel-snapshot');
			let summary = "";
			cards.forEach(c => { summary += c.innerText + "\n"; });
			return { channel_summary: summary.trim() };
		)");
		
		if (res.Is<ValueMap>()) return res;
	} catch (const Exc& e) {
		GetAriaLogger("youtube_studio").Error("Error scraping dashboard: " + e);
	}
	return ValueMap();
}

ValueArray YouTubeStudioScraper::ScrapeVideos() {
	try {
		GetAriaLogger("youtube_studio").Info("Scraping videos...");
		// Comprehensive JS extraction for videos and views
		Value res = navigator.Eval(R"(
			const titleEls = document.querySelectorAll('[id*="video-title"], [class*="video-title"]');
			const videos = [];
			titleEls.forEach(el => {
				const title = el.innerText.trim();
				if (!title || title.length < 3) return;
				
				let views = "0";
				let container = el.closest('div') || el.parentElement;
				if (container) {
					const text = container.innerText;
					const match = text.match(/(\d+)/);
					if (match) views = match[1];
				}
				videos.push({ title: title, views: views });
			});
			return videos;
		)");
		
		if (res.Is<ValueArray>()) return res;
	} catch (const Exc& e) {
		GetAriaLogger("youtube_studio").Error("Error scraping videos: " + e);
	}
	return ValueArray();
}

END_UPP_NAMESPACE
