#ifndef _clicore_CoreFileOps_h_
#define _clicore_CoreFileOps_h_

#include <Core/Core.h>

using namespace Upp;

class CoreFileOps {
public:
    CoreFileOps();
    ~CoreFileOps();

    // File operations
    bool OpenFile(const String& path, String& content, String& error);
    bool SaveFile(const String& path, const String& content, String& error);
    
    // File utilities
    bool FileExists(const String& path) const;
    String GetFileContent(const String& path, String& error) const;
    bool SetFileContent(const String& path, const String& content, String& error) const;

private:
    String current_file_path;
    String current_file_content;
    FileTime current_file_time;
};

#endif