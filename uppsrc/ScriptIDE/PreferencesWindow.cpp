#include "ScriptIDE.h"

namespace Upp {

void PreferencesWindow::AddPage(const String& id, const String& title, Image icon, PreferencesPage* page)
{
	PageEntry& e = pages.Add();
	e.id = id;
	e.title = title;
	e.icon = icon;
	e.page = page;
	
	nav.Add(title);
	page_host.Add(page->SizePos());
	page->Hide();
	
	page->Load(settings);
}

void PreferencesWindow::OnNavSelection()
{
	int idx = nav.GetCursor();
	if(idx < 0 || idx >= pages.GetCount()) return;
	
	for(int i = 0; i < pages.GetCount(); i++) {
		if(i == idx)
			pages[i].page->Show();
		else
			pages[i].page->Hide();
	}
}

void PreferencesWindow::OnOK()
{
	OnApply();
	Break(IDOK);
}

void PreferencesWindow::OnCancel()
{
	settings.CopyFrom(old_settings);
	Break(IDCANCEL);
}

void PreferencesWindow::OnApply()
{
	IDESettings new_settings;
	new_settings.CopyFrom(settings);
	
	for(int i = 0; i < pages.GetCount(); i++)
		pages[i].page->Save(new_settings);
	
	for(int i = 0; i < pages.GetCount(); i++)
		pages[i].page->Apply(ctx, settings, new_settings);
	
	settings.CopyFrom(new_settings);
}

void PreferencesWindow::OnResetDefaults()
{
	for(int i = 0; i < pages.GetCount(); i++) {
		pages[i].page->SetDefaults();
		pages[i].page->Load(settings);
	}
}

void PreferencesWindow::MarkModified() {}

}
