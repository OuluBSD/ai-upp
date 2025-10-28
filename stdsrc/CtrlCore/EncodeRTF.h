#pragma once
#ifndef _CtrlCore_EncodeRTF_h_
#define _CtrlCore_EncodeRTF_h_

#include "Ctrl.h"
#include <string>

namespace Upp {

// RTF (Rich Text Format) encoding utilities
class EncodeRTF {
public:
	// Encode text content to RTF format
	static String Encode(const WString& text);
	static String Encode(const String& text);
	
	// Encode text with formatting
	static String Encode(const WString& text, Font font, Color color = Black());
	static String Encode(const String& text, Font font, Color color = Black());
	
	// Encode formatted text block with various attributes
	static String EncodeBlock(const WString& text, 
	                         Font font = StdFont(), 
	                         Color color = Black(), 
	                         bool isBold = false, 
	                         bool isItalic = false, 
	                         bool isUnderline = false);
	
	// Create RTF document header and footer
	static String CreateHeader(int fontSize = 20, const String& fontFamily = "Times New Roman");
	static String CreateFooter();
	
	// Encode complete RTF document
	static String EncodeDocument(const WString& content, 
	                            Font defaultFont = StdFont(),
	                            Color defaultColor = Black());
	
	// Escape RTF special characters
	static String Escape(const String& text);
	static String Escape(const WString& text);
	
	// Color table operations
	static String ColorTable(const Vector<Color>& colors);
	
	// Font table operations
	static String FontTable(const Vector<String>& fonts);
};

// RTF parsing utilities
class ParseRTF {
public:
	// Parse RTF content to extract text
	static WString ParseText(const String& rtf);
	
	// Parse RTF with formatting information
	static WString ParseFormattedText(const String& rtf);
	
	// Parse RTF document and return structured content
	static Value ParseDocument(const String& rtf);
	
	// Extract specific properties from RTF
	static int GetFontSize(const String& rtf);
	static Color GetColor(const String& rtf);
	static Font GetFont(const String& rtf);
};

}

#endif