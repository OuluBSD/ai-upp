#ifndef UPP_CLICORE_COREREFACTOR_H
#define UPP_CLICORE_COREREFACTOR_H

#include <CoreAssist/CoreAssist.h>
#include <CoreEditor/CoreEditor.h>
#include <CoreIde/CoreIde.h>
#include <CoreBuild/CoreBuild.h>
#include <CoreGraph/CoreGraph.h>
#include <CoreWorkspace/CoreWorkspace.h>

class CoreIde;

class CoreRefactor : Moveable<CoreRefactor> {
public:
    CoreRefactor();

    // Rename symbol everywhere in the workspace
    bool RenameSymbol(const String& old_name,
                      const String& new_name,
                      CoreIde& ide,
                      String& error);

    // Remove unused includes (#include lines not referenced by symbols)
    bool RemoveDeadIncludes(const String& path,
                            CoreIde& ide,
                            String& error,
                            int* out_count = nullptr);

    // Canonicalize include paths (optional v1)
    bool CanonicalizeIncludes(const String& path,
                              CoreIde& ide,
                              String& error,
                              int* out_count = nullptr);

private:
    // internal helpers
    bool CollectSymbolEdits(const String& old_name,
                            const String& new_name,
                            CoreIde& ide,
                            Vector<String>& out_files,
                            String& error);

    bool ApplyEdits(const String& path,
                    const Vector<Tuple<int, int, String>>& edits,
                    CoreIde& ide,
                    String& error);
};

#endif