#ifndef _ide_Meta_Meta_h_
#define _ide_Meta_Meta_h_

#include <ide/clang/clang.h>
#include <Meta/Meta.h>

NAMESPACE_UPP

#include "ClangTypeResolver.h"
#include "EcsLang.h"

void Assign(MetaNode& mn, MetaNode* owner, const ClangNode& n);
void Store(MetaEnvironment& env, String& includes, const String& path, ClangNode& n);
void UpdateWorkspace(MetaEnvironment& env, Workspace& wspc);

END_UPP_NAMESPACE
	
#endif
