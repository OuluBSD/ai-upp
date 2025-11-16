#ifndef _Qwen_VfsWrapper_h_
#define _Qwen_VfsWrapper_h_

#include <Core/VfsBase/VfsBase.h>
#include <string>
#include <optional>
#include <vector>

namespace Qwen {

// Wrapper class to provide high-level VFS operations using U++ MountManager
class VfsWrapper {
private:
    Upp::VFS* vfs_impl;  // Not directly used, but kept for compatibility

public:
    explicit VfsWrapper(Upp::VFS* vfs) : vfs_impl(vfs) {}

    // Check if a file or directory exists
    bool exists(const std::string& path) const;
    
    // Check if path is a file
    bool is_file(const std::string& path) const;
    
    // Check if path is a directory
    bool is_directory(const std::string& path) const;
    
    // Create a directory
    bool mkdir(const std::string& path, int permissions = 0755);
    
    // Remove a file or directory
    bool rm(const std::string& path, int flags = 0);
    
    // Read file content
    std::optional<std::string> read(const std::string& path, const std::optional<size_t>& size_hint = std::nullopt) const;
    
    // Write file content
    bool write(const std::string& path, const std::string& content, int flags = 0);
    
    // List directory contents
    std::vector<std::string> list(const std::string& path) const;
    
    // Get full path resolution (if needed)
    std::string resolve(const std::string& path) const;
};

} // namespace Qwen

#endif // _Qwen_VfsWrapper_h_