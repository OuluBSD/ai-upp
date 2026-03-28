#include "GitContext.h"

String GitContext::RunGit(const String& dir, const Vector<String>& args) {
	String cmd = "git";
	for(const String& a : args) cmd << " " << a;
	
	LocalProcess lp;
	if(!lp.Start(cmd, Vector<String>(), dir)) return "";
	
	String out;
	while(lp.IsRunning()) {
		out << lp.Get();
		Sleep(1);
	}
	out << lp.Get();
	return out;
}

void GitContext::Refresh(const String& working_dir) {
	repo_detected = false;
	status_cache.Clear();
	
	String root = TrimBoth(RunGit(working_dir, {"rev-parse", "--show-toplevel"}));
	if(root.IsEmpty() || root.StartsWith("fatal")) return;
	
	repo_detected = true;
	repo_root = root;
	branch = TrimBoth(RunGit(working_dir, {"rev-parse", "--abbrev-ref", "HEAD"}));
	head_hash = TrimBoth(RunGit(working_dir, {"rev-parse", "--short", "HEAD"}));
	
	// Parse status
	String st = RunGit(working_dir, {"status", "--porcelain"});
	Vector<String> lines = Split(st, "\n");
	for(const String& l : lines) {
		if(l.GetCount() < 4) continue;
		String s = l.Mid(0, 2);
		String path = l.Mid(3);
		int type = GIT_NONE;
		if(s == "??") type = GIT_UNTRACKED;
		else if(s == " M" || s == "M ") type = GIT_MODIFIED;
		else if(s == " A" || s == "A ") type = GIT_ADDED;
		else if(s == " D" || s == "D ") type = GIT_DELETED;
		else if(s == " R" || s == "R ") type = GIT_RENAMED;
		
		status_cache.Add(path, type);
	}
}

Vector<GitCommit> GitContext::GetHistory(const String& working_dir, const String& rel_path, int limit) {
	Vector<GitCommit> res;
	if(!repo_detected) return res;
	
	Vector<String> v_args;
	v_args.Add("log");
	v_args.Add("-" + AsString(limit));
	v_args.Add("--pretty=format:%h|%an|%ai|%s");
	if(!rel_path.IsEmpty()) {
		v_args.Add("--");
		v_args.Add(rel_path);
	}
	
	String out = RunGit(working_dir, v_args);
	Vector<String> lines = Split(out, "\n");
	for(const String& l : lines) {
		Vector<String> parts = Split(l, "|");
		if(parts.GetCount() >= 4) {
			GitCommit& c = res.Add();
			c.hash = parts[0];
			c.author = parts[1];
			c.date = ScanTime("%Y-%m-%d %H:%M:%S", parts[2]);
			c.subject = parts[3];
		}
	}
	return res;
}

int GitContext::GetStatus(const String& rel_path) {
	return status_cache.Get(rel_path, GIT_NONE);
}
