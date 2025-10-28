#ifndef _Draw_ImageOp_h_
#define _Draw_ImageOp_h_

#include "Draw.h"
#include "Image.h"
#include <functional>
#include <cmath>

// Image operation filters and effects
class ImageOp {
public:
    // Basic image operations
    static Image Crop(const Image& img, const Rect& r);
    static Image Resize(const Image& img, int width, int height);
    static Image Resize(const Image& img, Size sz);
    static Image Scale(const Image& img, double scale_x, double scale_y);
    static Image Rotate(const Image& img, double angle); // angle in radians
    static Image FlipH(const Image& img);
    static Image FlipV(const Image& img);
    static Image FlipD(const Image& img); // Flip diagonal (transpose)
    
    // Color adjustment operations
    static Image AdjustBrightness(const Image& img, double factor); // factor > 1 brightens, < 1 darkens
    static Image AdjustContrast(const Image& img, double factor); // factor > 1 increases contrast, < 1 decreases
    static Image AdjustSaturation(const Image& img, double factor); // factor > 1 increases saturation, < 1 decreases
    static Image AdjustHue(const Image& img, double angle); // angle in degrees
    static Image InvertColors(const Image& img);
    static Image ToGrayscale(const Image& img);
    static Image ToSepia(const Image& img);
    static Image ChannelSwap(const Image& img, int channel1, int channel2); // 0=R, 1=G, 2=B
    
    // Filtering operations
    static Image GaussianBlur(const Image& img, double radius);
    static Image MotionBlur(const Image& img, double angle, double distance);
    static Image Sharpen(const Image& img, double factor = 1.0);
    static Image Emboss(const Image& img);
    static Image EdgeDetect(const Image& img);
    static Image FindEdges(const Image& img);
    static Image Smooth(const Image& img, int radius = 1);
    static Image MedianFilter(const Image& img, int radius = 1);
    static Image Pixelate(const Image& img, int pixel_size);
    
    // Artistic effects
    static Image OilPainting(const Image& img, int radius = 3, int levels = 20);
    static Image Pointalize(const Image& img, int radius = 3);
    static Image Solarize(const Image& img, int threshold = 128);
    static Image Posterize(const Image& img, int levels = 4);
    static Image Dither(const Image& img, int levels = 2); // Apply dithering with specified levels
    
    // Geometric transformations
    static Image ShearH(const Image& img, double factor);
    static Image ShearV(const Image& img, double factor);
    static Image Skew(const Image& img, double x_angle, double y_angle);
    static Image Perspective(const Image& img, const Point src[4], const Point dst[4]);
    
    // Color blending operations
    static Image Blend(const Image& img1, const Image& img2, double opacity); // Blend two images
    static Image BlendWithColor(const Image& img, Color color, double opacity);
    static Image ScreenBlend(const Image& img1, const Image& img2);
    static Image MultiplyBlend(const Image& img1, const Image& img2);
    static Image OverlayBlend(const Image& img1, const Image& img2);
    static Image SoftLightBlend(const Image& img1, const Image& img2);
    static Image HardLightBlend(const Image& img1, const Image& img2);
    
    // Masking operations
    static Image ApplyMask(const Image& img, const Image& mask); // Use another image as alpha mask
    static Image ApplyMask(const Image& img, const Rect& region); // Use rectangle as mask
    static Image AlphaBlend(const Image& img1, const Image& img2); // Blend using alpha channels
    
    // Effects
    static Image Glow(const Image& img, Color glow_color, int radius = 5, double intensity = 0.5);
    static Image DropShadow(const Image& img, int offset_x = 3, int offset_y = 3, 
                           Color shadow_color = Color::Black(), int blur_radius = 3);
    static Image Outline(const Image& img, Color outline_color, int thickness = 1);
    static Image Bevel(const Image& img, int depth = 2, bool raised = true);
    
    // Noise operations
    static Image AddNoise(const Image& img, double amount = 0.1); // Add random noise
    static Image AddGaussianNoise(const Image& img, double std_dev = 10.0);
    static Image RemoveNoise(const Image& img, int radius = 1);
    
    // Text operations
    static Image DrawText(const Image& img, int x, int y, const String& text, 
                         Font font, Color color);
    
    // Pattern operations
    static Image ApplyPattern(const Image& img, const Image& pattern, double opacity = 0.5);
    
    // Utility methods
    static Size GetRotatedSize(const Image& img, double angle);
    static Size GetScaledSize(const Image& img, double scale_x, double scale_y);
    
private:
    // Internal helper functions
    static Color BlendColors(Color bg, Color fg, double opacity);
    static Color GetPixelSafe(const Image& img, int x, int y, Color default_color = Color::Null());
    static void SetPixelSafe(Image& img, int x, int y, Color color);
    static double Gaussian(double x, double sigma);
};

// Image processing pipeline
class ImagePipeline {
private:
    Image image;
    Vector<std::function<Image(const Image&)>> operations;
    
public:
    ImagePipeline(const Image& img);
    
    // Add operations to pipeline
    ImagePipeline& Crop(const Rect& r);
    ImagePipeline& Resize(int width, int height);
    ImagePipeline& Resize(Size sz);
    ImagePipeline& Scale(double scale_x, double scale_y);
    ImagePipeline& Rotate(double angle);
    ImagePipeline& FlipH();
    ImagePipeline& FlipV();
    ImagePipeline& AdjustBrightness(double factor);
    ImagePipeline& AdjustContrast(double factor);
    ImagePipeline& AdjustSaturation(double factor);
    ImagePipeline& GaussianBlur(double radius);
    ImagePipeline& Sharpen(double factor = 1.0);
    ImagePipeline& ToGrayscale();
    
    // Execute the pipeline
    Image Execute() const;
    
    // Get the internal image
    const Image& GetImage() const { return image; }
    
    // Add a custom operation
    ImagePipeline& AddOperation(std::function<Image(const Image&)> op);
};

// Histogram for image analysis
class ImageHistogram {
private:
    Vector<int> red_hist, green_hist, blue_hist, luminance_hist;
    
public:
    ImageHistogram();
    explicit ImageHistogram(const Image& img);
    
    void Analyze(const Image& img);
    
    // Get histogram data
    const Vector<int>& GetRedHistogram() const { return red_hist; }
    const Vector<int>& GetGreenHistogram() const { return green_hist; }
    const Vector<int>& GetBlueHistogram() const { return blue_hist; }
    const Vector<int>& GetLuminanceHistogram() const { return luminance_hist; }
    
    // Statistics
    int GetRedMax() const;
    int GetGreenMax() const;
    int GetBlueMax() const;
    int GetLuminanceMax() const;
    
    double GetRedMean() const;
    double GetGreenMean() const;
    double GetBlueMean() const;
    double GetLuminanceMean() const;
    
    double GetRedStdDev() const;
    double GetGreenStdDev() const;
    double GetBlueStdDev() const;
    double GetLuminanceStdDev() const;
    
    // Generate histogram as an image
    Image GenerateHistogramImage() const;
};

// Image comparison utilities
class ImageCompare {
public:
    // Calculate mean squared error between two images
    static double MSE(const Image& img1, const Image& img2);
    
    // Calculate peak signal-to-noise ratio
    static double PSNR(const Image& img1, const Image& img2);
    
    // Calculate structural similarity index
    static double SSIM(const Image& img1, const Image& img2);
    
    // Check if two images are approximately equal
    static bool ApproximatelyEqual(const Image& img1, const Image& img2, 
                                  double threshold = 10.0);
    
    // Calculate histogram correlation
    static double HistogramCorrelation(const Image& img1, const Image& img2);
};

#endif