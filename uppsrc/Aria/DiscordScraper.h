#ifndef _Aria_DiscordScraper_h_
#define _Aria_DiscordScraper_h_

class DiscordScraper {
	AriaNavigator& navigator;
	SiteManager& sm;
	String site_name;
	static const char* URL;

public:
	DiscordScraper(AriaNavigator& navigator, SiteManager& sm);
	
	bool Navigate();
	bool Refresh(bool deep = false);
};

#endif
