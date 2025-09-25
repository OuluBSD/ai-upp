// Vfs/Storage header (API scaffold)
#ifndef _Vfs_Storage_VfsStorage_h_
#define _Vfs_Storage_VfsStorage_h_

#include <Core/Core.h>
#include <Vfs/Core/VfsCore.h>

NAMESPACE_UPP

struct VfsValue;

// Save/load per-file fragments; overlay index handled separately
bool VfsSaveFragment(const String& path, const VfsValue& fragment);
bool VfsLoadFragment(const String& path, VfsValue& out_fragment);

// Back-compat loader for legacy IDE dumps (placeholder)
bool VfsLoadLegacy(const String& path, VfsValue& out_fragment);

END_UPP_NAMESPACE

#endif

