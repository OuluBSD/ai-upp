#include "OverlayVFS.h"
#include <Vfs/Overlay/Overlay.h>  // Include the overlay system here

NAMESPACE_UPP

OverlayVFS::OverlayVFS()
{
}

bool OverlayVFS::GetFiles(const VfsPath& rel_path, Vector<VfsItem>& items)
{
	// Convert VfsPath to string path for overlay system
	String path_str = (String)rel_path;
	
	// Use OverlayManager to get a list of items at the given path
	OverlayManager& overlay_manager = OverlayManager::GetInstance();
	Vector<String> children = overlay_manager.List(path_str);
	
	// Convert the string list to VfsItem list
	for (const String& child_name : children) {
		VfsItem item;
		item.name = child_name;
		// Try to determine the type by checking with overlay manager
		String child_path = path_str + "/" + child_name;
		Value merged_value = overlay_manager.GetMerged(child_path);
		if (!merged_value.IsVoid()) {
			item.type = VFS_FILE;
			item.type_str = "file";
		} else {
			item.type = VFS_DIRECTORY;
			item.type_str = "dir";
		}
		items.Add() = item;
	}
	
	return true;
}

VfsItemType OverlayVFS::CheckItem(const VfsPath& rel_path)
{
	// Convert VfsPath to string path for overlay system
	String path_str = (String)rel_path;
	
	// Check if the path exists in the overlay system
	OverlayManager& overlay_manager = OverlayManager::GetInstance();
	Value merged_value = overlay_manager.GetMerged(path_str);
	
	if (!merged_value.IsVoid()) {
		return VFS_FILE; // For simplicity, treat everything as file if it exists
	} else {
		return VFS_NULL;
	}
}

END_UPP_NAMESPACE