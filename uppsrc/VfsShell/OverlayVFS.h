#ifndef _VfsShell_OverlayVFS_h_
#define _VfsShell_OverlayVFS_h_

#include <Core/Core.h>
#include <Core/VfsBase/VfsBase.h>

NAMESPACE_UPP

// VFS implementation that uses the overlay system for managing multiple file layers
class OverlayVFS : public VFS {
public:
	OverlayVFS();
	virtual ~OverlayVFS() {}

	// VFS interface implementation
	virtual bool GetFiles(const VfsPath& rel_path, Vector<VfsItem>& items) override;
	virtual VfsItemType CheckItem(const VfsPath& rel_path) override;

private:
    // We'll use the overlay system that's already implemented in the U++ Vfs/Overlay module
};

END_UPP_NAMESPACE

#endif