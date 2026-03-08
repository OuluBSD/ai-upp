# GPGPU Plan - ComputerVision Backend API

## Objective
Create a backend abstraction in `uppsrc/ComputerVision` so algorithms can target CPU, AMP, and future OpenGL shader paths.

## Current Status
- Implemented: `CvBackend` enum + global backend get/set API.
- Implemented: support/status helpers (`IsCvBackendSupported`, `GetCvBackendName`, `GetCvBackendStatus`).
- Implemented: OpenGL backend marked as explicit stub.

## Next Tasks
1. Add optional per-algorithm capability map (not all algorithms will support all backends initially).
2. Add persistence hook for backend choice (app-level, not package-global static only).
3. Add debug tracing around backend dispatch at algorithm entry points.

## Risks
- Global backend state is process-wide and may be too coarse for multi-pipeline usage.
- Capability mismatch may surface only at runtime without per-algorithm validation.
