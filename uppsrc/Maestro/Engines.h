#ifndef _Maestro_Engines_h_
#define _Maestro_Engines_h_

inline CliMaestroEngine& ConfigureGemini(CliMaestroEngine& e) {
	e.Reset();
	e.Binary("gemini")
	 .Arg("-m").Arg("gemini-3-flash-preview")
	 .Arg("--approval-mode").Arg("yolo")
	 .Arg("-o").Arg("stream-json")
	 .Arg("-p");
	return e;
}

inline CliMaestroEngine& ConfigureQwen(CliMaestroEngine& e) {
	e.Reset();
	e.Binary("qwen")
	 .Arg("-y")
	 .Arg("-o").Arg("stream-json");
	return e;
}

inline CliMaestroEngine& ConfigureClaude(CliMaestroEngine& e) {
	e.Reset();
	e.Binary("claude")
	 .Arg("--output-format").Arg("stream-json");
	return e;
}

inline CliMaestroEngine& ConfigureCodex(CliMaestroEngine& e) {
	e.Reset();
	e.Binary("codex")
	 .Arg("exec")
	 .Arg("--dangerously-bypass-approvals-and-sandbox")
	 .Arg("--json");
	return e;
}

#endif
