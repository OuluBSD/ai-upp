#include "AICore.h"

INITBLOCK {
	using namespace Upp;
	
	TypedStringHasher<ActionNode>("ActionNode");
	
	VfsValueExtFactory::Register<SolverExt>(
		"SolverExt", VFSEXT_DEFAULT,
		"ai.vfs.module", "AI|Util");
	
	VfsValueExtFactory::Register<CommitTreeExt>(
		"CommitTreeExt", VFSEXT_DEFAULT,
		"ai.vfs.module.commit.tree", "AI|Util|Module");
	
	VfsValueExtFactory::Register<CommitDiffListExt>(
		"CommitDiffListExt", VFSEXT_DEFAULT,
		"ai.vfs.module.commit.list", "AI|Util|Module");
	
}
