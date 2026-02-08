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
	navigator.Create();
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
		}
	} else if (cmd == "site" && args.GetCount() > 1) {
		String site_cmd = args[1];
		if (site_cmd == "list") {
			for (const String& s : site_manager.ListSites()) Cout() << "- " << s << "\n";
		}
	} else if (cmd == "man") {
		Cout() << "Aria User Manual\n\nCommands:\n"
		       << "  open [url]      Open browser session\n"
		       << "  close           Close browser session\n"
		       << "  page list       List open tabs\n"
		       << "  page new [url]  Open new tab\n"
		       << "  script list     List saved scripts\n"
		       << "  site list       List sites with local data\n";
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
