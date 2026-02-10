#ifndef _AriaHub_ThreadsCtrl_h_
#define _AriaHub_ThreadsCtrl_h_

#include "ServiceCtrl.h"

NAMESPACE_UPP

class ThreadsCtrl : public ServiceCtrl {
public:
	typedef ThreadsCtrl CLASSNAME;
	ThreadsCtrl();
	
	virtual void   LoadData() override;
	virtual void   Scrape() override;
	virtual void   RefreshSubTab(int tab_index) override;
	virtual void   RefreshService() override { Scrape(); }
	virtual String GetTitle() override { return "Threads"; }
	virtual Image  GetIcon() override  { return Image(); } // Placeholder
	virtual int    GetActiveTab() override { return tabs.Get(); }

private:
	TabCtrl tabs;
	
	ArrayCtrl feed_list;
	ArrayCtrl public_list;
	ArrayCtrl private_list;
	ParentCtrl settings_tab;
	
	Button btnRefresh;
	Label lblDepth;
	EditInt scrape_depth;
	Option auto_refresh;
	
	void InitList(ArrayCtrl& list);
};

END_UPP_NAMESPACE

#endif