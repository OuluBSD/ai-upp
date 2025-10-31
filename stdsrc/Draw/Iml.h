#pragma once
#ifndef _Draw_Iml_h_
#define _Draw_Iml_h_

#include "Draw.h"
#include "Image.h"
#include <map>
#include <string>
#include <vector>
#include <memory>

// Iml - Image markup language implementation for stdsrc

class Iml {
private:
    // Image data storage
    struct ImageData {
        Image img;
        std::string name;
        std::string path;
        bool loaded = false;
        
        ImageData() = default;
        ImageData(const std::string& name, const std::string& path) 
            : name(name), path(path), loaded(false) {}
    };
    
    // Storage for images
    std::map<std::string, std::unique_ptr<ImageData>> images;
    std::vector<std::string> image_order;
    
    // Cache for loaded images
    mutable std::map<std::string, Image> image_cache;
    
    // Mutex for thread safety
    mutable std::mutex cache_mutex;
    
    // Load image from file
    bool LoadImageFromFile(ImageData& data) const;
    
    // Load image from embedded data
    bool LoadImageFromData(ImageData& data) const;
    
    // Get image data by name
    ImageData* GetImageData(const std::string& name);
    const ImageData* GetImageData(const std::string& name) const;
    
public:
    // Constructors
    Iml();
    Iml(const Iml& other);
    Iml(Iml&& other) noexcept;
    Iml& operator=(const Iml& other);
    Iml& operator=(Iml&& other) noexcept;
    ~Iml();
    
    // Image management
    void Add(const std::string& name, const std::string& path);
    void Add(const std::string& name, const Image& img);
    void Add(const std::string& name, Image&& img);
    void Remove(const std::string& name);
    void Clear();
    
    // Image retrieval
    const Image& Get(const std::string& name) const;
    Image& Get(const std::string& name);
    bool Has(const std::string& name) const;
    
    // Batch operations
    void LoadAll();
    void UnloadAll();
    void ReloadAll();
    
    // Information
    int GetCount() const;
    bool IsEmpty() const;
    std::vector<std::string> GetImageNames() const;
    
    // Caching
    void EnableCaching(bool enable = true);
    void DisableCaching();
    bool IsCachingEnabled() const;
    void ClearCache();
    
    // Serialization
    template<typename Stream>
    void Serialize(Stream& s) {
        int version = 1;
        s / version;
        
        if (s.IsStoring()) {
            int count = GetCount();
            s / count;
            for (const auto& name : image_order) {
                s % const_cast<std::string&>(name);
                const ImageData* data = GetImageData(name);
                if (data) {
                    s % const_cast<Image&>(data->img);
                    s % const_cast<std::string&>(data->path);
                }
            }
        } else {
            Clear();
            int count;
            s / count;
            for (int i = 0; i < count; ++i) {
                std::string name, path;
                Image img;
                s % name % img % path;
                Add(name, img);
                ImageData* data = GetImageData(name);
                if (data) {
                    data->path = path;
                }
            }
        }
    }
    
    // Streaming operator
    template<typename Stream>
    friend void operator%(Stream& s, Iml& iml) {
        iml.Serialize(s);
    }
    
    // String representation
    std::string ToString() const;
    
    // Operators
    const Image& operator[](const std::string& name) const { return Get(name); }
    Image& operator[](const std::string& name)             { return Get(name); }
    const Image& operator[](const char* name) const        { return Get(name); }
    Image& operator[](const char* name)                    { return Get(name); }
    
    // Utility functions
    static Iml& Global();
    static void Register(const std::string& name, const Iml& iml);
    static Iml& GetRegistered(const std::string& name);
    static bool HasRegistered(const std::string& name);
    static void Unregister(const std::string& name);
    static std::vector<std::string> GetRegisteredNames();
    
    // Image creation utilities
    static Image CreateSolid(int width, int height, Color color);
    static Image CreateGradient(int width, int height, Color start, Color end, bool vertical = true);
    static Image CreateCheckerboard(int width, int height, Color color1 = White, Color color2 = LtGray, int square_size = 8);
    static Image CreateBorder(int width, int height, int border_width, Color color);
    static Image CreateRoundedRect(int width, int height, int radius, Color fill_color, Color border_color = Null, int border_width = 1);
    static Image CreateCircle(int diameter, Color fill_color, Color border_color = Null, int border_width = 1);
    static Image CreateArrow(int width, int height, Color color, int direction = 0); // 0=right, 1=left, 2=up, 3=down
    
    // Image manipulation utilities
    static Image Scale(const Image& img, int width, int height);
    static Image Rotate(const Image& img, double angle); // angle in degrees
    static Image FlipHorizontal(const Image& img);
    static Image FlipVertical(const Image& img);
    static Image Crop(const Image& img, const Rect& rect);
    static Image Overlay(const Image& background, const Image& overlay, Point pos);
    static Image Blend(const Image& img1, const Image& img2, double alpha = 0.5);
    static Image Tint(const Image& img, Color tint_color, double amount = 0.5);
    static Image Grayscale(const Image& img);
    static Image Invert(const Image& img);
    static Image AdjustBrightness(const Image& img, double factor);
    static Image AdjustContrast(const Image& img, double factor);
    static Image AdjustSaturation(const Image& img, double factor);
    
    // Image conversion utilities
    static std::string ToBase64(const Image& img);
    static Image FromBase64(const std::string& base64);
    static std::vector<uint8_t> ToBytes(const Image& img);
    static Image FromBytes(const std::vector<uint8_t>& bytes);
    
    // File I/O utilities
    static bool SaveToFile(const Image& img, const std::string& path);
    static Image LoadFromFile(const std::string& path);
    static bool SaveToFile(const Iml& iml, const std::string& path);
    static Iml LoadFromFile(const std::string& path);
    
    // Format detection utilities
    static bool IsPng(const std::vector<uint8_t>& data);
    static bool IsJpeg(const std::vector<uint8_t>& data);
    static bool IsGif(const std::vector<uint8_t>& data);
    static bool IsBmp(const std::vector<uint8_t>& data);
    static bool IsTga(const std::vector<uint8_t>& data);
    static bool IsPpm(const std::vector<uint8_t>& data);
    static bool IsPgm(const std::vector<uint8_t>& data);
    static bool IsPbm(const std::vector<uint8_t>& data);
    
    // Format conversion utilities
    static std::vector<uint8_t> ToPng(const Image& img);
    static std::vector<uint8_t> ToJpeg(const Image& img, int quality = 90);
    static std::vector<uint8_t> ToBmp(const Image& img);
    static Image FromPng(const std::vector<uint8_t>& data);
    static Image FromJpeg(const std::vector<uint8_t>& data);
    static Image FromGif(const std::vector<uint8_t>& data);
    static Image FromBmp(const std::vector<uint8_t>& data);
    static Image FromTga(const std::vector<uint8_t>& data);
    static Image FromPpm(const std::vector<uint8_t>& data);
    static Image FromPgm(const std::vector<uint8_t>& data);
    static Image FromPbm(const std::vector<uint8_t>& data);
};

// Global IML functions
inline const Image& GetImlImage(const std::string& name) {
    return Iml::Global()[name];
}

inline Image& GetImlImageRef(const std::string& name) {
    return Iml::Global()[name];
}

inline bool HasImlImage(const std::string& name) {
    return Iml::Global().Has(name);
}

inline void AddImlImage(const std::string& name, const Image& img) {
    Iml::Global().Add(name, img);
}

inline void RemoveImlImage(const std::string& name) {
    Iml::Global().Remove(name);
}

inline void ClearIml() {
    Iml::Global().Clear();
}

inline int GetImlCount() {
    return Iml::Global().GetCount();
}

inline bool IsImlEmpty() {
    return Iml::Global().IsEmpty();
}

inline std::vector<std::string> GetImlImageNames() {
    return Iml::Global().GetImageNames();
}

// Streaming operators
template<typename Stream>
void operator%(Stream& s, Iml& iml) {
    iml.Serialize(s);
}

// String conversion
inline std::string AsString(const Iml& iml) {
    return iml.ToString();
}

// Global IML instance
inline Iml& GlobalIml() {
    return Iml::Global();
}

// IML registration functions
inline void RegisterIml(const std::string& name, const Iml& iml) {
    Iml::Register(name, iml);
}

inline Iml& GetRegisteredIml(const std::string& name) {
    return Iml::GetRegistered(name);
}

inline bool HasRegisteredIml(const std::string& name) {
    return Iml::HasRegistered(name);
}

inline void UnregisterIml(const std::string& name) {
    Iml::Unregister(name);
}

inline std::vector<std::string> GetRegisteredImlNames() {
    return Iml::GetRegisteredNames();
}

#endif