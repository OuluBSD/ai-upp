#include "Overlay.h"

NAMESPACE_UPP

// Implement the OverlayView interface for MetaEnvironment
Vector<String> MetaEnvironment::List(String logical_path) const {
    // Delegate to the root VfsValue
    Vector<String> result;
    
    // Parse the logical path and navigate the VFS tree
    VfsPath path(logical_path);
    const VfsValue* current = &root;
    
    // Navigate to the specified path
    const auto& parts = path.Parts();
    for (int i = 0; i < parts.GetCount() && current; i++) {
        const String& part = parts[i].ToString();
        int idx = current->Find(part);
        if (idx >= 0) {
            current = &current->sub[idx];
        } else {
            current = nullptr;
        }
    }
    
    // List children if we found the node
    if (current) {
        for (int i = 0; i < current->sub.GetCount(); i++) {
            result.Add(current->sub[i].id);
        }
    }
    
    return result;
}

Value MetaEnvironment::GetMerged(String logical_path) const {
    // Return merged JSON representation for debugging/inspection
    // This would typically merge values from multiple overlays
    
    VfsPath path(logical_path);
    const VfsValue* current = &root;
    
    // Navigate to the specified path
    const auto& parts = path.Parts();
    for (int i = 0; i < parts.GetCount() && current; i++) {
        const String& part = parts[i].ToString();
        int idx = current->Find(part);
        if (idx >= 0) {
            current = &current->sub[idx];
        } else {
            current = nullptr;
        }
    }
    
    // Convert the found node to a JSON value for inspection
    if (current) {
        return StoreAsJson(*current);
    }
    
    return Value();
}

// Method to add an overlay to the MetaEnvironment
void MetaEnvironment::AddOverlay(Ptr<VfsOverlay> overlay) {
    // In a real implementation, this would register the overlay
    // For now, we'll just acknowledge it
    if (overlay) {
        // Store reference to overlay for future use
        // This is a placeholder implementation
    }
}

END_UPP_NAMESPACE