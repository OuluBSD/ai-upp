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
	settings_tab.Add(btnRefresh.LeftPos(10, 150).TopPos(10, 30));
	btnRefresh.SetLabel("Refresh All Data");
	btnRefresh << [=] { Scrape(); };
}

void ThreadsCtrl::InitList(ArrayCtrl& list) {
	list.AddColumn("Author", 1);
	list.AddColumn("Content", 4);
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
	
	// Feed
	feed_list.Clear();
	Value feed = site_manager->GetSiteData("threads", "feed");
	if (feed.Is<ValueArray>()) {
		for (const Value& post : (ValueArray)feed) {
			feed_list.Add(post["author"], post["content"]);
		}
	}
	
	public_list.Clear();
	private_list.Clear();
}

END_UPP_NAMESPACE
