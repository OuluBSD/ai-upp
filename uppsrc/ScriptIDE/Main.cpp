#include "ScriptIDE.h"
#include <memory>

using namespace Upp;

GUI_APP_MAIN
{
    const Vector<String>& args = CommandLine();
    String path;
    bool dump_scene = false;
    bool dump_console = false;
    bool stdout_log = false;
    bool maximize_window = false;
    bool autostart = false;
    int timeout_ms = 1500;
    int stop_after_ms = -1;
    int run_after_ms = -1;
    int debug_after_ms = -1;
    int click_first_hand_cards = 0;
	Vector<String> click_cards;
	Vector<String> press_buttons;
	for(int i = 0; i < args.GetCount(); i++) {
		const String& arg = args[i];
        if(arg == "--dump-scene") {
            dump_scene = true;
            continue;
        }
        if(arg == "--dump-console") {
            dump_console = true;
            continue;
        }
        if(arg == "--stdout-log") {
            stdout_log = true;
            continue;
        }
        if(arg == "--maximize") {
            maximize_window = true;
            continue;
        }
        if(arg == "--autostart") {
            autostart = true;
            continue;
        }
        if(arg.StartsWith("--timeout-ms=")) {
            timeout_ms = max(0, ScanInt(arg.Mid(13)));
            continue;
        }
        if(arg.StartsWith("--stop-after-ms=")) {
            stop_after_ms = max(0, ScanInt(arg.Mid(16)));
            continue;
        }
        if(arg.StartsWith("--run-after-ms=")) {
            run_after_ms = max(0, ScanInt(arg.Mid(15)));
            continue;
        }
        if(arg.StartsWith("--debug-after-ms=")) {
            debug_after_ms = max(0, ScanInt(arg.Mid(17)));
            continue;
        }
        if(arg.StartsWith("--click-card=")) {
            click_cards.Add(arg.Mid(13));
            continue;
        }
        if(arg.StartsWith("--click-first-hand-cards=")) {
            click_first_hand_cards = max(0, ScanInt(arg.Mid(25)));
            continue;
        }
        if(arg.StartsWith("--press-button=")) {
            press_buttons.Add(arg.Mid(15));
            continue;
        }
        if(arg.StartsWith("--"))
            continue;
        if(path.IsEmpty())
            path = GetFullPath(arg);
    }
    if(!path.IsEmpty() && FileExists(path)) {
        SetCurrentDirectory(GetFileDirectory(path));
    }

    PythonIDE ide;
    CardGameDocumentHost::log_to_stdout = stdout_log;
    ide.console_pane->MirrorStdout(dump_console);
    if(maximize_window)
        ide.Maximize();

	if(!path.IsEmpty() && FileExists(path)) {
		ide.LoadFile(path);
		if(GetFileExt(path) != ".gamestate" || autostart)
			ide.OnRun();
	}

    if(stop_after_ms >= 0)
        SetTimeCallback(stop_after_ms, [&] { ide.OnStop(); }, (void*)0xC0E0);
    if(run_after_ms >= 0)
        SetTimeCallback(run_after_ms, [&] { ide.OnRun(); }, (void*)0xC0E1);
    if(debug_after_ms >= 0)
        SetTimeCallback(debug_after_ms, [&] { ide.OnDebug(); }, (void*)0xC0E2);

    if(click_cards.GetCount() || press_buttons.GetCount() || click_first_hand_cards > 0) {
        SetTimeCallback(300, [&] {
            if(click_first_hand_cards > 0)
                ide.InvokeActiveSceneFirstHandCards(click_first_hand_cards);
            for(int i = 0; i < click_cards.GetCount(); i++)
                ide.InvokeActiveSceneCard(click_cards[i]);
        }, (void*)0xC0DE);
        if(press_buttons.GetCount()) {
            SetTimeCallback(650, [&] {
                for(int i = 0; i < press_buttons.GetCount(); i++)
                    ide.InvokeActiveSceneButton(press_buttons[i]);
            }, (void*)0xC0DF);
        }
    }

	if(dump_scene || dump_console) {
		SetTimeCallback(timeout_ms, [&ide, dump_scene, dump_console, timeout_ms] {
			Cout() << "[timeout] fired at " << timeout_ms << "ms\n";
			KillTimeCallback((void*)0xC0E0);
			KillTimeCallback((void*)0xC0E1);
			KillTimeCallback((void*)0xC0E2);
			KillTimeCallback((void*)0xC0E3);
			Cout() << "[timeout] cancelled timed actions\n";
			ide.OnStop();
			Cout() << "[timeout] stop requested\n";
			auto poll_done = std::make_shared<Function<void ()>>();
			dword wait0 = msecs();
			*poll_done = [&, dump_scene, dump_console, wait0, poll_done] {
				bool runners = ide.HasActiveRunners();
				int elapsed = int(msecs(wait0));
				Cout() << "[timeout] poll elapsed=" << elapsed << " runners=" << (runners ? 1 : 0) << "\n";
				if(runners && elapsed < 1000) {
					SetTimeCallback(50, *poll_done, (void*)0xC0E3);
					return;
				}
				Cout() << "[timeout] dumping\n";
				if(dump_scene)
					Cout() << ide.DumpActiveScene();
				Cout() << "[timeout] force close\n";
				ide.ForceCloseNow();
			};
			SetTimeCallback(50, *poll_done, (void*)0xC0E3);
        }, &ide);
    }

    ide.Run();
}
