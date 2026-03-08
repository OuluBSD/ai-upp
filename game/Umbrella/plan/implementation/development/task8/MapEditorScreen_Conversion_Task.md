# MapEditorScreen.java Conversion Task

## Overview
Convert the complex MapEditorScreen.java file to U++ equivalent as part of the Umbrella project. This is a high-priority task due to the file's complexity and importance.

## Shell Command to View File
```bash
head -20 /common/active/sblo/Dev/RainbowGame/trash/core/src/main/java/com/rainbowgame/editor/MapEditorScreen.java
```

## Original File
- Location: `/common/active/sblo/Dev/RainbowGame/trash/core/src/main/java/com/rainbowgame/editor/MapEditorScreen.java`

## Purpose
This is the main map editor interface with complex UI and functionality for creating and editing game levels.

## Key Responsibilities
1. Provides the main map editing interface
2. Manages multiple layers (terrain, entities, annotations, etc.)
3. Implements various painting tools (brushes, erasers, etc.)
4. Handles grid-based level editing
5. Manages editor state and modes
6. Provides UI controls for editing parameters
7. Implements minimap and preview functionality

## Conversion Strategy
1. Create XML-based GUI design for the editor interface
2. Map libGDX Scene2D UI components to U++ CtrlLib widgets:
   - Stage → Top-level Ctrl container
   - Table → U++ TableCtrl or layout system
   - TextField → U++ EditField
   - SelectBox → U++ DropList
   - ScrollPane → U++ ScrollBar integration
   - Button → U++ Button
   - Label → U++ StaticText
3. Implement custom drawing for the map canvas using Ctrl::Paint(Draw&)
4. Implement input handling for mouse interactions on the map
5. Create layer management system
6. Implement tool selection and functionality

## Implementation Plan
### Phase 1: XML GUI Design
- Design the overall layout of the editor interface
- Define all UI elements that need to be converted
- Plan the arrangement of toolbars, panels, and canvas

### Phase 2: Widget Mapping
- Create U++ widget classes that replicate libGDX Scene2D functionality
- Implement the UI framework components
- Connect UI events to backend functionality

### Phase 3: Canvas Implementation
- Create the main map canvas using U++ drawing system
- Implement grid rendering
- Add mouse interaction for map editing
- Implement zoom and pan functionality

### Phase 4: Tool Implementation
- Implement brush and painting tools
- Add selection and manipulation tools
- Create layer management functionality

### Phase 5: Integration
- Connect the editor to the level loading/saving system
- Integrate with entity management
- Test the complete editing workflow

## Challenges
1. Complex UI with multiple panels and controls
2. Sophisticated drawing and interaction system
3. Layer management system
4. Grid-based coordinate system
5. Performance considerations for large maps

## Dependencies
- Depends on level loading/saving systems
- Works with entity management
- Interacts with texture/resource systems
- May depend on other editor components

## Success Criteria
- All UI elements from MapEditorScreen replicated in U++
- Equivalent functionality maintained
- Smooth performance for map editing
- Proper integration with U++ drawing and input systems