#ifndef STDSRC_DRAW_DRAW_H
#define STDSRC_DRAW_DRAW_H

// Aggregator header for stdsrc/Draw
// Provides U++-compatible Draw API surface backed by STL/Boost and native drawing systems (WXWidgets/Gtk/Qt).

// Standard library includes live here (not in leaf headers)
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <array>
#include <map>
#include <unordered_map>

// Native drawing library includes (based on platform detection)
#ifdef _WIN32
  #include <windows.h>
  #undef min
  #undef max
#elif __APPLE__
  #include <CoreGraphics/CoreGraphics.h>
  #include <AppKit/AppKit.h>
#else // Assume POSIX/X11
  #include <X11/Xlib.h>
  #include <X11/Xutil.h>
#endif

// Optional: Graphics library support (WXWidgets/Gtk/Qt)
// These can be enabled/disabled with preprocessor flags
#ifdef STDSRC_USE_WXWIDGETS
  #include <wx/wx.h>
  #include <wx/dc.h>
  #include <wx/image.h>
#endif

#ifdef STDSRC_USE_GTK
  #include <gtk/gtk.h>
  #include <gdk/gdk.h>
#endif

#ifdef STDSRC_USE_QT
  #include <QPainter>
  #include <QImage>
  #include <QColor>
  #include <QFont>
#endif

// Namespace wrappers
#ifndef NAMESPACE_UPP
#define NAMESPACE_UPP     namespace Upp {
#define END_UPP_NAMESPACE }
#define UPP               Upp
#endif

// Token pasting helper for macros
#ifndef UPP_COMBINE
#define UPP_COMBINE__(a,b) a##b
#define UPP_COMBINE(a,b) UPP_COMBINE__(a,b)
#endif

// Bring in U++-style platform/CPU/compiler macros
#include "config.h"

NAMESPACE_UPP

// Forward declarations
class Point;
class Size;
class Rect;
class Color;
class Image;
class Draw;
class Font;

// Include Draw module public headers
#include "Point.h"
#include "Size.h"
#include "Rect.h"
#include "Color.h"
#include "Image.h"
#include "DrawCore.h"  // Renamed to avoid conflict with aggregator header
#include "Font.h"

END_UPP_NAMESPACE

#endif // STDSRC_DRAW_DRAW_H