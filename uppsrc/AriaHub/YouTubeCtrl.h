#ifndef _AriaHub_YouTubeCtrl_h_
#define _AriaHub_YouTubeCtrl_h_

#include "ServiceCtrl.h"

NAMESPACE_UPP

class YouTubeCtrl : public ServiceCtrl {
public:
	typedef YouTubeCtrl CLASSNAME;
	YouTubeCtrl();
	
	virtual void   LoadData() override;
	virtual void   Scrape() override;
	virtual void   RefreshSubTab(int tab_index) override { Scrape(); }
	virtual void   RefreshService() override { Scrape(); }
	virtual String GetTitle() override { return "YouTube"; }
	virtual Image  GetIcon() override  { return Image(); }
	virtual int    GetActiveTab() override { return 0; }

private:
	ArrayCtrl list;
	Button    btnRefresh;
};

END_UPP_NAMESPACE

#endif
