#include "ThreadsCtrl.h"
#include "AriaHub.h"

NAMESPACE_UPP

ThreadsCtrl::ThreadsCtrl() {
	Add(tabs.SizePos());
	
	// Initialize Lists
	InitList(feed_list);
	InitList(public_list);
	InitList(private_list);
	
	// Add Tabs
	tabs.Add(feed_list.SizePos(), "Feed");
	tabs.Add(public_list.SizePos(), "Public Messages");
	tabs.Add(private_list.SizePos(), "Private Messages");
	tabs.Add(settings_tab.SizePos(), "Config");
	
	// Settings Tab Layout
	int y = 10;
	settings_tab.Add(btnRefresh.LeftPos(10, 150).TopPos(y, 30));
	btnRefresh.SetLabel("Refresh All Data");
	btnRefresh << [=] { Scrape(); };
	y += 40;
	
	settings_tab.Add(lblDepth.LeftPos(10, 100).TopPos(y, 20));
	lblDepth.SetLabel("Scrape Depth:");
	settings_tab.Add(scrape_depth.LeftPos(110, 50).TopPos(y, 20));
	scrape_depth <<= 50;
	y += 30;
	
	settings_tab.Add(auto_refresh.LeftPos(10, 150).TopPos(y, 20));
	auto_refresh.SetLabel("Auto Refresh");
}

void ThreadsCtrl::InitList(ArrayCtrl& list) {
	list.AddColumn("Author", 1);
	list.AddColumn("Content", 4);
}

void ThreadsCtrl::RefreshSubTab(int tab_index) {
	if (!navigator || !site_manager) return;
	
	ThreadsScraper scraper(*navigator, *site_manager);
	try {
		bool success = false;
		if (tab_index == 0) { // Feed
			success = scraper.RefreshFeed();
		} else if (tab_index == 1) { // Public
			success = scraper.RefreshPublic();
		} else if (tab_index == 2) { // Private
			success = scraper.RefreshPrivate();
		} else {
			success = scraper.Refresh(false); 
		}
		
		if (success) LoadData();
		else AriaAlert("Threads refresh failed. Check logs.");
	} catch (const std::exception& e) {
		AriaAlert("Error refreshing Threads tab: " + String(e.what()));
	}
}

void ThreadsCtrl::Scrape() {
	if (!navigator || !site_manager) return;
	
	ThreadsScraper scraper(*navigator, *site_manager);
	try {
		if (scraper.Refresh(true)) { // Deep refresh
			LoadData();
		} else {
			AriaAlert("Threads refresh failed (possibly not logged in). Check logs.");
		}
	} catch (const Exc& e) {
		AriaAlert("Error scraping Threads: " + e);
	} catch (const std::exception& e) {
		AriaAlert("System Error scraping Threads: " + String(e.what()));
	}
}

void ThreadsCtrl::LoadData() {
	if (!site_manager) return;
	
	auto LoadTab = [&](ArrayCtrl& list, const String& key) {
		list.Clear();
		Value data = site_manager->GetSiteData("threads", key);
		if (data.Is<ValueArray>()) {
			for (const Value& post : (ValueArray)data) {
				list.Add(post["author"], post["content"]);
			}
		}
	};

	LoadTab(feed_list, "feed");
	LoadTab(public_list, "public");
	LoadTab(private_list, "private");
}

END_UPP_NAMESPACE
