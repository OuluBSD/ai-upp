#include "Maestro.h"

namespace Upp {

void DiscussCommand::Execute(const Vector<String>& args) {
	CommandLineArguments cla;
	cla.AddArg("session", 's', "Session ID to resume", true);
	cla.AddPositional("prompt", UNKNOWN_V);
	if(!cla.Parse(args)) return;
	
	String sid = cla.GetArg("session").ToString();
	String initial_prompt;
	if(cla.GetPositionalCount() > 0) initial_prompt = AsString(cla.GetPositional(0));

	String docs_root = GetDocsRoot(FindPlanRoot());
	
	PlanParser parser;
	parser.Load(FindPlanRoot());
	
	MaestroToolRegistry tool_reg;
	RegisterMaestroTools(tool_reg);

	String context = PlanSummarizer::GetPlanSummaryText(parser.tracks, "", "", "");
	String system_prompt = "You are the Maestro AI Assistant. Your goal is to help manage the project plan.\n"
	                       "Project Context:\n" + context + "\n\n";
	
	String tools_summary = tool_reg.GetToolSummary();
	if(tools_summary.GetCount()) {
		system_prompt << tools_summary << "\n"
		              << "You can use these tools by outputting a JSON object with type 'tool_use'.\n\n";
	}
	
	CliMaestroEngine engine;
	ConfigureGemini(engine);

	auto DoTurn = [&](String prompt) {
		Cout() << "Thinking...\n";
		Breadcrumb bc;
		bc.prompt = prompt;
		bc.model_used = engine.model;
		
		if(!sid.IsEmpty()) {
			engine.args.Clear();
			engine.Arg("-s").Arg(sid);
		}

		bool done = false;
		engine.Send(prompt, [&](const MaestroEvent& ev) {
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
		
		if(sid.IsEmpty()) sid = engine.session_id;
		if(sid.IsEmpty()) {
			WorkSession s = WorkSessionManager::CreateSession(docs_root, "discussion", "MaestroCLI Discussion");
			sid = s.session_id;
		}
		BreadcrumbManager::SaveBreadcrumb(bc, docs_root, sid);
	};

	if(!initial_prompt.IsEmpty()) {
		DoTurn(system_prompt + initial_prompt);
	} else {
		Cout() << "Entering interactive Maestro discussion. Type 'exit' or 'quit' to end.\n";
		while(true) {
			Cout() << "> ";
			String p = ReadStdIn();
			if(p.IsEmpty()) continue;
			p = TrimBoth(p);
			if(p == "exit" || p == "quit") break;
			
			if(sid.IsEmpty()) DoTurn(system_prompt + p);
			else DoTurn(p);
		}
	}
	
	Cout() << "[Interaction saved to session " << sid << "]\n";
}

}
