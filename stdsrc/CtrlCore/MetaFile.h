#pragma once
#ifndef _CtrlCore_MetaFile_h_
#define _CtrlCore_MetaFile_h_

#include "CtrlCore.h"
#include "Ctrl.h"
#include "Draw.h"
#include <vector>
#include <memory>

namespace Upp {

// MetaFile - represents a collection of drawing operations that can be replayed
// Cross-platform implementation using U++/std drawing system
class MetaFile {
private:
    std::vector<byte> data;  // Serialized drawing operations
    Size              size;  // Size of the metafile
    String            description;  // Optional description
    bool              is_valid;    // Whether the metafile is valid

public:
    void  Clear();
    void  Paint(Draw& w, const Rect& r) const;
    void  Paint(Draw& w, int x, int y, int cx, int cy) const;
    
    void  ReadClipboard();
    void  WriteClipboard() const;
    
    void  Set(const void *data, dword len);
    void  Set(const String& data);
    String Get() const;
    
    void  Load(const char *filename);
    void  Save(const char *filename) const;
    
    void  Serialize(Stream& s);
    
    Size  GetSize() const                        { return size; }
    void  SetSize(Size sz)                       { size = sz; }

    operator bool() const                        { return is_valid; }

    MetaFile();
    MetaFile(void *data, int len);
    MetaFile(const String& data);
    MetaFile(const char *file);
    ~MetaFile()                                  {}
};

// MetaFileDraw - creates metafiles by capturing drawing operations
class MetaFileDraw : public Draw {
private:
    Size   size;
    bool   is_valid;
    
public:
    virtual dword GetInfo() const override;
    virtual Size  GetPageSize() const override { return size; }
    virtual Rect  GetPaintRect() const override { return Rect(size); }
    virtual void  BeginOp() override;
    virtual void  EndOp() override;
    virtual void  OffsetOp(Point p) override;
    virtual bool  ClipOp(const Rect& r) override;
    virtual bool  ClipoffOp(const Rect& r) override;
    virtual bool  ExcludeClipOp(const Rect& r) override;
    virtual bool  IntersectClipOp(const Rect& r) override;
    virtual bool  IsPaintingOp(const Rect& r) const override;

    virtual void  DrawRectOp(int x, int y, int cx, int cy, Color color) override;
    virtual void  DrawImageOp(int x, int y, int cx, int cy, const Image& img, const Rect& src, Color color) override;
    virtual void  DrawLineOp(int x1, int y1, int x2, int y2, int width, Color color) override;
    virtual void  DrawPolyPolylineOp(const Point *vertices, int vertex_count,
                                     const int *counts, int count_count,
                                     int width, Color color, Color doxor) override;
    virtual void  DrawPolyPolyPolygonOp(const Point *vertices, int vertex_count,
                                        const int *subpolygon_counts, int subpolygon_count_count,
                                        const int *disjunct_polygon_counts, int disjunct_polygon_count_count,
                                        Color color, int width, Color outline, uint64 pattern, Color doxor) override;
    virtual void  DrawEllipseOp(const Rect& r, Color color, int pen, Color pencolor) override;
    virtual void  DrawArcOp(const Rect& rc, Point start, Point end, int width, Color color) override;
    virtual void  DrawTextOp(int x, int y, int angle, const wchar *text, Font font, Color ink,
                            int n, const int *dx) override;
    virtual void  DrawDrawingOp(const Rect& target, const Drawing& w) override;
    virtual void  DrawPaintingOp(const Rect& target, const Painting& w) override;
    virtual void  DrawDataOp(int x, int y, int cx, int cy, const String& data, const char *id) override;
    virtual void  Escape(const String& data) override;

    Size GetSize() const                           { return size; }
    
    bool Create(int cx, int cy);
    MetaFile Close();
    
    operator bool() const                          { return is_valid; }

    MetaFileDraw();
    MetaFileDraw(int cx, int cy);
    ~MetaFileDraw();
};

// Helper functions
void DrawWMF(Draw& w, int x, int y, int cx, int cy, const String& wmf);
void DrawWMF(Draw& w, int x, int y, const String& wmf);
Drawing LoadWMF(const char *path, int cx, int cy);
Drawing LoadWMF(const char *path);

String AsWMF(const Drawing& iw);

}

#endif