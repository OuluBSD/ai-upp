# U++ to STL Mapping Summary

## Overview

This document provides a summary of all mappings between U++ framework components and their C++ STL equivalents being implemented in the stdmap project.

## Core Package Mappings

### Containers
- `Vector<T>` → `std::vector<T>`
- `Array<T>` → `std::vector<std::unique_ptr<T>>`
- `Index<T>` → `std::unordered_set<T>` or `std::set<T>`
- `VectorMap<K,V>` → `std::unordered_map<K,V>` or `std::map<K,V>`
- `ArrayMap<K,V>` → `std::unordered_map<K,std::unique_ptr<V>>`

### Strings
- `String` → `std::string`
- `WString` → `std::wstring`
- `StringBuffer` → `std::string` with reserve/capacity operations

### Smart Pointers
- `One<T>` → `std::unique_ptr<T>`
- `Ptr<T>` → `std::shared_ptr<T>` (or raw pointer depending on context)

### Utilities
- `Value` → `std::any` or `std::variant`
- `Function<...>` → `std::function<...>`
- `Callback` → `std::function` or callback pattern
- `Tuple<...>` → `std::tuple<...>`

### Threading
- `Thread` → `std::thread`
- `Mutex` → `std::mutex`
- `Semaphore` → `std::counting_semaphore` (C++20) or custom implementation
- `Atomic<T>` → `std::atomic<T>`

### Time/Date
- `Time` → `std::chrono::time_point`
- `Date` → Custom date type or `std::chrono::day/month/year`

### Algorithms
- `Find` → `std::find`
- `FindIndex` → `std::find` + `std::distance`
- `Sort` → `std::sort`
- `IndexOf` → `std::find` + `std::distance`

### I/O
- `Stream` → `std::iostream` hierarchy
- `FileIn` → `std::ifstream`
- `FileOut` → `std::ofstream`
- `StringStream` → `std::stringstream`

## Draw Package Mappings

### Graphics Types
- `Image` → Custom image class using std containers and pixel data
- `ImageBuffer` → `std::vector<RGBA>` with size information
- `Draw` → Abstract drawing interface mapping to graphics libraries
- `RGBA` → `struct` with r, g, b, a fields

### Drawing Operations
- Drawing operations will map to underlying graphics library (e.g., Cairo, Direct2D)
- Basic operations like DrawRect, DrawLine, DrawImage will map to equivalent calls

## CtrlCore Package Mappings

### GUI Base Classes
- `Ctrl` → Base GUI widget class mapping to target GUI framework
- `TopWindow` → Main window class (e.g., wxFrame, QMainWindow)
- `Draw` (in GUI context) → Paint context (e.g., wxPaintDC, QPainter)

### GUI Concepts
- Events → Target framework's event system (e.g., wxEvent, Qt signals/slots)
- Layout → Target framework's layout system
- Painting → Target framework's painting model

## CtrlLib Package Mappings

### Standard Controls
- `Button` → Target framework's button (e.g., wxButton, QPushButton)
- `EditField` → Target framework's text input (e.g., wxTextCtrl, QLineEdit)
- `ComboBox` → Target framework's combo box (e.g., wxComboBox, QComboBox)
- `ListBox` → Target framework's list box (e.g., wxListBox, QListWidget)

### Layout Controls
- `CtrlLayout` → Target framework's layout system
- `ArrayCtrl` → Target framework's list control (e.g., wxListCtrl, QTableWidget)

## wxWidgets Mapping Strategy

For the GUI components (CtrlCore and CtrlLib), we will map to wxWidgets where possible:

### GUI Class Mapping
- `Upp::Ctrl` → `wxWindow` base class
- `Upp::TopWindow` → `wxFrame`
- `Upp::Button` → `wxButton`
- `Upp::EditField` → `wxTextCtrl`
- `Upp::ComboBox` → `wxComboBox`

### Event Mapping
- `Ctrl::LeftDown` → `wxEVT_LEFT_DOWN` event
- `Ctrl::Key` → `wxEVT_CHAR` or specific key events
- `Ctrl::Paint` → `wxEVT_PAINT` event

### Layout Mapping
- U++'s layout system → wxWidgets sizer system
- `Ctrl::SetRect` → `wxWindow::SetSize`
- `Ctrl::Add` → `wxSizer::Add`

## Implementation Status

### Complete Mappings
- Core containers (Vector, String, etc.)
- Basic string operations
- Core algorithms
- Some threading primitives

### In Progress
- Complete Core package mapping
- Draw package analysis
- wxWidgets GUI mapping design

### Planned
- Complete Draw package implementation
- CtrlCore/CtrlLib GUI mapping with wxWidgets
- Full documentation for all mappings

## Notes on Implementation

1. **Memory Management**: Maintaining U++'s memory management patterns while using STL containers
2. **Thread Safety**: Preserving U++'s thread safety model with STL equivalents
3. **Performance**: Ensuring no significant performance regression
4. **Compatibility**: Maintaining API compatibility where possible
5. **GUI Mapping**: Using compatibility layers for GUI components to wxWidgets/Qt

This summary provides an overview of the current and planned mappings in the stdmap project, showing the comprehensive approach to converting U++ code to standard C++ with STL usage.