#include <Core/Core.h>
#include <AnimEditLib/AnimExport.h>
#include <AnimEditLib/AnimBuild.h>
#include <AnimEditLib/AnimSerialize.h>

using namespace Upp;

CONSOLE_APP_MAIN {
    // Test export and build functionality
    std::cout << "Testing AnimExport and AnimBuild functionality...\n";
    
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
    project.sprites.Add(sprite);
    
    // Add a test frame
    Frame frame;
    frame.id = "test_frame_1";
    frame.name = "Test Frame";
    
    // Add a sprite instance to the frame
    SpriteInstance si;
    si.sprite_id = sprite.id;
    si.position = Vec2(0, 0);
    frame.sprites.Add(si);
    
    project.frames.Add(frame);
    
    // Add a test animation
    Animation anim;
    anim.id = "test_anim_1";
    anim.name = "Test Animation";
    
    FrameRef fr;
    fr.frame_id = frame.id;
    anim.frames.Add(fr);
    
    project.animations.Add(anim);
    
    // Add a test entity
    Entity entity;
    entity.id = "test_entity_1";
    entity.name = "Test Entity";
    
    NamedAnimationSlot slot;
    slot.name = "idle";
    slot.animation_id = anim.id;
    entity.animation_slots.Add(slot);
    
    project.entities.Add(entity);
    
    // Test dependency analysis
    std::cout << "\nTesting dependency analysis...\n";
    DependencyGraph graph = AnalyzeDependencies(project);
    std::cout << "Found " << graph.dependencies.GetCount() << " resources\n";
    std::cout << "Root nodes (not depended on by others): " << graph.root_nodes.GetCount() << "\n";
    std::cout << "Leaf nodes (depend on nothing): " << graph.leaf_nodes.GetCount() << "\n";
    
    // Test build order
    Vector<String> build_order = GetBuildOrder(graph);
    std::cout << "Suggested build order (" << build_order.GetCount() << " items): ";
    for (int i = 0; i < build_order.GetCount(); i++) {
        std::cout << build_order[i];
        if (i < build_order.GetCount() - 1) std::cout << " -> ";
    }
    std::cout << std::endl;
    
    // Test unused resources
    Vector<String> unused = FindUnusedResources(project);
    std::cout << "Unused resources: " << unused.GetCount() << std::endl;
    
    // Test missing dependencies
    Vector<String> missing = FindMissingDependencies(project, GetHomeDir()); // Use home dir as placeholder
    std::cout << "Missing dependencies: " << missing.GetCount() << std::endl;
    
    // Test export with compression
    std::cout << "\nTesting export with compression...\n";
    ExportOptions export_options;
    export_options.format = ExportFormat::JSON;
    export_options.output_path = GetTempDir();
    export_options.optimize_animations = true;
    export_options.compress_sprites = false; // We don't have actual textures to compress
    
    ExportResult export_result = ExportProject(project, export_options);
    if (export_result.success) {
        std::cout << "Export successful: " << export_result.summary << std::endl;
    } else {
        std::cout << "Export failed: " << export_result.error_message << std::endl;
    }
    
    // Test build pipeline
    std::cout << "\nTesting build pipeline...\n";
    
    // First, save the test project to a temporary file
    String project_json = SaveProjectJson(project);
    String project_file = AppendFileName(GetTempDir(), "test_build_project.json");
    if (!SaveFile(project_file, project_json)) {
        std::cout << "Failed to save test project for build test" << std::endl;
        return;
    }
    
    BuildConfig build_config;
    build_config.source_path = GetTempDir();
    build_config.output_path = AppendFileName(GetTempDir(), "build_output");
    build_config.format = ExportFormat::GAME_READY;
    build_config.optimize_sprites = false;  // Skip actual sprite compression for test
    build_config.optimize_animations = true;
    build_config.create_spritesheets = false;
    build_config.verbose_output = true;
    
    BuildResult build_result = BuildProject(build_config);
    if (build_result.success) {
        std::cout << "Build successful: " << build_result.summary << std::endl;
        std::cout << "Generated files: " << build_result.generated_files.GetCount() << std::endl;
    } else {
        std::cout << "Build failed: " << build_result.error_message << std::endl;
    }
    
    // Test version compatibility
    std::cout << "\nTesting version compatibility...\n";
    bool compat1 = IsVersionCompatible("1.0.0", "1.2.3");
    std::cout << "Version 1.0.0 compatible with 1.2.3: " << (compat1 ? "Yes" : "No") << std::endl;
    
    bool compat2 = IsVersionCompatible("1.0.0", "2.0.0");
    std::cout << "Version 1.0.0 compatible with 2.0.0: " << (compat2 ? "Yes" : "No") << std::endl;
    
    VersionInfo ver = ParseVersion("1.2.3-beta");
    std::cout << "Parsed version 1.2.3-beta: " << ver.ToString() << std::endl;
    
    std::cout << "\nAll export and build tests completed!\n";
}