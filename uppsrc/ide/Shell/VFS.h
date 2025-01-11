#ifndef _ide_Shell_VFS_h_
#define _ide_Shell_VFS_h_

NAMESPACE_UPP

struct VFS : Pte<VFS> {
	VFS() {}
	virtual ~VFS() {}
	
};

struct IdeInternalFS : VFS {
	IdeInternalFS() {}
	
};

END_UPP_NAMESPACE

#endif
