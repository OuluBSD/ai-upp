#ifndef _VfsShell_NcursesEditor_h_
#define _VfsShell_NcursesEditor_h_

#include <Core/Core.h>
#include <Core/VfsBase/VfsBase.h>

NAMESPACE_UPP

// A simple ncurses-based text editor for VfsShell
class NcursesEditor {
public:
    static bool RunEditor(const String& vfsPath, const String& initialContent);
    
private:
    static bool fileModified;
    static String currentPath;
};

END_UPP_NAMESPACE

#endif