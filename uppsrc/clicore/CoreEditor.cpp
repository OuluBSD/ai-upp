#include "CoreEditor.h"

CoreEditor::CoreEditor() : dirty(false) {
    // Initialize the editor with empty content
}

bool CoreEditor::LoadFile(const String& path, String& error) {
    FileIn in(path);
    if (!in.IsOpen()) {
        error = "Could not open file: " + path;
        return false;
    }
    
    text = in.GetAll(4 * 1024 * 1024 - 1);
    this->path = path;
    dirty = false;
    
    // Clear undo/redo stacks since we're loading fresh content
    undo_stack.Clear();
    redo_stack.Clear();
    
    return true;
}

bool CoreEditor::SaveFile(String& error) {
    if (path.IsEmpty()) {
        error = "No path specified for saving";
        return false;
    }
    
    FileOut out(path);
    if (!out.IsOpen()) {
        error = "Could not open file for writing: " + path;
        return false;
    }
    
    out.Put(text);
    if (out.IsError()) {
        error = "Error writing to file: " + path;
        return false;
    }
    
    dirty = false;
    return true;
}

bool CoreEditor::SaveFileAs(const String& path, String& error) {
    String oldPath = this->path;
    this->path = path;
    
    bool result = SaveFile(error);
    if (!result) {
        // Restore the old path if saving failed
        this->path = oldPath;
    } else {
        dirty = false;
    }
    
    return result;
}

const String& CoreEditor::GetPath() const {
    return path;
}

const String& CoreEditor::GetContent() const {
    return text;
}

bool CoreEditor::IsDirty() const {
    return dirty;
}

// Editing operations
bool CoreEditor::Insert(int pos, const String& text_to_insert) {
    if (pos < 0 || pos > this->text.GetCount()) {
        return false; // Position out of bounds
    }
    
    String before = this->text;
    this->text.Insert(pos, text_to_insert);
    String after = this->text;
    
    PushEdit(pos, before, after);
    
    dirty = true;
    return true;
}

bool CoreEditor::Erase(int pos, int count) {
    if (pos < 0 || pos >= this->text.GetCount() || count <= 0 || (pos + count) > this->text.GetCount()) {
        return false; // Invalid parameters
    }
    
    String before = this->text;
    String removed_text = this->text.Mid(pos, count);
    this->text.Remove(pos, count);
    String after = this->text;
    
    PushEdit(pos, before, after);
    
    dirty = true;
    return true;
}

bool CoreEditor::Replace(int pos, int count, const String& replacement) {
    if (pos < 0 || pos >= this->text.GetCount() || count <= 0 || (pos + count) > this->text.GetCount()) {
        return false; // Invalid parameters
    }
    
    String before = this->text;
    this->text.Remove(pos, count);
    this->text.Insert(pos, replacement);
    String after = this->text;
    
    PushEdit(pos, before, after);
    
    dirty = true;
    return true;
}

// Navigation helpers
bool CoreEditor::GotoLine(int line, int& out_pos) const {
    if (line <= 0) {
        out_pos = -1;
        return false; // Line numbers start from 1
    }
    
    int current_line = 1;
    int pos = 0;
    
    // If requesting the first line, position is 0
    if (line == 1) {
        out_pos = 0;
        return true;
    }
    
    // Scan through the text to find the requested line
    for (; pos < text.GetCount(); pos++) {
        if (text[pos] == '\n') {
            current_line++;
            if (current_line == line) {
                out_pos = pos + 1; // Position after the newline character
                return true;
            }
        }
    }
    
    // Line number exceeds the number of lines in the text
    out_pos = -1;
    return false;
}

// Search / replace in buffer
bool CoreEditor::FindFirst(const String& pattern, int start_pos, bool case_sensitive, int& out_pos) const {
    if (start_pos < 0 || start_pos >= text.GetCount() || pattern.IsEmpty()) {
        out_pos = -1;
        return false;
    }
    
    if (case_sensitive) {
        out_pos = text.Find(pattern, start_pos);
    } else {
        out_pos = ToLower(text).Find(ToLower(pattern), start_pos);
    }
    
    return out_pos != -1;
}

bool CoreEditor::ReplaceAll(const String& pattern, const String& replacement, bool case_sensitive, int& out_count) {
    if (pattern.IsEmpty()) {
        out_count = 0;
        return true; // Nothing to replace
    }
    
    String current_text = text;
    int count = 0;
    
    if (case_sensitive) {
        int pos = 0;
        while ((pos = current_text.Find(pattern, pos)) != -1) {
            current_text.Insert(pos, replacement);
            current_text.Remove(pos + replacement.GetCount(), pattern.GetCount());
            pos += replacement.GetCount();
            count++;
        }
    } else {
        // For case-insensitive replace, we need to do it manually
        String lower_text = ToLower(current_text);
        String lower_pattern = ToLower(pattern);
        int pos = 0;

        while ((pos = lower_text.Find(lower_pattern, pos)) != -1) {
            current_text.Insert(pos, replacement);
            current_text.Remove(pos + replacement.GetCount(), pattern.GetCount());

            // Update the lower_text to reflect the changes
            lower_text = ToLower(current_text);

            pos += replacement.GetCount();
            count++;
        }
    }
    
    if (count > 0) {
        String before = text;
        text = current_text;
        out_count = count;
        
        // Record a single undo step for the entire replacement operation
        PushEdit(0, before, text);
        
        dirty = true;
    } else {
        out_count = 0;
    }
    
    return true;
}

// Undo/redo
bool CoreEditor::Undo() {
    if (undo_stack.GetCount() == 0) {
        return false; // Nothing to undo
    }
    
    EditOp& op = undo_stack.Top();
    
    // Apply the inverse operation (restore 'before' state)
    String temp_text = text;
    text = op.before;
    
    // Push the inverse operation to the redo stack
    EditOp redo_op;
    redo_op.pos = op.pos;
    redo_op.before = temp_text;
    redo_op.after = op.before;
    redo_stack.Add(redo_op);
    
    undo_stack.Remove(undo_stack.GetCount() - 1);
    
    dirty = true;
    return true;
}

bool CoreEditor::Redo() {
    if (redo_stack.GetCount() == 0) {
        return false; // Nothing to redo
    }
    
    EditOp& op = redo_stack.Top();
    
    // Reapply the operation
    String temp_text = text;
    text = op.after;
    
    // Push the inverse operation to the undo stack
    EditOp undo_op;
    undo_op.pos = op.pos;
    undo_op.before = temp_text;
    undo_op.after = op.after;
    undo_stack.Add(undo_op);
    
    redo_stack.Remove(redo_stack.GetCount() - 1);
    
    dirty = true;
    return true;
}

void CoreEditor::PushEdit(int pos, const String& before, const String& after) {
    // Create a new edit operation
    EditOp op;
    op.pos = pos;
    op.before = before;
    op.after = after;
    
    // Add to undo stack
    undo_stack.Add(op);
    
    // Clear the redo stack since a new edit breaks the redo chain
    redo_stack.Clear();
}