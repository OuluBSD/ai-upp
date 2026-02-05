#include "Umbrella.h"
#include "MapEditor.h"

#include <CtrlLib/CtrlLib.h>
#include <Draw/Draw.h>
#include <Core/Core.h>

using namespace Upp;

// Implementation of MapCanvas
MapCanvas::MapCanvas() {
	zoom = 1.0;
	offset = Point(0, 0);
}

void MapCanvas::Paint(Draw& w) {
	// Draw the map grid
	w.DrawRect(GetSize(), SColorPaper());

	// Draw grid lines
	int gridSize = 32;
	Size sz = GetSize();

	for(int x = 0; x < sz.cx; x += gridSize) {
		w.DrawLine(x, 0, x, sz.cy, 1, SColorShadow());
	}

	for(int y = 0; y < sz.cy; y += gridSize) {
		w.DrawLine(0, y, sz.cx, y, 1, SColorShadow());
	}

	// Draw sample tiles to demonstrate the canvas
	w.DrawRect(100, 100, 64, 64, LtBlue());
	w.DrawRect(200, 150, 64, 64, LtGreen());
	w.DrawRect(150, 200, 64, 64, LtRed());

	// Draw text overlay
	w.DrawText(10, 10, "Map Canvas - Use mouse wheel to zoom", StdFont(), Black());
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

// Implementation of MapEditorApp
MapEditorApp::MapEditorApp() {
	Title("Umbrella Map Editor");
	Sizeable().Zoomable();
	SetRect(0, 0, 1400, 900);

	AddFrame(mainMenuBar);
	AddFrame(mainToolBar);
	AddFrame(mainStatusBar);

	// Initialize layer manager with default 100x100 grid
	layerManager.InitializeDefaultLayers(100, 100);

	mainMenuBar.Set(callback(this, &MapEditorApp::SetupMenuBar));
	SetupToolBar();
	SetupUI();
}

void MapEditorApp::SetupMenuBar(Bar& bar) {
	bar.Sub("File", callback(this, &MapEditorApp::SetupFileMenu));
	bar.Sub("Edit", callback(this, &MapEditorApp::SetupEditMenu));
	bar.Sub("View", callback(this, &MapEditorApp::SetupViewMenu));
}

void MapEditorApp::SetupFileMenu(Bar& bar) {
	bar.Add("New", CtrlImg::new_doc(), callback(this, &MapEditorApp::NewMapAction))
		.Key(K_CTRL_N)
		.Help("Create a new map");
	bar.Add("Open...", CtrlImg::open(), callback(this, &MapEditorApp::OpenFileAction))
		.Key(K_CTRL_O)
		.Help("Open an existing map file");
	bar.Add("Save", CtrlImg::save(), callback(this, &MapEditorApp::SaveFileAction))
		.Key(K_CTRL_S)
		.Help("Save the current map");
	bar.Separator();
	bar.Add("Exit", callback(this, &MapEditorApp::ExitAction))
		.Key(K_ALT_F4)
		.Help("Exit the map editor");
}

void MapEditorApp::SetupEditMenu(Bar& bar) {
	bar.Add("Undo", CtrlImg::undo(), callback(this, &MapEditorApp::UndoAction))
		.Key(K_CTRL_Z)
		.Help("Undo the last action");
	bar.Add("Redo", CtrlImg::redo(), callback(this, &MapEditorApp::RedoAction))
		.Key(K_CTRL_Y)
		.Help("Redo the last undone action");
}

void MapEditorApp::SetupViewMenu(Bar& bar) {
	bar.Add("Zoom In", callback(this, &MapEditorApp::ZoomInAction))
		.Key(K_PLUS)
		.Help("Zoom in on the map");
	bar.Add("Zoom Out", callback(this, &MapEditorApp::ZoomOutAction))
		.Key(K_MINUS)
		.Help("Zoom out from the map");
	bar.Add("Reset Zoom", callback(this, &MapEditorApp::ResetZoomAction))
		.Key(K_CTRL_0)
		.Help("Reset zoom to 100%");
}

void MapEditorApp::SetupToolBar() {
	// Set up the toolbar with common actions
	newMapBtn.SetImage(CtrlImg::new_doc()).Tip("New Map");
	newMapBtn <<= callback(this, &MapEditorApp::NewMapAction);
	mainToolBar.Add(newMapBtn);

	openFileBtn.SetImage(CtrlImg::open()).Tip("Open Map");
	openFileBtn <<= callback(this, &MapEditorApp::OpenFileAction);
	mainToolBar.Add(openFileBtn);

	saveFileBtn.SetImage(CtrlImg::save()).Tip("Save Map");
	saveFileBtn <<= callback(this, &MapEditorApp::SaveFileAction);
	mainToolBar.Add(saveFileBtn);

	mainToolBar.Separator();

	undoBtn.SetImage(CtrlImg::undo()).Tip("Undo");
	undoBtn <<= callback(this, &MapEditorApp::UndoAction);
	mainToolBar.Add(undoBtn);

	redoBtn.SetImage(CtrlImg::redo()).Tip("Redo");
	redoBtn <<= callback(this, &MapEditorApp::RedoAction);
	mainToolBar.Add(redoBtn);

	mainToolBar.Separator();

	zoomInBtn.SetImage(CtrlImg::plus()).Tip("Zoom In");
	zoomInBtn <<= callback(this, &MapEditorApp::ZoomInAction);
	mainToolBar.Add(zoomInBtn);

	zoomOutBtn.SetImage(CtrlImg::minus()).Tip("Zoom Out");
	zoomOutBtn <<= callback(this, &MapEditorApp::ZoomOutAction);
	mainToolBar.Add(zoomOutBtn);

	resetZoomBtn.SetLabel("100%").Tip("Reset Zoom");
	resetZoomBtn <<= callback(this, &MapEditorApp::ResetZoomAction);
	mainToolBar.Add(resetZoomBtn);
}

void MapEditorApp::SetupUI() {
	// Set up the tools panel (left side)
	toolsPanel.SetFrame(InsetFrame());
	toolsLabel.SetText("Tools");
	toolsLabel.SetAlign(ALIGN_CENTER);
	toolsPanel.Add(toolsLabel.HSizePos(2, 2).TopPos(2, 20));

	// Set up the entity panel (right side)
	entityPanel.SetFrame(InsetFrame());
	entityLabel.SetText("Entities");
	entityLabel.SetAlign(ALIGN_CENTER);
	entityPanel.Add(entityLabel.HSizePos(2, 2).TopPos(2, 20));

	// Set up bottom tabs
	propertiesPanel.SetFrame(InsetFrame());
	propertiesLabel.SetText("Properties panel - TBD");
	propertiesPanel.Add(propertiesLabel.HSizePos().VSizePos());

	minimapPanel.SetFrame(InsetFrame());
	minimapLabel.SetText("Minimap - TBD");
	minimapPanel.Add(minimapLabel.HSizePos().VSizePos());

	tilesPanel.SetFrame(InsetFrame());
	tilesLabel.SetText("Tiles palette - TBD");
	tilesPanel.Add(tilesLabel.HSizePos().VSizePos());

	bottomTabs.Add(propertiesPanel.SizePos(), "Properties");
	bottomTabs.Add(minimapPanel.SizePos(), "Minimap");
	bottomTabs.Add(tilesPanel.SizePos(), "Tiles");

	// Set up the map canvas
	mapCanvas.SetFrame(InsetFrame());

	// Create the layout using splitters
	// Vertical splitter for canvas and bottom tabs
	canvasSplitter.Vert(mapCanvas, bottomTabs);
	canvasSplitter.SetPos(7000, 0);  // 70% for canvas, 30% for tabs

	// Horizontal splitter for left panel, center (canvas+tabs), right panel
	mainSplitter.Horz();
	mainSplitter << toolsPanel << canvasSplitter << entityPanel;
	mainSplitter.SetPos(1500, 0);  // Left panel position
	mainSplitter.SetPos(8500, 1);  // Right panel position

	Add(mainSplitter.SizePos());

	// Set up status bar
	mainStatusBar.Set("Ready");
}

bool MapEditorApp::Key(dword key, int) {
	switch(key) {
		case K_ESCAPE:
			Close();
			return true;
		case K_CTRL_S:
			SaveFileAction();
			return true;
		case K_CTRL_O:
			OpenFileAction();
			return true;
		case K_CTRL_N:
			NewMapAction();
			return true;
		case K_CTRL_Z:
			UndoAction();
			return true;
		case K_CTRL_Y:
			RedoAction();
			return true;
		default:
			break;
	}
	return false;
}

void MapEditorApp::NewMapAction() {
	PromptOK("Creating new map...");
	// TODO: Implementation for creating a new map
}

void MapEditorApp::OpenFileAction() {
	FileSel fs;
	fs.Type("Map files", "*.map");
	fs.AllFilesType();
	if(fs.ExecuteOpen("Open Map File")) {
		OpenFile(fs.Get());
	}
}

void MapEditorApp::SaveFileAction() {
	FileSel fs;
	fs.Type("Map files", "*.map");
	if(fs.ExecuteSaveAs("Save Map File")) {
		SaveFile(fs.Get());
	}
}

void MapEditorApp::ExitAction() {
	Close();
}

void MapEditorApp::UndoAction() {
	PromptOK("Undo action");
	// TODO: Implementation for undo
}

void MapEditorApp::RedoAction() {
	PromptOK("Redo action");
	// TODO: Implementation for redo
}

void MapEditorApp::ZoomInAction() {
	mapCanvas.SetZoom(1.2);
}

void MapEditorApp::ZoomOutAction() {
	mapCanvas.SetZoom(0.8);
}

void MapEditorApp::ResetZoomAction() {
	mapCanvas.SetZoom(1.0);
}

void MapEditorApp::NewMap() {
	PromptOK("Creating new map...");
	// TODO: Implementation for creating a new map
}

void MapEditorApp::OpenFile(const String& fileName) {
	PromptOK("Opening file: " + fileName);
	// TODO: Implementation for opening a file
}

void MapEditorApp::SaveFile(const String& fileName) {
	PromptOK("Saving file: " + fileName);
	// TODO: Implementation for saving a file
}

// MapEditorApp is now available for use from main.cpp
// The GUI_APP_MAIN is in main.cpp, not here
