#include "Maestro.h"

namespace Upp {

void DiscussCommand::Execute(const Vector<String>& args) {
	CommandLineArguments cla;
	cla.AddPositional("prompt", UNKNOWN_V);
	cla.Parse(args);
	
	String prompt;
	if(cla.GetPositionalCount() > 0) prompt = AsString(cla.GetPositional(0));
	else { Cout() << "Enter your prompt: "; prompt = ReadStdIn(); } // Read from stdin if no prompt is provided
	if(prompt.IsEmpty()) return;

	String docs_root = GetDocsRoot(FindPlanRoot());
	
	PlanParser parser;
	parser.Load(FindPlanRoot());
	
	MaestroToolRegistry tool_reg;
	RegisterMaestroTools(tool_reg);

	String context = PlanSummarizer::GetPlanSummaryText(parser.tracks, "", "", "");
	String full_prompt = "You are the Maestro AI Assistant. Your goal is to help manage the project plan.\n"
	                     "Project Context:\n" + context + "\n\n";
	
	String tools_summary = tool_reg.GetToolSummary();
	if(tools_summary.GetCount()) {
		full_prompt << tools_summary << "\n"
		            << "You can use these tools by outputting a JSON object with type 'tool_use'.\n\n";
	}
	
	full_prompt << "User Question: " << prompt;

	CliMaestroEngine engine;

	Cout() << "Thinking...\n";
	Breadcrumb bc;
	bc.prompt = full_prompt;
	bc.model_used = "gemini-1.5-flash";
	
	bool done = false;
	engine.Send(full_prompt, [&](const MaestroEvent& ev) {
		if(ev.type == "message") {
			if(ev.role == "assistant" || ev.role.IsEmpty()) {
				if(ev.delta) { Cout() << ev.text; bc.response << ev.text; }
				else { Cout() << ev.text << "\n"; bc.response = ev.text; }
			}
		}
		else if(ev.type == "tool_use" || ev.type == "tool_call") {
			Cout() << "\n[AI calls tool: " << ev.tool_name << "(" << ev.tool_input << ")]\n";
			ToolCall& tc = bc.tools_called.Add();
			tc.tool = ev.tool_name;
			tc.timestamp = GetSysTime();
			const MaestroTool* tool = tool_reg.Find(ev.tool_name);
			if(tool) {
				Value input = ParseJSON(ev.tool_input);
				if(IsError(input)) input = ValueMap(); 
				tc.args = input;
				Value result = tool->Execute(input);
				tc.result = result;
				Cout() << "[Tool Result: " << result << "]\n";
				engine.WriteToolResult(ev.tool_id, result);
			} else {
				tc.error = "Tool not found";
				engine.WriteToolResult(ev.tool_id, "Error: Tool not found");
			}
		}
		else if (ev.type == "result") {
			if(ev.text.StartsWith("[API Error:")) { Cerr() << "\nAI " << ev.text << "\n"; bc.error = ev.text; }
			done = true;
		}
		else if (ev.type == "done") done = true;
	});

	while(!done && engine.Do()) Sleep(10);
	Cout() << "\n";
	
	String sid = engine.session_id;
	if(sid.IsEmpty()) {
		WorkSession s = WorkSessionManager::CreateSession(docs_root, "discussion", "MaestroCLI Discussion");
		sid = s.session_id;
	}
	BreadcrumbManager::SaveBreadcrumb(bc, docs_root, sid);
	Cout() << "[Interaction saved to session " << sid << "]\n";
}

}
