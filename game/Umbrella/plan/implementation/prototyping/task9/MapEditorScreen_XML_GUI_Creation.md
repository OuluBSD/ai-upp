# MapEditorScreen XML GUI Creation Task

## Overview
Create XML-based GUI design for the MapEditorScreen as part of the Umbrella project conversion.

## Shell Command to View Original
```bash
head -20 /common/active/sblo/Dev/RainbowGame/trash/core/src/main/java/com/rainbowgame/editor/MapEditorScreen.java
```

## Objective
Design the XML layout for the Map Editor interface to replicate the functionality of libGDX Scene2D UI components in U++ CtrlLib.

## Components to Design
- Main window layout with menu bar, toolbar, and canvas area
- Layer management panel
- Tool selection panel
- Properties panel
- Minimap display
- Grid controls
- Brush settings panel
- Entity palette

## Design Elements
- Define all UI elements that need to be converted from libGDX Scene2D to U++ CtrlLib
- Map libGDX UI components to U++ equivalents:
  - Stage → Top-level Ctrl container
  - Table → U++ TableCtrl or Layout
  - TextField → U++ EditField
  - SelectBox → U++ DropList
  - ScrollPane → U++ ScrollBar integration
  - Button → U++ Button
  - Label → U++ StaticText

## Reference
- Use examples from `reference/` directory for XML GUI patterns
- Follow U++ GUI design conventions

## Deliverables
- XML layout file for the Map Editor interface
- Component specifications for each UI element
- Layout mockup showing the arrangement of panels and controls