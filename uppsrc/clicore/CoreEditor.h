#ifndef _clicore_CoreEditor_h_
#define _clicore_CoreEditor_h_

#include <Core/Core.h>

using namespace Upp;

class CoreEditor : Moveable<CoreEditor> {
public:
    CoreEditor();

    bool LoadFile(const String& path, String& error);
    bool SaveFile(String& error);                    // save to original path
    bool SaveFileAs(const String& path, String& error);

    // Basic properties
    const String& GetPath() const;
    const String& GetContent() const;
    bool IsDirty() const;

    // Editing operations
    bool Insert(int pos, const String& text);
    bool Erase(int pos, int count);
    bool Replace(int pos, int count, const String& text);

    // Navigation helpers
    bool GotoLine(int line, int& out_pos) const;     // returns byte/char offset for line

    // Search / replace in buffer
    bool FindFirst(const String& pattern, int start_pos, bool case_sensitive, int& out_pos) const;
    bool ReplaceAll(const String& pattern, const String& replacement, bool case_sensitive, int& out_count);

    // Undo/redo
    bool Undo();
    bool Redo();

private:
    String path;
    String text;        // full buffer
    bool dirty;

    // Minimal undo stack
    struct EditOp : Moveable<EditOp> {
        int pos;
        String before;
        String after;
    };
    Vector<EditOp> undo_stack;
    Vector<EditOp> redo_stack;

    void PushEdit(int pos, const String& before, const String& after);
};

#endif