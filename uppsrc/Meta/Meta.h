#ifndef _Meta_Meta_h_
#define _Meta_Meta_h_

#include <Core/Core.h>
#include <ide/clang/clang.h>

struct FileAnnotation;

#ifndef flagAI
	#error AI flag is currently still required
#endif

NAMESPACE_UPP

#include "Defs.h"
#include "Enum.h"
#include "Common.h"
#include "ExtList.h"
#include "Node.h"
#include "Entity.h"
#include "ClangTypeResolver.h"
#include "CodeVisitor.h"
#include "CodeGenerator.h"
#include "SolverBase.h"

END_UPP_NAMESPACE

#endif
