# Umbrella Game Project - Continuation Guide

## Current Status
I have created the track/phase/task plan architecture for the Umbrella game project in `./game/Umbrella/plan/` and implemented basic stubs for the MapEditor functionality. The project builds with some errors related to the MapEditor implementation that need to be fixed.

## Completed Tasks
1. Created the plan directory structure with tracks, phases, and tasks
2. Created documentation files for various components
3. Created a basic MapEditor.cpp stub implementation
4. Updated the package manifest to include the new file
5. Created multiple task files for different aspects of the conversion
6. Made git commits for the work completed

## Current Issues
The current build has errors in the MapEditor.cpp file:
- Unknown type name 'Point2D' - should use U++ Point or Pointf
- Menu bar and toolbar API usage issues
- Method name mismatches in callback functions
- Incorrect class member access

## Next Steps Required

### 1. Fix MapEditor.cpp Implementation
- Replace 'Point2D' with appropriate U++ type (Point or Pointf)
- Fix MenuBar and ToolBar API usage according to U++ documentation
- Correct the callback method names to match the class methods
- Ensure proper inheritance and method overrides

### 2. Complete the MapEditor Implementation
- Implement the actual XML GUI design for the MapEditorScreen
- Create the proper layout with splitters, panels, and controls
- Implement the canvas drawing functionality
- Add proper event handling for map editing

### 3. Create Additional Conversion Tasks
- Continue creating individual tasks for each Java file in the RainbowGame project
- Focus on core gameplay components like Player, Enemy, LevelLoader, etc.
- Implement the conversion strategy for replacing libGDX dependencies with U++ equivalents

### 4. Implement Core Systems
- Audio system using uppsrc/Sound
- Input handling using Ctrl::Key(...) override
- Drawing using Ctrl::Paint(Draw&) override
- Entity management system

## Key Requirements to Remember
- Use Ctrl::Paint(Draw&) override for drawing (not libGDX rendering)
- Use Ctrl::Key(...) override for keyboard input (not libGDX input)
- Use uppsrc/Sound for audio functionality (not libGDX audio)
- Focus on MapEditorScreen.java conversion which was specified as important
- Use ShareDirFile for locating assets in the proper directory structure

## Files to Work On
- `/common/active/sblo/Dev/ai-upp/game/Umbrella/MapEditor.cpp` - Fix the implementation
- Continue creating task files in the plan directory for remaining Java files
- Implement the actual functionality for the map editor

## Build Command
Use `script/build.py -j12 -mc 0 Umbrella` to build the project.

## Git Status
Last commit message: "Add detailed tasks for MapEditorScreen conversion phases and additional Java file conversions"