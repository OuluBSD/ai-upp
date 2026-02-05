#ifndef _MaestroHub_TUBrowser_h_
#define _MaestroHub_TUBrowser_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

NAMESPACE_UPP

#ifndef _MaestroHub_Layout_Included_
#define _MaestroHub_Layout_Included_
#define LAYOUTFILE <MaestroHub/MaestroHub.lay>
#include <CtrlCore/lay.h>
#endif

class TUBrowser : public WithTUBrowserLayout<ParentCtrl> {
	ParentCtrl left_pane;
	EditString pkg_search;
	ArrayCtrl  pkg_list;
	
	ParentCtrl right_pane;
	TabCtrl    details;
	
	ParentCtrl symbol_pane;
	EditString sym_search;
	ArrayCtrl  sym_list;
	
	RichTextView dep_view;
	
	One<TuManager> tum;
	String root;

public:
	void Load(const String& maestro_root);
	void UpdatePackages();
	void OnPackageCursor();
	void UpdateSymbols();
	
	typedef TUBrowser CLASSNAME;
	TUBrowser();
};

END_UPP_NAMESPACE

#endif
