// Small common typedefs/enums extracted from legacy VfsValue API (no behavior)
#ifndef _Vfs_Core_Types_h_
#define _Vfs_Core_Types_h_


typedef uint64 hash_t;

inline TypeCls AsTypeClsStatic(const std::type_info& ti) { return TypeCls(ti); }


#endif

