// Test for stdsrc Draw functionality

#include "Draw/Draw.h"

using namespace Upp;

CONSOLE_APP_MAIN {
    // Test Point functionality
    {
        Point p1(10, 20);
        Point p2(5, 15);
        
        ASSERT(p1.x == 10);
        ASSERT(p1.y == 20);
        
        Point p3 = p1 + p2;
        ASSERT(p3.x == 15);
        ASSERT(p3.y == 35);
        
        Point p4 = p1 - p2;
        ASSERT(p4.x == 5);
        ASSERT(p4.y == 5);
        
        Point p5 = p1 * 2;
        ASSERT(p5.x == 20);
        ASSERT(p5.y == 40);
        
        Point p6 = p1 / 2;
        ASSERT(p6.x == 5);
        ASSERT(p6.y == 10);
        
        ASSERT(p1.GetX() == 10);
        ASSERT(p1.GetY() == 20);
        
        p1.Set(100, 200);
        ASSERT(p1.x == 100);
        ASSERT(p1.y == 200);
        
        Point p7 = Point::Make(50, 75);
        ASSERT(p7.x == 50);
        ASSERT(p7.y == 75);
        
        Point p8 = Point::Zero();
        ASSERT(p8.x == 0);
        ASSERT(p8.y == 0);
        
        RLOG("Point tests passed!");
    }
    
    // Test Size functionality
    {
        Size s1(100, 200);
        Size s2(50, 75);
        
        ASSERT(s1.cx == 100);
        ASSERT(s1.cy == 200);
        
        Size s3 = s1 + s2;
        ASSERT(s3.cx == 150);
        ASSERT(s3.cy == 275);
        
        Size s4 = s1 - s2;
        ASSERT(s4.cx == 50);
        ASSERT(s4.cy == 125);
        
        Size s5 = s1 * 2;
        ASSERT(s5.cx == 200);
        ASSERT(s5.cy == 400);
        
        Size s6 = s1 / 2;
        ASSERT(s6.cx == 50);
        ASSERT(s6.cy == 100);
        
        ASSERT(s1.GetWidth() == 100);
        ASSERT(s1.GetHeight() == 200);
        
        s1.Set(300, 400);
        ASSERT(s1.cx == 300);
        ASSERT(s1.cy == 400);
        
        Size s7 = Size::Make(150, 250);
        ASSERT(s7.cx == 150);
        ASSERT(s7.cy == 250);
        
        Size s8 = Size::Zero();
        ASSERT(s8.cx == 0);
        ASSERT(s8.cy == 0);
        
        ASSERT(s8.IsZero());
        ASSERT(!s1.IsZero());
        ASSERT(s8.IsEmpty());
        ASSERT(!s1.IsEmpty());
        
        RLOG("Size tests passed!");
    }
    
    // Test Rect functionality
    {
        Rect r1(10, 20, 110, 220);  // left, top, right, bottom
        Rect r2(5, 15, 55, 115);
        
        ASSERT(r1.left == 10);
        ASSERT(r1.top == 20);
        ASSERT(r1.right == 110);
        ASSERT(r1.bottom == 220);
        
        ASSERT(r1.GetWidth() == 100);
        ASSERT(r1.GetHeight() == 200);
        
        Point topLeft = r1.GetTopLeft();
        ASSERT(topLeft.x == 10);
        ASSERT(topLeft.y == 20);
        
        Point bottomRight = r1.GetBottomRight();
        ASSERT(bottomRight.x == 110);
        ASSERT(bottomRight.y == 220);
        
        Point center = r1.GetCenter();
        ASSERT(center.x == 60);
        ASSERT(center.y == 120);
        
        Size size = r1.GetSize();
        ASSERT(size.cx == 100);
        ASSERT(size.cy == 200);
        
        Point testPt(50, 100);
        ASSERT(r1.IsPtInside(testPt));
        
        Point outsidePt(0, 0);
        ASSERT(!r1.IsPtInside(outsidePt));
        
        Rect r3 = r1.GetUnion(r2);
        ASSERT(r3.left == 5);
        ASSERT(r3.top == 15);
        ASSERT(r3.right == 110);
        ASSERT(r3.bottom == 220);
        
        Rect r4 = r1.GetIntersection(r2);
        ASSERT(!r4.IsEmpty());
        ASSERT(r4.left == 10);
        ASSERT(r4.top == 20);
        ASSERT(r4.right == 55);
        ASSERT(r4.bottom == 115);
        
        r1.Set(0, 0, 50, 50);
        ASSERT(r1.GetWidth() == 50);
        ASSERT(r1.GetHeight() == 50);
        
        r1.SetSize(Size(100, 100));
        ASSERT(r1.GetWidth() == 100);
        ASSERT(r1.GetHeight() == 100);
        ASSERT(r1.right == 100);
        ASSERT(r1.bottom == 100);
        
        Rect r5 = Rect::Make(1, 2, 3, 4);
        ASSERT(r5.left == 1);
        ASSERT(r5.top == 2);
        ASSERT(r5.right == 3);
        ASSERT(r5.bottom == 4);
        
        Rect r6 = Rect::MakeP(10, 20, 100, 200);  // x, y, width, height
        ASSERT(r6.left == 10);
        ASSERT(r6.top == 20);
        ASSERT(r6.right == 110);
        ASSERT(r6.bottom == 220);
        ASSERT(r6.GetWidth() == 100);
        ASSERT(r6.GetHeight() == 200);
        
        RLOG("Rect tests passed!");
    }
    
    // Test Color functionality
    {
        Color c1(255, 128, 64);
        Color c2(0, 255, 0);
        
        ASSERT(c1.r == 255);
        ASSERT(c1.g == 128);
        ASSERT(c1.b == 64);
        ASSERT(c1.a == 255);  // Default alpha
        
        ASSERT(c1.GetR() == 255);
        ASSERT(c1.GetG() == 128);
        ASSERT(c1.GetB() == 64);
        ASSERT(c1.GetA() == 255);
        
        c1.SetR(200);
        ASSERT(c1.r == 200);
        
        c1.SetG(100);
        ASSERT(c1.g == 100);
        
        c1.SetB(50);
        ASSERT(c1.b == 50);
        
        c1.SetA(128);
        ASSERT(c1.a == 128);
        
        Color white = Color::White();
        ASSERT(white.r == 255 && white.g == 255 && white.b == 255);
        
        Color black = Color::Black();
        ASSERT(black.r == 0 && black.g == 0 && black.b == 0);
        
        Color red = Color::Red();
        ASSERT(red.r == 255 && red.g == 0 && red.b == 0);
        
        Color green = Color::Green();
        ASSERT(green.r == 0 && green.g == 255 && green.b == 0);
        
        Color blue = Color::Blue();
        ASSERT(blue.r == 0 && blue.g == 0 && blue.b == 255);
        
        Color null = Color::Null();
        ASSERT(null.a == 0);
        ASSERT(null.IsNull());
        ASSERT(!null.Is());
        
        Color regular(100, 150, 200);
        ASSERT(regular.Is());
        ASSERT(!regular.IsNull());
        
        Color blended = c1.Blend(c2, 0.5);
        ASSERT(blended.r == (200 + 0) / 2); // ~100
        ASSERT(blended.g == (100 + 255) / 2); // ~177
        ASSERT(blended.b == (50 + 0) / 2); // 25
        
        Color bright = c1.AdjustBrightness(2.0);
        ASSERT(bright.r == 255); // Clamped to 255
        ASSERT(bright.g == 200); // 100*2 = 200
        ASSERT(bright.b == 100); // 50*2 = 100
        
        Color inverted = red.Invert();
        ASSERT(inverted.r == 0);   // 255-255
        ASSERT(inverted.g == 255); // 255-0
        ASSERT(inverted.b == 255); // 255-0
        
        Color gray = red.ToGray();
        // For red, the gray value should be ~76 (0.299*255)
        int expectedGray = static_cast<int>(0.299 * 255 + 0.587 * 0 + 0.114 * 0);
        ASSERT(gray.r == expectedGray);
        ASSERT(gray.g == expectedGray);
        ASSERT(gray.b == expectedGray);
        
        RLOG("Color tests passed!");
    }
    
    // Test Image functionality
    {
        Size imgSize(10, 10);
        Image img(imgSize);
        
        ASSERT(img.GetWidth() == 10);
        ASSERT(img.GetHeight() == 10);
        ASSERT(img.GetSize() == imgSize);
        ASSERT(img.Is());
        ASSERT(!img.IsEmpty());
        
        Color testColor(255, 128, 64);
        img.SetPixel(5, 5, testColor);
        
        Color retrievedColor = img.GetPixel(5, 5);
        ASSERT(retrievedColor.r == 255);
        ASSERT(retrievedColor.g == 128);
        ASSERT(retrievedColor.b == 64);
        
        // Test bounds checking
        Color outOfBounds = img.GetPixel(-1, -1);
        ASSERT(outOfBounds.IsNull());
        
        // Create a sub-image
        Rect subRect(2, 2, 7, 7);  // 5x5 region
        Image subImg = img.SubImage(subRect);
        ASSERT(subImg.GetWidth() == 5);
        ASSERT(subImg.GetHeight() == 5);
        
        // Test rescaling
        Size newSize(5, 5);
        Image scaled = img.Rescale(newSize);
        ASSERT(scaled.GetWidth() == 5);
        ASSERT(scaled.GetHeight() == 5);
        
        // Test creating from raw data
        Color rawData[] = {
            Color(255, 0, 0), Color(0, 255, 0),
            Color(0, 0, 255), Color(255, 255, 0)
        };
        Image fromRaw(Size(2, 2), rawData);
        ASSERT(fromRaw.GetWidth() == 2);
        ASSERT(fromRaw.GetHeight() == 2);
        ASSERT(fromRaw.GetPixel(0, 0).r == 255); // Red
        ASSERT(fromRaw.GetPixel(1, 0).g == 255); // Green
        ASSERT(fromRaw.GetPixel(0, 1).b == 255); // Blue
        ASSERT(fromRaw.GetPixel(1, 1).r == 255 && fromRaw.GetPixel(1, 1).g == 255); // Yellow
        
        img.Clear();
        ASSERT(img.IsEmpty());
        ASSERT(!img.Is());
        
        RLOG("Image tests passed!");
    }
    
    // Test Font functionality
    {
        Font f1;
        ASSERT(f1.GetFace() == "Arial");
        ASSERT(f1.GetHeight() == 12);
        ASSERT(!f1.IsBold());
        ASSERT(!f1.IsItalic());
        
        Font f2 = Font::Arial(16);
        ASSERT(f2.GetFace() == "Arial");
        ASSERT(f2.GetHeight() == 16);
        
        Font f3 = Font::System();
        ASSERT(f3.GetFace() == "System");
        ASSERT(f3.GetHeight() == 12);
        
        Font f4 = Font::ArialBold(14);
        ASSERT(f4.GetFace() == "Arial");
        ASSERT(f4.GetHeight() == 14);
        ASSERT(f4.IsBold());
        
        Font f5;
        f5.Face("Times New Roman").Height(18).Bold().Italic();
        ASSERT(f5.GetFace() == "Times New Roman");
        ASSERT(f5.GetHeight() == 18);
        ASSERT(f5.IsBold());
        ASSERT(f5.IsItalic());
        
        ASSERT(f5.GetAscent() > 0);
        ASSERT(f5.GetDescent() > 0);
        
        Size textSize = f5.GetTextSize("Hello World");
        ASSERT(textSize.cx > 0);
        ASSERT(textSize.cy > 0);
        
        ASSERT(f5.GetCy() == 18);
        
        Font f6 = Font::Std();
        ASSERT(f6.GetFace() == "Arial");
        ASSERT(f6.GetHeight() == 12);
        
        RLOG("Font tests passed!");
    }
    
    RLOG("All Draw tests passed successfully!");
}