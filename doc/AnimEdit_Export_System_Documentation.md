# AnimEdit Export System Documentation

## Overview

The AnimEdit export system provides functionality to export animation projects in various formats suitable for different use cases. The system includes performance optimizations, detailed validation, progress reporting, and batch processing capabilities.

## Key Components

### Export Formats

The system supports multiple export formats:

- **JSON**: Standard format for development and debugging
- **BINARY**: Optimized binary format for runtime efficiency
- **SPRITESHEET**: Combined spritesheet with animation data
- **GAME_READY**: Optimized format for specific game engines

### Core Structures

#### ExportOptions
Configuration options for export operations:
- `format`: Enum specifying the export format
- `include_sprites`, `include_animations`, `include_entities`: Flags for content inclusion
- `compress_sprites`, `optimize_animations`: Performance optimization flags
- `output_path`, `base_resource_path`: Path configuration
- `progress_callback`: Function for progress reporting during export
- `enable_caching`, `multi_threaded`: Performance-related options

#### ExportResult
Results returned by export operations:
- `success`: Boolean indicating success or failure
- `error_message`: Human-readable error description
- `exported_files`: List of files created during export
- `file_size_bytes`: Total size of exported data
- `summary`: Human-readable summary of the operation
- `processing_time_ms`: Time taken for export
- `validation_result`: Detailed validation results

#### ValidationResult
Detailed validation results:
- `is_valid`: Boolean indicating if the project passes validation
- `issues`: List of ValidationIssue objects found during validation
- `error_count`, `warning_count`: Counts of different severity issues

#### ValidationIssue
Individual validation findings:
- `id`: ID of the problematic resource
- `type`: Type of resource (sprite, animation, entity, etc.)
- `severity`: "error", "warning", or "info"
- `description`: Detailed description of the issue
- `location`: Where in the project the issue occurs

## Basic Export Operations

### Simple Export
```cpp
AnimationProject project = /* your project */;
ExportOptions options;
options.format = ExportFormat::JSON;
options.output_path = "/path/to/output";

ExportResult result = ExportProject(project, options);
if (result.success) {
    LOG("Export successful: " + result.summary);
} else {
    LOG("Export failed: " + result.error_message);
}
```

### Optimized Export with Progress Reporting
```cpp
ExportOptions options;
options.format = ExportFormat::GAME_READY;
options.output_path = "/path/to/output";
options.progress_callback = [](int progress, const String& msg) {
    LOG("Export progress: " + IntStr(progress) + "% - " + msg);
};

ExportResult result = ExportProjectOptimized(project, options);
```

## Validation

The export system includes comprehensive validation to identify potential issues:

```cpp
ValidationResult vresult = ValidateProjectForExport(project);
if (vresult.HasErrors()) {
    LOG("Project has " + IntStr(vresult.error_count) + " errors");
    for (const auto& issue : vresult.issues) {
        if (issue.severity == "error") {
            LOG("Error: " + issue.description);
        }
    }
} else if (vresult.HasWarnings()) {
    LOG("Project has " + IntStr(vresult.warning_count) + " warnings (export still possible)");
}
```

## Batch Export

For processing multiple projects:

```cpp
BatchExportOptions batch_options;
batch_options.project_paths.Add("/path/to/project1.json");
batch_options.project_paths.Add("/path/to/project2.json");
batch_options.output_directory = "/path/to/output";
batch_options.export_options.format = ExportFormat::GAME_READY;

// Optional progress callback
batch_options.progress_callback = [](int progress, const String& msg) {
    LOG("Batch progress: " + IntStr(progress) + "% - " + msg);
};

ExportResult batch_result = BatchExportProjects(batch_options);
```

## Performance Tips

1. **Use Optimized Export**: For large projects, use `ExportProjectOptimized` which includes progress reporting
2. **Enable Caching**: Set `enable_caching = true` for repeated exports
3. **Validate First**: Run validation separately to catch issues before export
4. **Monitor Progress**: Implement progress callbacks for UI updates during long operations

## Common Issues and Solutions

### Validation Errors
- **Empty IDs**: All sprites, animations, frames, and entities must have non-empty IDs
- **Dangling References**: All references must point to existing resources
- **Missing Animations**: Projects must contain at least one animation
- **Duplicate IDs**: Each resource must have a unique ID

### Performance Issues
- **Large Projects**: Use progress callbacks to track status
- **Missing Textures**: Warnings about missing texture paths don't prevent export but should be addressed

## File Structure

The exported files will be organized as follows:
- JSON exports: `project_id.json`
- Binary exports: `project_id.bin`
- Spritesheet exports: `project_id_spritesheet.json`
- Game-ready exports: `project_id_game.json`

## Integration Tips

When integrating the export system into your application:
1. Always validate projects before export
2. Provide user feedback via progress callbacks
3. Handle validation warnings appropriately
4. Check export results before assuming success