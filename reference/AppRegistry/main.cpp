#include "Core/Core.h"

using namespace Upp;

struct SampleState {
	String active_tab_id;
	int    active_tab_index = 0;
	String mode;

	void Jsonize(JsonIO& json) {
		json("active_tab_id",    active_tab_id)
		    ("active_tab_index", active_tab_index)
		    ("mode",             mode);
	}
};

static bool CheckFailed(const char* label)
{
	Cout() << "FAIL: " << label << "\n";
	SetExitCode(1);
	return false;
}

static void PrintLog(const AppLog& log)
{
	static const char* kLevelName[] = { "TRACE", "DEBUG", "INFO ", "WARN ", "ERROR" };
	Cout() << "=== Structured log (" << log.GetRecords().GetCount() << " records) ===\n";
	for(const AppLogRecord& r : log.GetRecords()) {
		int lv = clamp(r.level, 0, 4);
		String loc = r.file.IsEmpty() ? String() : " (" + r.file + ":" + IntStr(r.line) + ")";
		Cout() << Format("[%02d:%02d:%02d][%s][%s] %s%s\n",
		                 r.time.hour, r.time.minute, r.time.second,
		                 kLevelName[lv], r.channel, r.message, loc);
	}
	Cout() << "\n";
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);

	AppLog app_log;
	app_log.SetForwardToUppLog(false);  // avoid double-printing in console example

	AppRegistry reg;
	reg.Vendor("AiUpp").AppId("AppRegistry").Profile("default").SetLog(&app_log);

	// ---- Print directories --------------------------------------------------
	Cout() << "=== Directories ===\n";
	Cout() << "Config : " << reg.GetConfigDir() << "\n";
	Cout() << "State  : " << reg.GetStateDir()  << "\n";
	Cout() << "Cache  : " << reg.GetCacheDir()  << "\n\n";

	// ---- Save scalars -------------------------------------------------------
	Cout() << "=== Scalar values ===\n";
	reg.Set("sample.string", "hello");
	reg.Set("sample.int",    42);
	reg.Set("sample.bool",   true);

	if(!reg.Save()) CheckFailed("Save scalars");

	AppRegistry reg2;
	reg2.Vendor("AiUpp").AppId("AppRegistry").Profile("default").SetLog(&app_log);
	if(!reg2.Load()) CheckFailed("Load registry");

	if(String(reg2.Get("sample.string")) != "hello")
		CheckFailed("sample.string round-trip");
	if(int(reg2.Get("sample.int")) != 42)
		CheckFailed("sample.int round-trip");
	if(bool(reg2.Get("sample.bool")) != true)
		CheckFailed("sample.bool round-trip");

	Cout() << "Scalars: OK\n\n";

	// ---- JSON state ---------------------------------------------------------
	Cout() << "=== JSON state ===\n";
	SampleState st;
	st.active_tab_id    = "Tab 1";
	st.active_tab_index = 0;
	st.mode             = "View";

	reg.SaveJson("app.state", st);
	reg.Save();

	AppRegistry reg3;
	reg3.Vendor("AiUpp").AppId("AppRegistry").Profile("default").SetLog(&app_log);
	reg3.Load();

	SampleState st2;
	if(!reg3.LoadJson("app.state", st2)) CheckFailed("LoadJson app.state");
	if(st2.active_tab_id != "Tab 1")    CheckFailed("active_tab_id round-trip");
	if(st2.active_tab_index != 0)       CheckFailed("active_tab_index round-trip");
	if(st2.mode != "View")              CheckFailed("mode round-trip");

	Cout() << "JSON state: OK\n\n";

	// ---- Inline blob (small) ------------------------------------------------
	Cout() << "=== Inline blob ===\n";
	String small_data = "small binary payload";
	reg.SaveBlob("sample.small", small_data, AppRegistry::BLOB_INLINE_BASE64);
	reg.Save();

	AppRegistry reg4;
	reg4.Vendor("AiUpp").AppId("AppRegistry").Profile("default").SetLog(&app_log);
	reg4.Load();

	String loaded_small;
	if(!reg4.LoadBlob("sample.small", loaded_small)) CheckFailed("LoadBlob small");
	if(loaded_small != small_data) CheckFailed("Inline blob round-trip");
	Cout() << "Inline blob: OK\n\n";

	// ---- External blob (large) ----------------------------------------------
	Cout() << "=== External blob ===\n";
	String large_data('X', 80 * 1024);  // 80 KiB, exceeds inline threshold
	reg.SaveBlob("sample.large", large_data, AppRegistry::BLOB_EXTERNAL_FILE);
	reg.Save();
	Cout() << "External blob path: " << reg.GetBlobPath("sample.large") << "\n";

	AppRegistry reg5;
	reg5.Vendor("AiUpp").AppId("AppRegistry").Profile("default").SetLog(&app_log);
	reg5.Load();

	String loaded_large;
	if(!reg5.LoadBlob("sample.large", loaded_large)) CheckFailed("LoadBlob large");
	if(loaded_large != large_data) CheckFailed("External blob round-trip");
	Cout() << "External blob: OK\n\n";

	// ---- Auto mode: small inline, large external ----------------------------
	Cout() << "=== Auto-mode blob ===\n";
	String auto_small = "auto small";
	reg.SaveBlob("sample.auto_small", auto_small);   // BLOB_AUTO -> inline
	String auto_large('Y', 100 * 1024);              // 100 KiB -> external
	reg.SaveBlob("sample.auto_large", auto_large);
	reg.Save();

	AppRegistry reg6;
	reg6.Vendor("AiUpp").AppId("AppRegistry").Profile("default").SetLog(&app_log);
	reg6.Load();

	String out_as, out_al;
	if(!reg6.LoadBlob("sample.auto_small", out_as)) CheckFailed("auto_small load");
	if(out_as != auto_small) CheckFailed("auto_small round-trip");
	if(!reg6.LoadBlob("sample.auto_large", out_al)) CheckFailed("auto_large load");
	if(out_al != auto_large) CheckFailed("auto_large round-trip");
	Cout() << "Auto-mode blob: OK\n\n";

	// ---- Print structured log -----------------------------------------------
	PrintLog(app_log);

	if(GetExitCode() == 0)
		Cout() << "All checks passed.\n";
}
