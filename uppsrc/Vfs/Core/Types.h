// Small common typedefs/enums extracted from legacy VfsValue API (no behavior)
#ifndef _Vfs_Core_Types_h_
#define _Vfs_Core_Types_h_

#include <Core/Core.h>

NAMESPACE_UPP

typedef uint64 hash_t;

// Placeholder for type classification used across Vfs
using TypeCls = std::type_index;

inline TypeCls AsTypeClsStatic(const std::type_info& ti) { return TypeCls(ti); }

END_UPP_NAMESPACE

#endif

