#include "clicore.h"

using namespace Upp;

CoreFileOps::CoreFileOps() {
    // Initialize file operations
}

CoreFileOps::~CoreFileOps() {
    // Cleanup
}

bool CoreFileOps::OpenFile(const String& path, String& content, String& error) {
    if (!FileExists(path)) {
        error = "File does not exist: " + path;
        return false;
    }
    
    FileIn in(path);
    if (!in) {
        error = "Cannot open file: " + path;
        return false;
    }
    
    content = LoadFile(path);
    current_file_path = path;
    current_file_content = content;
    current_file_time = GetFileTime(path);
    
    return true;
}

bool CoreFileOps::SaveFile(const String& path, const String& content, String& error) {
    FileOut out(path);
    if (!out) {
        error = "Cannot open file for writing: " + path;
        return false;
    }

    out.Put(content);
    if (out.IsError()) {
        error = "Error writing to file: " + path;
        return false;
    }

    // Update internal state
    current_file_path = path;
    current_file_content = content;
    current_file_time = GetFileTime(path);

    return true;
}

bool CoreFileOps::FileExists(const String& path) const {
    return Upp::FileExists(path);
}

String CoreFileOps::GetFileContent(const String& path, String& error) const {
    if (!Upp::FileExists(path)) {
        error = "File does not exist: " + path;
        return String();
    }

    String content = LoadFile(path);
    return content;
}

bool CoreFileOps::SetFileContent(const String& path, const String& content, String& error) const {
    FileOut out(path);
    if (!out) {
        error = "Cannot open file for writing: " + path;
        return false;
    }

    out.Put(content);
    if (out.IsError()) {
        error = "Error writing to file: " + path;
        return false;
    }
    return true;
}