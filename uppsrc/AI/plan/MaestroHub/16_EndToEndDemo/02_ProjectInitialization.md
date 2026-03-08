# Task: Project Initialization (Accounting App)
# Status: TODO

## Objective
Use `MaestroHub` (driven by a Python automation script) to initialize the "SmallGuiAppForAccounting" project in `./tmp/SmallGuiAppForAccounting`.

## Scenario
1.  **Start MaestroHub** via automation.
2.  **Open "New Project" / "Init" Dialog**.
3.  **Configure**:
    - Name: `SmallGuiAppForAccounting`
    - Directory: `./tmp/SmallGuiAppForAccounting`
    - Template: Basic U++ GUI (CtrlLib).
4.  **Execute**: Click "Create".
5.  **Verify**:
    - Check that the directory exists.
    - Check that `main.cpp` and `.upp` files are created.
    - Check that the project builds (invoke build via MaestroHub ops or externally).

## Artifacts
- Python script: `tests/maestro_demo_01_init.py`.
- Target Dir: `./tmp/SmallGuiAppForAccounting`.

## Feedback
- Log any awkward UI interactions or missing default settings in `FEEDBACK.md`.
