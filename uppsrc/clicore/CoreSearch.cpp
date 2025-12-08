#include "clicore.h"

using namespace Upp;

CoreSearch::CoreSearch() {
    // Initialize search utilities
}

CoreSearch::~CoreSearch() {
    // Cleanup
}

void CoreSearch::SearchForFiles(Index<String>& files, const String& dir, const String& mask, 
                                int readonly, Time since) {
    FindFile ff(AppendFileName(dir, "*.*"));
    while(ff) {
        if(ff.IsFolder() && *ff.GetName() != '.') {
            // Recursively search subdirectories
            SearchForFiles(files, AppendFileName(dir, ff.GetName()), mask, readonly, since);
        } else if(ff.IsFile() && PatternMatchMulti(mask, ff.GetName())) {
            // Check readonly and time conditions
            if((IsNull(readonly) || !!readonly == !!ff.IsReadOnly()) &&
               (IsNull(since) || ff.GetLastWriteTime() >= since)) {
                files.FindAdd(AppendFileName(dir, ff.GetName()));
            }
        }
        ff.Next();
    }
}

bool CoreSearch::FindInFiles(const String& pattern, const String& directory, bool replace,
                             const String& replacement, Vector<String>& results, String& error) {
    // Find files matching the pattern in the given directory
    Index<String> files;
    SearchForFiles(files, directory, "*.cpp *.h *.hpp *.c *.m *.C *.M *.cxx *.cc *.mm *.MM *.icpp *.sch *.lay *.rc *.txt *.upx *.upp *.tpp");

    // Search each file for the pattern
    for(const String& file : files) {
        Vector<String> file_results;
        if(SearchInFile(file, pattern, file_results, error)) {
            results.Append(file_results);
        }
    }

    return true;
}

bool CoreSearch::SearchCode(const String& query, Vector<String>& results, String& error) {
    // TODO: Implement actual code search logic
    // This would use the indexer and assist features from the original IDE
    // For now, this is a simplified implementation that searches in a general directory
    error = "TODO: Implement code search using indexer";
    return false;
}

bool CoreSearch::SearchCodeInPackage(const String& query, const CoreWorkspace::Package& pkg,
                                     Vector<String>& results, String& error) {
    // Search through the files specified in the package
    for (const String& file_path : pkg.files) {
        if (FileExists(file_path)) {
            // Only search in source files that are likely to contain code
            String ext = ToLower(GetFileExt(file_path));
            if (ext == ".cpp" || ext == ".h" || ext == ".hpp" ||
                ext == ".c" || ext == ".cc" || ext == ".cxx" ||
                ext == ".tpp" || ext == ".upx") {

                Vector<String> file_results;
                if (SearchInFile(file_path, query, file_results, error)) {
                    results.Append(file_results);
                }
            }
        }
    }

    // Also search in dependent packages if needed
    // TODO: Optionally recurse through dependencies (package.uses)

    return true;
}

bool CoreSearch::SearchInFile(const String& file_path, const String& pattern,
                              Vector<String>& results, String& error) {
    FileIn in(file_path);
    if(!in) {
        error = "Could not open file: " + file_path;
        return false;
    }

    int line_num = 1;
    String line;
    while(!in.IsEof()) {
        line = in.GetLine();
        int pos = 0;
        while((pos = line.Find(pattern, pos)) >= 0) {
            // Pattern found, add to results
            String result = Format("%s:%d:%d: %s", file_path, line_num, pos + 1, line);
            results.Add(result);
            pos++; // Move to next position to find other occurrences on the same line
        }
        line_num++;
    }

    in.Close();
    return true;
}