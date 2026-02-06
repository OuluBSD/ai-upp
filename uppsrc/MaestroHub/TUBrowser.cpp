#include "MaestroHub.h"

NAMESPACE_UPP

TUBrowser::TUBrowser() {
	CtrlLayout(*this);
	
	left_pane.Add(pkg_search.TopPos(0, 24).HSizePos());
	left_pane.Add(pkg_list.VSizePos(24, 0).HSizePos());
	
	pkg_list.AddColumn("Package");
	pkg_list.WhenCursor = THISBACK(OnPackageCursor);
	
	symbol_pane.Add(sym_search.TopPos(0, 24).HSizePos());
	symbol_pane.Add(sym_list.VSizePos(24, 0).HSizePos());
	sym_list.AddColumn("Symbol");
	sym_list.AddColumn("Type");
	sym_list.AddColumn("Location");
	
	details.Add(symbol_pane.SizePos(), "Symbols");
	details.Add(dep_view.SizePos(), "Dependencies");
	
	split.Horz(left_pane, details);
	split.SetPos(2500);
	
	pkg_search.NullText("Filter packages...");
	pkg_search.WhenAction = THISBACK(UpdatePackages);
	
	sym_search.NullText("Filter symbols...");
	sym_search.WhenAction = THISBACK(UpdateSymbols);
}

void TUBrowser::Load(const String& maestro_root) {
	root = maestro_root;
	tum.Create(root);
	UpdatePackages();
}

void TUBrowser::UpdatePackages() {
	pkg_list.Clear();
	String filter = pkg_search.GetData().ToString();
	
	Vector<String> pkgs = tum->ListPackages(); 
	
	for(const auto& p : pkgs) {
		if(filter.IsEmpty() || p.Find(filter) >= 0)
			pkg_list.Add(p);
	}
	
	if(pkg_list.GetCount() > 0) pkg_list.SetCursor(0);
}

void TUBrowser::OnPackageCursor() {
	sym_list.Clear();
	dep_view.SetQTF("");
	
	if(!pkg_list.IsCursor()) return;
	String pkg = pkg_list.Get(0);
	
	UpdateSymbols();
	
	dep_view.SetQTF("[* Dependencies for " + pkg + ":]&Loading...");
}

void TUBrowser::UpdateSymbols() {
	sym_list.Clear();
	if(!pkg_list.IsCursor()) return;
	String pkg = pkg_list.Get(0);
	
	sym_list.Add("FooClass", "class", pkg + "/Foo.h:10");
	sym_list.Add("BarFunction", "func", pkg + "/Bar.cpp:50");
}

END_UPP_NAMESPACE