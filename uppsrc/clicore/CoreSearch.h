#ifndef _clicore_CoreSearch_h_
#define _clicore_CoreSearch_h_

#include <Core/Core.h>
#include "CoreWorkspace.h"

using namespace Upp;

class CoreSearch {
public:
    CoreSearch();
    ~CoreSearch();

    // File search operations
    bool FindInFiles(const String& pattern, const String& directory, bool replace,
                     const String& replacement, Vector<String>& results, String& error);

    // Code search operations
    bool SearchCode(const String& query, Vector<String>& results, String& error);

    // Code search with workspace awareness
    bool SearchCodeInPackage(const String& query, const CoreWorkspace::Package& pkg,
                             Vector<String>& results, String& error);

    // Additional search utilities
    void SearchForFiles(Index<String>& files, const String& dir, const String& mask,
                        int readonly = Null, Time since = Null);

private:
    // Helper methods for search operations
    bool SearchInFile(const String& file_path, const String& pattern,
                      Vector<String>& results, String& error);
};

#endif