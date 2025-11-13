#ifndef _AnimEdit_AnimEditMain_h_
#define _AnimEdit_AnimEditMain_h_

#include <CtrlLib/CtrlLib.h>
#include "AnimEditorState.h"

using namespace Upp;

class AnimEditorWindow : public TopWindow {
public:
    AnimEditorWindow();
    
    // Public API for main window to call
    void NewProject();
    void OpenProject();
    void SaveProject();
    void SaveProjectAs();

private:
    AnimEditorState state;
    
    void InitLayout();
    void UpdateTitle();
    
    // Main splitters
    Splitter  hsplit_main;      // left | center+bottom | right
    Splitter  vsplit_center;    // canvas (top) | timeline (bottom)

    // Left panel
    ParentCtrl parts_panel;
    Label      parts_label;

    // Center-top (canvas)
    ParentCtrl canvas_panel;
    Label      canvas_label;

    // Center-bottom (timeline)
    ParentCtrl timeline_panel;
    Label      timeline_label;

    // Right side split components
    Splitter  hsplit_right;     // frames | mid | animations
    ParentCtrl frames_panel;
    Splitter  vsplit_mid;       // sprites (top) | collisions (bottom)
    ParentCtrl sprites_panel;
    ParentCtrl collisions_panel;
    ParentCtrl animations_panel;
    
    // Labels for panels
    Label      frames_label;
    Label      sprites_label;
    Label      collisions_label;
    Label      animations_label;
};

class EntityEditorWindow : public TopWindow {
public:
    EntityEditorWindow() {
        Title("Entity Editor");
        Sizeable().Zoomable();
    }
};

class TextureEditorWindow : public TopWindow {
public:
    TextureEditorWindow() {
        Title("Texture Editor");
        Sizeable().Zoomable();
    }
};

class AnimEditMain : public TopWindow {
public:
    AnimEditMain();
    
private:
    void Menu(Bar& bar);
    void MenuFile(Bar& bar);
    void MenuEditors(Bar& bar);
    void NewProject();
    void OpenProject();
    void SaveProject();
    void Exit();
    void OpenAnimEditor();
    void OpenEntityEditor();
    void OpenTextureEditor();
    
    AnimEditorWindow animwin;
    EntityEditorWindow entitywin;
    TextureEditorWindow texwin;
};

#endif