#include <Core/Core.h>
#include <AnimEditLib/AnimCore.h>
#include <AnimEditLib/AnimSerialize.h>
#include <AnimEditLib/AnimUtils.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
    StdLogSetup(LOG_COUT | LOG_FILE);

    AnimationProject proj;
    proj.id = "test_project";
    proj.name = "Test Project";

    // Sprite
    Sprite s("hero_head");
    s.category     = "character";
    s.texture_path = "hero.png";
    s.region       = RectF(0, 0, 32, 32);
    s.pivot        = Vec2(16, 16);
    proj.sprites.Add(s);

    // Frame with one sprite instance
    Frame f;
    f.id   = "frame_idle_0";
    f.name = "Idle0";
    f.default_duration = 0.15;

    SpriteInstance si;
    si.sprite_id  = "hero_head";
    si.position   = Vec2(0, 0);
    si.rotation   = 0.0;
    si.scale      = Vec2(1.0, 1.0);
    si.alpha      = 1.0;
    si.zindex     = 0;
    f.sprites.Add(pick(si));

    proj.frames.Add(pick(f));

    // Animation referencing the frame
    Animation a;
    a.id       = "idle";
    a.name     = "Idle";
    a.category = "character";

    FrameRef fr;
    fr.frame_id     = "frame_idle_0";
    fr.has_duration = true;
    fr.duration     = 0.15;
    a.frames.Add(pick(fr));

    proj.animations.Add(pick(a));

    // Serialize
    String json = SaveProjectJson(proj);
    LOG("Serialized JSON:");
    LOG(json);

    // Deserialize into a new object
    AnimationProject loaded;
    if(!LoadProjectJson(loaded, json)) {
        LOG("ERROR: Failed to load project JSON");
        SetExitCode(1);
        return;
    }

    // Basic checks
    LOG("Loaded project id: " << loaded.id);
    LOG("Loaded project name: " << loaded.name);
    LOG("Sprites: " << loaded.sprites.GetCount());
    LOG("Frames: " << loaded.frames.GetCount());
    LOG("Animations: " << loaded.animations.GetCount());

    if(loaded.sprites.GetCount() != 1 ||
       loaded.frames.GetCount() != 1 ||
       loaded.animations.GetCount() != 1) {
        LOG("ERROR: Counts do not match expected values");
        SetExitCode(1);
        return;
    }

    const Sprite* hs = loaded.FindSprite("hero_head");
    const Frame*  hf = loaded.FindFrame("frame_idle_0");
    const Animation* ha = loaded.FindAnimation("idle");

    if(!hs || !hf || !ha) {
        LOG("ERROR: Failed to find one or more objects by id after load");
        SetExitCode(1);
        return;
    }

    LOG("SUCCESS: Round-trip serialization test passed.");
    
    // Test ID generation utilities
    LOG("\n--- Testing ID Generation ---");
    
    // Test generating unique sprite IDs
    String newSpriteId1 = GenerateSpriteId(proj, "hero_head");
    String newSpriteId2 = GenerateSpriteId(proj, "hero_head"); // This should get a suffix since the first exists
    LOG("Generated sprite ID 1: " << newSpriteId1);
    LOG("Generated sprite ID 2: " << newSpriteId2);
    
    // Add another sprite with generated ID to test collision detection
    Sprite newSprite;
    newSprite.id = newSpriteId1;
    newSprite.category = "character";
    newSprite.texture_path = "new_sprite.png";
    newSprite.region = RectF(0, 0, 16, 16);
    newSprite.pivot = Vec2(8, 8);
    proj.sprites.Add(pick(newSprite));
    
    String newSpriteId3 = GenerateSpriteId(proj, "hero_head"); // This should get _2 since _1 already exists
    LOG("Generated sprite ID 3: " << newSpriteId3);
    
    // Test other ID generation functions
    String newFrameId = GenerateFrameId(proj, "test_frame");
    LOG("Generated frame ID: " << newFrameId);
    
    String newAnimId = GenerateAnimationId(proj, "test_animation");
    LOG("Generated animation ID: " << newAnimId);
    
    String newCollisionId = GenerateCollisionId(proj, "frame_idle_0", "collision");
    LOG("Generated collision ID: " << newCollisionId);
    
    // Test validation utilities
    LOG("\n--- Testing Validation ---");
    
    String errorOut;
    
    // Test valid project
    bool validResult = ValidateProject(proj, errorOut);
    LOG("Valid project validation result: " << (validResult ? "PASS" : "FAIL"));
    if (!validResult) {
        LOG("Error: " << errorOut);
    } else {
        LOG("Valid project passed validation as expected");
    }
    
    // Create an invalid project for testing
    AnimationProject invalidProj;
    invalidProj.id = "invalid_test"; 
    invalidProj.name = "Invalid Test";
    
    // Add an animation that references a non-existent frame
    Animation invalidAnim;
    invalidAnim.id = "invalid_animation";
    invalidAnim.name = "Invalid Animation";
    invalidAnim.category = "test";
    
    FrameRef invalidFrameRef;
    invalidFrameRef.frame_id = "non_existent_frame";  // This frame doesn't exist
    invalidFrameRef.has_duration = true;
    invalidFrameRef.duration = 0.1;  // This is valid
    invalidAnim.frames.Add(pick(invalidFrameRef));
    
    invalidProj.animations.Add(pick(invalidAnim));
    
    // Test invalid project
    bool invalidResult = ValidateProject(invalidProj, errorOut);
    LOG("Invalid project validation result: " << (invalidResult ? "PASS (UNEXPECTED!)" : "FAIL (as expected)"));
    if (!invalidResult) {
        LOG("Correctly caught validation error: " << errorOut);
    } else {
        LOG("ERROR: Invalid project should have failed validation");
        SetExitCode(1);
        return;
    }
    
    // Create another invalid project with missing sprite reference
    AnimationProject invalidProj2;
    invalidProj2.id = "invalid_test2";
    invalidProj2.name = "Invalid Test 2";
    
    // Add a frame that references a non-existent sprite
    Frame invalidFrame;
    invalidFrame.id = "test_frame";
    invalidFrame.name = "Test Frame";
    invalidFrame.default_duration = 0.1;
    
    SpriteInstance invalidSpriteInst;
    invalidSpriteInst.sprite_id = "non_existent_sprite";  // This sprite doesn't exist
    invalidSpriteInst.position = Vec2(0, 0);
    invalidSpriteInst.rotation = 0.0;
    invalidSpriteInst.scale = Vec2(1.0, 1.0);
    invalidSpriteInst.alpha = 1.0;
    invalidSpriteInst.zindex = 0;
    invalidFrame.sprites.Add(pick(invalidSpriteInst));
    
    invalidProj2.frames.Add(pick(invalidFrame));
    
    // Test second invalid project
    bool invalidResult2 = ValidateProject(invalidProj2, errorOut);
    LOG("Second invalid project validation result: " << (invalidResult2 ? "PASS (UNEXPECTED!)" : "FAIL (as expected)"));
    if (!invalidResult2) {
        LOG("Correctly caught second validation error: " << errorOut);
    } else {
        LOG("ERROR: Second invalid project should have failed validation");
        SetExitCode(1);
        return;
    }
    
    // Test for invalid collision rectangle (negative size)
    AnimationProject invalidProj3;
    invalidProj3.id = "invalid_test3";
    invalidProj3.name = "Invalid Test 3";
    
    Frame frameWithBadCollision;
    frameWithBadCollision.id = "frame_with_bad_collision";
    frameWithBadCollision.name = "Frame with Bad Collision";
    frameWithBadCollision.default_duration = 0.1;
    
    CollisionRect badCollision;
    badCollision.id = "bad_collision";
    badCollision.rect = RectF(0, 0, -5, 10);  // Negative width makes it invalid
    frameWithBadCollision.collisions.Add(pick(badCollision));
    
    invalidProj3.frames.Add(pick(frameWithBadCollision));
    
    // Test third invalid project
    bool invalidResult3 = ValidateProject(invalidProj3, errorOut);
    LOG("Third invalid project (bad collision) validation result: " << (invalidResult3 ? "PASS (UNEXPECTED!)" : "FAIL (as expected)"));
    if (!invalidResult3) {
        LOG("Correctly caught third validation error: " << errorOut);
    } else {
        LOG("ERROR: Third invalid project should have failed validation");
        SetExitCode(1);
        return;
    }
    
    LOG("\nAll validation tests completed successfully!");
}