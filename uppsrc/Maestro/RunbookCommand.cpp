#include "Maestro.h"

namespace Upp {

void RunbookCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI runbook [-h]\n"
	       << "                          {list,ls,show,sh,add,new,rm,remove,delete,step-add,sa,resolve,res,derive-constraints}\n"
	       << "                          ...\n"
	       << "Manage runbook entries as first-class project assets.\n"
	       << "positional arguments:\n"
	       << "    list (ls)           List all runbooks\n"
	       << "    show (sh)           Show a specific runbook\n"
	       << "    add (new)           Create a new runbook\n"
	       << "    rm (remove, delete) Delete a runbook\n"
	       << "    step-add (sa)       Add a step to a runbook\n"
	       << "    resolve (res)       Resolve freeform text to structured runbook JSON\n"
	       << "    derive-constraints  Derive formal UGUI logic constraints from a runbook\n";
}

void RunbookCommand::Execute(const Vector<String>& args) {
	CommandLineArguments cla;
	cla.AddPositional("subcommand", UNKNOWN_V);
	cla.AddPositional("arg1", UNKNOWN_V);
	cla.AddPositional("arg2", UNKNOWN_V);
	cla.Parse(args);
	
	if (cla.GetPositionalCount() == 0) { ShowHelp(); return; }
	
	String sub = AsString(cla.GetPositional(0));
	RunbookManager rbm;
	
	if (sub == "list" || sub == "ls") {
		Array<Runbook> list = rbm.ListRunbooks();
		Cout() << "Found " << list.GetCount() << " runbook(s):\n";
		for(const auto& rb : list)
			Cout() << Format("  %-30s %s\n", rb.id, rb.title);
	}
	else if (sub == "show" || sub == "sh") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires runbook ID.\n"; return; }
		Runbook rb = rbm.LoadRunbook(AsString(cla.GetPositional(1)));
		Cout() << "Runbook: " << rb.title << "\nID:      " << rb.id << "\nGoal:    " << rb.goal << "\n";
		Cout() << "\nSteps (" << rb.steps.GetCount() << "):\n";
		for(const auto& s : rb.steps)
			Cout() << Format("  %d. [%s] %s (Cmd: %s)\n", s.n, s.actor, s.action, s.command);
	}
	else if (sub == "add" || sub == "new") {
		String title = cla.GetPositionalCount() > 1 ? AsString(cla.GetPositional(1)) : "New Runbook";
		Runbook rb;
		rb.title = title;
		rb.id = "rb-" + FormatIntHex(Random(), 8);
		if(rbm.SaveRunbook(rb)) Cout() << "✓ Created runbook: " << rb.id << "\n";
		else Cerr() << "Error: Failed to save runbook.\n";
	}
	else if (sub == "rm" || sub == "remove" || sub == "delete") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires runbook ID.\n"; return; }
		if(rbm.DeleteRunbook(AsString(cla.GetPositional(1)))) Cout() << "✓ Deleted runbook.\n";
		else Cerr() << "Error: Failed to delete runbook.\n";
	}
	else if (sub == "resolve" || sub == "res") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires text input.\n"; return; }
		Cout() << "Resolving runbook via AI...\n";
		Runbook rb = rbm.Resolve(AsString(cla.GetPositional(1)));
		if(rbm.SaveRunbook(rb)) Cout() << "✓ Created/Updated runbook: " << rb.id << "\n";
		else Cerr() << "Error: Failed to resolve runbook.\n";
	}
	else if (sub == "derive-constraints") {
		String id;
		if(cla.GetPositionalCount() >= 2)
			id = AsString(cla.GetPositional(1));
		
		if(id.IsEmpty()) {
			Array<Runbook> list = rbm.ListRunbooks();
			if(list.GetCount() == 1) {
				id = list[0].id;
				Cout() << "Using the only available runbook: " << id << "\n";
			} else if (list.GetCount() > 1) {
				// Pick latest (assuming they are named or dated somehow, or just first)
				id = list[0].id; 
				Cout() << "Multiple runbooks found. Using: " << id << "\n";
			} else {
				Cerr() << "Error: No runbooks found. Please resolve or add one first.\n";
				return;
			}
		}
		Runbook rb = rbm.LoadRunbook(id);
		if(rb.id.IsEmpty()) { Cerr() << "Error: Runbook not found.\n"; return; }
		
		Cout() << "Deriving formal constraints from runbook: " << rb.title << "...\n";
		
		CliMaestroEngine engine;
		engine.binary = "gemini";
		engine.model = "gemini-1.5-pro";
		engine.Arg("-y");
		
		String prompt;
		prompt << "You are a Formal Verification Assistant. Based on the following runbook, derive a set of formal logic constraints in FOL (First-Order Logic) format compatible with the Maestro Logic Engine.\n"
		       << "RUNBOOK: " << StoreAsJson(rb) << "\n\n"
		       << "SYNTAX RULES:\n"
		       << "1. Predicates must be ALL-UPPERCASE. Available: VISIBLE(ctrl), ENABLED(ctrl), BUTTON(ctrl), LABEL(ctrl), OPTION(ctrl).\n"
		       << "2. Constants (Ctrls) must be lowercase camelCase identifiers. No quotes! Example: mainWindow, drawingCanvas, pencilBtn, saveButton.\n"
		       << "3. Connectives: implies, and, or, not, iff.\n"
		       << "4. Example: VISIBLE(mainWindow) and ENABLED(saveButton).\n"
		       << "5. Each line must be exactly one constraint string.\n"
		       << "6. Do NOT include markdown blocks, preambles, or explanations.";
		
		String response;
		bool done = false;
		engine.Send(prompt, [&](const MaestroEvent& ev) {
			if(ev.type == "message") response << ev.text;
			else if(ev.type == "done") done = true;
		});
		while(!done && engine.Do()) Sleep(10);
		
		// Clean response: remove markdown blocks and empty lines
		String cleaned;
		String block_marker = "```";
		int first_block = response.Find(block_marker);
		if(first_block >= 0) {
			int second_block = response.Find(block_marker, first_block + 3);
			if(second_block > first_block) {
				String inside = response.Mid(first_block + 3, second_block - first_block - 3);
				// Remove optional language identifier (e.g. "json" or "yaml")
				int nl = inside.Find('\n');
				if(nl >= 0 && nl < 10) inside.Remove(0, nl + 1);
				cleaned = inside;
			}
		}
		
		if(cleaned.IsEmpty()) cleaned = response;
		
		Vector<String> lines = Split(cleaned, '\n');
		String final_out;
		for(String l : lines) {
			l = TrimBoth(l);
			if(l.StartsWith("- ")) l.Remove(0, 1);
			l = TrimBoth(l);
			if(l.StartsWith("\"") && l.EndsWith("\"")) l = l.Mid(1, l.GetCount() - 2);
			if(l.IsEmpty()) continue;
			final_out << l << "\n";
		}
		
		String root = FindPlanRoot();
		String out_dir = AppendFileName(root, "docs/maestro/plans/constraints");
		RealizeDirectory(out_dir);
		String out_path = AppendFileName(out_dir, id + ".ugui");
		
		if(SaveFile(out_path, final_out)) {
			Cout() << "✓ Formal constraints derived and saved to: " << out_path << "\n";
		} else {
			Cerr() << "Error: Failed to save constraints.\n";
		}
	}
	else {
		Cout() << "Subcommand '" << sub << "' is not yet fully implemented in C++ but is on the roadmap.\n";
	}
}

} // namespace Upp