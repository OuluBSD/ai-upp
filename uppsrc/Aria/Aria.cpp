#include "Aria.h"
#include <plugin/pcre/Pcre.h>

NAMESPACE_UPP

String GeminiProvider::Generate(const String& prompt, const String& context, const String& output_format) {
	String full_prompt = prompt;
	if (!context.IsEmpty()) {
		full_prompt = "Context:\n" + context + "\n\nUser Request: " + prompt;
	}
	
	if (output_format == "json") {
		full_prompt << "\n\nIMPORTANT: Return ONLY valid JSON. Do not include markdown code blocks or any other text.";
	} else if (output_format == "markdown") {
		full_prompt << "\n\nIMPORTANT: Use Markdown formatting.";
	}
	
	String gemini_cli = GetHomeDirFile("node_modules/.bin/gemini");
	if (!FileExists(gemini_cli)) {
		return "Error: Gemini CLI not found at " + gemini_cli;
	}
	
	String cmd = gemini_cli + " -y --model gemini-3-flash-preview -o stream-json -";
	
	LocalProcess p;
	if (!p.Start2(cmd)) return "Error starting Gemini CLI";
	
	p.Write(full_prompt);
	p.CloseWrite();
	
	String stdout_str, stderr_str;
	while (p.IsRunning()) {
		String o, e;
		if (p.Read2(o, e)) {
			stdout_str << o;
			stderr_str << e;
		}
		Sleep(1);
	}
	
	if (p.GetExitCode() != 0) {
		return "Gemini CLI error: " + stderr_str;
	}
	
	String response_text;
	Vector<String> lines = Split(stdout_str, '\n');
	for (const String& line : lines) {
		if (TrimBoth(line).IsEmpty()) continue;
		Value v = ParseJSON(line);
		if (v.Is<ValueMap>()) {
			ValueMap vm = v;
			if (vm["type"] == "message" && vm["role"] == "assistant") {
				response_text << (String)vm["content"];
			}
		}
	}
	
	String text = TrimBoth(response_text);
	if (output_format == "json") {
		if (text.StartsWith("```json")) {
			text = text.Mid(7);
			int q = text.ReverseFind("```");
			if (q >= 0) text = text.Left(q);
		} else if (text.StartsWith("```")) {
			text = text.Mid(3);
			int q = text.ReverseFind("```");
			if (q >= 0) text = text.Left(q);
		}
	}
	
	return TrimBoth(text);
}

String OtpManager::ExtractOtp(const String& text) {
	// Look for 4-8 digit codes, often surrounded by boundaries
	RegExp re("(\\b\\d{4,8}\\b)");
	if (re.GlobalMatch(text)) {
		return re[0];
	}
	return "";
}

Aria::Aria() {
	plugin_manager.LoadPlugins();
	navigator.Create();
	// Use default gemini provider
	static GeminiProvider default_gemini;
	navigator->SetAIProvider(&default_gemini);
}

void Aria::Run(const Vector<String>& args) {
	CommandLineArguments cla;
	cla.SetDescription("Aria CLI - Your web automation assistant.");
	cla.AddArg("log-level", 0, "Set the logging level.", true, "{DEBUG,INFO,WARNING,ERROR,CRITICAL}");
	cla.AddArg("json-logs", 0, "Use JSON format for file logging.", false);
	cla.AddArg("trace-id", 0, "Optional trace ID for this execution.", true, "TRACE_ID");
	cla.AddArg("force", 0, "Force actions and bypass safety warnings.", false);
	cla.AddArg("slow-mo", 0, "Add a delay in seconds between browser actions.", true, "SLOW_MO");
	cla.AddArg("provider", 0, "The AI provider to use for generation.", true, "PROVIDER");
	cla.AddArg("navigator", 0, "The navigator engine to use.", true, "NAVIGATOR");
	cla.AddArg("version", 'v', "Show version information.", false);
	cla.AddArg("help", 'h', "show this help message and exit", false);
	
	cla.SetStopOnPositional();
	cla.AddPositional("{help,open,close,page,script,report,site,diag,settings,man,tutorial,version,security,vault,test,plugin}");
	cla.AddHelpText("    help                Show this help message and exit.");
	cla.AddHelpText("    open                Open a browser instance.");
	cla.AddHelpText("    close               Close browser instances.");
	cla.AddHelpText("    page                Manage browser pages.");
	cla.AddHelpText("    script              Manage automation scripts.");
	cla.AddHelpText("    report              Manage local reports.");
	cla.AddHelpText("    site                Manage site-specific data and scraping.");
	cla.AddHelpText("    diag                Show diagnostic information.");
	cla.AddHelpText("    settings            Show current configuration.");
	cla.AddHelpText("    man                 Show manual pages.");
	cla.AddHelpText("    tutorial            Start the interactive tutorial.");
	cla.AddHelpText("    version             Show version information.");
	cla.AddHelpText("    security            Show security best practices.");
	cla.AddHelpText("    vault               Manage credentials.");
	cla.AddHelpText("    test                Run diagnostic tests.");
	cla.AddHelpText("    plugin              Manage plugins.");
	
	bool parsed = cla.Parse(args);
	if (!parsed || cla.IsArg("help") || (cla.GetPositionalCount() == 0 && !cla.IsArg("version"))) {
		cla.PrintHelp();
		return;
	}
	
	if (cla.IsArg("version")) {
		Cout() << "Aria CLI version 0.1.0\n";
		return;
	}

	String cmd = cla.GetPositional(0);
	Vector<String> rest = cla.GetRest();
	
	if (cmd == "version") {
		Cout() << "Aria CLI version 0.1.0\n";
	} else if (cmd == "open") {
		CommandLineArguments sub;
		sub.SetDescription("Open a browser instance.");
		sub.AddArg("help", 'h', "show this help message and exit", false);
		sub.AddArg("headless", 0, "Run in headless mode", false);
		sub.AddArg("profile", 0, "Use default browser profile (default)", false);
		sub.AddArg("no-profile", 0, "Do not use browser profile", false);
		sub.AddArg("browser", 0, "Browser to use (default: firefox)", true, "BROWSER");
		sub.AddPositional("url", "URL to open", STRING_V, "");
		
		if (!sub.Parse(rest) || sub.IsArg("help")) { sub.PrintHelp(); return; }
		
		safety_manager.EnsureDisclaimerAccepted();
		
		String browser = sub.GetArg("browser");
		if (browser.IsEmpty()) browser = "firefox";
		String url = sub.GetPositional(0);
		if (!url.IsEmpty() && !url.StartsWith("http://") && !url.StartsWith("https://") && !url.StartsWith("about:"))
			url = "https://" + url;
		bool headless = sub.IsArg("headless");
		bool use_profile = !sub.IsArg("no-profile");
		
		navigator->StartSession(browser, headless, use_profile);
		if (!url.IsEmpty()) {
			if (safety_manager.CheckUrlSafety(url))
				navigator->Navigate(url);
		}
	} else if (cmd == "close") {
		navigator->CloseSession();
	} else if (cmd == "page") {
		CommandLineArguments sub;
		sub.SetDescription("Manage browser pages.");
		sub.AddArg("help", 'h', "show this help message and exit", false);
		sub.AddPositional("{list,new,source,eval}", "Command to perform");
		sub.AddPositional("arg", "Command argument", STRING_V, "");
		
		if (!sub.Parse(rest) || sub.IsArg("help") || sub.GetPositionalCount() == 0) { sub.PrintHelp(); return; }
		
		String p_cmd = sub.GetPositional(0);
		if (p_cmd == "list") {
			ValueArray tabs = navigator->ListTabs();
			if (tabs.IsEmpty()) Cout() << "No active tabs.\n";
			else {
				for (int i = 0; i < tabs.GetCount(); i++)
					Cout() << i << ": " << AsJSON(tabs[i]) << "\n";
			}
		} else if (p_cmd == "new") {
			String url = sub.GetPositionalCount() > 1 ? (String)sub.GetPositional(1) : "about:blank";
			navigator->NewTab(url);
		} else if (p_cmd == "source") {
			Cout() << navigator->GetPageContent() << "\n";
		} else if (p_cmd == "eval") {
			if (sub.GetPositionalCount() < 2) { Cout() << "Error: eval requires JS code.\n"; return; }
			Cout() << AsJSON(navigator->Eval(sub.GetPositional(1))) << "\n";
		}
	} else if (cmd == "script") {
		CommandLineArguments sub;
		sub.SetDescription("Manage automation scripts.");
		sub.AddArg("help", 'h', "show this help message and exit", false);
		sub.AddPositional("{list,create,run}", "Command to perform");
		
		if (!sub.Parse(rest) || sub.IsArg("help") || sub.GetPositionalCount() == 0) { sub.PrintHelp(); return; }
		
		String s_cmd = sub.GetPositional(0);
		Vector<String> s_rest = sub.GetRest();
		
		if (s_cmd == "list") {
			for (const auto& s : script_manager.ListScripts())
				Cout() << s.id << ": " << s.name << " - " << s.prompt.Left(50) << "...\n";
		} else if (s_cmd == "create") {
			if (s_rest.GetCount() < 1) { Cout() << "Usage: script create <prompt> [name]\n"; return; }
			String prompt = s_rest[0];
			String name = s_rest.GetCount() > 1 ? s_rest[1] : "";
			int id = script_manager.CreateScript(prompt, name);
			Cout() << "Created script " << id << "\n";
		} else if (s_cmd == "run") {
			if (s_rest.GetCount() < 1) { Cout() << "Usage: script run <id_or_name> [k=v...]\n"; return; }
			String identifier = s_rest[0];
			ValueMap params;
			for (int i = 1; i < s_rest.GetCount(); i++) {
				Vector<String> kv = Split(s_rest[i], '=');
				if (kv.GetCount() == 2) params.Set(kv[0], kv[1]);
			}
			Cout() << "Running script: " << identifier << "\n";
			try {
				script_manager.RunScript(identifier, ~navigator, params);
			} catch (const Exc& e) {
				Cout() << "Script Error: " << e << "\n";
			}
		}
	} else if (cmd == "site") {
		CommandLineArguments sub;
		sub.SetDescription("Manage site-specific data and scraping.");
		sub.AddArg("help", 'h', "show this help message and exit", false);
		sub.AddPositional("{list,discord,whatsapp,messages,calendar,youtube,threads}", "Command to perform");
		
		if (!sub.Parse(rest) || sub.IsArg("help") || sub.GetPositionalCount() == 0) { sub.PrintHelp(); return; }
		
		String site_cmd = sub.GetPositional(0);
		Vector<String> s_rest = sub.GetRest();
		
		if (site_cmd == "list") {
			for (const String& s : site_manager.ListSites()) Cout() << "- " << s << "\n";
		} else if (site_cmd == "discord") {
			Cout() << "Discord scraping is currently disabled.\n";
			/*
			if (s_rest.GetCount() < 1) { Cout() << "Usage: site discord <channel>\n"; return; }
			String channel = s_rest[0];
			DiscordScraper scraper(*navigator, site_manager);
			Cout() << "Scraping Discord channel: " << channel << "\n";
			try {
				scraper.Refresh();
			} catch (const Exc& e) {
				Cout() << "Scrape Error: " << e << "\n";
			}
			*/
		} else if (site_cmd == "whatsapp") {
			WhatsAppScraper scraper(*navigator, site_manager);
			Cout() << "Refreshing WhatsApp data...\n";
			scraper.Refresh();
		} else if (site_cmd == "messages") {
			GoogleMessagesScraper scraper(*navigator, site_manager);
			Cout() << "Refreshing Google Messages data...\n";
			scraper.Refresh();
		} else if (site_cmd == "calendar") {
			CalendarScraper scraper(*navigator, site_manager);
			Cout() << "Refreshing Google Calendar data...\n";
			scraper.Refresh();
		} else if (site_cmd == "youtube") {
			YouTubeStudioScraper scraper(*navigator, site_manager);
			Cout() << "Refreshing YouTube Studio data...\n";
			scraper.Refresh();
		} else if (site_cmd == "threads") {
			ThreadsScraper scraper(*navigator, site_manager);
			Cout() << "Refreshing Threads data...\n";
			scraper.Refresh();
		}
	} else if (cmd == "plugin") {
		CommandLineArguments sub;
		sub.SetDescription("Manage plugins.");
		sub.AddArg("help", 'h', "show this help message and exit", false);
		sub.AddPositional("{list}", "Command to perform");
		
		if (!sub.Parse(rest) || sub.IsArg("help") || sub.GetPositionalCount() == 0) { sub.PrintHelp(); return; }
		
		String p_cmd = sub.GetPositional(0);
		if (p_cmd == "list") {
			Cout() << "AI Providers:\n";
			for (const String& p : plugin_manager.ListAIProviders())
				Cout() << "- " << p << "\n";
			Cout() << "- gemini (built-in)\n";
		}
	} else if (cmd == "vault") {
		CommandLineArguments sub;
		sub.SetDescription("Manage credentials.");
		sub.AddArg("help", 'h', "show this help message and exit", false);
		sub.AddPositional("{set,get,list,remove}", "Command to perform");
		
		if (!sub.Parse(rest) || sub.IsArg("help") || sub.GetPositionalCount() == 0) { sub.PrintHelp(); return; }
		
		CredentialManager cm;
		String v_cmd = sub.GetPositional(0);
		Vector<String> v_rest = sub.GetRest();
		
		if (v_cmd == "set") {
			if (v_rest.GetCount() < 2) { Cout() << "Usage: vault set <key> <value>\n"; return; }
			cm.SetCredential(v_rest[0], v_rest[1]);
			Cout() << "Credential set: " << v_rest[0] << "\n";
		} else if (v_cmd == "get") {
			if (v_rest.GetCount() < 1) { Cout() << "Usage: vault get <key>\n"; return; }
			Cout() << cm.GetCredential(v_rest[0]) << "\n";
		} else if (v_cmd == "list") {
			for (const auto& k : cm.ListKeys()) Cout() << "- " << k << "\n";
		} else if (v_cmd == "remove") {
			if (v_rest.GetCount() < 1) { Cout() << "Usage: vault remove <key>\n"; return; }
			if (cm.RemoveCredential(v_rest[0])) Cout() << "Credential removed.\n";
			else Cout() << "Credential not found.\n";
		}
	} else if (cmd == "test") {
		CommandLineArguments sub;
		sub.SetDescription("Run diagnostic tests.");
		sub.AddArg("help", 'h', "show this help message and exit", false);
		sub.AddPositional("{bot}", "Command to perform");
		sub.AddArg("no-headless", 0, "Disable headless mode for test", false);
		
		if (!sub.Parse(rest) || sub.IsArg("help") || sub.GetPositionalCount() == 0) { sub.PrintHelp(); return; }
		
		String test_cmd = sub.GetPositional(0);
		if (test_cmd == "bot") {
			Cout() << "Running Bot Detection Test (browserscan.net)...\n";
			
			bool headless = !sub.IsArg("no-headless");
				
			navigator->StartSession("firefox", headless);
			navigator->Navigate("https://www.browserscan.net/bot-detection");
			Cout() << "Waiting for scan to complete (15s)...\n";
			Sleep(15000);
			
			String body_text;
			try {
				body_text = navigator->GetPageContent();
				Cout() << "Page Title: " << (String)navigator->Eval("return document.title") << "\n";
			} catch (const Exc& e) {
				Cout() << "Warning: Failed to get page content: " << e << "\n";
			}
			
			// More specific detection check: Look for "Your browser is a BOT" or similar negative results
			// BrowserScan typically shows a badge or specific text if it detects automation.
			bool detected_as_bot = body_text.Find("Your browser is a BOT") >= 0 || 
			                       body_text.Find("Bot detected") >= 0;
			
			if (detected_as_bot) {
				Cout() << "❌ FAILED: Browser detected as BOT by BrowserScan\n";
			} else {
				Cout() << "✅ PASSED: No bot detection message found\n";
			}
			
			Value webdriver = navigator->Eval("return navigator.webdriver");
			Cout() << "navigator.webdriver: " << AsJSON(webdriver) << "\n";
			
			Value comp = navigator->Eval(R"(
				return {
					webdriver: navigator.webdriver,
					pluginsLength: navigator.plugins.length,
					languages: navigator.languages,
					chrome: !!window.chrome,
					permissions: !!navigator.permissions,
					hardwareConcurrency: navigator.hardwareConcurrency
				};
			)");
			Cout() << "Comprehensive Result: " << AsJSON(comp, true) << "\n";
		}
	} else if (cmd == "report") {
		Cout() << "Manage local reports (not yet implemented).\n";
	} else if (cmd == "diag") {
		Cout() << "Show diagnostic information (not yet implemented).\n";
	} else if (cmd == "settings") {
		Cout() << "Show current configuration (not yet implemented).\n";
	} else if (cmd == "tutorial") {
		Cout() << "Start the interactive tutorial (not yet implemented).\n";
	} else if (cmd == "security") {
		Cout() << "Show security best practices (not yet implemented).\n";
	} else if (cmd == "man") {
		cla.PrintHelp();
	} else if (cmd == "help") {
		cla.PrintHelp();
	} else {
		Cout() << "Unknown command: " << cmd << "\n";
		cla.PrintHelp();
	}
}

String Aria::GenerateAIResponse(const String& prompt, const String& context, const String& output_format, const String& provider_name) {
	BaseAIProvider* provider = plugin_manager.GetAIProvider(provider_name);
	if (!provider && provider_name == "gemini") {
		static GeminiProvider default_gemini;
		provider = &default_gemini;
	}
	
	if (!provider) return "Error: AI provider not found: " + provider_name;
	
	return provider->Generate(prompt, context, output_format);
}

END_UPP_NAMESPACE

#ifdef flagMAIN
CONSOLE_APP_MAIN
{
	Upp::StdLogSetup(Upp::LOG_COUT | Upp::LOG_FILE);
	try {
		Upp::Aria aria;
		aria.Run(Upp::CommandLine());
	} catch (const Upp::Exc& e) {
		Upp::Cout() << "Error: " << e << "\n";
	} catch (const std::exception& e) {
		Upp::Cout() << "System Error: " << e.what() << "\n";
	} catch (...) {
		Upp::Cout() << "Unknown error occurred.\n";
	}
}
#endif
