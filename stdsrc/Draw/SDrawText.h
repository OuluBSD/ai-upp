#pragma once
#ifndef _Draw_SDrawText_h_
#define _Draw_SDrawText_h_

#include "Draw.h"

namespace Upp {

// SDraw text rendering functionality
// This header contains text rendering methods for SDraw class

class SDraw; // Forward declaration

// Text rendering function for SDraw
void SDrawTextOp(SDraw& sdraw, int x, int y, int angle, const wchar *text, Font font, Color ink, int n, const int *dx);

// Glyph rendering for SDraw
Image SDrawRenderGlyph(SDraw& sdraw, Point at, int angle, int chr, Font fnt, Color color, Size sz);

// Text drawing utilities
class SDrawTextRenderer {
public:
    // Render text to SDraw context
    static void DrawText(SDraw& sdraw, int x, int y, int angle, const wchar *text, Font font, Color ink, int n = -1, const int *dx = nullptr);
    
    // Render single character as image
    static Image RenderGlyph(SDraw& sdraw, Point at, int angle, int chr, Font fnt, Color color, Size sz);
    
    // Measure text dimensions
    static Size MeasureText(const wchar *text, Font font, int n = -1, const int *dx = nullptr);
    
    // Calculate text bounds
    static Rect GetTextBounds(int x, int y, int angle, const wchar *text, Font font, int n = -1, const int *dx = nullptr);
};

} // namespace Upp

#endif