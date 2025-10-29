#include "Core.h"
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>
#endif
#include <cstdlib>
#include <string>
#include <vector>

NAMESPACE_UPP

// Cross-platform utility functions

std::string GetExeFilePath() {
#ifdef _WIN32
    wchar_t wide_path[MAX_PATH];
    GetModuleFileNameW(NULL, wide_path, MAX_PATH);
    
    // Convert to UTF-8
    int narrow_len = WideCharToMultiByte(CP_UTF8, 0, wide_path, -1, NULL, 0, NULL, NULL);
    std::vector<char> narrow_path(narrow_len);
    WideCharToMultiByte(CP_UTF8, 0, wide_path, -1, narrow_path.data(), narrow_len, NULL, NULL);
    
    return std::string(narrow_path.data());
#else
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path)-1);
    if (len != -1) {
        path[len] = '\0';
        return std::string(path);
    }
    return "";
#endif
}

std::string GetExeFileDirectory() {
    std::string exe_path = GetExeFilePath();
    size_t pos = exe_path.find_last_of("/\\");
    if (pos != std::string::npos) {
        return exe_path.substr(0, pos);
    }
    return exe_path;
}

std::string GetHomeDirectory() {
#ifdef _WIN32
    // On Windows, use SHGetFolderPath for the user's profile directory
    wchar_t wide_path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, wide_path))) {
        // Convert to UTF-8
        int narrow_len = WideCharToMultiByte(CP_UTF8, 0, wide_path, -1, NULL, 0, NULL, NULL);
        std::vector<char> narrow_path(narrow_len);
        WideCharToMultiByte(CP_UTF8, 0, wide_path, -1, narrow_path.data(), narrow_len, NULL, NULL);
        return std::string(narrow_path.data());
    }
    return "";
#else
    const char* home = std::getenv("HOME");
    if (home) {
        return std::string(home);
    }
    
    // Fallback to password database
    struct passwd* pw = getpwuid(getuid());
    if (pw) {
        return std::string(pw->pw_dir);
    }
    
    return "";
#endif
}

std::string GetTempDirectory() {
#ifdef _WIN32
    wchar_t wide_path[MAX_PATH];
    DWORD len = GetTempPathW(MAX_PATH, wide_path);
    if (len > 0 && len < MAX_PATH) {
        // Convert to UTF-8
        int narrow_len = WideCharToMultiByte(CP_UTF8, 0, wide_path, -1, NULL, 0, NULL, NULL);
        std::vector<char> narrow_path(narrow_len);
        WideCharToMultiByte(CP_UTF8, 0, wide_path, -1, narrow_path.data(), narrow_len, NULL, NULL);
        return std::string(narrow_path.data());
    }
    return "";
#else
    const char* tmpdir = std::getenv("TMPDIR");
    if (tmpdir) {
        return std::string(tmpdir);
    }
    return "/tmp";
#endif
}

bool FileExists(const std::string& path) {
#ifdef _WIN32
    return _access(path.c_str(), 0) == 0;
#else
    return access(path.c_str(), F_OK) == 0;
#endif
}

bool IsDirectory(const std::string& path) {
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(path.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat path_stat;
    if (stat(path.c_str(), &path_stat) != 0) {
        return false;  // Path doesn't exist
    }
    return S_ISDIR(path_stat.st_mode);
#endif
}

std::string GetCommandLine() {
#ifdef _WIN32
    LPWSTR wide_cmd = GetCommandLineW();
    int narrow_len = WideCharToMultiByte(CP_UTF8, 0, wide_cmd, -1, NULL, 0, NULL, NULL);
    std::vector<char> narrow_cmd(narrow_len);
    WideCharToMultiByte(CP_UTF8, 0, wide_cmd, -1, narrow_cmd.data(), narrow_len, NULL, NULL);
    return std::string(narrow_cmd.data());
#else
    // On Unix-like systems, we'd typically reconstruct this from argv
    // For now, return an empty string
    return "";
#endif
}

void OpenUrl(const std::string& url) {
#ifdef _WIN32
    ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif __APPLE__
    std::string command = "open \"" + url + "\"";
    system(command.c_str());
#else
    std::string command = "xdg-open \"" + url + "\"";
    system(command.c_str());
#endif
}

int GetProcessId() {
#ifdef _WIN32
    return static_cast<int>(GetCurrentProcessId());
#else
    return static_cast<int>(getpid());
#endif
}

END_UPP_NAMESPACE