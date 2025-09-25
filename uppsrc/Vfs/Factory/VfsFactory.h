// Vfs/Factory header (API scaffold)
#ifndef _Vfs_Factory_VfsFactory_h_
#define _Vfs_Factory_VfsFactory_h_

#include <Core/Core.h>
#include <Vfs/Core/VfsCore.h>

NAMESPACE_UPP

struct VfsValue;
struct VfsValueExt;

struct VfsValueExtFactory {
    // Staged signatures; implementation remains in legacy location for now
    static VfsValueExt* Create(hash_t type_hash, VfsValue& owner);
};

END_UPP_NAMESPACE

#endif

