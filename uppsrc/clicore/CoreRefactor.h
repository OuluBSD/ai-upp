#ifndef UPP_CLICORE_COREREFACTOR_H
#define UPP_CLICORE_COREREFACTOR_H

#include <clicore/CoreAssist.h>
#include <clicore/CoreEditor.h>
#include <clicore/CoreIde.h>
#include <clicore/CoreBuild.h>
#include <clicore/CoreGraph.h>
#include <clicore/CoreWorkspace.h>

class CoreIde;

// Define a struct to replace Tuple usage
struct EditOperation : Moveable<EditOperation> {
    int pos;
    int length;
    String replacement;

    EditOperation() : pos(0), length(0) {}
    EditOperation(int p, int l, const String& r) : pos(p), length(l), replacement(r) {}
};

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
                    const Vector<EditOperation>& edits,
                    CoreIde& ide,
                    String& error);
};

#endif