#pragma once
#ifndef _CtrlCore_CtrlClip_h_
#define _CtrlCore_CtrlClip_h_

#include "CtrlCore.h"
#include "Ctrl.h"
#include "Draw.h"

namespace Upp {

// Clipboard and drag-and-drop related functionality
// This header provides the interfaces for clipboard and drag-and-drop operations
// that are actually implemented in the Ctrl class in U++

class CtrlClip {
protected:
    Ctrl& ctrl;
    
public:
    explicit CtrlClip(Ctrl& c) : ctrl(c) {}
    
    // Access to clipboard operations (these call Ctrl methods)
    PasteClip& GetClipboard() { return ctrl.Clipboard(); }
    PasteClip& GetSelection() { return ctrl.Selection(); }
    
    // High-level clipboard access
    static bool SetText(const String& text);
    static String GetText();
    static bool HasText();
    
    static bool SetImage(const Image& img);
    static Image GetImage();
    static bool HasImage();
    
    static bool SetFiles(const Vector<String>& files);
    static Vector<String> GetFiles();
    static bool HasFiles();
    
    // Drag and drop operations
    void DragAndDrop(Point p, PasteClip& clip);
    void FrameDragAndDrop(Point p, PasteClip& clip);
    void DragEnter();
    void DragLeave();
    void DragRepeat(Point p);
    
    String GetDropData(const String& fmt) const;
    String GetSelectionData(const String& fmt) const;
    
    // Do drag and drop operation
    int DoDragAndDrop(const char *fmts, const Image& sample, dword actions = DND_ALL);
    int DoDragAndDrop(const VectorMap<String, ClipData>& data, const Image& sample = Null, dword actions = DND_ALL);
    
    // Static access for global clipboard operations
    static void Clear();
    static void AppendText(const String& text);
    static void AppendImage(const Image& img);
    static void AppendFiles(const Vector<String>& files);
    
    // Check for clipboard formats
    static bool IsAvailable(const char *fmts);
    
    // Get drag and drop source/target
    static Ctrl *GetDragAndDropSource();
    static Ctrl *GetDragAndDropTarget();
    
    bool IsDragAndDropSource() { return ctrl.IsDragAndDropSource(); }
    bool IsDragAndDropTarget() { return ctrl.IsDragAndDropTarget(); }
};

// Helper functions for common clipboard operations
inline bool SetClipboardText(const String& text) {
    return CtrlClip::SetText(text);
}

inline String GetClipboardText() {
    return CtrlClip::GetText();
}

inline bool SetClipboardImage(const Image& img) {
    return CtrlClip::SetImage(img);
}

inline Image GetClipboardImage() {
    return CtrlClip::GetImage();
}

// Drag image creation
Image MakeDragImage(const Image& arrow, Image sample);

}

#endif