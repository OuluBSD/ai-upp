#include "Umbrella.h"

#include <CtrlLib/CtrlLib.h>
#include <Draw/Draw.h>
#include <Core/Core.h>
#include <Sound/Sound.h>

using namespace Upp;

// Simple Map Editor Window
class MapEditorApp : public TopWindow {
private:
    bool editorMode = false;
    String importConfigPath;
    String modId;
    
public:
    MapEditorApp();
    
    virtual void Paint(Draw& draw) override;
    virtual bool Key(dword key, int) override;
    virtual void LeftDown(Point p, dword keyflags) override;
};

MapEditorApp::MapEditorApp() {
    Title("Umbrella Map Editor");
    Sizeable().Zoomable();
    SetRect(0, 0, 1280, 720);
    
    // Simple implementation for now
}

void MapEditorApp::Paint(Draw& draw) {
    // Draw a simple grid for the map editor
    draw.DrawRect(GetSize(), White());
    
    // Draw a simple grid
    for(int x = 0; x < GetSize().cx; x += 32) {
        draw.DrawLine(x, 0, x, GetSize().cy, 1, LtGray());
    }
    for(int y = 0; y < GetSize().cy; y += 32) {
        draw.DrawLine(0, y, GetSize().cx, y, 1, LtGray());
    }
    
    // Draw some sample tiles
    draw.DrawRect(100, 100, 64, 64, Blue());
    draw.DrawRect(200, 150, 64, 64, Green());
    draw.DrawRect(150, 200, 64, 64, Red());
    
    draw.DrawText(10, 10, "Umbrella Map Editor", Arial(20), Black());
    if(editorMode) {
        draw.DrawText(10, 40, "Editor Mode Active", Arial(16), Red());
    }
}

bool MapEditorApp::Key(dword key, int) {
    switch(key) {
        case K_ESCAPE:
            Close();
            return true;
        default:
            break;
    }
    return false;
}

void MapEditorApp::LeftDown(Point p, dword keyflags) {
    Refresh();
}

// Global function to launch the map editor
void LaunchMapEditor() {
    MapEditorApp().Run();
}