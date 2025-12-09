#include "clicore.h"
#include "CoreAssist.h"
#include "CoreFileOps.h"

using namespace Upp;

CoreAssist::CoreAssist() {
}

bool CoreAssist::IndexWorkspace(const CoreWorkspace& ws, String& error) {
    CoreFileOps fileOps;
    const VectorMap<String, CoreWorkspace::Package>& packages = ws.GetPackages();

    for (int i = 0; i < packages.GetCount(); i++) {
        const CoreWorkspace::Package& pkg = packages[i];
        for (int j = 0; j < pkg.files.GetCount(); j++) {
            String filePath = ws.GetSourcePath(pkg.name, pkg.files[j]);
            if (!IndexFile(filePath, error)) {
                return false;
            }
        }
    }

    return true;
}

bool CoreAssist::IndexFile(const String& path, String& error) {
    CoreFileOps fileOps;
    String content;

    if (!fileOps.OpenFile(path, content, error)) {
        return false;
    }

    Vector<String> symbols;
    Vector<String> includes;

    if (!ExtractSymbols(content, symbols, includes)) {
        error = "Failed to extract symbols from " + path;
        return false;
    }

    // Parse content line by line to get accurate line numbers
    Vector<String> lines = Split(content, "\n");

    for (int i = 0; i < symbols.GetCount(); i++) {
        const String& symbol = symbols[i];
        int line = -1;

        // Search for the symbol's line number
        for (int j = 0; j < lines.GetCount(); j++) {
            if (lines[j].Find(symbol) >= 0) {
                // Check if this is a real occurrence of the symbol (not just part of a longer identifier)
                int pos = lines[j].Find(symbol);
                if (pos >= 0) {
                    // Check if it's a complete word - not part of a longer identifier
                    bool isCompleteWord = true;
                    if (pos > 0 && IsAlNum(lines[j][pos-1])) {
                        isCompleteWord = false;
                    } else if (pos + symbol.GetLength() < lines[j].GetCount() &&
                               IsAlNum(lines[j][pos + symbol.GetLength()])) {
                        isCompleteWord = false;
                    }

                    if (isCompleteWord) {
                        line = j + 1; // 1-based line numbers
                        break;
                    }
                }
            }
        }

        if (line > 0) {
            SymbolLocation loc;
            loc.file = path;
            loc.line = line;

            // If it's a definition (first occurrence), add to definitions
            if (!definitions.Find(symbol)) {
                definitions.GetAdd(symbol) = loc;
            }

            // Add to usages regardless
            usages.GetAdd(symbol).Add(loc);
        }
    }

    // Handle includes as well
    for (int i = 0; i < includes.GetCount(); i++) {
        const String& include = includes[i];
        int line = -1;

        for (int j = 0; j < lines.GetCount(); j++) {
            if (lines[j].Find(include) >= 0 && lines[j].Find("#include") >= 0) {
                line = j + 1; // 1-based line numbers
                break;
            }
        }

        if (line > 0) {
            SymbolLocation loc;
            loc.file = path;
            loc.line = line;

            // Add include to usages
            usages.GetAdd(include).Add(loc);
        }
    }

    return true;
}

bool CoreAssist::FindDefinition(const String& symbol, String& out_file, int& out_line) const {
    const SymbolLocation* loc = definitions.FindPtr(symbol);
    if (loc) {
        out_file = loc->file;
        out_line = loc->line;
        return true;
    }
    return false;
}

bool CoreAssist::FindUsages(const String& symbol, Vector<String>& out_locations) const {
    const Vector<SymbolLocation>* locations = usages.FindPtr(symbol);
    if (locations) {
        for (const auto& loc : *locations) {
            out_locations.Add(loc.file + ":" + IntStr(loc.line));
        }
        return true;
    }
    return false;
}

bool CoreAssist::ExtractSymbols(const String& content, Vector<String>& out_symbols, Vector<String>& out_includes) const {
    Vector<String> lines = Split(content, "\n");

    for (const auto& line : lines) {
        String trimmedLine = TrimLeft(line);
        if (trimmedLine.IsEmpty()) continue;

        // Skip comments
        if (trimmedLine.StartsWith("//") || trimmedLine.StartsWith("/*") || trimmedLine.StartsWith("*")) {
            continue;
        }

        // Include pattern: #include <...> or #include "..."
        int includePos = line.Find("#include");
        if (includePos >= 0) {
            // Look for <...> or "..." after #include
            int start = line.Find('<', includePos);
            if (start < 0) start = line.Find('"', includePos);
            if (start > 0) {
                int end = line.Find('>', start);
                if (end < 0) end = line.Find('"', start + 1);
                if (end > start) {
                    String includeFile = line.Mid(start + 1, end - start - 1);
                    if (includeFile.GetLength() > 0) {
                        out_includes.Add(includeFile);
                    }
                }
            }
        }

        // Look for function definitions (heuristic: identifier followed by parentheses)
        // This is a simplified approach without regex
        int parenPos = line.Find('(');
        if (parenPos > 0) {
            // Find the beginning of the identifier before the parenthesis
            int start = parenPos - 1;
            while (start >= 0 && (IsAlNum(line[start]) || line[start] == '_')) {
                start--;
            }
            start++;

            if (start < parenPos) {
                String funcName = line.Mid(start, parenPos - start);
                if (funcName.GetLength() > 0 && !IsDigit(funcName[0])) {
                    // Additional check to ensure it's not a type name
                    if (funcName != "int" && funcName != "void" && funcName != "char" &&
                        funcName != "double" && funcName != "float" && funcName != "bool" &&
                        funcName != "const" && funcName != "unsigned" && funcName != "signed" &&
                        funcName != "long" && funcName != "short" && funcName != "static" &&
                        funcName != "virtual" && funcName != "inline") {
                        out_symbols.Add(funcName);
                    }
                }
            }
        }

        // Look for class definitions
        int classPos = line.Find("class ");
        if (classPos < 0) classPos = line.Find("class\t");
        if (classPos >= 0) {
            int start = classPos + 6; // length of "class "
            while (start < line.GetLength() && (line[start] == ' ' || line[start] == '\t')) {
                start++;
            }
            if (start < line.GetLength()) {
                int end = start;
                while (end < line.GetLength() && (IsAlNum(line[end]) || line[end] == '_')) {
                    end++;
                }
                if (end > start) {
                    String className = line.Mid(start, end - start);
                    out_symbols.Add(className);
                }
            }
        }

        // Look for struct definitions
        int structPos = line.Find("struct ");
        if (structPos < 0) structPos = line.Find("struct\t");
        if (structPos >= 0) {
            int start = structPos + 7; // length of "struct "
            while (start < line.GetLength() && (line[start] == ' ' || line[start] == '\t')) {
                start++;
            }
            if (start < line.GetLength()) {
                int end = start;
                while (end < line.GetLength() && (IsAlNum(line[end]) || line[end] == '_')) {
                    end++;
                }
                if (end > start) {
                    String structName = line.Mid(start, end - start);
                    out_symbols.Add(structName);
                }
            }
        }

        // Look for enum definitions
        int enumPos = line.Find("enum ");
        if (enumPos < 0) enumPos = line.Find("enum\t");
        if (enumPos >= 0) {
            int start = enumPos + 5; // length of "enum "
            while (start < line.GetLength() && (line[start] == ' ' || line[start] == '\t')) {
                start++;
            }
            if (start < line.GetLength()) {
                int end = start;
                while (end < line.GetLength() && (IsAlNum(line[end]) || line[end] == '_')) {
                    end++;
                }
                if (end > start) {
                    String enumName = line.Mid(start, end - start);
                    out_symbols.Add(enumName);
                }
            }
        }

        // Look for global variable definitions (very heuristic)
        // Look for lines with semicolons and check for variable patterns
        int semicolonPos = line.Find(';');
        if (semicolonPos >= 0) {
            // Simple heuristic: look for identifier followed by semicolon
            // This is very basic and will produce false positives
            int start = 0;
            // Skip spaces at the beginning
            while (start < semicolonPos && (line[start] == ' ' || line[start] == '\t')) {
                start++;
            }
            // Look for potential type
            while (start < semicolonPos && (IsAlNum(line[start]) || line[start] == '_')) {
                start++;
            }
            // Skip spaces
            while (start < semicolonPos && (line[start] == ' ' || line[start] == '\t')) {
                start++;
            }
            // Now look for variable name
            int varStart = start;
            while (start < semicolonPos && (IsAlNum(line[start]) || line[start] == '_')) {
                start++;
            }
            if (start > varStart) {
                String varName = line.Mid(varStart, start - varStart);
                // Additional checks to filter out some obvious non-variables
                if (varName != "return" && varName != "if" && varName != "for" &&
                    varName != "while" && varName != "else" && varName != "switch" &&
                    varName != "case" && varName != "break" && varName != "continue" &&
                    varName != "goto" && varName != "sizeof" && varName != "new" &&
                    varName != "delete" && varName.GetLength() > 0) {
                    out_symbols.Add(varName);
                }
            }
        }
    }

    return true;
}