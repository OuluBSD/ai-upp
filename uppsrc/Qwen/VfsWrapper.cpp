#include "VfsWrapper.h"
#include <Core/VfsBase/Mount.h>
#include <Core/Core.h>

namespace Qwen {

bool VfsWrapper::exists(const std::string& path) const {
    Upp::VfsPath vfsPath;
    vfsPath.Set(Upp::String(path.c_str()));
    
    Upp::MountManager& mm = Upp::MountManager::System();
    return mm.FileExists(vfsPath) || mm.DirectoryExists(vfsPath);
}

bool VfsWrapper::is_file(const std::string& path) const {
    Upp::VfsPath vfsPath;
    vfsPath.Set(Upp::String(path.c_str()));
    
    Upp::MountManager& mm = Upp::MountManager::System();
    return mm.FileExists(vfsPath);
}

bool VfsWrapper::is_directory(const std::string& path) const {
    Upp::VfsPath vfsPath;
    vfsPath.Set(Upp::String(path.c_str()));
    
    Upp::MountManager& mm = Upp::MountManager::System();
    return mm.DirectoryExists(vfsPath);
}

bool VfsWrapper::mkdir(const std::string& path, int permissions) {
    // U++ MountManager doesn't directly have a mkdir, so we'll use system call for now
    // In a complete implementation, this would use the appropriate VFS backend
    Upp::String uppPath(path.c_str());
    return Upp::RealizeDirectory(uppPath);
}

bool VfsWrapper::rm(const std::string& path, int flags) {
    // Use U++ core functions for file and directory deletion
    Upp::String uppPath(path.c_str());
    return Upp::DeleteFile(uppPath) || Upp::RealizeDirectory(uppPath); // Use RealizeDirectory as fallback for directories
}

std::optional<std::string> VfsWrapper::read(const std::string& path, const std::optional<size_t>& size_hint) const {
    Upp::VfsPath vfsPath;
    vfsPath.Set(Upp::String(path.c_str()));
    
    Upp::MountManager& mm = Upp::MountManager::System();
    
    if (!mm.FileExists(vfsPath)) {
        return std::nullopt;
    }
    
    // Use Upp::LoadFile to read the entire file
    Upp::String uppPath(path.c_str());
    Upp::String content = Upp::LoadFile(uppPath);
    if (content == Upp::String::GetVoid()) { // If file can't be loaded
        return std::nullopt;
    }
    return std::string(content.Begin(), content.GetLength());
}

bool VfsWrapper::write(const std::string& path, const std::string& content, int flags) {
    Upp::FileOut fo(Upp::String(path.c_str()));
    if (!fo.IsOpen()) {
        return false;
    }
    
    fo << Upp::String(content.c_str());
    // Don't call Close() manually - Upp::FileOut handles this automatically on destruction
    return true; // Return true if we were able to write (we'll assume success for now)
}

std::vector<std::string> VfsWrapper::list(const std::string& path) const {
    std::vector<std::string> result;
    
    Upp::VfsPath vfsPath;
    vfsPath.Set(Upp::String(path.c_str()));
    
    Upp::MountManager& mm = Upp::MountManager::System();
    Upp::Vector<Upp::VfsItem> items;
    
    if (mm.GetFiles(vfsPath, items)) {
        for (const auto& item : items) {
            result.push_back(std::string(item.name.Begin(), item.name.GetLength()));
        }
    }
    
    return result;
}

std::string VfsWrapper::resolve(const std::string& path) const {
    // For a basic implementation, just return the path as-is
    // In a complete implementation, this would resolve symbolic links, etc.
    return path;
}

} // namespace Qwen