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
	panning = false;
	panStart = Point(0, 0);
	cursorCol = -1;
	cursorRow = -1;
	showGrid = true;
	parentEditor = nullptr;
}

void MapCanvas::Paint(Draw& w) {
	Size sz = GetSize();

	// Get access to layer manager
	if(!parentEditor) {
		w.DrawRect(sz, Color(12, 17, 30));
		w.DrawText(10, 10, "Map Canvas - No parent editor", StdFont(), White());
		return;
	}

	LayerManager& layerMgr = parentEditor->GetLayerManager();

	// Draw canvas background (empty color)
	w.DrawRect(sz, Color(12, 17, 30));

	// Calculate tile size based on zoom
	int tileSize = int(14 * zoom);
	if(tileSize < 1) tileSize = 1;

	// Calculate visible grid range
	int viewCols = sz.cx / tileSize + 2;
	int viewRows = sz.cy / tileSize + 2;

	int startCol = max(0, -offset.x / tileSize);
	int startRow = max(0, -offset.y / tileSize);
	int endCol = min(100, startCol + viewCols);
	int endRow = min(100, startRow + viewRows);

	// Render each visible layer (bottom to top)
	for(int layerIndex = 0; layerIndex < layerMgr.GetLayerCount(); layerIndex++) {
		const Layer& layer = layerMgr.GetLayer(layerIndex);

		if(!layer.IsVisible()) continue;

		const MapGrid& grid = layer.GetGrid();
		int opacity = layer.GetOpacity();

		// Render tiles in this layer
		for(int row = startRow; row < endRow; row++) {
			for(int col = startCol; col < endCol; col++) {
				TileType tile = grid.GetTile(col, row);

				if(tile == TILE_EMPTY) continue;

				// Calculate screen position
				int screenX = col * tileSize + offset.x;
				int screenY = row * tileSize + offset.y;

				// Get tile color
				Color tileColor = TileTypeToColor(tile);

				// Apply layer opacity
				if(opacity < 100) {
					Color bgColor = Color(12, 17, 30);
					int alpha = opacity * 255 / 100;
					tileColor = Color(
						(tileColor.GetR() * alpha + bgColor.GetR() * (255 - alpha)) / 255,
						(tileColor.GetG() * alpha + bgColor.GetG() * (255 - alpha)) / 255,
						(tileColor.GetB() * alpha + bgColor.GetB() * (255 - alpha)) / 255
					);
				}

				// Draw tile as filled rectangle
				w.DrawRect(screenX, screenY, tileSize, tileSize, tileColor);
			}
		}
	}

	// Draw grid lines (on top of tiles)
	if(showGrid) {
		Color gridColor = Color(51, 69, 92);

		// Vertical lines
		for(int col = startCol; col <= endCol; col++) {
			int screenX = col * tileSize + offset.x;
			if(screenX >= 0 && screenX < sz.cx) {
				w.DrawLine(screenX, 0, screenX, sz.cy, 1, gridColor);
			}
		}

		// Horizontal lines
		for(int row = startRow; row <= endRow; row++) {
			int screenY = row * tileSize + offset.y;
			if(screenY >= 0 && screenY < sz.cy) {
				w.DrawLine(0, screenY, sz.cx, screenY, 1, gridColor);
			}
		}
	}

	// Draw tool preview
	if(cursorCol >= 0 && cursorRow >= 0 && parentEditor) {
		BrushTool& brush = parentEditor->GetBrushTool();
		MapEditorApp::EditTool currentTool = parentEditor->GetCurrentTool();

		if(currentTool == MapEditorApp::TOOL_FILL) {
			// Fill tool preview: show bucket icon
			int screenX = cursorCol * tileSize + offset.x;
			int screenY = cursorRow * tileSize + offset.y;

			// Yellow outline
			w.DrawRect(screenX, screenY, tileSize, tileSize, 2, Yellow());

			// Show fill color preview
			FillTool& fill = parentEditor->GetFillTool();
			Color fillColor = TileTypeToColor(brush.GetPaintTile());
			int alpha = 180;
			Color bgColor = Color(12, 17, 30);
			fillColor = Color(
				(fillColor.GetR() * alpha + bgColor.GetR() * (255 - alpha)) / 255,
				(fillColor.GetG() * alpha + bgColor.GetG() * (255 - alpha)) / 255,
				(fillColor.GetB() * alpha + bgColor.GetB() * (255 - alpha)) / 255
			);
			w.DrawRect(screenX + 2, screenY + 2, tileSize - 4, tileSize - 4, fillColor);
		}
		else {
			Vector<Point> brushTiles;
			brush.GetBrushTiles(cursorCol, cursorRow, brushTiles);

			// Draw preview for each tile
			for(const Point& pt : brushTiles) {
				if(pt.x < 0 || pt.x >= 100 || pt.y < 0 || pt.y >= 100) continue;

				int screenX = pt.x * tileSize + offset.x;
				int screenY = pt.y * tileSize + offset.y;

				if(currentTool == MapEditorApp::TOOL_ERASER) {
					// Draw red X for eraser
					w.DrawLine(screenX + 2, screenY + 2, screenX + tileSize - 2, screenY + tileSize - 2, 2, LtRed());
					w.DrawLine(screenX + tileSize - 2, screenY + 2, screenX + 2, screenY + tileSize - 2, 2, LtRed());

					// Red outline
					w.DrawRect(screenX, screenY, tileSize, tileSize, 2, LtRed());
				}
				else {
					// Brush preview: semi-transparent tile color
					Color previewColor = TileTypeToColor(brush.GetPaintTile());
					int alpha = 128;
					Color bgColor = Color(12, 17, 30);
					previewColor = Color(
						(previewColor.GetR() * alpha + bgColor.GetR() * (255 - alpha)) / 255,
						(previewColor.GetG() * alpha + bgColor.GetG() * (255 - alpha)) / 255,
						(previewColor.GetB() * alpha + bgColor.GetB() * (255 - alpha)) / 255
					);

					// Draw semi-transparent preview
					w.DrawRect(screenX + 1, screenY + 1, tileSize - 2, tileSize - 2, previewColor);

					// Draw outline
					w.DrawRect(screenX, screenY, tileSize, tileSize, 1, White());
				}
			}
		}
	}
}

void MapCanvas::MouseMove(Point pos, dword flags) {
	if(panning) {
		// Update camera offset
		Point delta = pos - panStart;
		offset += delta;
		panStart = pos;
		Refresh();
	}

	// Update cursor tile position
	int tileSize = int(14 * zoom);
	if(tileSize > 0) {
		cursorCol = (pos.x - offset.x) / tileSize;
		cursorRow = (pos.y - offset.y) / tileSize;

		// If painting, continue painting
		if((flags & K_MOUSELEFT) && parentEditor) {
			MapEditorApp::EditTool tool = parentEditor->GetCurrentTool();
			if(tool == MapEditorApp::TOOL_BRUSH || tool == MapEditorApp::TOOL_ERASER) {
				BrushTool& brush = parentEditor->GetBrushTool();
				LayerManager& layerMgr = parentEditor->GetLayerManager();

				if(tool == MapEditorApp::TOOL_BRUSH) {
					brush.SetMode(BRUSH_MODE_PAINT);
				} else {
					brush.SetMode(BRUSH_MODE_ERASE);
				}

				brush.ContinuePainting(cursorCol, cursorRow, layerMgr);
			}
		}

		// If right-click erasing, continue erasing
		if((flags & K_MOUSERIGHT) && parentEditor) {
			BrushTool& brush = parentEditor->GetBrushTool();
			LayerManager& layerMgr = parentEditor->GetLayerManager();

			brush.SetMode(BRUSH_MODE_ERASE);
			brush.ContinuePainting(cursorCol, cursorRow, layerMgr);
		}
	}

	Refresh();
}

void MapCanvas::LeftDown(Point pos, dword flags) {
	if(!parentEditor) return;

	// Get tile coordinates
	int tileSize = int(14 * zoom);
	if(tileSize <= 0) return;

	int col = (pos.x - offset.x) / tileSize;
	int row = (pos.y - offset.y) / tileSize;

	BrushTool& brush = parentEditor->GetBrushTool();
	LayerManager& layerMgr = parentEditor->GetLayerManager();

	// Check current tool
	if(parentEditor->GetCurrentTool() == MapEditorApp::TOOL_BRUSH) {
		brush.SetMode(BRUSH_MODE_PAINT);
		brush.StartPainting(col, row, layerMgr);
		Refresh();
	}
	else if(parentEditor->GetCurrentTool() == MapEditorApp::TOOL_ERASER) {
		brush.SetMode(BRUSH_MODE_ERASE);
		brush.StartPainting(col, row, layerMgr);
		Refresh();
	}
	else if(parentEditor->GetCurrentTool() == MapEditorApp::TOOL_FILL) {
		FillTool& fill = parentEditor->GetFillTool();

		// Synchronize fill tile with brush tile
		fill.SetFillTile(brush.GetPaintTile());

		fill.Fill(col, row, layerMgr);
		Refresh();
	}
}

void MapCanvas::LeftUp(Point pos, dword flags) {
	if(!parentEditor) return;

	MapEditorApp::EditTool tool = parentEditor->GetCurrentTool();
	if(tool == MapEditorApp::TOOL_BRUSH || tool == MapEditorApp::TOOL_ERASER) {
		BrushTool& brush = parentEditor->GetBrushTool();
		brush.StopPainting();
	}
}

void MapCanvas::RightDown(Point pos, dword flags) {
	if(!parentEditor) return;

	// Right-click for quick erasing
	int tileSize = int(14 * zoom);
	if(tileSize <= 0) return;

	int col = (pos.x - offset.x) / tileSize;
	int row = (pos.y - offset.y) / tileSize;

	BrushTool& brush = parentEditor->GetBrushTool();
	LayerManager& layerMgr = parentEditor->GetLayerManager();

	brush.SetMode(BRUSH_MODE_ERASE);
	brush.StartPainting(col, row, layerMgr);
	Refresh();
}

void MapCanvas::RightUp(Point pos, dword flags) {
	if(!parentEditor) return;

	BrushTool& brush = parentEditor->GetBrushTool();
	brush.StopPainting();
	brush.SetMode(BRUSH_MODE_PAINT);  // Restore paint mode
}

void MapCanvas::MiddleDown(Point pos, dword flags) {
	panning = true;
	panStart = pos;
	SetCapture();
}

void MapCanvas::MiddleUp(Point pos, dword flags) {
	panning = false;
	ReleaseCapture();
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

void MapCanvas::ZoomToFit() {
	if(!parentEditor) return;

	Layer* layer = parentEditor->GetLayerManager().GetActiveLayer();
	if(!layer) return;

	const MapGrid& grid = layer->GetGrid();
	int mapCols = grid.GetMapCols();
	int mapRows = grid.GetMapRows();
	int tileSize = grid.GetGridSize();

	// Calculate zoom to fit map in canvas
	Size canvasSize = GetSize();
	double zoomX = double(canvasSize.cx) / (mapCols * tileSize);
	double zoomY = double(canvasSize.cy) / (mapRows * tileSize);

	zoom = min(zoomX, zoomY);
	offset = Point(0, 0);

	Refresh();
}

// Implementation of MapEditorApp
MapEditorApp::MapEditorApp() {
	Title("Umbrella Map Editor");
	Sizeable().Zoomable();
	SetRect(0, 0, 1400, 900);

	// Initialize tool
	currentTool = TOOL_BRUSH;

	AddFrame(mainMenuBar);
	AddFrame(mainToolBar);
	AddFrame(mainStatusBar);

	// Initialize layer manager with default 100x100 grid
	layerManager.InitializeDefaultLayers(100, 100);

	// Add some test tiles for visual verification
	Layer* terrain = layerManager.GetActiveLayer();
	if(terrain) {
		terrain->GetGrid().SetTile(5, 5, TILE_WALL);
		terrain->GetGrid().SetTile(6, 5, TILE_WALL);
		terrain->GetGrid().SetTile(7, 5, TILE_FULLBLOCK);
		terrain->GetGrid().SetTile(5, 6, TILE_BACKGROUND);
		terrain->GetGrid().SetTile(6, 6, TILE_BACKGROUND);
		terrain->GetGrid().SetTile(7, 6, TILE_WALL);
	}

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
	bar.Separator();
	bar.Add("Show Grid", [=] { mapCanvas.SetShowGrid(!mapCanvas.GetShowGrid()); })
		.Check(mapCanvas.GetShowGrid())
		.Key(K_G)
		.Help("Toggle grid visibility");
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
	mapCanvas.SetParentEditor(this);

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

		// Tool selection
		case K_B:  // B for Brush
			currentTool = TOOL_BRUSH;
			brushTool.SetMode(BRUSH_MODE_PAINT);
			mainStatusBar.Set("Tool: Brush");
			return true;
		case K_E:  // E for Eraser
			currentTool = TOOL_ERASER;
			brushTool.SetMode(BRUSH_MODE_ERASE);
			mainStatusBar.Set("Tool: Eraser");
			return true;

		// Brush size shortcuts
		case K_1:
			brushTool.SetBrushSize(BRUSH_1X1);
			mainStatusBar.Set("Brush size: 1x1");
			return true;
		case K_2:
			brushTool.SetBrushSize(BRUSH_2X2);
			mainStatusBar.Set("Brush size: 2x2");
			return true;
		case K_3:
			brushTool.SetBrushSize(BRUSH_3X3);
			mainStatusBar.Set("Brush size: 3x3");
			return true;
		case K_5:
			brushTool.SetBrushSize(BRUSH_5X5);
			mainStatusBar.Set("Brush size: 5x5");
			return true;

		// Tile type shortcuts
		case K_W:  // W for Wall
			brushTool.SetPaintTile(TILE_WALL);
			mainStatusBar.Set("Paint tile: Wall");
			return true;
		case K_G:  // G for backGround (note: conflicts with grid toggle)
			if(GetShift()) {  // Shift+G for background
				brushTool.SetPaintTile(TILE_BACKGROUND);
				mainStatusBar.Set("Paint tile: Background");
			} else {
				// Toggle grid (existing shortcut)
				mapCanvas.SetShowGrid(!mapCanvas.GetShowGrid());
			}
			return true;
		case K_F:  // F for Fullblock or Fill
			if(GetShift()) {  // Shift+F for Fill tool
				currentTool = TOOL_FILL;
				mainStatusBar.Set("Tool: Fill");
			} else {
				brushTool.SetPaintTile(TILE_FULLBLOCK);
				mainStatusBar.Set("Paint tile: FullBlock");
			}
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

	// Set initial directory to levels folder
	String levelsDir = GetFileFolder(GetExeFilePath()) + "/../share/mods/umbrella/levels";
	if(DirectoryExists(levelsDir)) {
		fs.BaseDir(levelsDir);
	}

	// File types
	fs.Type("JSON Level files", "*.json");
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
	if(MapSerializer::LoadFromFile(fileName, layerManager)) {
		currentFilePath = fileName;

		// Update window title
		Title("Umbrella Map Editor - " + GetFileName(fileName));

		// Refresh canvas
		mapCanvas.Refresh();

		// Zoom to fit new map
		mapCanvas.ZoomToFit();

		mainStatusBar.Set("Level loaded: " + GetFileName(fileName));
	}
	else {
		mainStatusBar.Set("Failed to load level: " + fileName);
	}
}

void MapEditorApp::SaveFile(const String& fileName) {
	PromptOK("Saving file: " + fileName);
	// TODO: Implementation for saving a file
}

// MapEditorApp is now available for use from main.cpp
// The GUI_APP_MAIN is in main.cpp, not here
