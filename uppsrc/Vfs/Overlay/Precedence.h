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

// Default implementation that returns packages in insertion order
class DefaultPrecedenceProvider : public PackagePrecedenceProvider {
    Vector<hash_t> package_order;
    
public:
    virtual Vector<hash_t> GetPackageOrder() const override {
        return package_order;
    }
    
    void AddPackage(hash_t pkg_hash) {
        package_order.Add(pkg_hash);
    }
    
    void ClearPackages() {
        package_order.Clear();
    }
    
    void SetPackageOrder(const Vector<hash_t>& order) {
        package_order = order;
    }
};

END_UPP_NAMESPACE

#endif