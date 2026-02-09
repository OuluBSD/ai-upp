#include "WhatsAppCtrl.h"
#include "AriaHub.h"

NAMESPACE_UPP

WhatsAppCtrl::WhatsAppCtrl() {
	Add(list.VSizePos(0, 40).HSizePos());
	Add(btnRefresh.BottomPos(5, 30).RightPos(5, 100));
	
	list.AddColumn("Metadata", 2);
	list.AddColumn("Message", 5);
	
	btnRefresh.SetLabel("Refresh WhatsApp");
	btnRefresh << [=] { Scrape(); };
}

void WhatsAppCtrl::Scrape() {

	if (!navigator || !site_manager) return;

	

	WhatsAppScraper scraper(*navigator, *site_manager);

	try {

		if (scraper.Refresh()) {

			LoadData();

		} else {

			AriaAlert("WhatsApp refresh failed. Check logs.");

		}

	} catch (const Exc& e) {

		AriaAlert("Error scraping WhatsApp: " + e);

	} catch (const std::exception& e) {

		AriaAlert("System Error scraping WhatsApp: " + String(e.what()));

	}

}

void WhatsAppCtrl::LoadData() {
	if (!site_manager) return;
	list.Clear();
	Value data = site_manager->GetSiteData("whatsapp", "messages");
	if (data.Is<ValueArray>()) {
		for (const Value& msg : (ValueArray)data) {
			list.Add(msg["raw_metadata"], msg["content"]);
		}
	}
}

END_UPP_NAMESPACE
