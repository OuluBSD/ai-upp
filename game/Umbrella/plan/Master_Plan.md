# Umbrella Game Project - Master Plan

## Project Overview
Convert the RainbowGame/libGDX project to a U++-based game framework as outlined in the requirements.

## Goals
1. Convert RainbowGame from libGDX to U++ framework
2. Replace libGDX dependencies with U++ equivalents
3. Use U++ CtrlLib widgets for UI components
4. Implement drawing via Ctrl::Paint(Draw&) override
5. Implement input via Ctrl::Key(...) override
6. Use uppsrc/Sound for audio functionality
7. Focus on MapEditorScreen.java conversion as a complex example

## Key Requirements
- RainbowGame uses `./gradlew desktop:build` command (build.sh)
- Special focus on MapEditorScreen.java which is quite large and complex
- Replace libGDX acceleration with U++ drawing system
- Use U++ CtrlLib widgets instead of libGDX Scene2D
- Implement simple drawing with Ctrl::Paint(Draw&) override
- Use U++ sound system instead of libGDX audio
- Use Ctrl::Key(...) override for keyboard input

## Project Structure
```
game/Umbrella/
├── plan/                     # Planning documents (this structure)
│   ├── analysis/
│   ├── design/
│   ├── implementation/
│   ├── testing/
│   └── deployment/
├── src/                      # U++ source code
├── res/                      # Resources
└── Umbrella.upp             # U++ package manifest
```

## Work Breakdown

### Track 1: Analysis & Discovery
- Analyzed RainbowGame build outputs
- Identified all Java files requiring conversion
- Documented libGDX dependencies

### Track 2: Design & Planning
- Created conversion strategy for libGDX to U++
- Designed XML GUI approach for UI components
- Planned widget mapping from libGDX to U++ CtrlLib
- Created detailed plan for MapEditorScreen conversion

### Track 3: Implementation
- Phase 1: Infrastructure setup
- Phase 2: Graphics foundation
- Phase 3: UI framework conversion
- Phase 4: Game logic conversion
- Phase 5: Special components (MapEditorScreen)

### Track 4: Testing
- Unit testing of converted components
- Integration testing
- Performance validation
- Functional parity verification

### Track 5: Deployment
- U++ build system integration
- Distribution packaging
- Documentation and handoff

## Critical Path
1. MapEditorScreen.java conversion (most complex component)
2. Graphics system replacement
3. UI system conversion
4. Input system replacement

## Success Metrics
- All RainbowGame functionality preserved in U++ version
- Performance comparable to original
- Code follows U++ conventions
- Build system properly integrated
- All Java files converted to U++ equivalents

## Timeline
- Phase 1-3: 2 weeks
- Phase 4: 3 weeks
- Phase 5: 2 weeks
- Testing: 1 week

## Risks
- Complexity of MapEditorScreen.java conversion
- Differences between libGDX and U++ coordinate systems
- UI layout challenges when converting from Scene2D to CtrlLib
- Potential performance differences in rendering