#pragma once
#ifndef STDSRC_CTRLLIB_CTRLLIB_H
#define STDSRC_CTRLLIB_CTRLLIB_H

// Aggregator header for stdsrc/CtrlLib
// Provides U++-compatible CtrlLib API surface backed by STL/Boost and native UI systems (WXWidgets/Gtk/Qt).

// Standard library includes live here (not in leaf headers)
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <map>
#include <unordered_map>

// Include necessary Draw and CtrlCore components
#include "../Draw/Draw.h"  // We'll need drawing components
#include "../CtrlCore/CtrlCore.h"  // We'll need control components

// Native UI library includes (based on platform detection)
#ifdef _WIN32
  #include <windows.h>
  #undef min
  #undef max
#elif __APPLE__
  #include <Cocoa/Cocoa.h>
#else // Assume POSIX/X11
  #include <X11/Xlib.h>
  #include <X11/Xutil.h>
#endif

// Optional: UI library support (WXWidgets/Gtk/Qt)
// These can be enabled/disabled with preprocessor flags
#ifdef STDSRC_USE_WXWIDGETS
  #include <wx/wx.h>
  #include <wx/button.h>
  #include <wx/textctrl.h>
  #include <wx/choice.h>
#endif

#ifdef STDSRC_USE_GTK
  #include <gtk/gtk.h>
  #include <gdk/gdk.h>
#endif

#ifdef STDSRC_USE_QT
  #include <QPushButton>
  #include <QLineEdit>
  #include <QComboBox>
  #include <QLabel>
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
class Ctrl;
class TopWindow;
class Button;
class Label;
class EditField;
class ArrayCtrl;

// Include CtrlLib module public headers
#include "Button.h"
#include "Label.h"
#include "EditField.h"
#include "ArrayCtrl.h"
#include "Splitter.h"
#include "ScrollBar.h"
#include "SliderCtrl.h"

END_UPP_NAMESPACE

#endif // STDSRC_CTRLLIB_CTRLLIB_H