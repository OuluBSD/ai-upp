#ifndef STDSRC_CTRLCORE_CTRLCORE_H
#define STDSRC_CTRLCORE_CTRLCORE_H

// Aggregator header for stdsrc/CtrlCore
// Provides U++-compatible CtrlCore API surface backed by STL/Boost and native UI systems (WXWidgets/Gtk/Qt).

// Standard library includes live here (not in leaf headers)
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>

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
  #include <wx/frame.h>
  #include <wx/window.h>
  #include <wx/event.h>
#endif

#ifdef STDSRC_USE_GTK
  #include <gtk/gtk.h>
  #include <gdk/gdk.h>
#endif

#ifdef STDSRC_USE_QT
  #include <QWidget>
  #include <QApplication>
  #include <QMainWindow>
  #include <QEvent>
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
class Display;
class Event;

// Include CtrlCore module public headers
#include "Ctrl.h"
#include "TopWindow.h"
#include "Display.h"
#include "Event.h"

END_UPP_NAMESPACE

#endif // STDSRC_CTRLCORE_CTRLCORE_H