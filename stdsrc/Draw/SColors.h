#pragma once
#ifndef _Draw_SColors_h_
#define _Draw_SColors_h_

#include "Draw.h"

namespace Upp {

// System colors that adjust according to dark/light theme
extern Color SBlack;
extern Color SGray;
extern Color SLtGray;
extern Color SWhiteGray;
extern Color SWhite;
extern Color SRed;
extern Color SGreen;
extern Color SBrown;
extern Color SBlue;
extern Color SMagenta;
extern Color SCyan;
extern Color SYellow;
extern Color SLtRed;
extern Color SLtGreen;
extern Color SLtYellow;
extern Color SLtBlue;
extern Color SLtMagenta;
extern Color SLtCyan;
extern Color SOrange;
extern Color SPink;
extern Color SDkRed;
extern Color SDkGreen;
extern Color SDkBlue;
extern Color SDkYellow;
extern Color SDkMagenta;
extern Color SDkCyan;

// Standard system colors
extern Color SColorPaper;      // Background color for controls
extern Color SColorFace;       // Face color for controls
extern Color SColorText;       // Text color
extern Color SColorHighlight;  // Highlight color (selection)
extern Color SColorHighlightText;  // Text color for highlighted items
extern Color SColorMenu;       // Menu background color
extern Color SColorMenuText;   // Menu text color
extern Color SColorInfo;       // Info area background
extern Color SColorInfoText;   // Info area text
extern Color SColorDisabled;   // Color for disabled items
extern Color SColorLight;      // Light shadow color
extern Color SColorShadow;     // Dark shadow color
extern Color SColorMark;       // Marking color
extern Color SColorMenuMark;   // Menu marking color

// Derived colors
extern Color SColorLtFace;     // Light face (Blend of face and light)
extern Color SColorDkShadow;   // Dark shadow (Blend of shadow and black)
extern Color SColorLabel;      // Label text color

// Function to adjust colors for dark theme
Color AdjustIfDark(Color c);
Color DarkTheme(Color c);

// Check if dark theme is active
bool IsDarkTheme();

// Toggle dark theme
void SetDarkTheme(bool dark);
bool GetDarkTheme();

}

#endif