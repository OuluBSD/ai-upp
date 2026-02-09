#ifndef _Umbrella_MapEditor_h_
#define _Umbrella_MapEditor_h_

#include <CtrlLib/CtrlLib.h>
#include <Docking/Docking.h>
#include "LayerManager.h"
#include "MapSerializer.h"
#include "BrushTool.h"
#include "FillTool.h"
#include "EntityPlacementTool.h"
#include "Enemy.h"
#include "Droplet.h"

using namespace Upp;

// Forward declaration
class MapEditorApp;
class MapCanvas;

// Canvas for drawing and editing the map
class MapCanvas : public Ctrl {
private:
	Image mapImage;
	Image referenceImage;
	Point offset;
	double zoom;
	bool panning;
	Point panStart;
	int cursorCol;
	int cursorRow;
	bool showGrid;
	bool showReferenceImage;
	int referenceImageOpacity;
	Point referenceImageOffset;
	double referenceImageScale;
	bool referenceImagePanning;
	Point referenceImagePanStart;
	MapEditorApp* parentEditor;

public:
	MapCanvas();

	void Paint(Draw& w) override;
	void MouseMove(Point pos, dword flags) override;
	void LeftDown(Point pos, dword flags) override;
	void LeftUp(Point pos, dword flags) override;
	void RightDown(Point pos, dword flags) override;
	void RightUp(Point pos, dword flags) override;
	void MiddleDown(Point pos, dword flags) override;
	void MiddleUp(Point pos, dword flags) override;
	void MouseWheel(Point pos, int zdelta, dword flags) override;

	void SetZoom(double newZoom);
	void PanTo(Point newOffset);
	Point GetOffset() const { return offset; }
	void SetParentEditor(MapEditorApp* parent) { parentEditor = parent; }
	void SetShowGrid(bool show) { showGrid = show; Refresh(); }
	bool GetShowGrid() const { return showGrid; }
	void SetReferenceImage(const Image& img);
	void SetShowReferenceImage(bool show) { showReferenceImage = show; Refresh(); }
	bool GetShowReferenceImage() const { return showReferenceImage; }
	void SetReferenceImageOpacity(int opacity) { referenceImageOpacity = clamp(opacity, 0, 100); Refresh(); }
	int GetReferenceImageOpacity() const { return referenceImageOpacity; }
	void ZoomToFit();
};

// Main Map Editor Application Class
class MapEditorApp : public DockWindow {
public:
	// Tool selection
	enum EditTool {
		TOOL_BRUSH,
		TOOL_ERASER,
		TOOL_FILL,
		TOOL_SELECT,
		TOOL_ENEMY_PLACEMENT,
		TOOL_DROPLET_PLACEMENT
	};

private:
	bool editorMode = false;
	String importConfigPath;
	String modId;
	String currentFilePath;
	String referenceImagePath;

	// Editing tools
	BrushTool brushTool;
	FillTool fillTool;
	EnemyPlacementTool enemyTool;
	DropletPlacementTool dropletTool;
	EditTool currentTool;

	// Spawn point data
	Array<EnemySpawnPoint> enemySpawns;
	Array<DropletSpawnPoint> dropletSpawns;

	// UI Components
	MenuBar mainMenuBar;
	ToolBar mainToolBar;
	StatusBar mainStatusBar;

	// Dockable Panels
	ParentCtrl toolsPanel;
	ParentCtrl layersPanel;
	ParentCtrl entitiesPanel;
	ParentCtrl propertiesPanel;

	// Tools Panel Controls
	Label toolsLabel;
	Label brushSizeLabel;
	Button brush1x1Btn, brush2x2Btn, brush3x3Btn, brush5x5Btn;
	Label tileTypeLabel;
	Button wallBtn, bgBtn, blockBtn;
	Label toolSelectionLabel;
	Button brushToolBtn, eraserToolBtn, fillToolBtn;

	// Layers Panel Controls
	TreeArrayCtrl layersList;
	Button addLayerBtn, removeLayerBtn;
	Button moveLayerUpBtn, moveLayerDownBtn;
	Label layerOpacityLabel;
	SliderCtrl layerOpacitySlider;

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
	MapEditorApp(const String& levelPath);

	void OpenFile(const String& fileName);
	void SaveFile(const String& fileName);
	void NewMap();
	void LoadReferenceImage(const String& imagePath);
	void BrowseReferenceImage();

	// Accessors
	LayerManager& GetLayerManager() { return layerManager; }
	const String& GetCurrentFilePath() const { return currentFilePath; }
	BrushTool& GetBrushTool() { return brushTool; }
	FillTool& GetFillTool() { return fillTool; }
	EnemyPlacementTool& GetEnemyTool() { return enemyTool; }
	DropletPlacementTool& GetDropletTool() { return dropletTool; }
	EditTool GetCurrentTool() const { return currentTool; }
	void SetCurrentTool(EditTool tool) { currentTool = tool; }
	Array<EnemySpawnPoint>& GetEnemySpawns() { return enemySpawns; }
	Array<DropletSpawnPoint>& GetDropletSpawns() { return dropletSpawns; }

	// UI Setup
	virtual void DockInit() override;
	void SetupMenuBar(Bar& bar);
	void SetupFileMenu(Bar& bar);
	void SetupEditMenu(Bar& bar);
	void SetupBackgroundMenu(Bar& bar);
	void SetupViewMenu(Bar& bar);
	void SetupToolBar();
	void SetupToolsPanel();
	void SetupLayersPanel();
	void RefreshLayersList();

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
	void PlaytestAction();
};

#endif
