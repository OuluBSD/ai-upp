#include "ScriptIDE.h"

namespace Upp {

namespace {

String PreferencesNavLabel(const String& category, const String& title)
{
	return category.IsEmpty() ? title : category + " / " + title;
}

struct PreferencesWindowState {
	String selected_page_id;

	void Serialize(Stream& s) {
		s % selected_page_id;
	}
};

}

String PreferencesWindow::ConfigPath()
{
	return ConfigFile("scriptide-preferences-window.bin");
}

void PreferencesWindow::AddPage(const String& id, const String& title, Image icon, PreferencesPage* page)
{
	AddPage(String(), id, title, icon, page);
}

void PreferencesWindow::AddPage(const String& category, const String& id, const String& title, Image icon, PreferencesPage* page)
{
	PageEntry& e = pages.Add();
	e.id = id;
	e.category = category;
	e.title = title;
	e.icon = icon;
	e.page = page;
	
	nav.Add(PreferencesNavLabel(category, title));
	page_host.Add(page->SizePos());
	page->Hide();
	
	page->Load(settings);
}

void PreferencesWindow::AddPage(const String& id, const String& title, Image icon, IPluginPreferencesPage* page)
{
	AddPage(String(), id, title, icon, page);
}

void PreferencesWindow::AddPage(const String& category, const String& id, const String& title, Image icon, IPluginPreferencesPage* page)
{
	PageEntry& e = pages.Add();
	e.id = id;
	e.category = category;
	e.title = title;
	e.icon = icon;
	e.plugin_page = page;

	nav.Add(PreferencesNavLabel(category, title));
	Ctrl& ctrl = page->GetCtrl();
	page_host.Add(ctrl.SizePos());
	ctrl.Hide();

	page->LoadConfig();
}

void PreferencesWindow::OnNavSelection()
{
	int idx = nav.GetCursor();
	if(idx < 0 || idx >= pages.GetCount()) return;

	PreferencesWindowState st;
	st.selected_page_id = pages[idx].id;
	StoreToFile(st, ConfigPath());
	
	for(int i = 0; i < pages.GetCount(); i++) {
		Ctrl* ctrl = pages[i].page
			? static_cast<Ctrl*>(pages[i].page)
			: &pages[i].plugin_page->GetCtrl();
		if(i == idx)
			ctrl->Show();
		else
			ctrl->Hide();
	}
}

void PreferencesWindow::OnOK()
{
	OnApply();
	Break(IDOK);
}

void PreferencesWindow::OnCancel()
{
	if(ctx.main_window && ctx.main_window->plugin_manager) {
		const auto& plugins = ctx.main_window->plugin_manager->GetPlugins();
		for(int i = 0; i < plugins.GetCount(); i++) {
			bool enabled = true;
			for(const auto& ps : old_settings.plugins.states) {
				if(ps.id == plugins.GetKey(i)) {
					enabled = ps.enabled;
					break;
				}
			}
			ctx.main_window->plugin_manager->EnablePlugin(plugins.GetKey(i), enabled);
		}
	}
	settings.CopyFrom(old_settings);
	Break(IDCANCEL);
}

void PreferencesWindow::OnApply()
{
	IDESettings new_settings;
	new_settings.CopyFrom(settings);
	
	for(int i = 0; i < pages.GetCount(); i++) {
		if(pages[i].page)
			pages[i].page->Save(new_settings);
	}
	
	for(int i = 0; i < pages.GetCount(); i++) {
		if(pages[i].page)
			pages[i].page->Apply(ctx, settings, new_settings);
		else if(pages[i].plugin_page) {
			pages[i].plugin_page->SaveConfig();
			pages[i].plugin_page->ApplyConfig(ctx);
		}
	}
	
	settings.CopyFrom(new_settings);
	old_settings.CopyFrom(settings);
	StoreToFile(settings, ConfigFile("ide_settings.bin"));
	RefreshPluginPages();
}

void PreferencesWindow::OnResetDefaults()
{
	for(int i = 0; i < pages.GetCount(); i++) {
		if(pages[i].page) {
			pages[i].page->SetDefaults();
			pages[i].page->Load(settings);
		}
		else if(pages[i].plugin_page) {
			pages[i].plugin_page->SetDefaults();
			pages[i].plugin_page->LoadConfig();
		}
	}
}

void PreferencesWindow::RefreshPluginPages()
{
	String selected_id;
	int cur = nav.GetCursor();
	if(cur >= 0 && cur < pages.GetCount())
		selected_id = pages[cur].id;
	ClearPages();
	PopulatePages();
	int target = 0;
	if(!selected_id.IsEmpty()) {
		for(int i = 0; i < pages.GetCount(); i++) {
			if(pages[i].id == selected_id) {
				target = i;
				break;
			}
		}
	}
	if(pages.GetCount()) {
		nav.SetCursor(target);
		OnNavSelection();
	}
}

void PreferencesWindow::MarkModified() {}

}
