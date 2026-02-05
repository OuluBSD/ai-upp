#ifndef _Umbrella_MapEditor_h_
#define _Umbrella_MapEditor_h_

#include <CtrlLib/CtrlLib.h>
#include "LayerManager.h"

using namespace Upp;

// Forward declaration
class MapEditorApp;
class MapCanvas;

// Canvas for drawing and editing the map
class MapCanvas : public Ctrl {
private:
	Image mapImage;
	Point offset;
	double zoom;
	bool panning;
	Point panStart;
	int cursorCol;
	int cursorRow;
	bool showGrid;
	MapEditorApp* parentEditor;

public:
	MapCanvas();

	void Paint(Draw& w) override;
	void MouseMove(Point pos, dword flags) override;
	void LeftDown(Point pos, dword flags) override;
	void LeftUp(Point pos, dword flags) override;
	void MiddleDown(Point pos, dword flags) override;
	void MiddleUp(Point pos, dword flags) override;
	void MouseWheel(Point pos, int zdelta, dword flags) override;

	void SetZoom(double newZoom);
	void PanTo(Point newOffset);
	void SetParentEditor(MapEditorApp* parent) { parentEditor = parent; }
	void SetShowGrid(bool show) { showGrid = show; Refresh(); }
	bool GetShowGrid() const { return showGrid; }
	void ZoomToFit();
};

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

	// Layout
	Splitter mainSplitter;
	Splitter canvasSplitter;

	// Panels
	ParentCtrl toolsPanel;
	ParentCtrl entityPanel;
	ParentCtrl propertiesPanel;
	ParentCtrl minimapPanel;
	ParentCtrl tilesPanel;
	TabCtrl bottomTabs;

	// Labels for panels
	Label toolsLabel;
	Label entityLabel;
	Label propertiesLabel;
	Label minimapLabel;
	Label tilesLabel;

	// Canvas
	MapCanvas mapCanvas;

	// Map data
	LayerManager layerManager;

	// Buttons for toolbar
	Button newMapBtn;
	Button openFileBtn;
	Button saveFileBtn;
	Button undoBtn;
	Button redoBtn;
	Button zoomInBtn;
	Button zoomOutBtn;
	Button resetZoomBtn;

public:
	MapEditorApp();

	void OpenFile(const String& fileName);
	void SaveFile(const String& fileName);
	void NewMap();

	// Accessors
	LayerManager& GetLayerManager() { return layerManager; }

	// UI Setup
	void SetupUI();
	void SetupMenuBar(Bar& bar);
	void SetupFileMenu(Bar& bar);
	void SetupEditMenu(Bar& bar);
	void SetupViewMenu(Bar& bar);
	void SetupToolBar();

	// Event Handlers
	virtual bool Key(dword key, int) override;

	// UI Event Handlers
	void NewMapAction();
	void OpenFileAction();
	void SaveFileAction();
	void ExitAction();
	void UndoAction();
	void RedoAction();
	void ZoomInAction();
	void ZoomOutAction();
	void ResetZoomAction();
};

#endif
