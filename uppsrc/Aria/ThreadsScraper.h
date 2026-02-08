#ifndef _Aria_ThreadsScraper_h_
#define _Aria_ThreadsScraper_h_

class ThreadsScraper {
	AriaNavigator& navigator;
	SiteManager& sm;
	String site_name;
	static const char* URL;

public:
	ThreadsScraper(AriaNavigator& navigator, SiteManager& sm);
	
	bool Navigate();
	bool Refresh(bool deep = false);
	ValueArray ScrapeFeed();
};

#endif
