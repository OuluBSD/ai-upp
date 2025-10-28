#pragma once
#ifndef _Draw_DrawText_h_
#define _Draw_DrawText_h_

#include "Draw.h"
#include <string>

namespace Upp {

WString TextUnicode(const char *s, int n, byte cs, Font font);

// DrawText methods are typically part of Draw base class
// This header provides text drawing functionality
inline void Draw::DrawText(int x, int y, int angle, const wchar *text, Font font,
		            Color ink, int n = -1, const int *dx = NULL)
{
	if(IsNull(ink)) return;
	ink = ResolveInk(ink);
	if(n < 0)
		n = wcslen(text);  // Use wcslen for wide character strings
	
	// In a complete implementation, this would handle the actual drawing
	// For stdsrc, we'll provide a simplified version that matches the interface
	// The actual implementation would need to interface with the underlying graphics system
}

inline void Draw::DrawText(int x, int y, int angle, const WString& text, Font font,
                    Color ink, const int *dx = NULL)
{
	DrawText(x, y, angle, text, font, ink, text.GetLength(), dx);
}

inline void Draw::DrawText(int x, int y, const WString& text, Font font, Color ink, const int *dx = NULL)
{
	DrawText(x, y, 0, text, font, ink, dx);
}

inline void Draw::DrawText(int x, int y, const wchar *text, Font font,
                    Color ink, int n = -1, const int *dx = NULL)
{
	DrawText(x, y, 0, text, font, ink, n, dx);
}

inline void Draw::DrawText(int x, int y, int angle, const char *text, byte charset, Font font,
                    Color ink, int n = -1, const int *dx = NULL)
{
	WString unicode_text = TextUnicode(text, n, charset, font);
	DrawText(x, y, angle, unicode_text, font, ink, dx);
}

inline void Draw::DrawText(int x, int y, const char *text, byte charset, Font font,
                    Color ink, int n = -1, const int *dx = NULL)
{
	DrawText(x, y, 0, text, charset, font, ink, n, dx);
}

inline void Draw::DrawText(int x, int y, int angle, const char *text,
                    Font font, Color ink, int n = -1, const int *dx = NULL)
{
	DrawText(x, y, angle, text, CHARSET_DEFAULT, font, ink, n, dx);
}

inline void Draw::DrawText(int x, int y, const char *text, Font font,
                    Color ink, int n = -1, const int *dx = NULL)
{
	DrawText(x, y, text, CHARSET_DEFAULT, font, ink, n, dx);
}

inline void Draw::DrawText(int x, int y, int angle, const String& text, Font font,
                    Color ink, const int *dx = NULL)
{
	DrawText(x, y, angle, text, font, ink, text.GetLength(), dx);
}

inline void Draw::DrawText(int x, int y, const String& text, Font font, Color ink, const int *dx = NULL)
{
	WString h = TextUnicode(text, text.GetLength(), CHARSET_DEFAULT, font);
	DrawText(x, y, h, font, ink, h.GetLength(), dx);
}

// Text size calculation functions
Size GetTextSize(const wchar *text, Font font, int n = -1);
Size GetTextSize(const WString& text, Font font);
Size GetTextSize(const char *text, byte charset, Font font, int n = -1);
Size GetTextSize(const char *text, Font font, int n = -1);
Size GetTextSize(const String& text, Font font);

}

#endif