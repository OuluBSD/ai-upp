#include "Maestro.h"

namespace Upp {

void PlanParser::Load(const String& plan_root) {
	tracks.Clear();
	FindFile ff(AppendFileName(plan_root, "*"));
	while(ff) {
		if(ff.IsDirectory() && ff.GetName() != "." && ff.GetName() != "..") {
			LoadTrack(ff.GetPath());
		}
		ff.Next();
	}
}

void PlanParser::LoadTrack(const String& track_dir) {
	Track& t = tracks.Add();
	t.path = track_dir;
	t.name = GetFileTitle(track_dir);
	
	FindFile ff(AppendFileName(track_dir, "*"));
	while(ff) {
		if(ff.IsDirectory() && ff.GetName() != "." && ff.GetName() != "..") {
			LoadPhase(t, ff.GetPath());
		}
		ff.Next();
	}
}

void PlanParser::LoadPhase(Track& track, const String& phase_dir) {
	Phase& p = track.phases.Add();
	p.path = phase_dir;
	p.name = GetFileTitle(phase_dir);
	
	FindFile ff(AppendFileName(phase_dir, "*.md"));
	while(ff) {
		LoadTask(p, ff.GetPath());
		ff.Next();
	}
}

void PlanParser::LoadTask(Phase& phase, const String& task_file) {
	Task& t = phase.tasks.Add();
	t.path = task_file;
	t.name = GetFileTitle(task_file);
	
	String content = LoadFile(task_file);
	RegExp reStatus("# Status:\\s+(\\w+)");
	if(reStatus.Match(content)) {
		t.status = StringToStatus(reStatus[0]);
	}
	
	RegExp reDesc("## Objective\\n([^#]+)");
	if(reDesc.Match(content)) {
		t.description = TrimBoth(reDesc[0]);
	}
}

} 
