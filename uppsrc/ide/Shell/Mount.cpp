#include "Shell.h"

NAMESPACE_UPP

void VfsPath::Set(String path) {
	this->str = path;
	parts = Split(path , "/");
}


MountManager::MountManager() {
	
}

bool MountManager::HasRoot() const {
	for (const auto& mm : mounts)
		if (mm.path.str == "/")
			return true;
	return false;
}

MountManager::MountPoint* MountManager::Find(const String& path) {
	for (auto& mm : mounts)
		if (mm.path.str == path)
			return &mm;
	return 0;
}

bool MountManager::Mount(String path, VFS* vfs, String type) {
	if (Find(path)) {
		last_error = "Path already mounted '" + path + "'";
		return false;
	}
	
	MountPoint& mp = mounts.Add();
	mp.path.Set(path);
	mp.type = type;
	mp.vfs = vfs;
	
	return true;
}

	
END_UPP_NAMESPACE
