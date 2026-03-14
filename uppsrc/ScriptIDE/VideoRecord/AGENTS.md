# VideoRecord

This package provides optional `ScriptIDE` plugins related to `.gamestate` video export.

Rules:
- Keep all integration with `ScriptIDE` behind plugin interfaces declared in `uppsrc/ScriptIDE/PluginInterfacesGUI.h`.
- Do not make `uppsrc/ScriptIDE/*` include files from this package.
- Register plugins from `.icpp` files using the ScriptIDE plugin registration macro.
- Store plugin-owned persistent config in plugin-specific files under `ConfigFile(...)`.
