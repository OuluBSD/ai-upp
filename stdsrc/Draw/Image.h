#pragma once
// U++-compatible Image wrapper implemented using std::vector or native image formats
// This header is aggregated and wrapped into namespace Upp by Draw.h

// Forward declarations
class Point;
class Size;
class Rect;
class Color;

class Image {
private:
    std::vector<Color> pixels;
    Size size;

public:
    // Constructors
    Image() : size(0, 0) {}
    Image(const Size& sz) : size(sz) {
        if (sz.cx > 0 && sz.cy > 0) {
            pixels.resize(sz.cx * sz.cy);
        }
    }
    Image(int width, int height) : Image(Size(width, height)) {}
    
    // Create from raw data
    Image(const Size& sz, const Color* data) : size(sz) {
        if (sz.cx > 0 && sz.cy > 0 && data != nullptr) {
            pixels.assign(data, data + sz.cx * sz.cy);
        }
    }

    // U++-style static constructors
    static Image RGBA(const Size& sz) { return Image(sz); }  // For RGBA images
    static Image RGBA(int width, int height) { return Image(width, height); }
    static Image System(const Size& sz) { return Image(sz); }  // System-specific format
    static Image System(int width, int height) { return Image(width, height); }

    // Assignment operators
    Image& operator=(const Image& other) {
        if (this != &other) {
            pixels = other.pixels;
            size = other.size;
        }
        return *this;
    }

    // U++-style access methods
    Size GetSize() const { return size; }
    int GetWidth() const { return size.cx; }
    int GetHeight() const { return size.cy; }
    
    // Get pixel with bounds checking
    Color GetPixel(int x, int y) const {
        if (x >= 0 && x < size.cx && y >= 0 && y < size.cy) {
            return pixels[y * size.cx + x];
        }
        return Color::Null();  // Return null color if out of bounds
    }
    
    Color GetPixel(const Point& pt) const {
        return GetPixel(pt.x, pt.y);
    }

    // Set pixel with bounds checking
    void SetPixel(int x, int y, const Color& color) {
        if (x >= 0 && x < size.cx && y >= 0 && y < size.cy) {
            pixels[y * size.cx + x] = color;
        }
    }
    
    void SetPixel(const Point& pt, const Color& color) {
        SetPixel(pt.x, pt.y, color);
    }

    // U++-style methods
    bool Is() const { return !pixels.empty(); }
    bool IsEmpty() const { return pixels.empty() || size.IsEmpty(); }
    void Clear() { 
        pixels.clear(); 
        size = Size(0, 0); 
    }
    
    void Create(const Size& sz) {
        size = sz;
        if (sz.cx > 0 && sz.cy > 0) {
            pixels.resize(sz.cx * sz.cy);
        } else {
            pixels.clear();
        }
    }

    // Raw data access (for compatibility with native drawing systems)
    Color* GetRawImage() { return pixels.data(); }
    const Color* GetRawImage() const { return pixels.data(); }
    size_t GetRawSize() const { return pixels.size() * sizeof(Color); }

    // U++-style image operations
    Image Paste(const Rect& dest, const Image& source, const Point& srcPos = Point(0, 0)) const {
        Image result = *this;  // Copy current image
        
        // Calculate intersection between dest rectangle and image bounds
        Rect destClip = dest.GetIntersection(Rect(0, 0, size.cx, size.cy));
        Rect srcClip(
            srcPos.x, 
            srcPos.y, 
            srcPos.x + (destClip.GetWidth()), 
            srcPos.y + (destClip.GetHeight())
        );
        
        // Clip source to its bounds
        Rect srcBounds(0, 0, source.GetWidth(), source.GetHeight());
        srcClip = srcClip.GetIntersection(srcBounds);
        
        if (!destClip.IsEmpty() && !srcClip.IsEmpty()) {
            // Adjust source position relative to clipped region
            Point srcOffset(srcClip.left - srcPos.x, srcClip.top - srcPos.y);
            
            for (int y = 0; y < srcClip.GetHeight(); y++) {
                for (int x = 0; x < srcClip.GetWidth(); x++) {
                    int srcX = srcClip.left + x - srcOffset.x;
                    int srcY = srcClip.top + y - srcOffset.y;
                    int destX = destClip.left + x;
                    int destY = destClip.top + y;
                    
                    result.SetPixel(destX, destY, source.GetPixel(srcX, srcY));
                }
            }
        }
        
        return result;
    }

    Image SubImage(const Rect& r) const {
        Rect clip = r.GetIntersection(Rect(0, 0, size.cx, size.cy));
        if (clip.IsEmpty()) {
            return Image();
        }
        
        Image sub(clip.GetSize());
        for (int y = 0; y < clip.GetHeight(); y++) {
            for (int x = 0; x < clip.GetWidth(); x++) {
                sub.SetPixel(x, y, GetPixel(clip.left + x, clip.top + y));
            }
        }
        
        return sub;
    }

    // U++-style scaling
    Image Rescale(const Size& newSz) const {
        if (newSz.IsEmpty() || size.IsEmpty()) {
            return Image(newSz);
        }
        
        Image scaled(newSz);
        
        for (int y = 0; y < newSz.cy; y++) {
            for (int x = 0; x < newSz.cx; x++) {
                // Calculate corresponding position in original image
                int origX = (x * size.cx) / newSz.cx;
                int origY = (y * size.cy) / newSz.cy;
                
                // Ensure we don't go out of bounds
                origX = std::max(0, std::min(size.cx - 1, origX));
                origY = std::max(0, std::min(size.cy - 1, origY));
                
                scaled.SetPixel(x, y, GetPixel(origX, origY));
            }
        }
        
        return scaled;
    }

    // U++-style color operations
    Image Adjust(const Color& add, const Color& factor) const {
        Image result = *this;
        for (auto& pixel : result.pixels) {
            pixel = Color(
                std::min(255, std::max(0, static_cast<int>(pixel.GetR()) + static_cast<int>(add.GetR()))),
                std::min(255, std::max(0, static_cast<int>(pixel.GetG()) + static_cast<int>(add.GetG()))),
                std::min(255, std::max(0, static_cast<int>(pixel.GetB()) + static_cast<int>(add.GetB()))),
                std::min(255, std::max(0, static_cast<int>(pixel.GetA()) + static_cast<int>(add.GetA())))
            );
        }
        return result;
    }
};