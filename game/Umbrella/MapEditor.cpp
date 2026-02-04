#include "Umbrella.h"

#include <CtrlLib/CtrlLib.h>
#include <Draw/Draw.h>
#include <Core/Core.h>
#include <Sound/Sound.h>

using namespace Upp;

// Forward declarations
class MapEditorApp;
class MapCanvas;

// Main Map Editor Application Class
class MapEditorApp : public TopWindow {
private:
    bool editorMode = false;
    String importConfigPath;
    String modId;
    
    // UI Components
    MenuBar mainMenuBar;
    ToolBar mainToolBar;
    StatusBar mainStatusBar;
    
    // Panels
    Ctrl toolsPanel;
    Ctrl entityPanel;
    Ctrl mapCanvasContainer;
    TabCtrl bottomTabs;
    
    // Buttons
    Button newMapBtn;
    Button openFileBtn;
    Button saveFileBtn;
    Button undoBtn;
    Button redoBtn;
    Button zoomInBtn;
    Button zoomOutBtn;
    Button resetZoomBtn;
    Button addLayerBtn;
    Button removeLayerBtn;
    Button moveLayerUpBtn;
    Button moveLayerDownBtn;
    Button addEntityBtn;
    Button removeEntityBtn;
    
    // Custom canvas for map editing
    MapCanvas* mapCanvas;

public:
    MapEditorApp();
    virtual ~MapEditorApp();

    void OpenFile(const String& fileName);
    void SaveFile(const String& fileName);
    void NewMap();

    // UI Setup
    void SetupUI();
    void SetupMenuBar();
    void SetupToolBar();

    // Event Handlers
    virtual void Paint(Draw& draw) override;
    virtual bool Key(dword key, int) override;
    virtual void LeftDown(Point p, dword keyflags) override;

    // UI Event Handlers
    void NewMapAction();
    void OpenFileAction();
    void SaveFileAction();
    void UndoAction();
    void RedoAction();
    void ZoomInAction();
    void ZoomOutAction();
    void ResetZoomAction();
    void AddLayerAction();
    void RemoveLayerAction();
    void MoveLayerUpAction();
    void MoveLayerDownAction();
    void AddEntityAction();
    void RemoveEntityAction();
};

// Canvas for drawing and editing the map
class MapCanvas : public Ctrl {
private:
    // Map data and rendering
    Image mapImage;
    Point offset;
    double zoom;

public:
    MapCanvas();
    virtual ~MapCanvas();

    void Paint(Draw& w) override;
    void MouseMove(Point pos, dword flags) override;
    void LeftDown(Point pos, dword flags) override;
    void LeftUp(Point pos, dword flags) override;
    void MouseWheel(Point pos, int zdelta, dword flags) override;

    void SetZoom(double newZoom);
    void PanTo(Point newOffset);
};

// Implementation of MapEditorApp
MapEditorApp::MapEditorApp() {
    Title("Umbrella Map Editor");
    Sizeable().Zoomable();
    SetRect(0, 0, 1400, 900);

    AddFrame(mainMenuBar);
    AddFrame(mainToolBar);
    AddFrame(mainStatusBar);

    SetupMenuBar();
    SetupToolBar();
    SetupUI();
}

MapEditorApp::~MapEditorApp() {
    if(mapCanvas) {
        delete mapCanvas;
    }
}

void MapEditorApp::SetupMenuBar() {
    // Set up the menu bar with proper callbacks
    mainMenuBar.Add("File");
    mainMenuBar.Add("New", K_CTRL_N, THISBACK(NewMapAction));
    mainMenuBar.Add("Open...", K_CTRL_O, THISBACK(OpenFileAction));
    mainMenuBar.Add("Save", K_CTRL_S, THISBACK(SaveFileAction));

    mainMenuBar.Add("Edit");
    mainMenuBar.Add("Undo", K_CTRL_Z, THISBACK(UndoAction));
    mainMenuBar.Add("Redo", K_CTRL_Y, THISBACK(RedoAction));

    mainMenuBar.Add("View");
    mainMenuBar.Add("Zoom In", K_PLUS, THISBACK(ZoomInAction));
    mainMenuBar.Add("Zoom Out", K_MINUS, THISBACK(ZoomOutAction));
    mainMenuBar.Add("Reset Zoom", K_KEY0, THISBACK(ResetZoomAction));
}

void MapEditorApp::SetupToolBar() {
    // Set up the toolbar with common actions
    mainToolBar.Add(newMapBtn.SetLabel("New"));
    mainToolBar.Add(openFileBtn.SetLabel("Open"));
    mainToolBar.Add(saveFileBtn.SetLabel("Save"));
    mainToolBar.Separator();
    mainToolBar.Add(undoBtn.SetLabel("Undo"));
    mainToolBar.Add(redoBtn.SetLabel("Redo"));
    mainToolBar.Separator();
    mainToolBar.Add(zoomInBtn.SetLabel("Zoom In"));
    mainToolBar.Add(zoomOutBtn.SetLabel("Zoom Out"));
    mainToolBar.Add(resetZoomBtn.SetLabel("Reset Zoom"));
    mainToolBar.Separator();
    mainToolBar.Add(addLayerBtn.SetLabel("Add Layer"));
    mainToolBar.Add(removeLayerBtn.SetLabel("Remove Layer"));
    mainToolBar.Separator();
    mainToolBar.Add(addEntityBtn.SetLabel("Add Entity"));
    mainToolBar.Add(removeEntityBtn.SetLabel("Remove Entity"));
}

void MapEditorApp::SetupUI() {
    // Connect UI events to handlers
    newMapBtn <<= THISBACK(NewMapAction);
    openFileBtn <<= THISBACK(OpenFileAction);
    saveFileBtn <<= THISBACK(SaveFileAction);

    undoBtn <<= THISBACK(UndoAction);
    redoBtn <<= THISBACK(RedoAction);

    zoomInBtn <<= THISBACK(ZoomInAction);
    zoomOutBtn <<= THISBACK(ZoomOutAction);
    resetZoomBtn <<= THISBACK(ResetZoomAction);

    addLayerBtn <<= THISBACK(AddLayerAction);
    removeLayerBtn <<= THISBACK(RemoveLayerAction);
    moveLayerUpBtn <<= THISBACK(MoveLayerUpAction);
    moveLayerDownBtn <<= THISBACK(MoveLayerDownAction);

    addEntityBtn <<= THISBACK(AddEntityAction);
    removeEntityBtn <<= THISBACK(RemoveEntityAction);

    // Create and set up the main layout using splitters
    WithSplitterHorz<TopWindow> *mainSplitter = new WithSplitterHorz<TopWindow>;

    // Left panel - Tools
    toolsPanel.SetFrame(BlackFrame());
    toolsPanel.SetRect(0, 0, 200, 600);

    // Right panel - Entities
    entityPanel.SetFrame(BlackFrame());
    entityPanel.SetRect(0, 0, 200, 600);

    // Center canvas area with bottom tabs
    WithSplitterVert<Ctrl> *canvasSplitter = new WithSplitterVert<Ctrl>;

    // Create the map canvas
    mapCanvas = new MapCanvas();
    mapCanvas->SetFrame(BlackFrame());
    mapCanvas->NoWantFocus();

    // Add canvas to the splitter
    canvasSplitter->Set(0, *mapCanvas);
    canvasSplitter->Set(1, bottomTabs);

    // Set up the main splitter: left panel, center canvas/tabs, right panel
    mainSplitter->Set(0, toolsPanel);
    mainSplitter->Set(1, *canvasSplitter);
    mainSplitter->Set(2, entityPanel);

    AddFrame(*mainSplitter);

    // Set up bottom tabs
    bottomTabs.Add("Properties", SizePos());
    bottomTabs.Add("Minimap", SizePos());
    bottomTabs.Add("Tiles", SizePos());

    // Update status bar
    mainStatusBar.Set(0, "Ready");
    mainStatusBar.Set(1, "(0,0)");
    mainStatusBar.Set(2, "100%");
}

void MapEditorApp::Paint(Draw& draw) {
    // Background
    draw.DrawRect(GetSize(), White());
}

bool MapEditorApp::Key(dword key, int) {
    switch(key) {
        case K_ESCAPE:
            Close();
            return true;
        case K_CTRL_S:  // Ctrl+S
            SaveFileAction();
            return true;
        case K_CTRL_O:  // Ctrl+O
            OpenFileAction();
            return true;
        case K_CTRL_N:  // Ctrl+N
            NewMapAction();
            return true;
        case K_CTRL_Z:  // Ctrl+Z
            UndoAction();
            return true;
        case K_CTRL_Y:  // Ctrl+Y
            RedoAction();
            return true;
        default:
            break;
    }
    return false;
}

void MapEditorApp::LeftDown(Point p, dword keyflags) {
    Refresh();
}

void MapEditorApp::NewMapAction() {
    PromptOK("Creating new map...");
    // Implementation for creating a new map
}

void MapEditorApp::OpenFileAction() {
    PromptOK("Open dialog would appear");
    // Implementation for opening a file
}

void MapEditorApp::SaveFileAction() {
    PromptOK("Save dialog would appear");
    // Implementation for saving a file
}

void MapEditorApp::UndoAction() {
    PromptOK("Undo action");
}

void MapEditorApp::RedoAction() {
    PromptOK("Redo action");
}

void MapEditorApp::ZoomInAction() {
    PromptOK("Zoom in");
}

void MapEditorApp::ZoomOutAction() {
    PromptOK("Zoom out");
}

void MapEditorApp::ResetZoomAction() {
    PromptOK("Reset zoom");
}

void MapEditorApp::AddLayerAction() {
    PromptOK("Add layer");
}

void MapEditorApp::RemoveLayerAction() {
    PromptOK("Remove layer");
}

void MapEditorApp::MoveLayerUpAction() {
    PromptOK("Move layer up");
}

void MapEditorApp::MoveLayerDownAction() {
    PromptOK("Move layer down");
}

void MapEditorApp::AddEntityAction() {
    PromptOK("Add entity");
}

void MapEditorApp::RemoveEntityAction() {
    PromptOK("Remove entity");
}

void MapEditorApp::NewMap() {
    PromptOK("Creating new map...");
    // Implementation for creating a new map
}

void MapEditorApp::OpenFile(const String& fileName) {
    PromptOK("Opening file: " + fileName);
    // Implementation for opening a file
}

void MapEditorApp::SaveFile(const String& fileName) {
    PromptOK("Saving file: " + fileName);
    // Implementation for saving a file
}

// Implementation of MapCanvas
MapCanvas::MapCanvas() {
    zoom = 1.0;
    offset.x = 0;
    offset.y = 0;
}

MapCanvas::~MapCanvas() {
}

void MapCanvas::Paint(Draw& w) {
    // Draw the map grid
    w.DrawRect(GetSize(), RGB(240, 240, 240));  // Light gray background

    // Draw grid lines
    int gridSize = 32;
    Size sz = GetSize();

    for(int x = 0; x < sz.cx; x += gridSize) {
        w.DrawLine(x, 0, x, sz.cy, 1, Gray());
    }

    for(int y = 0; y < sz.cy; y += gridSize) {
        w.DrawLine(0, y, sz.cx, y, 1, Gray());
    }

    // Draw a sample rectangle to represent a tile
    w.DrawRect(100, 100, 64, 64, Blue());
    w.DrawRect(200, 150, 64, 64, Green());
    w.DrawRect(150, 200, 64, 64, Red());
}

void MapCanvas::MouseMove(Point pos, dword flags) {
    // Handle mouse movement for drawing
}

void MapCanvas::LeftDown(Point pos, dword flags) {
    // Handle left click for placing tiles
    Refresh();
}

void MapCanvas::LeftUp(Point pos, dword flags) {
    // Handle mouse up
}

void MapCanvas::MouseWheel(Point pos, int zdelta, dword flags) {
    // Handle zoom with mouse wheel
    if(zdelta > 0) {
        zoom *= 1.1;
    } else {
        zoom /= 1.1;
    }
    if(zoom < 0.1) zoom = 0.1;
    if(zoom > 5.0) zoom = 5.0;

    Refresh();
}

void MapCanvas::SetZoom(double newZoom) {
    zoom = newZoom;
    if(zoom < 0.1) zoom = 0.1;
    if(zoom > 5.0) zoom = 5.0;
    Refresh();
}

void MapCanvas::PanTo(Point newOffset) {
    offset = newOffset;
    Refresh();
}

// Main application function
GUI_APP_MAIN
{
    MapEditorApp().Run();
}