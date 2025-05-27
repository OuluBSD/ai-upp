#ifndef _Vfs2_Vfs_h_
#define _Vfs2_Vfs_h_

#include <Draw2/Draw.h>
#include <Vfs/Vfs.h>

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
#include "ExtList.h"
#include "Types.h"
#include "Enum.h"
#include "Common.h"
#include "CodeVisitor.h"
#include "SolverBase.h"

END_UPP_NAMESPACE

#endif
