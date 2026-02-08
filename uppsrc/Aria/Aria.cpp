#include "Aria.h"

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

Aria::Aria() {
	plugin_manager.LoadPlugins();
	navigator.Create();
	// Use default gemini provider
	static GeminiProvider default_gemini;
	navigator->SetAIProvider(&default_gemini);
}

void Aria::Run(const Vector<String>& args) {
	if (args.GetCount() == 0) {
		Cout() << "Aria CLI version 0.1.0\n";
		Cout() << "Usage: Aria <command> [args]\n";
		return;
	}
	
	String cmd = args[0];
	if (cmd == "version") {
		Cout() << "Aria CLI version 0.1.0\n";
	} else if (cmd == "open") {
		safety_manager.EnsureDisclaimerAccepted();
		
		String browser = "firefox";
		String url = "";
		bool headless = false;
		
		for (int i = 1; i < args.GetCount(); i++) {
			String arg = args[i];
			if (arg == "--headless") {
				headless = true;
			} else if (arg == "chrome" || arg == "firefox" || arg == "edge") {
				browser = arg;
			} else {
				url = arg;
			}
		}
		
		navigator->StartSession(browser, headless);
		if (!url.IsEmpty()) {
			if (safety_manager.CheckUrlSafety(url))
				navigator->Navigate(url);
		}
	} else if (cmd == "close") {
		navigator->CloseSession();
	} else if (cmd == "page" && args.GetCount() > 1) {
		String p_cmd = args[1];
		if (p_cmd == "list") {
			ValueArray tabs = navigator->ListTabs();
			if (tabs.IsEmpty()) Cout() << "No active tabs.\n";
			else {
				for (int i = 0; i < tabs.GetCount(); i++)
					Cout() << i << ": " << AsJSON(tabs[i]) << "\n";
			}
		} else if (p_cmd == "new") {
			String url = args.GetCount() > 2 ? args[2] : "about:blank";
			navigator->NewTab(url);
		} else if (p_cmd == "source") {
			Cout() << navigator->GetPageContent() << "\n";
		} else if (p_cmd == "eval" && args.GetCount() > 2) {
			Cout() << AsJSON(navigator->Eval(args[2])) << "\n";
		}
	} else if (cmd == "script" && args.GetCount() > 1) {
		String s_cmd = args[1];
		if (s_cmd == "list") {
			for (const auto& s : script_manager.ListScripts())
				Cout() << s.id << ": " << s.name << " - " << s.prompt.Left(50) << "...\n";
		} else if (s_cmd == "create" && args.GetCount() > 2) {
			String prompt = args[2];
			String name = args.GetCount() > 3 ? args[3] : "";
			int id = script_manager.CreateScript(prompt, name);
			Cout() << "Created script " << id << "\n";
		} else if (s_cmd == "run" && args.GetCount() > 2) {
			String identifier = args[2];
			ValueMap params;
			// Simple param parsing: key=value
			for (int i = 3; i < args.GetCount(); i++) {
				Vector<String> kv = Split(args[i], '=');
				if (kv.GetCount() == 2) params.Set(kv[0], kv[1]);
			}
			Cout() << "Running script: " << identifier << "\n";
			try {
				script_manager.RunScript(identifier, ~navigator, params);
			} catch (const Exc& e) {
				Cout() << "Script Error: " << e << "\n";
			}
		}
	} else if (cmd == "site" && args.GetCount() > 1) {
		String site_cmd = args[1];
		if (site_cmd == "list") {
			for (const String& s : site_manager.ListSites()) Cout() << "- " << s << "\n";
		} else if (site_cmd == "discord" && args.GetCount() > 2) {
			String channel = args[2];
			DiscordScraper scraper(*navigator, site_manager);
			Cout() << "Scraping Discord channel: " << channel << "\n";
			try {
				scraper.Refresh();
			} catch (const Exc& e) {
				Cout() << "Scrape Error: " << e << "\n";
			}
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
	} else if (cmd == "plugin" && args.GetCount() > 1) {
		String p_cmd = args[1];
		if (p_cmd == "list") {
			Cout() << "AI Providers:\n";
			for (const String& p : plugin_manager.ListAIProviders())
				Cout() << "- " << p << "\n";
			Cout() << "- gemini (built-in)\n";
		}
	} else if (cmd == "vault" && args.GetCount() > 1) {
		CredentialManager cm;
		String v_cmd = args[1];
		if (v_cmd == "set" && args.GetCount() > 3) {
			cm.SetCredential(args[2], args[3]);
			Cout() << "Credential set: " << args[2] << "\n";
		} else if (v_cmd == "get" && args.GetCount() > 2) {
			Cout() << cm.GetCredential(args[2]) << "\n";
		} else if (v_cmd == "list") {
			for (const auto& k : cm.ListKeys()) Cout() << "- " << k << "\n";
		} else if (v_cmd == "remove" && args.GetCount() > 2) {
			if (cm.RemoveCredential(args[2])) Cout() << "Credential removed.\n";
			else Cout() << "Credential not found.\n";
		}
	} else if (cmd == "test" && args.GetCount() > 1) {
		String test_cmd = args[1];
		if (test_cmd == "bot") {
			Cout() << "Running Bot Detection Test (browserscan.net)...\n";
			
			bool headless = true;
			for (int i = 2; i < args.GetCount(); i++)
				if (args[i] == "--no-headless") headless = false;
				
			navigator->StartSession("firefox", headless);
			navigator->Navigate("https://www.browserscan.net/bot-detection");
			Cout() << "Waiting for scan to complete (10s)...\n";
			Sleep(10000);
			
			String body_text = navigator->GetPageContent();
			Cout() << "Page Title: " << (String)navigator->Eval("return document.title") << "\n";
			
			static const char* detection_keywords[] = { "Robot", "Webdriver", "Automated", "Bot" };
			Vector<String> detected;
			for (const char* kw : detection_keywords) {
				if (body_text.Find(kw) >= 0) detected.Add(kw);
			}
			
			if (detected.GetCount() > 0) {
				Cout() << "⚠️  STILL DETECTED: Keywords found - " << Join(detected, ", ") << "\n";
			} else {
				Cout() << "✅ IMPROVED: No obvious detection keywords found\n";
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
	} else if (cmd == "man") {
		Cout() << "Aria User Manual\n\nCommands:\n"
		       << "  open [url]      Open browser session\n"
		       << "  close           Close browser session\n"
		       << "  page list       List open tabs\n"
		       << "  page new [url]  Open new tab\n"
		       << "  page source     Print current page source\n"
		       << "  page eval [js]  Evaluate JavaScript in current page\n"
		       << "  script list     List saved scripts\n"
		       << "  script create [p] [n] Create new script\n"
		       << "  script run [i] [k=v] Run script by ID or name\n"
		       << "  plugin list     List available AI providers and plugins\n"
		       << "  vault set [k] [v] Set credential\n"
		       << "  vault get [k]   Get credential\n"
		       << "  vault list      List all credential keys\n"
		       << "  site list       List sites with local data\n"
		       << "  site discord [c] Scrape Discord channel\n"
		       << "  site whatsapp   Scrape WhatsApp messages\n"
		       << "  site messages   Scrape Google Messages\n"
		       << "  site calendar   Scrape Google Calendar\n"
		       << "  site youtube    Scrape YouTube Studio\n"
		       << "  site threads    Scrape Threads feed\n"
		       << "  test bot        Run bot detection test\n";
	} else {
		Cout() << "Unknown command: " << cmd << "\n";
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
