#include <Core/Core.h>
#include <AnimEditLib/AnimExport.h>
#include <AnimEditLib/AnimBuild.h>
#include <AnimEditLib/AnimSerialize.h>
#include <AnimEditLib/AnimUtils.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
    StdLogSetup(LOG_COUT | LOG_FILE);

    LOG("Testing AnimExport and AnimBuild functionality...\n");

    // Create a simple test project
    AnimationProject project;
    project.id = "test_build_project";
    project.name = "Test Build Project";

    // Add a test sprite
    Sprite sprite;
    sprite.id = "test_sprite_1";
    sprite.name = "Test Sprite";
    sprite.texture_path = "test_texture.png";
    sprite.region = RectF(0, 0, 32, 32);
    sprite.pivot = Vec2(16, 16);
    project.sprites.Add(pick(sprite));

    // Add a test frame
    AnimationFrame frame;
    frame.id = "test_frame_1";
    frame.name = "Test Frame";

    // Add a sprite instance to the frame
    SpriteInstance si;
    si.sprite_id = "test_sprite_1";  // Use the actual ID that was added to the project
    si.position = Vec2(0, 0);
    frame.sprites.Add(pick(si));

    project.frames.Add(pick(frame));

    // Add a test animation
    Animation anim;
    anim.id = "test_anim_1";
    anim.name = "Test Animation";

    AnimationFrameRef fr;
    fr.frame_id = "test_frame_1";  // Use the actual ID that was added to the project
    anim.frames.Add(pick(fr));

    project.animations.Add(pick(anim));

    // Add a test entity
    Entity entity;
    entity.id = "test_entity_1";
    entity.name = "Test Entity";

    NamedAnimationSlot slot;
    slot.name = "idle";
    slot.animation_id = "test_anim_1";  // Use the actual ID that was added to the project
    entity.animation_slots.Add(pick(slot));

    project.entities.Add(pick(entity));

    // Test dependency analysis
    LOG("\nTesting dependency analysis...\n");
    DependencyGraph graph = AnalyzeDependencies(project);
    LOG("Found " << graph.dependencies.GetCount() << " resources\n");
    LOG("Root nodes (not depended on by others): " << graph.root_nodes.GetCount() << "\n");
    LOG("Leaf nodes (depend on nothing): " << graph.leaf_nodes.GetCount() << "\n");

    // Test build order
    Vector<String> build_order = GetBuildOrder(graph);
    LOG("Suggested build order (" << build_order.GetCount() << " items): ");
    for (int i = 0; i < build_order.GetCount(); i++) {
        LOG(build_order[i]);
        if (i < build_order.GetCount() - 1) LOG(" -> ");
    }
    LOG("\n");

    // Test unused resources
    Vector<String> unused = FindUnusedResources(project);
    LOG("Unused resources: " << unused.GetCount() << "\n");

    // Test missing dependencies
    Vector<String> missing = FindMissingDependencies(project, GetHomeDirectory()); // Use home dir as placeholder
    LOG("Missing dependencies: " << missing.GetCount() << "\n");

    // Test export with compression
    LOG("\nTesting export with compression...\n");
    ExportOptions export_options;
    export_options.format = ExportFormat::JSON;
    export_options.output_path = GetTempDirectory();
    export_options.optimize_animations = true;
    export_options.compress_sprites = false; // We don't have actual textures to compress

    ExportResult export_result = ExportProject(project, export_options);
    if (export_result.success) {
        LOG("Export successful: " << export_result.summary << "\n");
    } else {
        LOG("Export failed: " << export_result.error_message << "\n");
    }

    // Test entity export
    LOG("\nTesting entity export...\n");
    ExportResult entity_export_result = ExportEntities(project, export_options);
    if (entity_export_result.success) {
        LOG("Entity export successful: " << entity_export_result.summary << "\n");
    } else {
        LOG("Entity export failed: " << entity_export_result.error_message << "\n");
    }

    // Test build pipeline
    LOG("\nTesting build pipeline...\n");

    BuildOptions build_options;
    build_options.format = ExportFormat::GAME_READY;
    build_options.output_path = AppendFileName(GetTempDirectory(), "build_output");
    build_options.optimize_animations = true;
    build_options.compress_sprites = false;
    build_options.include_entities = true;
    build_options.include_sprites = true;
    build_options.include_animations = true;

    // Ensure output directory exists
    if (!DirectoryExists(build_options.output_path)) {
        // Create directory if it doesn't exist - DirectoryCreate is the U++ equivalent to MkDirDeep
        if (!DirectoryCreate(build_options.output_path)) {
            LOG("Warning: Could not create output directory: " << build_options.output_path << "\n");
        }
    }

    BuildResult build_result = BuildProject(project, build_options);
    if (build_result.success) {
        LOG("Build successful: " << build_result.summary << "\n");
        LOG("Exported files: " << build_result.exported_files.GetCount() << "\n");
    } else {
        LOG("Build failed: " << build_result.error_message << "\n");
    }

    // Test version compatibility
    LOG("\nTesting version compatibility...\n");
    bool compat1 = IsVersionCompatible("1.0.0", "1.2.3");
    LOG("Version 1.0.0 compatible with 1.2.3: " << (compat1 ? "Yes" : "No") << "\n");

    bool compat2 = IsVersionCompatible("1.0.0", "2.0.0");
    LOG("Version 1.0.0 compatible with 2.0.0: " << (compat2 ? "Yes" : "No") << "\n");

    VersionInfo ver = ParseVersion("1.2.3-beta");
    LOG("Parsed version 1.2.3-beta: " << ver.ToString() << "\n");

    // Test circular dependency detection
    LOG("\nTesting circular dependency detection...\n");
    bool has_cycle = HasCircularDependencies(graph);
    LOG("Circular dependencies detected: " << (has_cycle ? "Yes" : "No") << "\n");

    LOG("\nAll export and build tests completed!\n");
}