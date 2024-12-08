#include "Core.h"

NAMESPACE_UPP

EcsIndexer::EcsIndexer() {
	
}

bool EcsIndexer::AcceptExt(String ext) {
	return ext == ".ecs" || ext == ".env" || ext == ".db-src";
}

void EcsIndexer::RunJob(IndexerJob& job) {
	RunPath(job.path);
}

void EcsIndexer::RunPath(String path) {
	MetaEnvironment& env = MetaEnv();
	if (!env.LoadFileRoot("", path, true)) {
		LOG(path + ": error: failed to run path (at EcsIndexer::RunPath)");
	}
}

#if 0
bool EcsIndexer::LoadEcsSpace(String path) {
	MetaEnvironment& env = MetaEnv();
	
	EcsSpace space;
	LoadFromJsonFile(space, path);
	
	MetaSrcPkg& pkg = env.Load("", path);
	String rel_path = pkg.GetRelativePath(path);
	pkg_i = pkg.id;
	file_i = pkg.filenames.Find(rel_path);
	
	// TODO file & pkg idx?
	MetaNode& ecs_root = env.root.GetAdd(space.id, "", METAKIND_ECS_SPACE);
	ecs_root.pkg = pkg_i;
	ecs_root.file = file_i;
	
	if (!MergeNode(ecs_root, space))
		return false;
	
	MetaNode file_nodes;
	env.SplitNode(env.root, file_nodes, pkg.id);
	file_nodes.SetPkgFileDeep(0,0);
	
	pkg.Store(file_nodes, false);
	
	return true;
}
#endif

bool EcsIndexer::RunCurrentFile() {
	Panic("TODO");
	
	return false;
}

bool EcsIndexer::IsDirty(const String& s) {
	FileTime cur_filetime = GetFileTime(s);
	FileTime& prev_filetime = last_checks.GetAdd(s, TimeToFileTime(Time(1970,1,1)));
	bool dirty = prev_filetime != cur_filetime;
	prev_filetime = cur_filetime;
	return dirty;
}

INITIALIZER_INDEXER_EXTENSION(EcsIndexer)

END_UPP_NAMESPACE
