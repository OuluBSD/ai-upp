#include "clicore.h"
#include "CoreRefactor.h"
#include <CoreAssist/CoreAssist.h>
#include <CoreEditor/CoreEditor.h>
#include <CoreIde/CoreIde.h>
#include <CoreBuild/CoreBuild.h>
#include <CoreGraph/CoreGraph.h>
#include <CoreWorkspace/CoreWorkspace.h>

CoreRefactor::CoreRefactor() {
}

bool CoreRefactor::RenameSymbol(const String& old_name,
                               const String& new_name,
                               CoreIde& ide,
                               String& error) {
    if (old_name == new_name) {
        error = "Old name and new name are identical";
        return false;
    }

    // Validate identifier (simple heuristic)
    if (old_name.IsEmpty() || new_name.IsEmpty()) {
        error = "Names cannot be empty";
        return false;
    }

    // Check if new_name is a valid identifier
    if (!IsAlNum(new_name[0]) && new_name[0] != '_') {
        error = "New name is not a valid identifier";
        return false;
    }
    for(int i = 1; i < new_name.GetLength(); i++) {
        if (!IsAlNum(new_name[i]) && new_name[i] != '_') {
            error = "New name is not a valid identifier";
            return false;
        }
    }

    Vector<String> affected_files;
    if (!CollectSymbolEdits(old_name, new_name, ide, affected_files, error)) {
        return false;
    }

    if (affected_files.IsEmpty()) {
        error = "Symbol not found in workspace";
        return false;
    }

    // Now collect and apply all edits for all affected files
    CoreAssist& assist = ide.GetAssist();
    Vector<SymbolUsage> usages = assist.FindSymbolUsages(old_name);

    if (usages.IsEmpty()) {
        error = "Symbol not found in workspace";
        return false;
    }

    // Collect all unique file paths where the symbol is used
    ArrayMap<String, bool> unique_files;
    for (const auto& usage : usages) {
        if (!unique_files.Contains(usage.file_path)) {
            unique_files.Add(usage.file_path, true);
        }
    }

    Vector<String> files_to_process;
    for (int i = 0; i < unique_files.GetCount(); i++) {
        files_to_process.Add(unique_files.GetKey(i));
    }

    for (const String& file_path : files_to_process) {
        CoreEditor* editor = ide.GetEditor(file_path);
        if (!editor) {
            error = "Could not open editor for file: " + file_path;
            return false;
        }

        String content = editor->GetText();
        Vector<Tuple<int, int, String>> edits;

        int pos = 0;
        while ((pos = content.Find(old_name, pos)) != -1) {
            // Check word boundaries to ensure exact match
            bool is_boundary_before = (pos == 0) ||
                                     (!IsAlNum(content[pos-1]) && content[pos-1] != '_');
            bool is_boundary_after = (pos + old_name.GetLength() == content.GetLength()) ||
                                    (!IsAlNum(content[pos + old_name.GetLength()]) &&
                                     content[pos + old_name.GetLength()] != '_');

            if (is_boundary_before && is_boundary_after) {
                // Add an edit to replace old_name with new_name
                edits.Add(MakeTuple(pos, old_name.GetLength(), new_name));
            }

            pos += old_name.GetLength();
        }

        // Apply the edits to this file
        if (!edits.IsEmpty()) {
            if (!ApplyEdits(file_path, edits, ide, error)) {
                return false;
            }
        }
    }

    return true;
}

bool CoreRefactor::RemoveDeadIncludes(const String& path,
                                     CoreIde& ide,
                                     String& error,
                                     int* out_count) {
    if (out_count) *out_count = 0; // Initialize count

    // Get the file content from the editor
    CoreEditor* editor = ide.GetEditor(path);
    if (!editor) {
        error = "Could not open file: " + path;
        return false;
    }

    String content = editor->GetText();
    Vector<String> lines = Split(content, "\n");

    // Find all #include directives
    Vector<int> include_line_indices;
    Vector<String> include_targets;

    for(int i = 0; i < lines.GetCount(); i++) {
        String line = TrimLeft(lines[i]);
        if (StartsWith(line, "#include")) {
            include_line_indices.Add(i);

            // Extract the include target
            String include_line = line;
            String target;

            int pos1 = include_line.Find('"');
            int pos2 = include_line.Find('<');

            if (pos1 >= 0 && pos2 >= 0) {
                pos1 = min(pos1, pos2);
            } else if (pos2 >= 0) {
                pos1 = pos2;
            }

            if (pos1 >= 0) {
                if (include_line[pos1] == '"') {
                    // Double quoted include
                    int end_pos = include_line.Find('"', pos1 + 1);
                    if (end_pos > pos1) {
                        target = include_line.Mid(pos1 + 1, end_pos - pos1 - 1);
                    }
                } else if (include_line[pos1] == '<') {
                    // Angle bracket include
                    int end_pos = include_line.Find('>', pos1 + 1);
                    if (end_pos > pos1) {
                        target = include_line.Mid(pos1 + 1, end_pos - pos1 - 1);
                    }
                }
            }
            include_targets.Add(target);
        }
    }

    // Get all symbols in the current file to check if includes are used
    CoreAssist& assist = ide.GetAssist();
    Vector<String> all_symbols = assist.ExtractSymbolsFromContent(content);

    // Determine which includes are dead
    Vector<int> dead_include_indices;
    for(int i = 0; i < include_targets.GetCount(); i++) {
        String target = include_targets[i];

        // Check if the include target is referenced in the symbols
        bool is_used = false;

        // Check if any symbol in the file contains the include target name as part of it
        for (const String& symbol : all_symbols) {
            if (symbol.Find(GetFileName(target)) >= 0) {
                is_used = true;
                break;
            }
            // Also check without extension
            String base_name = GetFileName(target);
            int dot_pos = base_name.ReverseFind('.');
            if (dot_pos > 0) {
                base_name = base_name.Mid(0, dot_pos);
                if (symbol.Find(base_name) >= 0) {
                    is_used = true;
                    break;
                }
            }
        }

        // Also check if the include target name appears in the raw content
        if (!is_used) {
            String target_name = GetFileName(target);
            int dot_pos = target_name.ReverseFind('.');
            if (dot_pos > 0) {
                target_name = target_name.Mid(0, dot_pos);  // Remove extension
            }

            if (content.Find(target_name) < 0) {
                // Check for common patterns where includes might be used
                is_used = false;
                // Check for specific include target in the content
                if (content.Find(GetFileName(target)) >= 0) {
                    is_used = true;
                }
            } else {
                is_used = true;  // Found in content
            }
        }

        if (!is_used) {
            dead_include_indices.Add(include_line_indices[i]);
        }
    }

    int count = dead_include_indices.GetCount();
    if (out_count) *out_count = count; // Set the count

    // Apply edits to remove dead includes (backwards to maintain line numbers)
    Vector<Tuple<int, int, String>> edits;
    for(int i = dead_include_indices.GetCount() - 1; i >= 0; i--) {
        int line_idx = dead_include_indices[i];
        int start_pos = 0;
        for(int j = 0; j < line_idx; j++) {
            start_pos += lines[j].GetCount() + 1;  // +1 for newline
        }

        // Include the newline character in the removal
        int end_pos = start_pos + lines[line_idx].GetCount();
        if (line_idx < lines.GetCount() - 1) {  // Not the last line, include the newline
            end_pos++;  // Include the newline character
        }

        // Create edit to remove the entire line
        edits.Add(MakeTuple(start_pos, end_pos - start_pos, ""));
    }

    return ApplyEdits(path, edits, ide, error);
}

bool CoreRefactor::CanonicalizeIncludes(const String& path,
                                       CoreIde& ide,
                                       String& error,
                                       int* out_count) {
    if (out_count) *out_count = 0; // Initialize count

    // Get the file content from the editor
    CoreEditor* editor = ide.GetEditor(path);
    if (!editor) {
        error = "Could not open file: " + path;
        return false;
    }

    String content = editor->GetText();
    Vector<String> lines = Split(content, "\n");

    Vector<Tuple<int, int, String>> edits;

    for(int i = 0; i < lines.GetCount(); i++) {
        String line = TrimLeft(lines[i]);
        if (StartsWith(line, "#include")) {
            String original_line = lines[i];

            // Extract include path
            String include_path;
            int quote_start = -1;
            char quote_type = 0;

            for(int j = 0; j < line.GetLength(); j++) {
                if (line[j] == '"' || line[j] == '<') {
                    quote_start = j;
                    quote_type = line[j];
                    break;
                }
            }

            if (quote_start >= 0) {
                int quote_end = -1;
                if (quote_type == '"') {
                    quote_end = line.Find('"', quote_start + 1);
                } else if (quote_type == '<') {
                    quote_end = line.Find('>', quote_start + 1);
                }

                if (quote_end > quote_start) {
                    include_path = line.Mid(quote_start + 1, quote_end - quote_start - 1);

                    // Canonicalize the path by resolving relative parts
                    String canonical_path = include_path;
                    if (include_path.Find("../") >= 0 || include_path.Find("./") >= 0) {
                        // Attempt to resolve relative paths
                        String dir = GetFileDirectory(path);
                        String resolved_path = AppendFileName(dir, include_path);

                        // Get the absolute path and make it canonical
                        String abs_path = GetFullPath(resolved_path);

                        // Now get the relative path back from the file's directory
                        String file_dir = GetFileDirectory(path);
                        canonical_path = GetRelativePath(abs_path, file_dir);
                    }

                    // Also canonicalize by replacing "./" and redundant "../"
                    if (canonical_path != include_path) {
                        // Create a replacement include line
                        String new_line = line.Mid(0, quote_start) + quote_type + canonical_path +
                                         quote_type + line.Mid(quote_end + 1, line.GetLength() - quote_end - 1);

                        // Find the position in the original content
                        int start_pos = 0;
                        for(int j = 0; j < i; j++) {
                            start_pos += lines[j].GetCount() + 1;  // +1 for newline
                        }

                        edits.Add(MakeTuple(start_pos, original_line.GetLength(), new_line));
                    }
                }
            }
        }
    }

    int count = edits.GetCount(); // Count of canonicalized includes
    if (out_count) *out_count = count; // Set the count

    if (edits.IsEmpty()) {
        return true;  // Nothing to canonicalize
    }

    return ApplyEdits(path, edits, ide, error);
}

bool CoreRefactor::CollectSymbolEdits(const String& old_name,
                                     const String& new_name,
                                     CoreIde& ide,
                                     Vector<String>& out_files,
                                     String& error) {
    // Query CoreAssist for all usages of the symbol
    CoreAssist& assist = ide.GetAssist();
    Vector<SymbolUsage> usages = assist.FindSymbolUsages(old_name);

    if (usages.IsEmpty()) {
        return true;  // No usages found, nothing to rename
    }

    // Collect all unique file paths where the symbol is used
    ArrayMap<String, bool> unique_files;
    for (const auto& usage : usages) {
        if (!unique_files.Contains(usage.file_path)) {
            unique_files.Add(usage.file_path, true);
        }
    }

    for (int i = 0; i < unique_files.GetCount(); i++) {
        out_files.Add(unique_files.GetKey(i));
    }

    return true;
}

bool CoreRefactor::ApplyEdits(const String& path,
                             const Vector<Tuple<int, int, String>>& edits,
                             CoreIde& ide,
                             String& error) {
    if (edits.IsEmpty()) {
        return true;  // Nothing to apply
    }
    
    CoreEditor* editor = ide.GetEditor(path);
    if (!editor) {
        error = "Could not open editor for file: " + path;
        return false;
    }
    
    // Sort edits in descending order by position to avoid offset shifting
    Vector<Tuple<int, int, String>> sorted_edits = edits;
    Sort(sorted_edits, [](const Tuple<int, int, String>& a, const Tuple<int, int, String>& b) {
        return Get0(a) > Get0(b);  // Sort by position descending
    });
    
    // Verify no overlapping edits
    for (int i = 0; i < sorted_edits.GetCount() - 1; i++) {
        int current_end = Get0(sorted_edits[i]);
        int next_start = Get0(sorted_edits[i+1]) + Get1(sorted_edits[i+1]);
        if (current_end < next_start) {
            error = "Overlapping edits detected";
            return false;
        }
    }
    
    // Apply edits in reverse order to maintain position accuracy
    for (const auto& edit : sorted_edits) {
        int pos = Get0(edit);
        int length = Get1(edit);
        String replacement = Get2(edit);
        
        editor->Replace(pos, pos + length, replacement);
    }
    
    return true;
}