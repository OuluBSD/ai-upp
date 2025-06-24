#ifndef _Vfs2_Vfs_h_
#define _Vfs2_Vfs_h_

#include <Draw2/Draw.h>
#include <Vfs/Vfs.h>

#ifdef flagGUI
#include <CtrlCore/CtrlCore.h>
#endif

struct FileAnnotation;

#ifdef flagV1
	#error V1 flag is incompatible with this package
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
