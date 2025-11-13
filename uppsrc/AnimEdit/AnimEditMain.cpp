#define CLASSNAME AnimEditMain
#include "AnimEditMain.h"
#include <AnimEditLib/AnimSerialize.h>
#include "AnimEditorState.h"

void AnimEditMain::Menu(Bar& bar) {
    bar.Add("File", THISBACK(MenuFile));
    bar.Add("Editors", THISBACK(MenuEditors));
}

void AnimEditMain::MenuFile(Bar& bar) {
    bar.Add("New Project", [this]() { animwin.Open(); animwin.NewProject(); });
    bar.Add("Open Project...", [this]() { animwin.Open(); animwin.OpenProject(); });
    bar.Add("Save Project", [this]() { animwin.SaveProject(); });
    bar.Add("Save Project As...", [this]() { animwin.SaveProjectAs(); });
    bar.Separator();
    bar.Add("Exit", THISBACK(Exit));
}

void AnimEditMain::MenuEditors(Bar& bar) {
    bar.Add("Animation Editor", THISBACK(OpenAnimEditor));
    bar.Add("Entity Editor", THISBACK(OpenEntityEditor));
    bar.Add("Texture Editor", THISBACK(OpenTextureEditor));
}

void AnimEditMain::Exit() { Break(); }

void AnimEditMain::OpenAnimEditor() { animwin.Open(); }
void AnimEditMain::OpenEntityEditor() { entitywin.Open(); }
void AnimEditMain::OpenTextureEditor() { texwin.Open(); }

AnimEditMain::AnimEditMain() {
    Title("AnimEdit");
    Sizeable().Zoomable();
    MaximizeBox();
    MinimizeBox();

    MenuBar menu;
    menu.Set(THISBACK(Menu));
    AddFrame(menu);
}

// AnimEditorWindow implementations

void AnimEditorWindow::UpdateTitle() {
    String t = "Animation Editor";
    if(!IsEmpty(state.project.name))
        t << " - " << state.project.name;
    if(!IsEmpty(state.current_path))
        t << " [" << GetFileName(state.current_path) << "]";
    if(state.dirty)
        t << " *";
    Title(t);
}

void AnimEditorWindow::NewProject() {
    state.Clear();
    state.project.id = Uuid().ToString();
    state.project.name = "Untitled";
    state.dirty = false;
    UpdateTitle();
    // TODO: refresh UI from empty project (later task)
}

void AnimEditorWindow::OpenProject() {
    FileSel fs;
    fs.Type("Animation Project (*.json)", "*.json");
    fs.AllFilesType();
    if(!fs.ExecuteOpen("Open Animation Project"))
        return;

    String path = ~fs;
    String json = LoadFile(path);
    if(IsNull(json)) {
        Exclamation(Format("Failed to read file:\n%s", path));
        return;
    }

    AnimationProject proj;
    if(!LoadProjectJson(proj, json)) {
        Exclamation("Failed to parse animation project JSON.");
        return;
    }

    state.project = pick(proj);
    state.current_path = path;
    state.dirty = false;
    UpdateTitle();
    // TODO: refresh UI from loaded project (later task)
}

void AnimEditorWindow::SaveProject() {
    if(IsEmpty(state.current_path)) {
        SaveProjectAs();
        return;
    }

    String json = SaveProjectJson(state.project);
    if(!SaveFile(state.current_path, json)) {
        Exclamation(Format("Failed to save file:\n%s", state.current_path));
        return;
    }

    state.dirty = false;
    UpdateTitle();
}

void AnimEditorWindow::SaveProjectAs() {
    FileSel fs;
    fs.Type("Animation Project (*.json)", "*.json");
    fs.AllFilesType();
    if(!IsEmpty(state.current_path))
        fs <<= state.current_path;
    if(!fs.ExecuteSaveAs("Save Animation Project As"))
        return;

    String path = ~fs;
    String json = SaveProjectJson(state.project);
    if(!SaveFile(path, json)) {
        Exclamation(Format("Failed to save file:\n%s", path));
        return;
    }

    state.current_path = path;
    state.dirty = false;
    UpdateTitle();
}

AnimEditorWindow::AnimEditorWindow() {
    Title("Animation Editor");
    Sizeable().Zoomable().MinimizeBox().MaximizeBox();

    InitLayout();
    NewProject();
}

void AnimEditorWindow::InitLayout() {
    // Setup labels
    parts_label.SetLabel("Parts");
    canvas_label.SetLabel("Canvas");
    timeline_label.SetLabel("Timeline");
    frames_label.SetLabel("Frames");
    sprites_label.SetLabel("Sprites");
    collisions_label.SetLabel("Collisions");
    animations_label.SetLabel("Animations");

    // Setup panel backgrounds and add labels
    parts_panel.BackPaint();
    parts_panel.Add(parts_label.SizePos());
    
    canvas_panel.BackPaint();
    canvas_panel.Add(canvas_label.SizePos());
    
    timeline_panel.BackPaint();
    timeline_panel.Add(timeline_label.SizePos());
    
    frames_panel.BackPaint();
    frames_panel.Add(frames_label.SizePos());
    
    sprites_panel.BackPaint();
    sprites_panel.Add(sprites_label.SizePos());
    
    collisions_panel.BackPaint();
    collisions_panel.Add(collisions_label.SizePos());
    
    animations_panel.BackPaint();
    animations_panel.Add(animations_label.SizePos());

    // Right mid (sprites + collisions)
    vsplit_mid.Horz()
        .SetPos(70) // Percentage
        << sprites_panel
        << collisions_panel;

    // Right side (frames | mid | animations)
    hsplit_right.Vert()
        .SetPos(33, 66) // Two split points
        << frames_panel
        << vsplit_mid
        << animations_panel;

    // Center (canvas | timeline)
    vsplit_center.Horz()
        .SetPos(80) // Percentage
        << canvas_panel
        << timeline_panel;

    // Main horizontal layout (left | center | right)
    hsplit_main.Vert()
        .SetPos(20, 75) // Left 20%, center 55%, right 25% (20+55=75)
        << parts_panel
        << vsplit_center
        << hsplit_right;

    Add(hsplit_main.SizePos());
}