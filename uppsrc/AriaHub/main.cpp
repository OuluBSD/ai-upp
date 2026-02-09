#include "AriaHub.h"

NAMESPACE_UPP

AriaMainWindow::AriaMainWindow()
{
	Title("AriaHub - Unified Web Services").Sizeable().Zoomable();
	Icon(CtrlImg::Network());

	AddFrame(menu);
	AddFrame(toolbar);
	AddFrame(statusbar);
	Add(tabs.SizePos());

	menu.LayoutId("MenuBar");
	toolbar.LayoutId("ToolBar");
	tabs.LayoutId("Tabs");

	menu.Set(THISBACK(MainMenu));

	toolbar.Set([=](Bar& bar) {
		bar.Add("Refresh All", CtrlImg::plus(), THISBACK(RefreshAll));
		bar.Separator();
		bar.Add("Settings", CtrlImg::exclamation(), THISBACK(OpenSettings));
		bar.Add("Stop Scrapers", CtrlImg::remove(), THISBACK(StopScrapers));
	});

	statusbar.Set("Ready");

	// Initialize backend links
	threads.SetNavigator(&aria.GetNavigator(), &aria.GetSiteManager());
	whatsapp.SetNavigator(&aria.GetNavigator(), &aria.GetSiteManager());

	// Services
	tabs.Add(threads.SizePos(), "Threads");
	tabs.Add(whatsapp.SizePos(), "WhatsApp");
	
	// Placeholder tabs for other services
	tabs.Add("Dashboard");
	tabs.Add("Google Messages");
	tabs.Add("Universal Inbox");
	tabs.Add("YouTube");
	tabs.Add("Calendar");
}

bool AriaMainWindow::Key(dword key, int count)
{
	if (menu.Key(key, count)) return true;
	return TopWindow::Key(key, count);
}

void AriaMainWindow::MainMenu(Bar& bar)
{
	bar.Sub("Service", THISBACK(ServiceMenu));
}

void AriaMainWindow::ServiceMenu(Bar& bar)
{
	bar.Add("Refresh Active Sub-tab", THISBACK(RefreshActiveSubTab)).Key(K_F5);
	bar.Add("Refresh Active Service", THISBACK(RefreshActiveService)).Key(K_SHIFT_F5);
	bar.Add("Refresh All Services", THISBACK(RefreshAllServices)).Key(K_CTRL | K_SHIFT | K_F5);
}

void AriaMainWindow::RefreshActiveSubTab()
{
	int active = tabs.Get();
	Ctrl* c = tabs.GetItem(active).GetSlave();
	ServiceCtrl* sc = dynamic_cast<ServiceCtrl*>(c);
	if (sc) {
		String title = sc->GetTitle();
		if (title == "Discord") {
			AriaAlert("Discord refresh is currently disabled.");
			return;
		}
		int sub = sc->GetActiveTab();
		String msg = Format("Refreshing %s: Sub-tab %d...", title, sub);
		statusbar.Set(msg);
		sc->RefreshSubTab(sub);
		statusbar.Set("Ready");
	}
}

void AriaMainWindow::RefreshActiveService()
{
	Ctrl* c = tabs.GetItem(tabs.Get()).GetSlave();
	ServiceCtrl* sc = dynamic_cast<ServiceCtrl*>(c);
	if (sc) {
		String title = sc->GetTitle();
		if (title == "Discord") {
			AriaAlert("Discord refresh is currently disabled.");
			return;
		}
		String msg = Format("Refreshing entire %s service...", title);
		statusbar.Set(msg);
		sc->RefreshService();
		statusbar.Set("Ready");
	}
}

void AriaMainWindow::RefreshAllServices()
{
	statusbar.Set("Refreshing all services...");
	for (int i = 0; i < tabs.GetCount(); i++) {
		Ctrl* c = tabs.GetItem(i).GetSlave();
		ServiceCtrl* sc = dynamic_cast<ServiceCtrl*>(c);
		if (sc) {
			if (sc->GetTitle() == "Discord") continue;
			sc->RefreshService();
		}
	}
	statusbar.Set("Ready");
}

void AriaMainWindow::RefreshAll()
{
	RefreshAllServices();
}

void AriaMainWindow::OpenSettings()
{
	PromptOK("Settings dialog placeholder");
}

void AriaMainWindow::StopScrapers()
{
	statusbar.Set("Stopping all scrapers...");
}

bool IsAutomation()
{
	static bool b = false;
	static bool checked = false;
	if(!checked) {
		const Vector<String>& args = CommandLine();
		for(int i = 0; i < args.GetCount(); i++)
			if(args[i] == "--test") b = true;
		checked = true;
	}
	return b;
}

void AriaAlert(const String& msg)
{
	GuiLock __;
	LOG("ALERT: " << msg);
	if(IsAutomation()) {
		Cout() << "ALERT: " << msg << "\n";
	}
	else {
		Exclamation(msg);
	}
}

END_UPP_NAMESPACE

GUI_APP_MAIN
{
	Upp::StdLogSetup(Upp::LOG_COUT | Upp::LOG_FILE);
	
	const Upp::Vector<Upp::String>& args = Upp::CommandLine();
	Upp::String test_script;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--test" && i + 1 < args.GetCount()) {
			test_script = args[i + 1];
			break;
		}
	}

	if(!test_script.IsEmpty()) {
		Upp::AriaMainWindow* main = new Upp::AriaMainWindow();
		main->Open();
		
		Upp::PyVM vm;
		Upp::RegisterAutomationBindings(vm);
		
		try {
			Upp::String source = Upp::LoadFile(test_script);
			if (source.IsEmpty()) throw Upp::Exc("Could not load script file");
			
			Upp::Vector<Upp::ProcMsg> errors;
			Upp::Tokenizer tokenizer;
			tokenizer.WhenMessage << [&](const Upp::ProcMsg& m) { if(m.severity == Upp::PROCMSG_ERROR) errors.Add(m); };
			tokenizer.SkipPythonComments(true);
			if (!tokenizer.Process(source, test_script)) {
				Upp::Cout() << "Tokenization failed:\n";
				for(const auto& e : errors)
					Upp::Cout() << e.line << ":" << e.col << ": " << e.msg << "\n";
				exit(1);
			}
			tokenizer.CombineTokens();
			
			Upp::PyCompiler compiler(tokenizer.GetTokens());
			Upp::Vector<Upp::PyIR> ir;
			compiler.Compile(ir);
			
			vm.SetIR(ir);
			vm.Run();
		} catch (const Upp::Exc& e) {
			if(e.Find("EXIT:0") >= 0) {
				_exit(0);
			}
			Upp::Cout() << "Test Error: " << e << "\n";
			_exit(1);
		}
		_exit(0);
		return;
	}

	Upp::AriaMainWindow().Run();
}