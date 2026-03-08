#include "Maestro.h"

namespace Upp {

static String GenerateTaskId() {
	return "task_" + FormatIntHex(Random(), 8);
}

WorkGraph ConversionPlanner::GeneratePlan(const MaestroInventory& source, const MaestroInventory& target, ConversionMemory& memory)
{
	WorkGraph wg;
	wg.title = "Conversion Plan: " + GetFileName(source.repository_path);
	wg.goal = "Convert " + source.repository_path + " to " + target.repository_path;
	
	// Phase 1: Scaffold
	WorkGraphPhase& p_scaf = wg.phases.Add();
	p_scaf.id = "phase_scaffold";
	p_scaf.name = "Scaffolding";
	
	WorkGraphTask& t_init = p_scaf.tasks.Add();
	t_init.id = GenerateTaskId();
	t_init.title = "Set up basic project files";
	t_init.intent = "Create README.md, .gitignore and basic project structure";
	t_init.outputs.Add("README.md");
	t_init.outputs.Add(".gitignore");
	
	// Phase 2: File Conversion
	WorkGraphPhase& p_conv = wg.phases.Add();
	p_conv.id = "phase_conversion";
	p_conv.name = "File Conversion";
	
	// Group files by language for conversion tasks
	VectorMap<String, Vector<String>> lang_groups;
	for(const auto& f : source.files) {
		lang_groups.GetAdd(f.language).Add(f.path);
	}
	
	for(int i = 0; i < lang_groups.GetCount(); i++) {
		String lang = lang_groups.GetKey(i);
		if(lang == "Unknown" || lang == "Binary") continue;
		
		WorkGraphTask& t = p_conv.tasks.Add();
		t.id = GenerateTaskId();
		t.title = "Convert " + lang + " files";
		t.intent = "Convert source " + lang + " files to target architecture";
		t.inputs = clone(lang_groups[i]);
		for(const auto& in : t.inputs) t.outputs.Add(in); // Placeholder for now
	}
	
	// Phase 3: Final Sweep
	WorkGraphPhase& p_sweep = wg.phases.Add();
	p_sweep.id = "phase_sweep";
	p_sweep.name = "Final Sweep";
	
	WorkGraphTask& t_verify = p_sweep.tasks.Add();
	t_verify.id = GenerateTaskId();
	t_verify.title = "Verify conversion coverage";
	t_verify.intent = "Check that all source files have been accounted for";
	
	AddAutoCheckpoints(wg, memory);
	
	return wg;
}

void ConversionPlanner::AddAutoCheckpoints(WorkGraph& wg, ConversionMemory& memory)
{
	// In Maestro, checkpoints can be represented by tasks that require manual approval or special stop conditions.
	// For now, we'll use WorkGraphStopCondition as a proxy or just rely on the runner.
}

}
