#ifndef _AnimEdit_AnimEditMain_h_
#define _AnimEdit_AnimEditMain_h_

#include <CtrlLib/CtrlLib.h>
#include "AnimEditorState.h"
#include "AnimCanvasCtrl.h"
#include "SpriteListCtrl.h"
#include "TimelineCtrl.h"

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
    const Animation* selected_animation; // Currently selected animation

    void InitLayout();
    void UpdateTitle();

    // Main splitters
    Splitter  hsplit_main;      // left | center+bottom | right
    Splitter  vsplit_center;    // canvas (top) | timeline (bottom)

    // Left panel
    ParentCtrl parts_panel;
    SpriteListCtrl sprite_list_ctrl;
    StaticRect parts_toolbar;   // Toolbar for parts panel controls
    EditField search_field;     // For text search
    Option category_option;     // For category filtering
    Option sort_option;         // For sorting options
    Button create_sprite_btn;   // For creating new sprites

    // Center-top (canvas)
    ParentCtrl canvas_panel;
    AnimCanvasCtrl canvas_ctrl;
    StaticRect canvas_toolbar;  // Toolbar for canvas controls
    Label zoom_label;           // Label to show zoom level
    CtrlLayout canvas_controls_layout;  // Layout for canvas controls
    CheckBox grid_snap_check;   // Checkbox for grid snapping
    CheckBox origin_snap_check; // Checkbox for origin snapping
    Button undo_btn;            // Button for undo
    Button redo_btn;            // Button for redo

    // Center-bottom (timeline)
    ParentCtrl timeline_panel;
    StaticRect timeline_toolbar;  // Toolbar for timeline controls
    TimelineCtrl timeline_ctrl;  // Timeline control
    Label      timeline_label;
    
    // Timeline toolbar buttons
    Button insert_frame_btn;      // Insert new frame
    Button add_existing_frame_btn; // Add existing frame
    Button duplicate_frame_btn;   // Duplicate selected frame
    Button delete_frame_btn;      // Delete selected frame
    CheckBox loop_preview_check;  // Loop mode for preview
    Button play_btn;              // Play button
    Button pause_btn;             // Pause button
    Button stop_btn;              // Stop button

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

    // Helper methods
    void UpdateZoomLabel();
    void UpdateUndoRedoButtons();
    void UpdateSpriteList();
    void SetActiveFrame(const Frame* frame);
    void CreateNewSprite();
    void SetSelectedAnimation(const Animation* anim);
    
    // Timeline methods
    void InsertFrame();
    void AddExistingFrame();
    void DuplicateFrame();
    void DeleteFrame();
    void PlayAnimation();
    void PauseAnimation();
    void StopAnimation();
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