# `.gamestate` YAML Schema Design

## Format Overview
The `.gamestate` file is a YAML-based configuration that serves as the "project file" for a game session or plugin-driven scenario. It links logic (Python) with presentation (Layout).

## Schema Fields

| Field | Type | Description |
| :--- | :--- | :--- |
| `entry_script` | String | Path to the `.py` file containing the main logic. |
| `entry_function` | String | Name of the function to call within the script. |
| `layout` | String | (Optional) Path to the `.xlay` layout file. |
| `metadata` | Map | Key-value pairs for initial game state (e.g., player count). |
| `resources` | List | (Optional) List of folders or files to preload. |

## Path Resolution
- Relative paths are resolved **relative to the `.gamestate` file's parent directory**.
- Absolute paths are supported but discouraged for portability.

## Example File (`hearts.gamestate`)
```yaml
entry_script: "hearts_main.py"
entry_function: "initialize_hearts"
layout: "classic_table.xlay"
metadata:
  players: 4
  rules: "kde_2004"
  difficulty: "normal"
resources:
  - "assets/cards"
  - "assets/sounds"
```

## Serialization
The file is parsed using `uppsrc/Core`'s YAML support. Deserialization into a `Value` map is the primary method of access for the `PluginManager`.
