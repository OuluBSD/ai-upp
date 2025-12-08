#include <Core/Core.h>
#include <AnimEditLib/AnimCore.h>
#include <AnimEditLib/AnimSerialize.h>
#include <AnimEditLib/AnimExport.h>
#include <AnimEditLib/AnimBuild.h>
#include <UnitTest/UnitTest.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
    // Initialize test framework
    UnitTest test;

    // Test 1: Basic export functionality
    TEST_CASE("Basic Export Functionality") {
        AnimationProject project;
        project.id = "test_project";
        project.name = "Test Project";
        
        // Add a sprite
        Sprite sprite;
        sprite.id = "test_sprite";
        sprite.name = "Test Sprite";
        sprite.texture_path = "textures/test.png";
        sprite.region = RectF(0, 0, 32, 32);
        sprite.pivot = Vec2(16, 16);
        project.sprites.Add(pick(sprite));
        
        // Add a frame
        AnimationFrame frame;
        frame.id = "test_frame";
        frame.name = "Test Frame";
        
        SpriteInstance instance;
        instance.sprite_id = "test_sprite";
        instance.position = Vec2(10, 10);
        frame.sprites.Add(instance);
        project.frames.Add(pick(frame));
        
        // Add an animation
        Animation anim;
        anim.id = "test_anim";
        anim.name = "Test Animation";
        
        AnimationFrameRef frame_ref;
        frame_ref.frame_id = "test_frame";
        anim.frames.Add(frame_ref);
        project.animations.Add(pick(anim));
        
        // Add an entity
        Entity entity;
        entity.id = "test_entity";
        entity.name = "Test Entity";
        
        NamedAnimationSlot slot;
        slot.name = "idle";
        slot.animation_id = "test_anim";
        entity.animation_slots.Add(slot);
        entity.properties.Add("health", 100);
        entity.properties.Add("speed", 5.0);
        project.entities.Add(pick(entity));
        
        // Test export
        ExportOptions options;
        options.format = ExportFormat::JSON;
        options.output_path = GetTempDir();  // Use temp directory for testing
        
        ExportResult result = ExportProject(project, options);
        TEST(result.success) << "Project export should succeed";
        
        // Test entity export
        ExportResult entity_result = ExportEntities(project, options);
        TEST(entity_result.success) << "Entity export should succeed";
        TEST(entity_result.exported_files.GetCount() == project.entities.GetCount()) << "Should export one file per entity";
    }
    
    // Test 2: Animation optimization
    TEST_CASE("Animation Optimization") {
        Animation anim;
        anim.id = "optimization_test";
        
        // Add duplicate frames to test optimization
        AnimationFrameRef frame1, frame2, frame3;
        frame1.frame_id = "same_frame";
        frame1.has_duration = true;
        frame1.duration = 0.1;
        
        frame2 = frame1;  // Same frame, same duration
        frame3.frame_id = "different_frame";
        frame3.has_duration = true;
        frame3.duration = 0.2;
        
        anim.frames.Add(frame1);
        anim.frames.Add(frame2);  // Duplicate frame to be optimized
        anim.frames.Add(frame3);
        
        int original_count = anim.frames.GetCount();
        OptimizeAnimation(anim);
        int optimized_count = anim.frames.GetCount();
        
        TEST(optimized_count < original_count) << "Optimization should reduce frame count when duplicates exist";
        TEST(anim.frames[0].duration == 0.2) << "Optimization should combine durations of duplicate frames";
    }
    
    // Test 3: Dependency analysis
    TEST_CASE("Dependency Analysis") {
        AnimationProject project;
        
        // Add a sprite
        Sprite sprite;
        sprite.id = "test_sprite";
        sprite.name = "Test Sprite";
        project.sprites.Add(pick(sprite));
        
        // Add a frame that uses the sprite
        AnimationFrame frame;
        frame.id = "test_frame";
        frame.name = "Test Frame";
        
        SpriteInstance instance;
        instance.sprite_id = "test_sprite";
        frame.sprites.Add(instance);
        project.frames.Add(pick(frame));
        
        // Add an animation that uses the frame
        Animation anim;
        anim.id = "test_anim";
        anim.name = "Test Animation";
        
        AnimationFrameRef frame_ref;
        frame_ref.frame_id = "test_frame";
        anim.frames.Add(frame_ref);
        project.animations.Add(pick(anim));
        
        // Add an entity that uses the animation
        Entity entity;
        entity.id = "test_entity";
        entity.name = "Test Entity";
        
        NamedAnimationSlot slot;
        slot.name = "idle";
        slot.animation_id = "test_anim";
        entity.animation_slots.Add(slot);
        project.entities.Add(pick(entity));
        
        // Analyze dependencies
        DependencyGraph graph = AnalyzeDependencies(project);
        
        TEST(graph.dependencies.GetCount() == 4) << "Should have dependencies for sprite, frame, animation, and entity";
        
        // Check if dependencies are correctly identified
        bool sprite_has_no_deps = false;
        bool frame_depends_on_sprite = false;
        bool anim_depends_on_frame = false;
        bool entity_depends_on_anim = false;
        
        for (const auto& dep : graph.dependencies) {
            if (dep.resource_id == "test_sprite" && dep.references.GetCount() == 0) {
                sprite_has_no_deps = true;
            } else if (dep.resource_id == "test_frame") {
                for (const auto& ref : dep.references) {
                    if (ref == "test_sprite") {
                        frame_depends_on_sprite = true;
                        break;
                    }
                }
            } else if (dep.resource_id == "test_anim") {
                for (const auto& ref : dep.references) {
                    if (ref == "test_frame") {
                        anim_depends_on_frame = true;
                        break;
                    }
                }
            } else if (dep.resource_id == "test_entity") {
                for (const auto& ref : dep.references) {
                    if (ref == "test_anim") {
                        entity_depends_on_anim = true;
                        break;
                    }
                }
            }
        }
        
        TEST(sprite_has_no_deps) << "Sprite should not depend on anything";
        TEST(frame_depends_on_sprite) << "Frame should depend on sprite";
        TEST(anim_depends_on_frame) << "Animation should depend on frame";
        TEST(entity_depends_on_anim) << "Entity should depend on animation";
    }
    
    // Test 4: Build pipeline
    TEST_CASE("Build Pipeline") {
        AnimationProject project;
        project.id = "build_test_project";
        project.name = "Build Test Project";
        
        // Add minimal content
        Sprite sprite;
        sprite.id = "build_test_sprite";
        sprite.name = "Build Test Sprite";
        sprite.texture_path = "textures/build_test.png";
        project.sprites.Add(pick(sprite));
        
        BuildOptions build_options;
        build_options.format = ExportFormat::BINARY;
        build_options.output_path = GetTempDir();
        build_options.optimize_animations = true;
        build_options.compress_sprites = true;
        
        BuildResult build_result = BuildProject(project, build_options);
        TEST(build_result.success) << "Build should succeed";
        TEST(build_result.summary.Find("Build completed successfully") >= 0) << "Build should complete successfully";
    }
    
    // Run the tests
    test.Run();
    
    // Print results
    RLOG("AnimExport tests completed. Passed: " << test.GetPass() << ", Failed: " << test.GetFail());
}