#include "Umbrella.h"
#include "MapEditor.h"

#include <CtrlLib/CtrlLib.h>
#include <Draw/Draw.h>
#include <Core/Core.h>
#include <Sound/Sound.h>

using namespace Upp;

struct UmbrellaApp : public TopWindow {
private:
    bool editorMode = false;
    String importConfigPath;
    String modId;
    
public:
    UmbrellaApp() {
        Title("Umbrella Game");
        Sizeable().Zoomable();
        SetRect(0, 0, 1280, 720);
        
        // Parse command line arguments
        for(const String& arg : CommandLine()) {
            if(arg == "--editor") {
                editorMode = true;
            } else if(arg == "--editor-parastar") {
                editorMode = true;
                modId = "parastar";
            } else if(arg.StartsWith("--import-config=")) {
                importConfigPath = arg.Mid(strlen("--import-config="));
            } else if(arg.StartsWith("--mod=")) {
                modId = arg.Mid(strlen("--mod="));
            }
        }
        
        // Additional initialization would go here
    }
    
    virtual void Paint(Draw& draw) override {
        // Drawing implementation goes here
        // This replaces the libGDX rendering
        draw.DrawRect(GetSize(), White());
        draw.DrawText(10, 10, "Umbrella Game - U++ Conversion", Arial(20), Black());
        
        if(editorMode) {
            draw.DrawText(10, 40, "Editor Mode Active", Arial(16), Red());
        }
    }
    
    virtual bool Key(dword key, int) override {
        // Handle keyboard input
        // This replaces libGDX input handling
        switch(key) {
            case K_ESCAPE:
                Close();
                return true;
            default:
                break;
        }
        return false;
    }
    
    virtual void LeftDown(Point p, dword keyflags) override {
        // Handle mouse input
        Refresh();
    }
};

GUI_APP_MAIN
{
    // Parse command line arguments
    bool editorMode = false;
    String levelPath;

    const Vector<String>& args = CommandLine();
    for(int i = 0; i < args.GetCount(); i++) {
        const String& arg = args[i];
        if(arg == "--editor" || arg == "--editor-parastar") {
            editorMode = true;
        }
        // If argument doesn't start with --, treat it as a file path
        else if(!arg.StartsWith("--") && !arg.StartsWith("-")) {
            levelPath = arg;
        }
    }

    if(editorMode) {
        if(!levelPath.IsEmpty()) {
            MapEditorApp(levelPath).Run();
        } else {
            MapEditorApp().Run();
        }
    } else {
        UmbrellaApp().Run();
    }
}