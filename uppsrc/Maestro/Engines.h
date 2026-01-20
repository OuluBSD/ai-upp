#ifndef _Maestro_Engines_h_
#define _Maestro_Engines_h_

inline CliMaestroEngine& ConfigureGemini(CliMaestroEngine& e) {
	e.Binary("gemini")
	 .Arg("--approval-mode").Arg("yolo")
	 .Arg("-o").Arg("stream-json");
	return e;
}

inline CliMaestroEngine& ConfigureQwen(CliMaestroEngine& e) {
	e.Binary("qwen")
	 .Arg("-y")
	 .Arg("-o").Arg("stream-json");
	return e;
}

inline CliMaestroEngine& ConfigureClaude(CliMaestroEngine& e) {
	e.Binary("claude")
	 .Arg("--output-format").Arg("stream-json");
	return e;
}

#endif
