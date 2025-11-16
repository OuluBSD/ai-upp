#include "VfsWrapper.h"
#include <Core/Core.h>
#include <filesystem>
#include <cstdlib>

namespace {

std::string GetHomeDirectory() {
    if (const char* home = std::getenv("HOME")) {
        return std::string(home);
    }
    std::error_code ec;
    return std::filesystem::current_path(ec).string();
}

std::string JoinPaths(const std::string& base, const std::string& relative) {
    if (base.empty()) {
        return std::filesystem::path(relative).lexically_normal().string();
    }
    if (relative.empty()) {
        return std::filesystem::path(base).lexically_normal().string();
    }
    std::filesystem::path path = std::filesystem::path(base) / relative;
    return path.lexically_normal().string();
}

const std::string& CacheRoot() {
    static const std::string root = JoinPaths(GetHomeDirectory(), ".cache/ai-upp/qwen");
    return root;
}

const std::string& ConfigRoot() {
    static const std::string root = JoinPaths(GetHomeDirectory(), ".config/ai-upp");
    return root;
}

std::string MapVirtualRoot(const std::string& path, const std::string& root, size_t prefix_len) {
    std::string suffix = path.substr(prefix_len);
    if (!suffix.empty() && suffix[0] == '/') {
        suffix.erase(0, 1);
    }
    return suffix.empty() ? root : JoinPaths(root, suffix);
}

} // namespace

namespace Qwen {

bool VfsWrapper::exists(const std::string& path) const {
    std::error_code ec;
    return std::filesystem::exists(resolve(path), ec);
}

bool VfsWrapper::is_file(const std::string& path) const {
    std::error_code ec;
    return std::filesystem::is_regular_file(resolve(path), ec);
}

bool VfsWrapper::is_directory(const std::string& path) const {
    std::error_code ec;
    return std::filesystem::is_directory(resolve(path), ec);
}

bool VfsWrapper::mkdir(const std::string& path, int /*permissions*/) {
    std::error_code ec;
    const auto real_path = resolve(path);
    if (real_path.empty()) {
        return false;
    }
    if (std::filesystem::exists(real_path, ec)) {
        return std::filesystem::is_directory(real_path, ec);
    }
    std::filesystem::create_directories(real_path, ec);
    return !ec;
}

bool VfsWrapper::rm(const std::string& path, int /*flags*/) {
    std::error_code ec;
    const auto real_path = resolve(path);
    if (real_path.empty()) {
        return false;
    }
    if (!std::filesystem::exists(real_path, ec)) {
        return true;
    }
    std::filesystem::remove_all(real_path, ec);
    return !ec;
}

std::optional<std::string> VfsWrapper::read(const std::string& path, const std::optional<size_t>& /*size_hint*/) const {
    auto real_path = resolve(path);
    Upp::String content = Upp::LoadFile(real_path.c_str());
    if (content.IsVoid()) {
        return std::nullopt;
    }
    return std::string(content.Begin(), content.GetLength());
}

bool VfsWrapper::write(const std::string& path, const std::string& content, int /*flags*/) {
    auto real_path = resolve(path);
    std::filesystem::path p(real_path);
    std::error_code ec;
    auto parent = p.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent, ec);
        if (ec) return false;
    }

    Upp::FileOut fo(Upp::String(real_path.c_str()));
    if (!fo.IsOpen()) {
        return false;
    }
    fo << Upp::String(content.c_str());
    return fo.IsOpen();
}

std::vector<std::string> VfsWrapper::list(const std::string& path) const {
    std::vector<std::string> result;
    std::error_code ec;
    auto real_path = resolve(path);
    if (!std::filesystem::exists(real_path, ec) || !std::filesystem::is_directory(real_path, ec)) {
        return result;
    }

    std::filesystem::directory_iterator it(real_path, ec);
    if (ec) {
        return result;
    }

    for (auto& entry : std::filesystem::directory_iterator(real_path, ec)) {
        if (ec) break;
        result.push_back(entry.path().filename().string());
    }
    return result;
}

std::string VfsWrapper::resolve(const std::string& path) const {
    if (path.rfind("/qwen", 0) == 0) {
        return MapVirtualRoot(path, CacheRoot(), 5);
    }
    if (path.rfind("/env", 0) == 0) {
        return MapVirtualRoot(path, ConfigRoot(), 4);
    }
    if (!path.empty() && path[0] == '~') {
        std::string suffix = path.substr(1);
        if (!suffix.empty() && suffix[0] == '/') {
            suffix.erase(0, 1);
        }
        return suffix.empty() ? GetHomeDirectory() : JoinPaths(GetHomeDirectory(), suffix);
    }
    return path;
}

} // namespace Qwen
