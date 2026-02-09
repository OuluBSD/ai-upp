#ifndef _AriaHub_ServiceCtrl_h_
#define _AriaHub_ServiceCtrl_h_

#include <CtrlLib/CtrlLib.h>
#include <Aria/Aria.h>

NAMESPACE_UPP

class ServiceCtrl : public ParentCtrl {
public:
	typedef ServiceCtrl CLASSNAME;
	ServiceCtrl() : navigator(nullptr), site_manager(nullptr) {}
	
	virtual void   SetNavigator(AriaNavigator* nav, SiteManager* sm) { navigator = nav; site_manager = sm; }
	virtual void   LoadData() = 0;
	virtual void   Scrape() = 0;
	virtual String GetTitle() = 0;
	virtual Image  GetIcon() { return Image(); }

protected:
	AriaNavigator* navigator;
	SiteManager*   site_manager;
};

END_UPP_NAMESPACE

#endif
