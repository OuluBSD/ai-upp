# MapEditorScreen Conversion Plan

## Overview
Convert the RainbowGame MapEditorScreen.java from libGDX to U++ CtrlLib widgets as part of the Umbrella game project.

## Current State Analysis
- File: `/common/active/sblo/Dev/RainbowGame/trash/core/src/main/java/com/rainbowgame/editor/MapEditorScreen.java`
- Uses libGDX framework extensively:
  - ScreenAdapter for screen management
  - Scene2D UI components (Stage, Table, ScrollPane, SelectBox, TextField, etc.)
  - Graphics components (SpriteBatch, ShapeRenderer, OrthographicCamera)
  - Input handling (InputAdapter, InputMultiplexer)

## Conversion Strategy
The conversion will be done in phases:

### Phase 1: XML GUI Design
- Create XML layout for the Map Editor interface
- Define all UI elements that need to be converted from libGDX Scene2D to U++ CtrlLib
- Map libGDX UI components to U++ equivalents:
  - Stage → Top-level Ctrl container
  - Table → U++ TableCtrl or Layout
  - TextField → U++ EditField
  - SelectBox → U++ DropList
  - ScrollPane → U++ ScrollBar integration
  - Button → U++ Button
  - Label → U++ StaticText

### Phase 2: Widget Mapping
- Create U++ widget classes that replicate the functionality of libGDX components
- Implement event handling systems equivalent to libGDX's listener system
- Map libGDX coordinate systems to U++ coordinate systems

### Phase 3: Implementation
- Implement the U++ GUI based on the XML design
- Integrate with U++ drawing system using Ctrl::Paint(Draw&) override
- Replace libGDX rendering with U++ Draw methods
- Replace libGDX input handling with U++ Ctrl::Key(...) override
- Integrate with U++ Sound system for audio

## Key Challenges
1. libGDX Scene2D UI system is complex with many nested containers
2. Coordinate system differences between libGDX and U++
3. Event handling differences
4. Graphics rendering differences (libGDX vs U++ Draw)

## Success Criteria
- All UI elements from MapEditorScreen replicated in U++
- Equivalent functionality maintained
- Proper integration with U++ drawing and input systems
- Performance comparable to original