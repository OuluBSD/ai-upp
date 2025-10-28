#pragma once
#ifndef _CtrlCore_ParseRTF_h_
#define _CtrlCore_ParseRTF_h_

#include "Ctrl.h"
#include <string>

namespace Upp {

// Forward declaration since ParseRTF was already included in EncodeRTF.h
// This file provides parsing functionality for RTF content

class ParseRTF {
public:
	// Parse RTF content to extract plain text
	static String Parse(const String& rtf);
	static WString ParseW(const String& rtf);
	
	// Parse RTF with formatting information
	struct RTFData {
		WString text;
		Font    font;
		Color   color;
		Rect    rect;
		bool    bold;
		bool    italic;
		bool    underline;
	};
	
	static Vector<RTFData> ParseFormatted(const String& rtf);
	
	// Parse RTF document structure
	struct RTFDocument {
		String              header;
		String              footer;
		Vector<Color>       colorTable;
		Vector<String>      fontTable;
		Vector<RTFData>     content;
	};
	
	static RTFDocument ParseDocument(const String& rtf);
	
	// Extract specific properties from RTF
	static int GetFontSize(const String& rtf);
	static Color GetColor(const String& rtf);
	static String GetFontName(const String& rtf);
	static Font GetFont(const String& rtf);
	
	// Utility methods for RTF handling
	static bool Validate(const String& rtf);
	static String Clean(const String& rtf);  // Remove formatting, keep text
	
	// Parse into a Value structure for generic access
	static Value ParseToValue(const String& rtf);
};

}

#endif