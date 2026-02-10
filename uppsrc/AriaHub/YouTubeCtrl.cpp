#include "YouTubeCtrl.h"
#include "AriaHub.h"

NAMESPACE_UPP

YouTubeCtrl::YouTubeCtrl() {
	Add(list.VSizePos(0, 40).HSizePos());
	Add(btnRefresh.BottomPos(5, 30).RightPos(5, 150));
	
	list.AddColumn("Thumb").FixedWidth(60);
	list.AddColumn("Title", 5);
	list.AddColumn("Views", 2).Sorting();
	list.AddColumn("Likes", 2);
	list.AddColumn("Comments", 2);
	list.AddColumn("CTR", 2);
	
	btnRefresh.SetLabel("Sync Analytics");
	btnRefresh << [=] { Scrape(); };
}

void YouTubeCtrl::Scrape() {
	if (!navigator || !site_manager) return;
	
	YouTubeStudioScraper scraper(*navigator, *site_manager);
	try {
		if (scraper.Refresh()) {
			LoadData();
		} else {
			AriaAlert("YouTube Studio sync failed. Check login.");
		}
	} catch (const Exc& e) {
		AriaAlert("Error syncing YouTube: " + e);
	}
}

void YouTubeCtrl::LoadData() {
	if (!site_manager) return;
	list.Clear();
	
	Value data = site_manager->GetSiteData("youtube_studio", "videos");
	if (data.Is<ValueArray>()) {
		for (const Value& v : (ValueArray)data) {
			list.Add(Image(), v["title"], v["views"], v["likes"], v["comments"], v["ctr"]);
		}
	}
}

END_UPP_NAMESPACE
