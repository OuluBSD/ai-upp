#ifndef _Meta_Meta_h_
#define _Meta_Meta_h_

#include <Draw2/Draw.h>

#ifndef flagLCLANG
	#ifdef PLATFORM_POSIX
	#define DYNAMIC_LIBCLANG // dynamic loading of clang experiment (does not seem to work in Win32)
	#endif
#endif
#ifdef DYNAMIC_LIBCLANG
#include <ide/clang/libclang.h>
#else
#include <clang-c/Index.h>
#endif

#ifdef flagGUI
#include <CtrlCore/CtrlCore.h>
#endif

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
#include "CodeVisitor.h"
#include "SolverBase.h"

END_UPP_NAMESPACE

#endif
