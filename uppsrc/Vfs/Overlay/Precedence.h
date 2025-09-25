// Overlay precedence provider (API scaffold)
#ifndef _Vfs_Overlay_Precedence_h_
#define _Vfs_Overlay_Precedence_h_

#include <Core/Core.h>

NAMESPACE_UPP

// Return ordered package names or hashes, highest precedence first.
struct PackagePrecedenceProvider {
    virtual Vector<hash_t> GetPackageOrder() const = 0;
    virtual ~PackagePrecedenceProvider() {}
};

END_UPP_NAMESPACE

#endif

