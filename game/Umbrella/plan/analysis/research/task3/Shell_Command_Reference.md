# Shell Command Reference Task

## Overview
Keep track of important shell commands for exploring and analyzing the RainbowGame project structure.

## Useful Commands

### Find all Java files in core
```bash
find /common/active/sblo/Dev/RainbowGame/trash/core/src/main/java -name "*.java"
```

### Explore desktop directory structure
```bash
find /common/active/sblo/Dev/RainbowGame/trash/desktop
```

### View the main entry point
```bash
cat /common/active/sblo/Dev/RainbowGame/trash/desktop/src/main/java/com/rainbowgame/desktop/DesktopLauncher.java
```

### View the main game class
```bash
head -20 /common/active/sblo/Dev/RainbowGame/trash/core/src/main/java/com/rainbowgame/RainbowGame.java
```

### View the important MapEditorScreen
```bash
head -20 /common/active/sblo/Dev/RainbowGame/trash/core/src/main/java/com/rainbowgame/editor/MapEditorScreen.java
```

### Check build script
```bash
cat /common/active/sblo/Dev/RainbowGame/trash/build.sh
```

### Build the original project
```bash
cd /common/active/sblo/Dev/RainbowGame/trash && ./gradlew desktop:build
```

## Purpose
These commands are useful for:
- Understanding the original RainbowGame project structure
- Identifying files that need to be converted
- Validating the original functionality during conversion
- Checking build outputs for comparison with U++ implementation