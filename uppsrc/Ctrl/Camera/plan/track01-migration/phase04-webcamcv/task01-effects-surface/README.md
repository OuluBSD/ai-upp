# Task 01 - WebcamCV Effects Surface

Status: pending

## Goal
Prepare Draw/Camera extension points for optional CV/effects adapters based on reference/WebcamCV.

## Scope
- Identify which WebcamCV effects can be cleanly exposed as Draw/Camera filters.
- Keep CV algorithms in uppsrc/ComputerVision; only add adapters/helpers in Draw/Camera.
- Define API for enabling/disabling effects in the capture pipeline.

## Acceptance
- Draw/Camera has a minimal effects interface with at least one no-op implementation.
- No code moved out of uppsrc/ComputerVision.
