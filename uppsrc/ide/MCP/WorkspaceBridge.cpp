#include "MCP.h"

NAMESPACE_UPP

// These use lightweight queries; extend as needed.
String GetCurrentWorkspaceName() {
    // Try to use existing IDE facilities if available; fallback to empty.
    extern String GetCurrentIdeWorkspaceName();
    if(&GetCurrentIdeWorkspaceName) // avoid ODR if symbol absent in some builds
        return GetCurrentIdeWorkspaceName();
    return String::GetVoid();
}

int GetCurrentWorkspacePackageCount() {
    extern int GetCurrentIdeWorkspacePackageCount();
    if(&GetCurrentIdeWorkspacePackageCount)
        return GetCurrentIdeWorkspacePackageCount();
    return 0;
}

END_UPP_NAMESPACE

