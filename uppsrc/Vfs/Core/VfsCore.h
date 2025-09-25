// Vfs/Core main header (API scaffold)
#ifndef _Vfs_Core_VfsCore_h_
#define _Vfs_Core_VfsCore_h_

#include <Core/Core.h>

NAMESPACE_UPP

// Forward declarations for staged refactor
struct VfsValue;
struct VfsValueExt;
struct VfsValueExtCtrl; // GUI-side counterpart lives outside core
struct AstValue;

// Minimal enums/placeholders to anchor includes without behavior changes
typedef uint64 hash_t;

END_UPP_NAMESPACE

#endif

