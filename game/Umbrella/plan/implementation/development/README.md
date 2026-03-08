# Development Tasks for Umbrella Game

This directory contains development tasks for converting RainbowGame from libGDX to U++.

## Shell Commands to Explore RainbowGame

```bash
find /common/active/sblo/Dev/RainbowGame/trash/core/src/main/java -name "*.java"
find /common/active/sblo/Dev/RainbowGame/trash/desktop
cat /common/active/sblo/Dev/RainbowGame/trash/desktop/src/main/java/com/rainbowgame/desktop/DesktopLauncher.java
head -20 /common/active/sblo/Dev/RainbowGame/trash/core/src/main/java/com/rainbowgame/RainbowGame.java
head -20 /common/active/sblo/Dev/RainbowGame/trash/core/src/main/java/com/rainbowgame/editor/MapEditorScreen.java
```

## Task Overview

- `task1/Java_Files_Conversion_Plan.md` - High-level plan for converting all Java files
- `task2/Main_CPP_Implementation.md` - Implementation of main.cpp based on DesktopLauncher.java
- `task5/Build_Success_Documentation.md` - Documentation of successful build
- `task6/Java_Files_Conversion_Tracking.md` - Detailed tracking of all Java files to convert
- `task7/RainbowGame_Conversion_Task.md` - Specific task for main game class conversion
- `task8/MapEditorScreen_Conversion_Task.md` - Specific task for complex editor screen conversion (high priority)

## Priority Order

1. **High Priority**: MapEditorScreen conversion (task8) - complex UI with special focus
2. **Medium Priority**: RainbowGame main class conversion (task7) - core game functionality
3. **Low Priority**: Other Java files conversion (task6) - remaining components

## Status

- Basic project structure created and building successfully
- Main application entry point implemented
- Comprehensive plan created for Java file conversions
- Special focus task created for MapEditorScreen.java