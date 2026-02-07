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
	
	// Execute: echo prompt | gemini -y --model gemini-3-flash-preview -o stream-json -
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
	
	// Simple NDJSON parsing
	String response_text;
	Vector<String> lines = Split(stdout_str, '\n');
	for (const String& line : lines) {
		if (TrimBoth(line).IsEmpty()) continue;
		Value v = ParseJSON(line);
		if (v.Is<ValueMap>()) {
			const ValueMap& vm = v.Get<ValueMap>();
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
		String url = args.GetCount() > 1 ? args[1] : "";
		navigator->StartSession();
		if (!url.IsEmpty()) navigator->Navigate(url);
	} else if (cmd == "close") {
		navigator->CloseSession();
	} else if (cmd == "page" && args.GetCount() > 1) {
		if (args[1] == "list") {
			ValueArray tabs = navigator->ListTabs();
			for (int i = 0; i < tabs.GetCount(); i++) {
				Cout() << AsJSON(tabs[i], true) << "\n";
			}
		}
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
	Upp::Aria aria;
	aria.Run(Upp::CommandLine());
}