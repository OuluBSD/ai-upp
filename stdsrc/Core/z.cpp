#include "Core.h"
#include <string>
#include <vector>
#include <zlib.h>  // Need to link with zlib library

NAMESPACE_UPP

// Compression utilities using zlib
std::string CompressZLib(const std::string& data) {
    if (data.empty()) return "";
    
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    
    if (deflateInit(&zs, Z_DEFAULT_COMPRESSION) != Z_OK) {
        return "";
    }
    
    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.data()));
    zs.avail_in = static_cast<uInt>(data.size());
    
    int ret;
    char outbuffer[32768];
    std::string compressed;
    
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);
        
        ret = deflate(&zs, Z_SYNC_FLUSH);
        
        if (compressed.size() < zs.total_out) {
            compressed.append(outbuffer, zs.total_out - compressed.size());
        }
    } while (ret == Z_OK);
    
    deflateEnd(&zs);
    
    if (ret != Z_STREAM_END) {
        return "";  // Error
    }
    
    return compressed;
}

std::string DecompressZLib(const std::string& compressed) {
    if (compressed.empty()) return "";
    
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    
    if (inflateInit(&zs) != Z_OK) {
        return "";
    }
    
    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(compressed.data()));
    zs.avail_in = static_cast<uInt>(compressed.size());
    
    int ret;
    char outbuffer[32768];
    std::string decompressed;
    
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);
        
        ret = inflate(&zs, 0);
        
        if (decompressed.size() < zs.total_out) {
            decompressed.append(outbuffer, zs.total_out - decompressed.size());
        }
    } while (ret == Z_OK);
    
    inflateEnd(&zs);
    
    if (ret != Z_STREAM_END) {
        return "";  // Error
    }
    
    return decompressed;
}

// ZIP file utilities (basic implementation)
class ZipFile {
private:
    std::string data;
    
public:
    ZipFile() = default;
    
    bool AddFile(const std::string& name, const std::string& content) {
        // This is a simplified implementation that just stores the content
        // with a simple format for this example
        // Real implementation would create proper ZIP format
        
        // In a real implementation, we would use minizip or similar library
        data += name + "\n" + std::to_string(content.size()) + "\n" + content + "\n";
        return true;
    }
    
    bool ExtractFile(const std::string& name, std::string& content) const {
        // Simplified extraction
        // Real implementation would parse proper ZIP format
        size_t pos = data.find(name + "\n");
        if (pos == std::string::npos) return false;
        
        pos += name.length() + 1; // Skip name and newline
        size_t size_pos = pos;
        size_t newline_pos = data.find('\n', size_pos);
        if (newline_pos == std::string::npos) return false;
        
        std::string size_str = data.substr(size_pos, newline_pos - size_pos);
        size_t file_size = static_cast<size_t>(std::stoll(size_str));
        
        pos = newline_pos + 1; // Skip size and newline
        if (pos + file_size > data.length()) return false;
        
        content = data.substr(pos, file_size);
        return true;
    }
    
    std::string GetContent() const {
        return data;
    }
    
    bool SaveToFile(const std::string& filename) const {
        // Write data to file
        FILE* file = fopen(filename.c_str(), "wb");
        if (!file) return false;
        
        size_t written = fwrite(data.data(), 1, data.size(), file);
        fclose(file);
        
        return written == data.size();
    }
    
    bool LoadFromFile(const std::string& filename) {
        // Read data from file
        FILE* file = fopen(filename.c_str(), "rb");
        if (!file) return false;
        
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        if (file_size < 0) {
            fclose(file);
            return false;
        }
        
        data.resize(file_size);
        size_t read = fread(&data[0], 1, file_size, file);
        fclose(file);
        
        return read == static_cast<size_t>(file_size);
    }
};

END_UPP_NAMESPACE