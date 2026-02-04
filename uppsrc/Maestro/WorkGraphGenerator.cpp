#include "WorkGraphGenerator.h"

namespace Upp {

WorkGraph WorkGraphGenerator::Generate(const String& freeform, const ValueMap& discovery, const String& domain, const String& profile) {
	for(int attempt = 0; attempt < 2; attempt++) {
		String prompt = CreatePrompt(freeform, discovery, domain, profile);
		if(attempt == 1) {
			prompt << "\n\nPREVIOUS RESPONSE WAS INVALID. Please ensure:\n"
			       << "- Every task has at least one definition_of_done with kind='command' or 'file'\n"
			       << "- Commands have 'cmd' field, files have 'path' field\n"
			       << "- No meta-runbook tasks (tasks must be executable)\n"
			       << "- Return ONLY the JSON object, no explanatory text\n";
		}

		if(verbose) Cout() << "Sending prompt to AI engine (attempt " << (attempt + 1) << "/2)...\n";
		
		String response;
		bool done = false;
		engine.session_id = "decompose-session"; 
		engine.Send(prompt, [&](const MaestroEvent& ev) {
			if(ev.type == "message") {
				if(ev.role == "assistant" || ev.role.IsEmpty()) {
					if(!ev.delta) response = ev.text;
					else response << ev.text;
				}
			}
			else if(ev.type == "done" || ev.type == "result") {
				done = true;
			}
		});

		while(!done && engine.Do()) Sleep(10);

		if(verbose) Cout() << "AI response received (len=" << response.GetCount() << ")\n";

		String json_str = ExtractJSON(response);
		WorkGraph wg;
		if(LoadFromJson(wg, json_str) && !wg.phases.IsEmpty()) {
			if(verbose) Cout() << "WorkGraph validation passed.\n";
			return wg;
		}
		
		if(verbose) Cout() << "Validation failed (empty or invalid), retrying...\n";
	}
	
	throw Exc("Failed to generate valid WorkGraph after retries.");
}

String WorkGraphGenerator::CreatePrompt(const String& freeform, const ValueMap& discovery, const String& domain, const String& profile) {
	String evidence = AsJSON(discovery, true);
	String res;
	res << "You are a project planning assistant that decomposes freeform requests into structured WorkGraph plans.\n\n"
		<< "FREEFORM REQUEST:\n" << freeform << "\n\n"
		<< "REPO EVIDENCE:\n" << evidence << "\n\n"
		<< "DOMAIN: " << domain << "\n"
		<< "PROFILE: " << profile << "\n\n"
		<< "YOUR TASK:\n"
		<< "Generate a WorkGraph JSON that decomposes this request into executable tracks/phases/tasks.\n\n"
		<< "CRITICAL RULES:\n"
		<< "1. Every task MUST have definition_of_done with kind=\"command\" OR kind=\"file\"\n"
		<< "2. Command DoDs must have \"cmd\" field (e.g., \"maestro runbook list\")\n"
		<< "3. File DoDs must have \"path\" field (e.g., \"docs/maestro/runbooks/index.json\")\n"
		<< "4. NO meta-runbook tasks like \"Organize documentation\" without executable DoD\n"
		<< "5. Tasks should reference maestro commands from evidence where possible\n"
		<< "6. Return ONLY the JSON object, no markdown wrappers or explanatory text\n\n"
		<< "EXACT JSON SCHEMA:\n"
		<< "{\n"
		<< "  \"schema_version\": \"v1\",\n"
		<< "  \"id\": \"\",\n"
		<< "  \"domain\": \"" << domain << "\",\n"
		<< "  \"profile\": \"" << profile << "\",\n"
		<< "  \"title\": \"Implement simple calculator\",\n"
		<< "  \"goal\": \"Create a C++ calculator CLI\",\n"
		<< "  \"track\": {\"id\": \"TRK-CALC\", \"name\": \"Calculator\", \"goal\": \"Track goal\"},\n"
		<< "  \"phases\": [\n"
		<< "    {\n"
		<< "      \"id\": \"PH-001\", \"name\": \"01_Foundation\",\n"
		<< "      \"tasks\": [\n"
		<< "        {\n"
		<< "          \"id\": \"TASK-001\", \"title\": \"01_ProjectSetup\", \"intent\": \"Setup project\",\n"
		<< "          \"definition_of_done\": [{\"kind\": \"command\", \"cmd\": \"maestro init\"}]\n"
		<< "        }\n"
		<< "      ]\n"
		<< "    }\n"
		<< "  ]\n"
		<< "}\n\n"
		<< "FILL IN ALL FIELDS based on the user request. DO NOT leave them empty.\n"
		<< "Return ONLY the JSON object.\n";
	return res;
}

String WorkGraphGenerator::ExtractJSON(const String& response) {
	int last_end = response.ReverseFind('}');
	if(last_end < 0) return response;
	
	int depth = 0;
	for(int i = last_end; i >= 0; i--) {
		if(response[i] == '}') depth++;
		else if(response[i] == '{') {
			depth--;
			if(depth == 0) {
				return response.Mid(i, last_end - i + 1);
			}
		}
	}
	return response;
}

} // namespace Upp