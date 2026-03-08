# Task B7: Undo/Redo System Implementation

## Priority: HIGH - Essential for Editing

## Overview
Implement undo/redo functionality using the Command pattern. Allows users to undo mistakes and redo operations.

## Dependencies
- **Task A1** (Grid/Tile model)
- **Task A2** (Layer system)
- **Task B1** (Brush tool)

## Command Pattern Design

### Core Concept
Each editing operation creates a Command object that can be executed, undone, and redone.

## Files to Create

### 1. EditorCommand.h
**Location**: `game/Umbrella/EditorCommand.h`

```cpp
// Base class for all editor commands
class EditorCommand {
public:
    virtual ~EditorCommand() {}

    // Execute the command
    virtual void Execute() = 0;

    // Undo the command
    virtual void Undo() = 0;

    // Get description for UI
    virtual String GetDescription() const = 0;
};

// Command for painting tiles
class PaintCommand : public EditorCommand {
private:
    LayerManager* layerMgr;
    int layerIndex;
    Vector<Point> positions;  // Tiles that were painted
    Vector<TileType> oldTiles;  // Original tile types
    TileType newTile;  // Tile type painted

public:
    PaintCommand(LayerManager* mgr, int layer, const Vector<Point>& pos, TileType tile);

    void Execute() override;
    void Undo() override;
    String GetDescription() const override;
};

// Command for erasing tiles
class EraseCommand : public EditorCommand {
private:
    LayerManager* layerMgr;
    int layerIndex;
    Vector<Point> positions;
    Vector<TileType> oldTiles;

public:
    EraseCommand(LayerManager* mgr, int layer, const Vector<Point>& pos);

    void Execute() override;
    void Undo() override;
    String GetDescription() const override;
};

// Command for fill operation
class FillCommand : public EditorCommand {
private:
    LayerManager* layerMgr;
    int layerIndex;
    Vector<Point> filledTiles;
    Vector<TileType> oldTiles;
    TileType newTile;

public:
    FillCommand(LayerManager* mgr, int layer, const Vector<Point>& tiles,
                const Vector<TileType>& old, TileType fill);

    void Execute() override;
    void Undo() override;
    String GetDescription() const override;
};

// Command for layer operations
class AddLayerCommand : public EditorCommand {
private:
    LayerManager* layerMgr;
    String layerName;
    LayerType layerType;
    int addedIndex;

public:
    AddLayerCommand(LayerManager* mgr, const String& name, LayerType type);

    void Execute() override;
    void Undo() override;
    String GetDescription() const override;
};

class RemoveLayerCommand : public EditorCommand {
private:
    LayerManager* layerMgr;
    int layerIndex;
    Layer* removedLayer;  // Keep for redo

public:
    RemoveLayerCommand(LayerManager* mgr, int index);
    ~RemoveLayerCommand();

    void Execute() override;
    void Undo() override;
    String GetDescription() const override;
};
```

---

### 2. UndoStack.h
**Location**: `game/Umbrella/UndoStack.h`

```cpp
class UndoStack {
private:
    Vector<EditorCommand*> undoStack;
    Vector<EditorCommand*> redoStack;
    int maxStackSize;

public:
    UndoStack(int maxSize = 100);
    ~UndoStack();

    // Execute and push command
    void Push(EditorCommand* command);

    // Undo/Redo operations
    void Undo();
    void Redo();

    // Query state
    bool CanUndo() const { return undoStack.GetCount() > 0; }
    bool CanRedo() const { return redoStack.GetCount() > 0; }

    String GetUndoDescription() const;
    String GetRedoDescription() const;

    // Clear stacks
    void Clear();

private:
    void ClearRedoStack();
    void LimitStackSize();
};
```

---

### 3. EditorCommand.cpp

```cpp
#include "EditorCommand.h"

// ===== PaintCommand =====

PaintCommand::PaintCommand(LayerManager* mgr, int layer,
                           const Vector<Point>& pos, TileType tile)
    : layerMgr(mgr), layerIndex(layer), positions(pos), newTile(tile) {

    // Store old tiles for undo
    Layer* targetLayer = layerMgr->GetLayer(layerIndex);
    if(targetLayer) {
        MapGrid& grid = targetLayer->GetGrid();
        for(const Point& pt : positions) {
            oldTiles.Add(grid.GetTile(pt.x, pt.y));
        }
    }
}

void PaintCommand::Execute() {
    Layer* targetLayer = layerMgr->GetLayer(layerIndex);
    if(!targetLayer) return;

    MapGrid& grid = targetLayer->GetGrid();
    for(const Point& pt : positions) {
        grid.SetTile(pt.x, pt.y, newTile);
    }
}

void PaintCommand::Undo() {
    Layer* targetLayer = layerMgr->GetLayer(layerIndex);
    if(!targetLayer) return;

    MapGrid& grid = targetLayer->GetGrid();
    for(int i = 0; i < positions.GetCount(); i++) {
        const Point& pt = positions[i];
        grid.SetTile(pt.x, pt.y, oldTiles[i]);
    }
}

String PaintCommand::GetDescription() const {
    return Format("Paint %d tile(s)", positions.GetCount());
}

// ===== EraseCommand =====

EraseCommand::EraseCommand(LayerManager* mgr, int layer, const Vector<Point>& pos)
    : layerMgr(mgr), layerIndex(layer), positions(pos) {

    Layer* targetLayer = layerMgr->GetLayer(layerIndex);
    if(targetLayer) {
        MapGrid& grid = targetLayer->GetGrid();
        for(const Point& pt : positions) {
            oldTiles.Add(grid.GetTile(pt.x, pt.y));
        }
    }
}

void EraseCommand::Execute() {
    Layer* targetLayer = layerMgr->GetLayer(layerIndex);
    if(!targetLayer) return;

    MapGrid& grid = targetLayer->GetGrid();
    for(const Point& pt : positions) {
        grid.SetTile(pt.x, pt.y, TILE_EMPTY);
    }
}

void EraseCommand::Undo() {
    Layer* targetLayer = layerMgr->GetLayer(layerIndex);
    if(!targetLayer) return;

    MapGrid& grid = targetLayer->GetGrid();
    for(int i = 0; i < positions.GetCount(); i++) {
        const Point& pt = positions[i];
        grid.SetTile(pt.x, pt.y, oldTiles[i]);
    }
}

String EraseCommand::GetDescription() const {
    return Format("Erase %d tile(s)", positions.GetCount());
}

// ===== FillCommand =====

FillCommand::FillCommand(LayerManager* mgr, int layer,
                         const Vector<Point>& tiles,
                         const Vector<TileType>& old, TileType fill)
    : layerMgr(mgr), layerIndex(layer), filledTiles(tiles),
      oldTiles(old), newTile(fill) {
}

void FillCommand::Execute() {
    Layer* targetLayer = layerMgr->GetLayer(layerIndex);
    if(!targetLayer) return;

    MapGrid& grid = targetLayer->GetGrid();
    for(const Point& pt : filledTiles) {
        grid.SetTile(pt.x, pt.y, newTile);
    }
}

void FillCommand::Undo() {
    Layer* targetLayer = layerMgr->GetLayer(layerIndex);
    if(!targetLayer) return;

    MapGrid& grid = targetLayer->GetGrid();
    for(int i = 0; i < filledTiles.GetCount(); i++) {
        const Point& pt = filledTiles[i];
        grid.SetTile(pt.x, pt.y, oldTiles[i]);
    }
}

String FillCommand::GetDescription() const {
    return Format("Fill %d tile(s)", filledTiles.GetCount());
}

// ===== AddLayerCommand =====

AddLayerCommand::AddLayerCommand(LayerManager* mgr, const String& name, LayerType type)
    : layerMgr(mgr), layerName(name), layerType(type), addedIndex(-1) {
}

void AddLayerCommand::Execute() {
    Layer* layer = layerMgr->AddLayer(layerName, layerType);
    addedIndex = layerMgr->GetLayerCount() - 1;
}

void AddLayerCommand::Undo() {
    if(addedIndex >= 0) {
        layerMgr->RemoveLayer(addedIndex);
    }
}

String AddLayerCommand::GetDescription() const {
    return Format("Add Layer: %s", layerName);
}

// ===== RemoveLayerCommand =====

RemoveLayerCommand::RemoveLayerCommand(LayerManager* mgr, int index)
    : layerMgr(mgr), layerIndex(index), removedLayer(nullptr) {
}

RemoveLayerCommand::~RemoveLayerCommand() {
    if(removedLayer) {
        delete removedLayer;
    }
}

void RemoveLayerCommand::Execute() {
    // Store layer before removing
    Layer* layer = layerMgr->GetLayer(layerIndex);
    if(layer) {
        removedLayer = layer;  // Take ownership
        layerMgr->RemoveLayer(layerIndex);
    }
}

void RemoveLayerCommand::Undo() {
    if(removedLayer) {
        // Re-add layer (TODO: Add insert-at-index method to LayerManager)
        layerMgr->AddLayer(removedLayer->GetName(), removedLayer->GetType());
        // Copy grid data back...
    }
}

String RemoveLayerCommand::GetDescription() const {
    return "Remove Layer";
}
```

---

### 4. UndoStack.cpp

```cpp
#include "UndoStack.h"

UndoStack::UndoStack(int maxSize)
    : maxStackSize(maxSize) {
}

UndoStack::~UndoStack() {
    Clear();
}

void UndoStack::Push(EditorCommand* command) {
    // Execute command
    command->Execute();

    // Push to undo stack
    undoStack.Add(command);

    // Clear redo stack (can't redo after new action)
    ClearRedoStack();

    // Limit stack size
    LimitStackSize();
}

void UndoStack::Undo() {
    if(!CanUndo()) return;

    // Pop command from undo stack
    EditorCommand* command = undoStack.Pop();

    // Undo it
    command->Undo();

    // Push to redo stack
    redoStack.Add(command);
}

void UndoStack::Redo() {
    if(!CanRedo()) return;

    // Pop command from redo stack
    EditorCommand* command = redoStack.Pop();

    // Redo it (execute again)
    command->Execute();

    // Push back to undo stack
    undoStack.Add(command);
}

String UndoStack::GetUndoDescription() const {
    if(!CanUndo()) return String();
    return undoStack.Top()->GetDescription();
}

String UndoStack::GetRedoDescription() const {
    if(!CanRedo()) return String();
    return redoStack.Top()->GetDescription();
}

void UndoStack::Clear() {
    for(EditorCommand* cmd : undoStack) {
        delete cmd;
    }
    undoStack.Clear();

    ClearRedoStack();
}

void UndoStack::ClearRedoStack() {
    for(EditorCommand* cmd : redoStack) {
        delete cmd;
    }
    redoStack.Clear();
}

void UndoStack::LimitStackSize() {
    while(undoStack.GetCount() > maxStackSize) {
        EditorCommand* cmd = undoStack[0];
        delete cmd;
        undoStack.Remove(0);
    }
}
```

---

## Integration with Tools

### Update BrushTool to Record Commands

Modify `BrushTool` to accumulate painted tiles:
```cpp
class BrushTool {
private:
    Vector<Point> currentStroke;  // Tiles painted in current stroke

public:
    void StartPainting(int col, int row, LayerManager& layerMgr, UndoStack& undo);
    void ContinuePainting(int col, int row, LayerManager& layerMgr);
    void StopPainting(UndoStack& undo);
};

void BrushTool::StartPainting(..., UndoStack& undo) {
    currentStroke.Clear();
    // ... existing paint code ...
    currentStroke.Add(Point(col, row));
}

void BrushTool::ContinuePainting(...) {
    // ... existing paint code ...
    Vector<Point> brushTiles;
    GetBrushTiles(col, row, brushTiles);
    currentStroke.Append(brushTiles);
}

void BrushTool::StopPainting(UndoStack& undo) {
    if(currentStroke.GetCount() > 0) {
        int layerIndex = layerMgr->GetActiveLayerIndex();
        PaintCommand* cmd = new PaintCommand(layerMgr, layerIndex,
                                             currentStroke, paintTile);
        undo.Push(cmd);
        currentStroke.Clear();
    }
}
```

---

## Integration with MapEditor

### Update MapEditor.h

Add undo stack:
```cpp
class MapEditorApp : public TopWindow {
private:
    UndoStack undoStack;

public:
    UndoStack& GetUndoStack() { return undoStack; }
```

---

### Update Undo/Redo Actions

```cpp
void MapEditorApp::UndoAction() {
    if(undoStack.CanUndo()) {
        undoStack.Undo();
        mapCanvas.Refresh();
        UpdateUndoRedoButtons();
    }
}

void MapEditorApp::RedoAction() {
    if(undoStack.CanRedo()) {
        undoStack.Redo();
        mapCanvas.Refresh();
        UpdateUndoRedoButtons();
    }
}

void MapEditorApp::UpdateUndoRedoButtons() {
    undoBtn.Enable(undoStack.CanUndo());
    redoBtn.Enable(undoStack.CanRedo());

    // Update tooltip with description
    if(undoStack.CanUndo()) {
        undoBtn.Tip("Undo: " + undoStack.GetUndoDescription());
    }
    if(undoStack.CanRedo()) {
        redoBtn.Tip("Redo: " + undoStack.GetRedoDescription());
    }
}
```

---

## Testing Requirements

### Manual Tests

1. **Basic Undo/Redo**:
   - Paint 5 tiles
   - Press Ctrl+Z (undo)
   - Verify tiles removed
   - Press Ctrl+Y (redo)
   - Verify tiles reappear

2. **Multiple Undo**:
   - Paint 3 separate strokes
   - Press Ctrl+Z three times
   - Verify each stroke undone separately

3. **Undo After Fill**:
   - Fill large area (100+ tiles)
   - Press Ctrl+Z
   - Verify entire fill undone at once

4. **Redo Stack Clear**:
   - Paint, undo, redo
   - Paint new stroke
   - Try to redo
   - Verify can't redo (stack cleared)

5. **Stack Limit**:
   - Paint 150 separate tiles (exceeds 100 limit)
   - Try to undo 150 times
   - Verify stops at 100 (oldest discarded)

6. **Layer Operations**:
   - Add new layer (undo should remove it)
   - Remove layer (undo should restore it)

---

## Success Criteria

- [x] EditorCommand.h/cpp created
- [x] UndoStack.h/cpp created
- [x] PaintCommand, EraseCommand, FillCommand implemented
- [x] Undo (Ctrl+Z) works
- [x] Redo (Ctrl+Y) works
- [x] Undo/Redo buttons enable/disable correctly
- [x] Tooltips show command descriptions
- [x] Stack size limit works (100 commands)
- [x] Redo stack clears on new action
- [x] Project builds and runs

---

## Next Tasks After Completion

- **Task B8**: Implement undo for entity operations
- **Task C3**: Implement undo for layer operations

---

## Estimated Effort

**Time**: 4-5 hours

**Complexity**: Medium-High (Command pattern, pointer management)

**Dependencies**: Task A1, A2, B1
