#include "TextCore.h"

NAMESPACE_UPP

EcsIndexer::EcsIndexer() {
	
}

bool EcsIndexer::AcceptExt(String ext) {
	return ext == ".ecs";
}

void EcsIndexer::RunJob(IndexerJob& job) {
	if (!LoadEcsSpace(job.path)) {
		LOG(job.path + ": error: failed to run job (at EcsIndexer::RunJob)");
	}
}

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
	
	pkg.Store(file_nodes);
	return true;
}

bool EcsIndexer::RunCurrentFile() {
	Panic("TODO");
	
	return false;
}

bool EcsIndexer::MergeNode(MetaNode& root, EcsSpace& other) {
	Vector<MetaNode*> scope;
	scope << &root;
	return MergeVisit(scope, other);
}

bool EcsIndexer:: MergeVisit(Vector<MetaNode*>& scope, EcsSpace& n1) {
	MetaNode& n0 = *scope.Top();
	ASSERT(n0.kind == METAKIND_ECS_SPACE && n0.id == n1.id);
	
	for (EcsSpace& sub1 : n1.sub) {
		auto& sub0 = n0.GetAdd(n1.id, "", METAKIND_ECS_SPACE);
		sub0.pkg = pkg_i;
		sub0.file = file_i;
		scope << &sub0;
		bool succ = MergeVisit(scope, sub1);
		scope.Pop();
		if (!succ)
			return false;
	}
	
	while (n1.entities.GetCount()) {
		MetaNode& sub0 = n0.Add(n1.entities.Detach(0));
		sub0.pkg = pkg_i;
		sub0.file = file_i;
	}
	
	return true;
}

INITIALIZER_INDEXER_EXTENSION(EcsIndexer)

END_UPP_NAMESPACE
