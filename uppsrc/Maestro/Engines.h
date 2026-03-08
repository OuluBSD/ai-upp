
#ifndef _Maestro_Engines_h_
#define _Maestro_Engines_h_

#include "QuotaManager.h"

inline CliMaestroEngine& ConfigureGemini(CliMaestroEngine& e) {
	e.Reset();
	e.Binary("gemini")
	 .Arg("--approval-mode").Arg("yolo")
	 .Arg("-o").Arg("stream-json")
	 .Arg("-e").Arg("none");
	if(e.model.IsEmpty() || QuotaManager::IsModelExhausted(e.model))
		e.model = QuotaManager::GetBestGeminiModel();
	e.Arg("-m").Arg(e.model);
	return e;
}

inline CliMaestroEngine& ConfigureQwen(CliMaestroEngine& e) {
	e.Reset();
	e.Binary("qwen")
	 .Arg("-y")
	 .Arg("-o").Arg("stream-json")
	 .Arg("-e").Arg("none");
	return e;
}

inline CliMaestroEngine& ConfigureClaude(CliMaestroEngine& e) {
	e.Reset();
	e.Binary("claude")
	 .Arg("--verbose")
	 .Arg("--output-format").Arg("stream-json")
	 .Arg("-e").Arg("none");
	return e;
}

inline CliMaestroEngine& ConfigureCodex(CliMaestroEngine& e) {
	e.Reset();
	e.Binary("codex");
	return e;
}


#endif
