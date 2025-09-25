#ifndef _Vfs_Factory_VfsFactory_h_
#define _Vfs_Factory_VfsFactory_h_

struct VfsValue;
struct VfsValueExt;

struct VfsValueExtFactory {
    // Staged signatures; implementation remains in legacy location for now
    static VfsValueExt* Create(hash_t type_hash, VfsValue& owner);
};

#endif

