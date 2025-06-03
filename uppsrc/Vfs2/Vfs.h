#ifndef _Vfs2_Vfs_h_
#define _Vfs2_Vfs_h_

#include <Draw2/Draw.h>
#include <Vfs/Vfs.h>

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
