#ifndef _Core_FileMapping_h_
#define _Core_FileMapping_h_

#include "Core.h"
#include <memory>
#include <string>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

// File mapping class for memory-mapped files
class FileMapping {
private:
    void* mapped_address;
    size_t mapped_size;
    String file_path;
    
#ifdef _WIN32
    HANDLE file_handle;
    HANDLE mapping_handle;
#else
    int file_descriptor;
#endif

public:
    FileMapping();
    explicit FileMapping(const String& path);
    ~FileMapping();
    
    // Open a file for memory mapping
    bool Open(const String& path);
    
    // Close the mapping and file
    void Close();
    
    // Create a new mapping with specified size
    bool Create(const String& path, size_t size);
    
    // Check if file is mapped
    bool IsOpen() const { return mapped_address != nullptr; }
    
    // Get pointer to mapped memory
    void* GetAddress() const { return mapped_address; }
    
    // Get size of mapped memory
    size_t GetSize() const { return mapped_size; }
    
    // Get file path
    const String& GetPath() const { return file_path; }
    
    // Read data from mapped memory
    bool Read(void* buffer, size_t offset, size_t size);
    
    // Write data to mapped memory
    bool Write(const void* buffer, size_t offset, size_t size);
    
    // Access operator - get reference to data at index
    template<typename T>
    T& operator[](size_t index) {
        return *static_cast<T*>(static_cast<void*>(static_cast<char*>(mapped_address) + index));
    }
    
    // Const access operator
    template<typename T>
    const T& operator[](size_t index) const {
        return *static_cast<const T*>(static_cast<const void*>(static_cast<const char*>(mapped_address) + index));
    }
    
    // Access to memory at specific offset
    void* At(size_t offset) const {
        if (offset >= mapped_size) {
            return nullptr;
        }
        return static_cast<char*>(mapped_address) + offset;
    }
    
    // Flush changes to file
    bool Flush();
    
    // Lock a region of the mapped file
    bool Lock(size_t offset, size_t size);
    
    // Unlock a region of the mapped file
    bool Unlock(size_t offset, size_t size);
    
    // Get file size
    static size_t GetFileSize(const String& path);
};

// Memory-mapped string class
class MappedString {
private:
    std::shared_ptr<FileMapping> mapping;
    size_t length;
    
public:
    MappedString();
    explicit MappedString(const String& file_path);
    
    // Load string from file
    bool Load(const String& file_path);
    
    // Save string to file
    bool Save(const String& file_path);
    
    // Get C string pointer
    const char* c_str() const;
    
    // Get length of string
    size_t GetLength() const { return length; }
    
    // Get character at position
    char operator[](size_t index) const;
    
    // Get substring
    String SubStr(size_t pos, size_t len) const;
    
    // Find substring
    size_t Find(const String& substr, size_t pos = 0) const;
    
    // Check if loaded
    bool IsLoaded() const { return mapping && mapping->IsOpen(); }
};

// Memory-mapped vector class
template<typename T>
class MappedVector {
private:
    std::shared_ptr<FileMapping> mapping;
    size_t count;
    size_t capacity;
    
public:
    MappedVector();
    explicit MappedVector(const String& file_path);
    
    // Initialize with file and size
    bool Initialize(const String& file_path, size_t element_count);
    
    // Get element at index
    T& operator[](size_t index) {
        if (index >= count) {
            throw std::out_of_range("MappedVector index out of range");
        }
        return mapping->operator[]<T>(index * sizeof(T));
    }
    
    // Get element at index (const)
    const T& operator[](size_t index) const {
        if (index >= count) {
            throw std::out_of_range("MappedVector index out of range");
        }
        return mapping->operator[]<T>(index * sizeof(T));
    }
    
    // Get size
    size_t GetCount() const { return count; }
    
    // Get capacity
    size_t GetCapacity() const { return capacity; }
    
    // Get pointer to data
    T* GetData() {
        if (!mapping || !mapping->IsOpen()) {
            return nullptr;
        }
        return static_cast<T*>(mapping->GetAddress());
    }
    
    // Const version
    const T* GetData() const {
        if (!mapping || !mapping->IsOpen()) {
            return nullptr;
        }
        return static_cast<const T*>(mapping->GetAddress());
    }
    
    // Check if loaded
    bool IsLoaded() const { return mapping && mapping->IsOpen(); }
    
    // Flush to file
    bool Flush() {
        return mapping && mapping->Flush();
    }
};

// File mapping utilities
class FileMappingUtils {
public:
    // Create a copy of a mapped file
    static bool Copy(const String& source, const String& dest);
    
    // Check if file exists
    static bool FileExists(const String& path);
    
    // Get available space on device
    static uint64_t GetAvailableSpace(const String& path);
    
    // Expand file size
    static bool ExpandFileSize(const String& path, size_t new_size);
};

#endif