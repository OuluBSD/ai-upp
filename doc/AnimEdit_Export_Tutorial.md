# AnimEdit Export System Tutorial

## Getting Started

This tutorial will guide you through using the AnimEdit export system to convert animation projects to formats suitable for game engines and other applications.

## Prerequisites

- Basic understanding of C++ and Ultimate++ framework
- AnimEditLib integrated into your project
- An AnimationProject ready for export

## Step 1: Basic Export

Let's start with a simple export operation:

```cpp
#include <AnimEditLib/AnimExport.h>
#include <AnimEditLib/AnimCore.h>
#include <AnimEditLib/AnimSerialize.h>

using namespace Upp;

void BasicExportExample() {
    // Load an existing project or create one
    AnimationProject project;
    
    // In a real scenario, you would load a project:
    // String projectJson = LoadFile("my_project.json");
    // LoadProjectJson(project, projectJson);
    
    // For this example, we'll create a simple project
    // (in practice, you'll have a project from the editor)
    project.id = "tutorial_project";
    project.name = "Tutorial Project";
    
    // Configure export options
    ExportOptions exportOptions;
    exportOptions.format = ExportFormat::JSON;  // JSON format for development
    exportOptions.output_path = "/path/to/output/directory";
    exportOptions.include_sprites = true;
    exportOptions.include_animations = true;
    exportOptions.include_entities = true;
    
    // Perform the export
    ExportResult result = ExportProject(project, exportOptions);
    
    if (result.success) {
        LOG("Export successful!");
        LOG("Files exported: " << result.exported_files.GetCount());
        LOG("Total size: " << result.file_size_bytes << " bytes");
        LOG("Summary: " << result.summary);
    } else {
        LOG("Export failed: " << result.error_message);
    }
}
```

## Step 2: Validating Projects Before Export

Always validate your project before attempting export:

```cpp
void ValidationExample() {
    AnimationProject project = /* your project */;
    
    // Validate the project
    ValidationResult validation = ValidateProjectForExport(project);
    
    if (validation.HasErrors()) {
        LOG("Project has " << validation.error_count << " error(s):");
        for (const auto& issue : validation.issues) {
            if (issue.severity == "error") {
                LOG("  - " << issue.description);
            }
        }
        // Don't proceed with export if there are errors
        return;
    }
    
    if (validation.HasWarnings()) {
        LOG("Project has " << validation.warning_count << " warning(s):");
        for (const auto& issue : validation.issues) {
            if (issue.severity == "warning") {
                LOG("  - " << issue.description);
            }
        }
        // Warnings don't prevent export, but should be reviewed
    }
    
    LOG("Project validation passed. Ready for export.");
}
```

## Step 3: Performance-Optimized Export with Progress Reporting

For larger projects, use the optimized export with progress reporting:

```cpp
void OptimizedExportExample() {
    AnimationProject project = /* your project */;
    
    // Set up progress reporting
    ExportOptions options;
    options.format = ExportFormat::GAME_READY;  // Optimized format for games
    options.output_path = "/path/to/output";
    options.optimize_animations = true;
    options.compress_sprites = true;
    
    // Add progress callback
    options.progress_callback = [](int progress, const String& message) {
        // Update your UI or log progress
        LOG("Export Progress: " << progress << "% - " << message);
    };
    
    // Use the optimized export function
    ExportResult result = ExportProjectOptimized(project, options);
    
    if (result.success) {
        LOG("Optimized export completed in " << result.processing_time_ms << " ms");
        LOG("Exported files: " << result.exported_files.GetCount());
        LOG("Total size: " << result.file_size_bytes << " bytes");
    } else {
        LOG("Optimized export failed: " << result.error_message);
    }
}
```

## Step 4: Batch Export Multiple Projects

To export multiple projects at once:

```cpp
void BatchExportExample() {
    BatchExportOptions batchOptions;
    
    // Add project files to export
    batchOptions.project_paths.Add("/projects/project1.json");
    batchOptions.project_paths.Add("/projects/project2.json");
    batchOptions.project_paths.Add("/projects/project3.json");
    
    // Set common export options
    batchOptions.export_options.format = ExportFormat::BINARY;
    batchOptions.export_options.optimize_animations = true;
    
    // Set output directory
    batchOptions.output_directory = "/output/batch_exports";
    
    // Optional: Add progress callback
    batchOptions.progress_callback = [](int progress, const String& message) {
        LOG("Batch Export Progress: " << progress << "% - " << message);
    };
    
    // Run batch export
    ExportResult result = BatchExportProjects(batchOptions);
    
    if (result.success) {
        LOG("Batch export completed!");
        LOG("Projects processed: " << batchOptions.project_paths.GetCount());
        LOG("Files exported: " << result.exported_files.GetCount());
        LOG("Total size: " << result.file_size_bytes << " bytes");
        LOG("Processing time: " << result.processing_time_ms << " ms");
    } else {
        LOG("Batch export had issues: " << result.error_message);
    }
}
```

## Step 5: Handling Export Results

Always check export results properly:

```cpp
void HandleExportResult(const ExportResult& result) {
    if (result.success) {
        LOG("Export successful!");
        
        // The validation results are available even on success
        if (result.validation_result.HasWarnings()) {
            LOG("Export completed with warnings:");
            for (const auto& issue : result.validation_result.issues) {
                if (issue.severity == "warning") {
                    LOG("  Warning: " << issue.description);
                }
            }
        }
        
        LOG("Exported files:");
        for (const auto& file : result.exported_files) {
            LOG("  - " << file);
        }
        
    } else {
        LOG("Export failed!");
        LOG("Error: " << result.error_message);
        
        // Even if export failed, validation results might provide useful information
        if (result.validation_result.HasErrors()) {
            LOG("Validation errors found:");
            for (const auto& issue : result.validation_result.issues) {
                if (issue.severity == "error") {
                    LOG("  - " << issue.description);
                }
            }
        }
    }
}
```

## Complete Example: Export Workflow

Here's a complete workflow combining all steps:

```cpp
void CompleteExportWorkflow() {
    AnimationProject project;
    // Load your project here
    // LoadProjectJson(project, LoadFile("my_project.json"));
    
    // Step 1: Validate project
    ValidationResult validation = ValidateProjectForExport(project);
    if (validation.HasErrors()) {
        LOG("Project has validation errors. Cannot export.");
        return;
    }
    
    // Step 2: Configure export options
    ExportOptions options;
    options.format = ExportFormat::GAME_READY;
    options.output_path = "/output";
    options.optimize_animations = true;
    options.progress_callback = [](int progress, const String& message) {
        // Update UI progress bar
        std::cout << "Export: " << progress << "% - " << message << std::endl;
    };
    
    // Step 3: Perform export
    ExportResult result = ExportProjectOptimized(project, options);
    
    // Step 4: Handle results
    HandleExportResult(result);
}
```

## Tips and Best Practices

1. **Always Validate First**: Run validation before export to catch issues early
2. **Use Appropriate Formats**: 
   - JSON for development and debugging
   - BINARY for runtime efficiency
   - GAME_READY for optimized game engine integration
3. **Monitor Progress**: Use progress callbacks for large projects
4. **Handle Warnings**: Don't ignore validation warnings even if export succeeds
5. **Test Exports**: Verify exported files work as expected in your target environment

## Troubleshooting

**Export fails with validation errors:**
- Check for missing IDs on sprites, animations, frames, or entities
- Verify all references point to existing resources
- Ensure project has at least one animation

**Export takes too long:**
- Use `ExportProjectOptimized` with progress callbacks
- Consider running validation separately to identify bottlenecks
- For very large projects, consider batch export

**Missing textures in exported files:**
- Verify texture paths are correctly configured
- Check that referenced files exist in the expected locations