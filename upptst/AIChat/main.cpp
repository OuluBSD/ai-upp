#include "AIChat.h"

AIChat::AIChat() {
	Title("Maestro AI Chat");
	SetRect(0, 0, 1000, 700);
	Sizeable().Zoomable();
	
	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));
	
	Add(tabs.SizePos());
	
tabs.Add(chat.SizePos(), "Chat Playground");
	
system_splitter.Horz() << repo_view << plan_view;
	tabs.Add(system_splitter.SizePos(), "System");
	
	// Initialize System Data
	String root = GetFileDirectory(GetExeFilePath());
	// Search upwards for .git or uppsrc to find repo root
	while(!root.IsEmpty() && !DirectoryExists(AppendFileName(root, "uppsrc"))) {
		String next = GetFileDirectory(root);
		if(next == root) break;
		root = next;
	}
	
	if(!root.IsEmpty()) {
		RepoScanner rs;
		rs.Scan(root);
		rs.DetectAssemblies();
		repo_view.Set(rs.assemblies, rs.packages);
		
		PlanParser pp;
		pp.Load(AppendFileName(root, "uppsrc/AI/plan"));
		plan_view.Set(pp.tracks);
	}
	
	// Automated testing support
	const Vector<String>& cmdline = CommandLine();
	String backend;
	String input;
	
	for(int i = 0; i < cmdline.GetCount(); i++) {
		if(cmdline[i] == "--backend" && i + 1 < cmdline.GetCount())
			backend = cmdline[++i];
		else if(cmdline[i] == "--input" && i + 1 < cmdline.GetCount())
			input = cmdline[++i];
	}
	
	if(!backend.IsEmpty() && !input.IsEmpty()) {
		chat.engine_select.SetData(backend);
		chat.input.SetData(input);
		
		chat.WhenDone = [=] {
			Cout() << "=== TEST RESULT DEBUG DUMP ===\n";
			Cout() << chat.engine.debug_log << "\n";
			Cout() << "=== END DUMP ===\n";
			PostCallback(callback(this, &TopWindow::Close));
		};
		
		PostCallback([=] { chat.OnSend(); });
	}
}

void AIChat::MainMenu(Bar& bar) {
	bar.Sub("Edit", [=](Bar& bar) {
		bar.Add("Copy All Chat", [=] { chat.CopyAllChat(); });
		bar.Add("Copy Debug Data", [=] { chat.CopyDebugData(); });
	});
}

GUI_APP_MAIN {
	AIChat().Run();
}
