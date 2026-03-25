#ifdef flagMAIN
#include "ScriptIDE.h"
#include <memory>

using namespace Upp;

GUI_APP_MAIN
{
	struct TimedHandClick : Moveable<TimedHandClick> { int ms = 0; int count = 0; };
	struct TimedButtonPress : Moveable<TimedButtonPress> { int ms = 0; String id; };
	struct TimedFormButtonPress : Moveable<TimedFormButtonPress> { int ms = 0; String id; };
	struct TimedFormButtonAction : Moveable<TimedFormButtonAction> { int ms = 0; String id; };
	struct TimedCardClick : Moveable<TimedCardClick> { int ms = 0; String id; };

	const Vector<String>& args = CommandLine();
	String path;
	bool dump_scene = false;
	bool dump_console = false;
	bool dump_python_stack = false;
	bool exit_on_assert = false;
	bool stdout_log = true;
	bool maximize_window = false;
	bool autostart = false;
	Size window_size = Size(0, 0);
	int timeout_ms = 0;
	int stop_after_ms = -1;
	int run_after_ms = -1;
	int debug_after_ms = -1;
	int click_after_ms = 800;
	int press_after_ms = 1400;
	int click_first_hand_cards = 0;
	Vector<String> click_cards;
	Vector<String> press_buttons;
	Vector<String> press_form_buttons;
	Vector<String> call_form_button_actions;
	Vector<TimedHandClick> timed_hand_clicks;
	Vector<TimedButtonPress> timed_button_presses;
	Vector<TimedFormButtonPress> timed_form_button_presses;
	Vector<TimedFormButtonAction> timed_form_button_actions;
	Vector<TimedCardClick> timed_card_clicks;

	for(int i = 0; i < args.GetCount(); i++) {
		const String& arg = args[i];
		if(arg == "--dump-scene") { dump_scene = true; continue; }
		if(arg == "--dump-console") { dump_console = true; continue; }
		if(arg == "--dump-python-stack") { dump_python_stack = true; continue; }
		if(arg == "--exit-on-assert") { exit_on_assert = true; continue; }
		if(arg == "--stdout-log") { stdout_log = true; continue; }
		if(arg == "--no-stdout-log") { stdout_log = false; continue; }
		if(arg == "--maximize") { maximize_window = true; continue; }
		if(arg == "--autostart") { autostart = true; continue; }
		if(arg.StartsWith("--size=")) {
			Vector<String> sz = Split(arg.Mid(7), "x");
			if(sz.GetCount() == 2) window_size = Size(StrInt(sz[0]), StrInt(sz[1]));
			continue;
		}
		if(arg.StartsWith("--timeout-ms=")) { timeout_ms = max(0, ScanInt(arg.Mid(13))); continue; }
		if(arg.StartsWith("--stop-after-ms=")) { stop_after_ms = max(0, ScanInt(arg.Mid(16))); continue; }
		if(arg.StartsWith("--run-after-ms=")) { run_after_ms = max(0, ScanInt(arg.Mid(15))); continue; }
		if(arg.StartsWith("--debug-after-ms=")) { debug_after_ms = max(0, ScanInt(arg.Mid(17))); continue; }
		if(arg.StartsWith("--click-after-ms=")) { click_after_ms = max(0, ScanInt(arg.Mid(17))); continue; }
		if(arg.StartsWith("--press-after-ms=")) { press_after_ms = max(0, ScanInt(arg.Mid(17))); continue; }
		if(arg.StartsWith("--click-card=")) { click_cards.Add(arg.Mid(13)); continue; }
		if(arg.StartsWith("--press-button=")) { press_buttons.Add(arg.Mid(15)); continue; }
		if(arg.StartsWith("--press-form-button=")) { press_form_buttons.Add(arg.Mid(20)); continue; }
		if(arg.StartsWith("--call-form-button-action=")) { call_form_button_actions.Add(arg.Mid(26)); continue; }
		if(arg.StartsWith("--click-first-hand-cards=")) { click_first_hand_cards = max(0, ScanInt(arg.Mid(25))); continue; }
		if(arg.StartsWith("--timed-hand-click=")) {
			Vector<String> p = Split(arg.Mid(19), ":");
			if(p.GetCount() == 2) { TimedHandClick& th = timed_hand_clicks.Add(); th.ms = StrInt(p[0]); th.count = StrInt(p[1]); }
			continue;
		}
		if(arg.StartsWith("--timed-button-press=")) {
			Vector<String> p = Split(arg.Mid(21), ":");
			if(p.GetCount() == 2) { TimedButtonPress& tp = timed_button_presses.Add(); tp.ms = StrInt(p[0]); tp.id = p[1]; }
			continue;
		}
		if(arg.StartsWith("--timed-form-button-press=")) {
			Vector<String> p = Split(arg.Mid(26), ":");
			if(p.GetCount() == 2) { TimedFormButtonPress& tp = timed_form_button_presses.Add(); tp.ms = StrInt(p[0]); tp.id = p[1]; }
			continue;
		}
		if(arg.StartsWith("--timed-form-button-action=")) {
			Vector<String> p = Split(arg.Mid(27), ":");
			if(p.GetCount() == 2) { TimedFormButtonAction& tp = timed_form_button_actions.Add(); tp.ms = StrInt(p[0]); tp.id = p[1]; }
			continue;
		}
		if(arg.StartsWith("--timed-card-click=")) {
			Vector<String> p = Split(arg.Mid(19), ":");
			if(p.GetCount() == 2) { TimedCardClick& tc = timed_card_clicks.Add(); tc.ms = StrInt(p[0]); tc.id = p[1]; }
			continue;
		}
		if(FileExists(arg)) path = NormalizePath(GetFullPath(arg));
	}

	if(!path.IsEmpty() && FileExists(path)) {
		SetCurrentDirectory(GetFileDirectory(path));
	}

	PythonIDE ide;
	ide.plugin_manager->SyncBindings(ide.vm);
	CardGameDocumentHost::log_to_stdout = stdout_log;
	CardGameDocumentHost::exit_on_assert = exit_on_assert;
	ide.console_pane->MirrorStdout(dump_console);
	
	if(maximize_window) ide.Maximize();

	if(!path.IsEmpty() && FileExists(path)) {
		ide.LoadFile(path);
		if(ToLower(GetFileExt(path)) == ".gamestate") {
			// Ensure the visible editor tab uses the actual card-game host.
			int file_idx = -1;
			for(int i = 0; i < ide.open_files.GetCount(); i++) {
				if(ide.open_files[i].path == path) {
					file_idx = i;
					break;
				}
			}
			if(file_idx < 0 && ide.active_file >= 0 && ide.active_file < ide.open_files.GetCount())
				file_idx = ide.active_file;

			CardGameDocumentHost* host = nullptr;
			if(file_idx >= 0 && file_idx < ide.open_files.GetCount())
				host = dynamic_cast<CardGameDocumentHost*>(ide.open_files[file_idx].editor);

			bool attached = host && host->GetCtrl().GetParent() == &ide.editor_area;
			if(!host || !attached) {
				host = new CardGameDocumentHost();
				if(file_idx >= 0 && file_idx < ide.open_files.GetCount()) {
					PythonIDE::FileInfo& fi = ide.open_files[file_idx];
					if(fi.editor) {
						fi.editor->GetCtrl().Remove();
						delete fi.editor;
					}
					fi.editor = host;
					fi.is_plugin = true;
					ide.editor_area.Add(host->GetCtrl().VSizePos(36, 0).HSizePos());
					ide.active_file = file_idx;
				}
				else {
					PythonIDE::FileInfo& fi = ide.open_files.Add();
					fi.path = path;
					fi.editor = host;
					fi.is_plugin = true;
					ide.editor_area.Add(host->GetCtrl().VSizePos(36, 0).HSizePos());
					ide.active_file = ide.open_files.GetCount() - 1;
					ide.editor_tabs->Add(GetFileName(path), Icons::File());
				}
				host->Load(path);
			}
			ide.editor_tabs->SetCursor(ide.active_file);
			ide.OnTabChanged();
			// --size sets the game-content fixed area, not the window size.
			if(window_size.cx > 0 && window_size.cy > 0)
				host->SetFixedArea(window_size);
		}
	}

	if(autostart || run_after_ms >= 0) {
		int delay = autostart ? 500 : run_after_ms;
		SetTimeCallback(delay, [&] { ide.OnRun(); }, (void*)0xC0A0);
	}

	if(stop_after_ms >= 0) SetTimeCallback(stop_after_ms, [&] { ide.OnStop(); }, (void*)0xC0E0);
	if(debug_after_ms >= 0) SetTimeCallback(debug_after_ms, [&] { ide.OnDebug(); }, (void*)0xC0E2);

	if(click_cards.GetCount() || press_buttons.GetCount() || press_form_buttons.GetCount() || call_form_button_actions.GetCount() || click_first_hand_cards > 0) {
		SetTimeCallback(click_after_ms, [&] {
			if(click_first_hand_cards > 0) ide.InvokeActiveSceneFirstHandCards(click_first_hand_cards);
			for(int i = 0; i < click_cards.GetCount(); i++) ide.InvokeActiveSceneCard(click_cards[i]);
		}, (void*)0xC0DE);
		if(press_buttons.GetCount() || press_form_buttons.GetCount() || call_form_button_actions.GetCount()) {
			SetTimeCallback(press_after_ms, [&] {
				for(int i = 0; i < press_buttons.GetCount(); i++) ide.InvokeActiveSceneButton(press_buttons[i]);
				for(int i = 0; i < press_form_buttons.GetCount(); i++) ide.PressActiveFormButton(press_form_buttons[i]);
				for(int i = 0; i < call_form_button_actions.GetCount(); i++) ide.CallActiveFormButtonAction(call_form_button_actions[i]);
			}, (void*)0xC0DF);
		}
	}
	
	for(int i = 0; i < timed_hand_clicks.GetCount(); i++) {
		TimedHandClick th = timed_hand_clicks[i];
		SetTimeCallback(th.ms, [&, th] { if(th.count > 0) ide.InvokeActiveSceneFirstHandCards(th.count); }, (void*)(uintptr_t)(0xC100 + i));
	}
	for(int i = 0; i < timed_button_presses.GetCount(); i++) {
		TimedButtonPress tp = timed_button_presses[i];
		SetTimeCallback(tp.ms, [&, tp] { ide.InvokeActiveSceneButton(tp.id); }, (void*)(uintptr_t)(0xC200 + i));
	}
	for(int i = 0; i < timed_form_button_presses.GetCount(); i++) {
		TimedFormButtonPress tp = timed_form_button_presses[i];
		SetTimeCallback(tp.ms, [&, tp] { ide.PressActiveFormButton(tp.id); }, (void*)(uintptr_t)(0xC280 + i));
	}
	for(int i = 0; i < timed_form_button_actions.GetCount(); i++) {
		TimedFormButtonAction tp = timed_form_button_actions[i];
		SetTimeCallback(tp.ms, [&, tp] { ide.CallActiveFormButtonAction(tp.id); }, (void*)(uintptr_t)(0xC290 + i));
	}
	for(int i = 0; i < timed_card_clicks.GetCount(); i++) {
		TimedCardClick tc = timed_card_clicks[i];
		SetTimeCallback(tc.ms, [&, tc] { ide.InvokeActiveSceneCard(tc.id); }, (void*)(uintptr_t)(0xC300 + i));
	}

	if(timeout_ms > 0 && (dump_scene || dump_console || dump_python_stack)) {
		SetTimeCallback(timeout_ms, [&ide, dump_scene, dump_console, dump_python_stack, timeout_ms] {
			if(dump_scene) LOG(ide.DumpActiveScene());
			if(dump_python_stack) LOG(ide.DumpActivePythonStack());
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
		}, (void*)0xC0E4);
	}

	ide.Run();
}
#endif
