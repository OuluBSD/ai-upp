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
	showReferenceImage = false;
	referenceImageOpacity = 50;
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

	// Draw reference image if enabled
	if(showReferenceImage && !referenceImage.IsEmpty()) {
		// Scale reference image to fit canvas with current zoom
		Size imgSize = referenceImage.GetSize();
		int scaledWidth = int(imgSize.cx * zoom);
		int scaledHeight = int(imgSize.cy * zoom);

		// Apply opacity
		if(referenceImageOpacity < 100) {
			ImageDraw iw(imgSize);
			iw.DrawImage(0, 0, referenceImage);
			iw.Alpha().DrawRect(0, 0, imgSize.cx, imgSize.cy, GrayColor(referenceImageOpacity * 255 / 100));
			Image alphaImg = iw;
			w.DrawImage(offset.x, offset.y, scaledWidth, scaledHeight, alphaImg);
		} else {
			w.DrawImage(offset.x, offset.y, scaledWidth, scaledHeight, referenceImage);
		}
	}

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

			// Yellow outline (2px thick)
			w.DrawRect(screenX, screenY, tileSize, tileSize, Null);
			w.DrawLine(screenX, screenY, screenX + tileSize, screenY, 2, Yellow());
			w.DrawLine(screenX + tileSize, screenY, screenX + tileSize, screenY + tileSize, 2, Yellow());
			w.DrawLine(screenX + tileSize, screenY + tileSize, screenX, screenY + tileSize, 2, Yellow());
			w.DrawLine(screenX, screenY + tileSize, screenX, screenY, 2, Yellow());

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

					// Red outline (2px thick)
					w.DrawLine(screenX, screenY, screenX + tileSize, screenY, 2, LtRed());
					w.DrawLine(screenX + tileSize, screenY, screenX + tileSize, screenY + tileSize, 2, LtRed());
					w.DrawLine(screenX + tileSize, screenY + tileSize, screenX, screenY + tileSize, 2, LtRed());
					w.DrawLine(screenX, screenY + tileSize, screenX, screenY, 2, LtRed());
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

					// Draw outline (1px white)
					w.DrawRect(screenX, screenY, tileSize, tileSize, Null);
					w.DrawLine(screenX, screenY, screenX + tileSize, screenY, 1, White());
					w.DrawLine(screenX + tileSize - 1, screenY, screenX + tileSize - 1, screenY + tileSize, 1, White());
					w.DrawLine(screenX + tileSize - 1, screenY + tileSize - 1, screenX, screenY + tileSize - 1, 1, White());
					w.DrawLine(screenX, screenY + tileSize - 1, screenX, screenY, 1, White());
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

void MapCanvas::SetReferenceImage(const Image& img) {
	referenceImage = img;
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
	Sizeable().MaximizeBox();
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
	SetupToolsPanel();
	SetupLayersPanel();

	// Set up canvas
	mapCanvas.SetFrame(InsetFrame());
	mapCanvas.SetParentEditor(this);
}

MapEditorApp::MapEditorApp(const String& levelPath) : MapEditorApp() {
	// Load the level after initialization
	if(FileExists(levelPath)) {
		OpenFile(levelPath);
	} else {
		Exclamation("Level file not found: " + levelPath);
	}
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
	bar.Add("Show Reference Image", [=] { mapCanvas.SetShowReferenceImage(!mapCanvas.GetShowReferenceImage()); })
		.Check(mapCanvas.GetShowReferenceImage())
		.Key(K_R)
		.Help("Toggle reference image visibility");
	bar.Separator();
	bar.Add("Load Reference Image...", callback(this, &MapEditorApp::BrowseReferenceImage))
		.Key(K_CTRL_R)
		.Help("Load reference image for tracing");
}

void MapEditorApp::SetupToolBar() {
	// Set up the toolbar with common actions
	// Set icon size to 24x24 for better visibility
	mainToolBar.MaxIconSize(Size(24, 24));
	mainToolBar.ButtonMinSize(Size(24, 24));

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

void MapEditorApp::DockInit() {
	// Set up dockable panels using DockWindow API
	DockLeft(Dockable(toolsPanel, "Tools").SizeHint(Size(250, 400)));
	DockRight(Dockable(layersPanel, "Layers").SizeHint(Size(250, 300)));
	DockRight(Dockable(entitiesPanel, "Entities").SizeHint(Size(250, 300)));
	DockBottom(Dockable(propertiesPanel, "Properties").SizeHint(Size(400, 200)));

	// Add canvas to center (fills remaining space)
	Add(mapCanvas.SizePos());

	// Set initial status
	mainStatusBar.Set("Ready");
}

void MapEditorApp::SetupToolsPanel() {
	toolsPanel.SetFrame(InsetFrame());

	int yPos = 10;

	// Tools title
	toolsLabel.SetText("TOOLS");
	toolsLabel.SetAlign(ALIGN_CENTER);
	toolsPanel.Add(toolsLabel.HSizePos(10, 10).TopPos(yPos, 20));
	yPos += 30;

	// Brush size section
	brushSizeLabel.SetText("Brush Size:");
	toolsPanel.Add(brushSizeLabel.LeftPos(10, 100).TopPos(yPos, 20));
	yPos += 25;

	brush1x1Btn.SetLabel("1x1");
	brush1x1Btn << [=] {
		brushTool.SetBrushSize(BRUSH_1X1);
		mainStatusBar.Set("Brush size: 1x1");
	};
	toolsPanel.Add(brush1x1Btn.LeftPos(10, 50).TopPos(yPos, 25));

	brush2x2Btn.SetLabel("2x2");
	brush2x2Btn << [=] {
		brushTool.SetBrushSize(BRUSH_2X2);
		mainStatusBar.Set("Brush size: 2x2");
	};
	toolsPanel.Add(brush2x2Btn.LeftPos(70, 50).TopPos(yPos, 25));
	yPos += 30;

	brush3x3Btn.SetLabel("3x3");
	brush3x3Btn << [=] {
		brushTool.SetBrushSize(BRUSH_3X3);
		mainStatusBar.Set("Brush size: 3x3");
	};
	toolsPanel.Add(brush3x3Btn.LeftPos(10, 50).TopPos(yPos, 25));

	brush5x5Btn.SetLabel("5x5");
	brush5x5Btn << [=] {
		brushTool.SetBrushSize(BRUSH_5X5);
		mainStatusBar.Set("Brush size: 5x5");
	};
	toolsPanel.Add(brush5x5Btn.LeftPos(70, 50).TopPos(yPos, 25));
	yPos += 40;

	// Tile type section
	tileTypeLabel.SetText("Tile Type:");
	toolsPanel.Add(tileTypeLabel.LeftPos(10, 100).TopPos(yPos, 20));
	yPos += 25;

	wallBtn.SetLabel("Wall");
	wallBtn << [=] {
		brushTool.SetPaintTile(TILE_WALL);
		mainStatusBar.Set("Paint tile: Wall");
	};
	toolsPanel.Add(wallBtn.LeftPos(10, 110).TopPos(yPos, 25));
	yPos += 30;

	bgBtn.SetLabel("Background");
	bgBtn << [=] {
		brushTool.SetPaintTile(TILE_BACKGROUND);
		mainStatusBar.Set("Paint tile: Background");
	};
	toolsPanel.Add(bgBtn.LeftPos(10, 110).TopPos(yPos, 25));
	yPos += 30;

	blockBtn.SetLabel("Full Block");
	blockBtn << [=] {
		brushTool.SetPaintTile(TILE_FULLBLOCK);
		mainStatusBar.Set("Paint tile: Full Block");
	};
	toolsPanel.Add(blockBtn.LeftPos(10, 110).TopPos(yPos, 25));
	yPos += 40;

	// Tool selection section
	toolSelectionLabel.SetText("Tool:");
	toolsPanel.Add(toolSelectionLabel.LeftPos(10, 100).TopPos(yPos, 20));
	yPos += 25;

	brushToolBtn.SetLabel("Brush");
	brushToolBtn << [=] {
		currentTool = TOOL_BRUSH;
		brushTool.SetMode(BRUSH_MODE_PAINT);
		mainStatusBar.Set("Tool: Brush");
	};
	toolsPanel.Add(brushToolBtn.LeftPos(10, 110).TopPos(yPos, 25));
	yPos += 30;

	eraserToolBtn.SetLabel("Eraser");
	eraserToolBtn << [=] {
		currentTool = TOOL_ERASER;
		brushTool.SetMode(BRUSH_MODE_ERASE);
		mainStatusBar.Set("Tool: Eraser");
	};
	toolsPanel.Add(eraserToolBtn.LeftPos(10, 110).TopPos(yPos, 25));
	yPos += 30;

	fillToolBtn.SetLabel("Fill");
	fillToolBtn << [=] {
		currentTool = TOOL_FILL;
		mainStatusBar.Set("Tool: Fill");
	};
	toolsPanel.Add(fillToolBtn.LeftPos(10, 110).TopPos(yPos, 25));
}

void MapEditorApp::SetupLayersPanel() {
	layersPanel.SetFrame(InsetFrame());

	int yPos = 10;

	// Layers list
	layersList.AddColumn("Layer", 150);
	layersList.AddColumn("Visible", 50);
	layersList.NoHeader();
	layersPanel.Add(layersList.HSizePos(10, 10).TopPos(yPos, 200));
	yPos += 210;

	// Layer management buttons
	addLayerBtn.SetLabel("Add Layer");
	addLayerBtn << [=] {
		// TODO: Implement add layer
		PromptOK("Add layer functionality not yet implemented");
	};
	layersPanel.Add(addLayerBtn.LeftPos(10, 100).TopPos(yPos, 25));

	removeLayerBtn.SetLabel("Remove");
	removeLayerBtn << [=] {
		// TODO: Implement remove layer
		PromptOK("Remove layer functionality not yet implemented");
	};
	layersPanel.Add(removeLayerBtn.LeftPos(120, 100).TopPos(yPos, 25));
	yPos += 30;

	moveLayerUpBtn.SetLabel("Move Up");
	moveLayerUpBtn << [=] {
		// TODO: Implement move layer up
		PromptOK("Move layer up functionality not yet implemented");
	};
	layersPanel.Add(moveLayerUpBtn.LeftPos(10, 100).TopPos(yPos, 25));

	moveLayerDownBtn.SetLabel("Move Down");
	moveLayerDownBtn << [=] {
		// TODO: Implement move layer down
		PromptOK("Move layer down functionality not yet implemented");
	};
	layersPanel.Add(moveLayerDownBtn.LeftPos(120, 100).TopPos(yPos, 25));
	yPos += 40;

	// Layer opacity slider
	layerOpacityLabel.SetText("Opacity: 100%");
	layersPanel.Add(layerOpacityLabel.LeftPos(10, 100).TopPos(yPos, 20));
	yPos += 25;

	layerOpacitySlider.Range(100);
	layerOpacitySlider.MinMax(0, 100);
	layerOpacitySlider << [=] {
		int opacity = (int)layerOpacitySlider.GetData();
		layerOpacityLabel.SetText(Format("Opacity: %d%%", opacity));

		Layer* activeLayer = layerManager.GetActiveLayer();
		if(activeLayer) {
			activeLayer->SetOpacity(opacity);
			mapCanvas.Refresh();
		}
	};
	layersPanel.Add(layerOpacitySlider.HSizePos(10, 10).TopPos(yPos, 20));

	// Populate layers list
	RefreshLayersList();
}

void MapEditorApp::RefreshLayersList() {
	layersList.Clear();

	for(int i = 0; i < layerManager.GetLayerCount(); i++) {
		const Layer& layer = layerManager.GetLayer(i);
		layersList.Add(layer.GetName(), layer.IsVisible() ? "Yes" : "No");
	}

	// Select active layer
	int activeIndex = layerManager.GetActiveLayerIndex();
	if(activeIndex >= 0 && activeIndex < layersList.GetCount()) {
		layersList.SetCursor(activeIndex);
	}
}

bool MapEditorApp::Key(dword key, int) {
	switch(key) {
		case K_ESCAPE:
			Break();
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
		case K_R:  // R for Reference image toggle
			mapCanvas.SetShowReferenceImage(!mapCanvas.GetShowReferenceImage());
			mainStatusBar.Set(mapCanvas.GetShowReferenceImage() ? "Reference image: ON" : "Reference image: OFF");
			return true;
		case K_CTRL_R:  // Ctrl+R to load reference image
			BrowseReferenceImage();
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
	Break();
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

void MapEditorApp::LoadReferenceImage(const String& imagePath) {
	Image img = StreamRaster::LoadFileAny(imagePath);
	if(img.IsEmpty()) {
		Exclamation("Failed to load image: " + imagePath);
		return;
	}

	referenceImagePath = imagePath;
	mapCanvas.SetReferenceImage(img);
	mapCanvas.SetShowReferenceImage(true);
	mainStatusBar.Set("Reference image loaded: " + GetFileName(imagePath));
}

void MapEditorApp::BrowseReferenceImage() {
	FileSel fs;

	// Set initial directory to proprietary maps
	String mapsDir = GetFileFolder(GetExeFilePath()) + "/../share/mods/umbrella/proprietary/maps";
	if(DirectoryExists(mapsDir)) {
		fs.BaseDir(mapsDir);
	}

	// File types
	fs.Type("Image files", "*.jpg *.jpeg *.png");
	fs.AllFilesType();

	if(fs.ExecuteOpen("Open Reference Image")) {
		LoadReferenceImage(fs.Get());
	}
}

// MapEditorApp is now available for use from main.cpp
// The GUI_APP_MAIN is in main.cpp, not here
