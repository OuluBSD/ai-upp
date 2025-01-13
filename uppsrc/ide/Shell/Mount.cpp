#include "Shell.h"

NAMESPACE_UPP



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

MountManager::MountPoint* MountManager::Find(const VfsPath& path, VfsPath* rel_path) {
	for (auto& mm : mounts) {
		if (path.IsLeft(mm.path)) {
			if (rel_path) {
				rel_path->Set(path, mm.path.parts.GetCount(), path.parts.GetCount());
			}
			return &mm;
		}
	}
	return 0;
}

bool MountManager::GetFiles(const VfsPath& path, Vector<VfsItem>& items) {
	items.Clear();
	for (auto& mm : mounts) {
		if (mm.path.parts.GetCount() == path.parts.GetCount()+1 &&
			mm.path.IsSame(path, 0, 0, mm.path.parts.GetCount())) {
			VfsItem& item = items.Add();
			item.name = mm.path.parts.Top();
			item.type_str = mm.type;
		}
	}
	if (!path.parts.IsEmpty()) {
		VfsPath rel_path;
		MountPoint* mp = Find(path, &rel_path);
		if (!mp) {
			last_error = "Path is not contained in any mount: " + path.str;
			return false;
		}
		if (!mp->vfs) {
			last_error = "Null vfs in mountpoint";
			return false;
		}
		return mp->vfs->GetFiles(rel_path, items);
	}
	return true;
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
