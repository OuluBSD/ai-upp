#include "ScriptIDE.h"
#include <memory>

using namespace Upp;

GUI_APP_MAIN
{
	struct TimedHandClick : Moveable<TimedHandClick> {
		int ms = 0;
		int count = 0;
	};
	struct TimedButtonPress : Moveable<TimedButtonPress> {
		int ms = 0;
		String id;
	};
	struct TimedCardClick : Moveable<TimedCardClick> {
		int ms = 0;
		String id;
	};

    const Vector<String>& args = CommandLine();
    String path;
    bool dump_scene = false;
    bool dump_console = false;
    bool dump_python_stack = false;
    bool exit_on_assert = false;
    bool stdout_log = false;
    bool maximize_window = false;
    bool autostart = false;
    int timeout_ms = 1500;
    int stop_after_ms = -1;
    int run_after_ms = -1;
    int debug_after_ms = -1;
    int click_after_ms = 800;
    int press_after_ms = 1400;
    int click_first_hand_cards = 0;
	Vector<String> click_cards;
	Vector<String> press_buttons;
	Vector<TimedHandClick> timed_hand_clicks;
	Vector<TimedButtonPress> timed_button_presses;
	Vector<TimedCardClick> timed_card_clicks;
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
        if(arg == "--dump-python-stack") {
            dump_python_stack = true;
            continue;
        }
        if(arg == "--exit-on-assert") {
            exit_on_assert = true;
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
        if(arg.StartsWith("--click-after-ms=")) {
            click_after_ms = max(0, ScanInt(arg.Mid(17)));
            continue;
        }
        if(arg.StartsWith("--press-after-ms=")) {
            press_after_ms = max(0, ScanInt(arg.Mid(17)));
            continue;
        }
        if(arg.StartsWith("--click-card=")) {
            click_cards.Add(arg.Mid(13));
            continue;
        }
        if(arg.StartsWith("--click-card-at=")) {
            String spec = arg.Mid(16);
            int colon = spec.Find(':');
            if(colon > 0) {
                TimedCardClick& tc = timed_card_clicks.Add();
                tc.ms = max(0, ScanInt(spec.Left(colon)));
                tc.id = spec.Mid(colon + 1);
            }
            continue;
        }
        if(arg.StartsWith("--click-first-hand-cards=")) {
            click_first_hand_cards = max(0, ScanInt(arg.Mid(25)));
            continue;
        }
        if(arg.StartsWith("--click-first-hand-cards-at=")) {
            String spec = arg.Mid(28);
            int colon = spec.Find(':');
            if(colon > 0) {
                TimedHandClick& th = timed_hand_clicks.Add();
                th.ms = max(0, ScanInt(spec.Left(colon)));
                th.count = max(0, ScanInt(spec.Mid(colon + 1)));
            }
            continue;
        }
        if(arg.StartsWith("--press-button=")) {
            press_buttons.Add(arg.Mid(15));
            continue;
        }
        if(arg.StartsWith("--press-button-at=")) {
            String spec = arg.Mid(18);
            int colon = spec.Find(':');
            if(colon > 0) {
                TimedButtonPress& tp = timed_button_presses.Add();
                tp.ms = max(0, ScanInt(spec.Left(colon)));
                tp.id = spec.Mid(colon + 1);
            }
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
    CardGameDocumentHost::exit_on_assert = exit_on_assert;
    ide.console_pane->MirrorStdout(dump_console);
    if(maximize_window)
        ide.Maximize();

	if(!path.IsEmpty() && FileExists(path)) {
		ide.LoadFile(path);
		if(GetFileExt(path) != ".gamestate")
			ide.OnRun();
		else if(autostart)
			SetTimeCallback(100, [&] { ide.OnRun(); }, (void*)0xC0A0);
	}

    if(stop_after_ms >= 0)
        SetTimeCallback(stop_after_ms, [&] { ide.OnStop(); }, (void*)0xC0E0);
    if(run_after_ms >= 0)
        SetTimeCallback(run_after_ms, [&] { ide.OnRun(); }, (void*)0xC0E1);
    if(debug_after_ms >= 0)
        SetTimeCallback(debug_after_ms, [&] { ide.OnDebug(); }, (void*)0xC0E2);

    if(click_cards.GetCount() || press_buttons.GetCount() || click_first_hand_cards > 0) {
        SetTimeCallback(click_after_ms, [&] {
            if(click_first_hand_cards > 0)
                ide.InvokeActiveSceneFirstHandCards(click_first_hand_cards);
            for(int i = 0; i < click_cards.GetCount(); i++)
                ide.InvokeActiveSceneCard(click_cards[i]);
        }, (void*)0xC0DE);
        if(press_buttons.GetCount()) {
            SetTimeCallback(press_after_ms, [&] {
                for(int i = 0; i < press_buttons.GetCount(); i++)
                    ide.InvokeActiveSceneButton(press_buttons[i]);
            }, (void*)0xC0DF);
        }
    }
    for(int i = 0; i < timed_hand_clicks.GetCount(); i++) {
        TimedHandClick th = timed_hand_clicks[i];
        SetTimeCallback(th.ms, [&, th] {
            if(th.count > 0)
                ide.InvokeActiveSceneFirstHandCards(th.count);
        }, (void*)(uintptr_t)(0xC100 + i));
    }
    for(int i = 0; i < timed_button_presses.GetCount(); i++) {
        TimedButtonPress tp = timed_button_presses[i];
        SetTimeCallback(tp.ms, [&, tp] {
            ide.InvokeActiveSceneButton(tp.id);
        }, (void*)(uintptr_t)(0xC200 + i));
    }
    for(int i = 0; i < timed_card_clicks.GetCount(); i++) {
        TimedCardClick tc = timed_card_clicks[i];
        SetTimeCallback(tc.ms, [&, tc] {
            ide.InvokeActiveSceneCard(tc.id);
        }, (void*)(uintptr_t)(0xC300 + i));
    }

	if(dump_scene || dump_console || dump_python_stack) {
		SetTimeCallback(timeout_ms, [&ide, dump_scene, dump_console, dump_python_stack, timeout_ms] {
			if(dump_scene)
				Cout() << ide.DumpActiveScene();
			if(dump_python_stack)
				Cout() << ide.DumpActivePythonStack();
			KillTimeCallback((void*)0xC0E0);
			KillTimeCallback((void*)0xC0E1);
			KillTimeCallback((void*)0xC0E2);
			KillTimeCallback((void*)0xC0E3);
			ide.OnStop();
			auto poll_done = std::make_shared<Function<void ()>>();
			dword wait0 = msecs();
			*poll_done = [&, dump_scene, dump_console, wait0, poll_done] {
				bool runners = ide.HasActiveRunners();
				int elapsed = int(msecs(wait0));
				if(runners && elapsed < 1000) {
					SetTimeCallback(50, *poll_done, (void*)0xC0E3);
					return;
				}
				ide.ForceCloseNow();
			};
			SetTimeCallback(50, *poll_done, (void*)0xC0E3);
        }, &ide);
    }

    ide.Run();
}
