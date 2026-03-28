#ifndef _Overviewer_GitContext_h_
#define _Overviewer_GitContext_h_

#include <Core/Core.h>

using namespace Upp;

struct GitCommit : Moveable<GitCommit> {
	String hash;
	String author;
	Time date;
	String subject;

	void Jsonize(JsonIO& jio) {
		jio("hash", hash)("author", author)("date", date)("subject", subject);
	}
};

enum GitStatusType {
	GIT_NONE = 0,
	GIT_UNTRACKED,
	GIT_MODIFIED,
	GIT_ADDED,
	GIT_DELETED,
	GIT_RENAMED,
	GIT_IGNORED
};

struct GitContext {
	bool repo_detected = false;
	String repo_root;
	String branch;
	String head_hash;
	
	VectorMap<String, int> status_cache; // path -> GitStatusType

	void Refresh(const String& working_dir);
	Vector<GitCommit> GetHistory(const String& working_dir, const String& rel_path, int limit = 10);
	int GetStatus(const String& rel_path);
	
	static String RunGit(const String& dir, const Vector<String>& args);
};

#endif
