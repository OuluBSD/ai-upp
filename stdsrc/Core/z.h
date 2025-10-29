#pragma once
#ifndef _Core_z_h_
#define _Core_z_h_

#include <string>
#include "Core.h"

NAMESPACE_UPP

// Compression utilities
std::string CompressZLib(const std::string& data);
std::string DecompressZLib(const std::string& compressed);

// ZIP file utilities
class ZipFile {
private:
    std::string data;
    
public:
    ZipFile();
    
    bool AddFile(const std::string& name, const std::string& content);
    bool ExtractFile(const std::string& name, std::string& content) const;
    std::string GetContent() const;
    
    bool SaveToFile(const std::string& filename) const;
    bool LoadFromFile(const std::string& filename);
};

END_UPP_NAMESPACE

#endif