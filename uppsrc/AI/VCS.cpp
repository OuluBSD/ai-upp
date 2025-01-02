#include "AI.h"


NAMESPACE_UPP


VersionControlSystem::VersionControlSystem() {
	
	
}

VersionControlSystem::~VersionControlSystem() {
	Close();
}

void VersionControlSystem::Initialize(String path) {
	ASSERT(!FileExists(path) || DirectoryExists(path));
	dir = path;
	RealizeDirectory(dir);
	
}

void VersionControlSystem::Close() {
	ASSERT(scopes.IsEmpty());
	scopes.Clear();
}

bool VersionControlSystem::IsStoring() const {
	return storing;
}

void VersionControlSystem::SetStoring() {
	storing = true;
}

void VersionControlSystem::SetLoading() {
	storing = false;
}

VersionControlSystem::Scope& VersionControlSystem::Push(ScopeType t, String name) {
	Scope& s = scopes.Add();
	s.type = t;
	s.name = name;
	RealizeDirectory(GetCurrentDirectory());
	return s;
}

void VersionControlSystem::Pop() {
	scopes.SetCount(scopes.GetCount()-1);
}

void VersionControlSystem::RemoveIdxFolders(int begin) {
	for (int i = begin;; i++) {
		String last_path = GetCurrentPath(IntStr(i));
		if (!DirectoryExists(last_path))
			break;
		DeleteFolderDeep(last_path);
	}
}

void VersionControlSystem::BeginObject(String key) {
	Push(ST_OBJECT, key);
}
void VersionControlSystem::BeginAt(int i) {
	Push(ST_OBJECT, IntStr(i));
}

void VersionControlSystem::BeginKey(int i, String key) {
	Panic("TODO"); // dir with "i" and key,value subdirs
	Push(ST_OBJECT, key);
}

void VersionControlSystem::End() {
	Pop();
}

String VersionControlSystem::GetCurrentDirectory() const {
	String path = this->dir;
	for (const auto& scope : scopes) {
		path = AppendFileName(path, scope.name);
	}
	return path;
}

String VersionControlSystem::GetCurrentPath(String name) const {
	return AppendFileName(GetCurrentDirectory(), name);
}

int VersionControlSystem::ResolveCount() const {
	FindFile ff;
	int max_count = 0;
	if (ff.Search(AppendFileName(GetCurrentDirectory(), "*"))) do {
		if (!ff.IsDirectory()) continue;
		String name = ff.GetName();
		if (IsAllDigit(name))
			max_count = max(max_count, ScanInt(name)+1);
	} while (ff.Next());
	return max_count;
}


END_UPP_NAMESPACE
