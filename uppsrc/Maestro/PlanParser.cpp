#include "Maestro.h"

namespace Upp {

void PlanParser::Reset() {
	tracks.Clear();
	runbooks.Clear();
	workgraphs.Clear();
}

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
	t.id = GetFileTitle(track_dir);
	t.name = t.id;
	
	FindFile ff(AppendFileName(track_dir, "*"));
	while(ff) {
		if(ff.IsDirectory() && ff.GetName() != "." && ff.GetName() != "..") {
			LoadPhase(t, ff.GetPath());
		}
		ff.Next();
	}
}

void PlanParser::LoadPhase(Track& track, const String& phase_path) {
	if(DirectoryExists(phase_path)) {
		Phase& p = track.phases.Add();
		p.path = phase_path;
		p.id = GetFileTitle(phase_path);
		p.name = p.id;
		
		FindFile ff(AppendFileName(phase_path, "*.md"));
		while(ff) {
			LoadTask(p, ff.GetPath());
			ff.Next();
		}
	} else if(FileExists(phase_path)) {
		Phase& p = track.phases.Add();
		p.path = phase_path;
		p.id = GetFileTitle(phase_path);
		p.name = p.id;
		
		String content = LoadFile(phase_path);
		Vector<String> lines = Split(content, '\n', false);
		for(const String& l : lines) {
			RegExp reTask("- \\[(.)\\] \\*\\*(.+)\\*\\*");
			if(reTask.Match(l)) {
				Task& t = p.tasks.Add();
				t.id = reTask[1];
				t.name = reTask[1];
				String status_char = reTask[0];
				if(status_char == "x") t.status = STATUS_DONE;
				else if(status_char == "/") t.status = STATUS_IN_PROGRESS;
				else t.status = STATUS_TODO;
			}
		}
	}
}

void PlanParser::LoadTask(Phase& phase, const String& task_file) {
	Task& t = phase.tasks.Add();
	t.path = task_file;
	t.id = GetFileTitle(task_file);
	t.name = t.id;
	
	String content = LoadFile(task_file);
	RegExp reStatus("# Status:\\s+(\\w+)");
	if(reStatus.Match(content)) {
		t.status = StringToStatus(ToLower(reStatus[0]));
	}
	
	RegExp reDesc("## Objective\\n([^#]+)");
	if(reDesc.Match(content)) {
		t.description = TrimBoth(reDesc[0]);
	}
	
	RegExp reDeps("# Depends on:\\s+(.+)");
	if(reDeps.Match(content)) {
		Vector<String> deps = Split(reDeps[0], ",", true);
		for(auto& d : deps) t.depends_on.Add(TrimBoth(d));
	}
}

void PlanParser::LoadRunbooks(const String& docs_root) {
	String rb_dir = AppendFileName(docs_root, "docs/maestro/runbooks/items");
	FindFile ff(AppendFileName(rb_dir, "*.json"));
	while(ff) {
		String content = LoadFile(ff.GetPath());
		Value v = ParseJSON(content);
		if(!v.IsError()) {
			Runbook& rb = runbooks.Add();
			LoadFromJson(rb, content);
		}
		ff.Next();
	}
}

void PlanParser::LoadWorkGraphs(const String& docs_root) {
	workgraphs.Clear();
	String wg_dir = AppendFileName(docs_root, "docs/maestro/plans/workgraphs");
	FindFile ff(AppendFileName(wg_dir, "*.json"));
	while(ff) {
		if(ff.GetName() == "index.json") {
			ff.Next();
			continue;
		}
		String content = LoadFile(ff.GetPath());
		Value v = ParseJSON(content);
		if(!v.IsError()) {
			WorkGraph& wg = workgraphs.Add();
			LoadFromJson(wg, content);
		}
		ff.Next();
	}
}

void PlanParser::LoadMaestroTracks(const String& docs_root) {
	String track_dir = AppendFileName(docs_root, "docs/maestro/tracks");
	String phase_root = AppendFileName(docs_root, "docs/phases");
	
	FindFile ff(AppendFileName(track_dir, "*.json"));
	while(ff) {
		String content = LoadFile(ff.GetPath());
		Value v = ParseJSON(content);
		if(!v.IsError()) {
			Track& t = tracks.Add();
			t.id = v["track_id"];
			t.name = v["name"];
			t.status = v["status"];
			t.completion = v["completion"];
			
			Value phases = v["phases"];
			if(phases.Is<ValueArray>()) {
				for(int i = 0; i < phases.GetCount(); i++) {
					String phase_id = phases[i];
					String phase_file = AppendFileName(phase_root, ToLower(phase_id) + ".md");
					if(FileExists(phase_file)) {
						LoadPhase(t, phase_file);
					} else {
						Phase& p = t.phases.Add();
						p.id = phase_id;
						p.name = phase_id;
					}
				}
			}
		}
		ff.Next();
	}
}

bool PlanParser::UpdateTaskStatus(const String& docs_root, const String& track_id, const String& phase_id, const String& task_id, TaskStatus status) {
	// 1. Try finding the task as a separate file in the phase directory
	String task_file = AppendFileName(AppendFileName(AppendFileName(docs_root, "uppsrc/AI/plan"), track_id), phase_id);
	task_file = AppendFileName(task_file, task_id + ".md"); // Assume task_id matches filename
	
	if(FileExists(task_file)) {
		String content = LoadFile(task_file);
		String s = ToUpper(StatusToString(status));
		
		// Replace # Status: ... with new status
		RegExp reStatus("# Status:\\s+(\\w+)");
		if(reStatus.Match(content)) {
			// Simple replace for now, ideally we'd use the match position
			// This assumes only one "Status:" line which is standard for these files
			int pos = content.Find("# Status:");
			int end = content.Find("\n", pos);
			if(pos >= 0 && end > pos) {
				content.Replace(content.Mid(pos, end - pos), "# Status: " + s);
				return SaveFile(task_file, content);
			}
		}
		// If no status line found, append it? For now, return false if format differs
		return false;
	}

	// 2. Fallback: Try finding the phase file (Maestro legacy format)
	String phase_file = AppendFileName(AppendFileName(docs_root, "docs/phases"), ToLower(phase_id) + ".md");
	if(!FileExists(phase_file)) return false;
	
	String content = LoadFile(phase_file);
	Vector<String> lines = Split(content, '\n', false);
	String new_content;
	String status_char = " ";
	if(status == STATUS_DONE) status_char = "x";
	else if(status == STATUS_IN_PROGRESS) status_char = "/";
	
	bool found = false;
	for(String l : lines) {
		RegExp reTask("- \\[(.)\\] \\*\\*(.+)\\*\\*");
		if(reTask.Match(l)) {
			String tid = reTask[1];
			if(tid == task_id) {
				if(l.GetCount() > 2) {
					l.Insert(2, status_char);
					l.Remove(3);
					found = true;
				}
			}
		}
		new_content << l << "\n";
	}
	
	if(found) return SaveFile(phase_file, new_content);
	return false;
}

}
