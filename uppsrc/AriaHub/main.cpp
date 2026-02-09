#include "AriaHub.h"

NAMESPACE_UPP

AriaMainWindow::AriaMainWindow()
{
	Title("AriaHub - Unified Web Services").Sizeable().Zoomable();
	Icon(CtrlImg::Network());

	AddFrame(toolbar);
	AddFrame(statusbar);
	Add(tabs.SizePos());

	toolbar.Set([=](Bar& bar) {
		bar.Add("Refresh All", CtrlImg::plus(), [=] { RefreshAll(); });
		bar.Separator();
		bar.Add("Settings", CtrlImg::exclamation(), [=] { OpenSettings(); });
		bar.Add("Stop Scrapers", CtrlImg::remove(), [=] { StopScrapers(); });
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

void AriaMainWindow::RefreshAll()
{
	statusbar.Set("Refreshing all services...");
	// Logic to trigger background scrapers
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