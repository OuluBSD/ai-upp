#include <Core/Core.h>
#include <AnimEditLib/AnimCore.h>
#include <AnimEditLib/AnimSerialize.h>
#include <AnimEditLib/AnimUtils.h>
#include <AnimEditLib/AnimResourceRegistry.h>

using namespace Upp;

// Test Vec2 operations
void TestVec2() {
    LOG("Testing Vec2 operations...");
    
    Vec2 v1(1.0, 2.0);
    Vec2 v2(3.0, 4.0);
    Vec2 v3(1.0, 2.0);
    
    // Test equality
    if (v1 == v3) {
        LOG("Vec2 equality test passed");
    } else {
        LOG("Vec2 equality test FAILED");
        SetExitCode(1);
    }
    if (v1 != v2) {
        LOG("Vec2 inequality test passed");
    } else {
        LOG("Vec2 inequality test FAILED");
        SetExitCode(1);
    }
    
    // Test default constructor
    Vec2 v0;
    if (v0.x == 0.0 && v0.y == 0.0) {
        LOG("Vec2 default constructor test passed");
    } else {
        LOG("Vec2 default constructor test FAILED");
        SetExitCode(1);
    }
    
    // Test parameterized constructor
    if (v1.x == 1.0 && v1.y == 2.0) {
        LOG("Vec2 parameterized constructor test passed");
    } else {
        LOG("Vec2 parameterized constructor test FAILED");
        SetExitCode(1);
    }
    
    LOG("Vec2 tests completed.\n");
}

// Test RectF operations
void TestRectF() {
    LOG("Testing RectF operations...");
    
    RectF r1(0.0, 0.0, 10.0, 20.0);
    RectF r2(1.0, 1.0, 15.0, 25.0);
    RectF r3(0.0, 0.0, 10.0, 20.0);
    
    // Test equality
    if (r1 == r3) {
        LOG("RectF equality test passed");
    } else {
        LOG("RectF equality test FAILED");
        SetExitCode(1);
    }
    if (r1 != r2) {
        LOG("RectF inequality test passed");
    } else {
        LOG("RectF inequality test FAILED");
        SetExitCode(1);
    }
    
    // Test default constructor
    RectF r0;
    if (r0.x == 0.0 && r0.y == 0.0 && r0.cx == 0.0 && r0.cy == 0.0) {
        LOG("RectF default constructor test passed");
    } else {
        LOG("RectF default constructor test FAILED");
        SetExitCode(1);
    }
    
    // Test parameterized constructor
    if (r1.x == 0.0 && r1.y == 0.0 && r1.cx == 10.0 && r1.cy == 20.0) {
        LOG("RectF parameterized constructor test passed");
    } else {
        LOG("RectF parameterized constructor test FAILED");
        SetExitCode(1);
    }
    
    LOG("RectF tests completed.\n");
}

// Test Sprite operations
void TestSprite() {
    LOG("Testing Sprite operations...");
    
    Sprite s1("sprite1");
    s1.category = "character";
    s1.texture_path = "texture.png";
    s1.region = RectF(0, 0, 32, 32);
    s1.pivot = Vec2(16, 16);
    
    Sprite s2("sprite1");
    s2.category = "character";
    s2.texture_path = "texture.png";
    s2.region = RectF(0, 0, 32, 32);
    s2.pivot = Vec2(16, 16);
    
    Sprite s3("sprite2");
    s3.category = "environment";
    
    // Test equality
    if (s1 == s2) {
        LOG("Sprite equality test passed");
    } else {
        LOG("Sprite equality test FAILED");
        SetExitCode(1);
    }
    if (s1 != s3) {
        LOG("Sprite inequality test passed");
    } else {
        LOG("Sprite inequality test FAILED");
        SetExitCode(1);
    }
    
    // Test default constructor
    Sprite s0;
    if (IsEmpty(s0.id)) {
        LOG("Sprite default constructor - id empty test passed");
    } else {
        LOG("Sprite default constructor - id empty test FAILED");
        SetExitCode(1);
    }
    if (s0.category.IsEmpty()) {  // Default constructor doesn't set category to "default"
        LOG("Sprite default constructor - category empty test passed");
    } else {
        LOG("Sprite default constructor - category empty test FAILED");
        SetExitCode(1);
    }
    if (s0.pivot.x == 0 && s0.pivot.y == 0) {
        LOG("Sprite default constructor - pivot default test passed");
    } else {
        LOG("Sprite default constructor - pivot default test FAILED");
        SetExitCode(1);
    }
    
    // Test parameterized constructor
    if (s1.id == "sprite1") {
        LOG("Sprite parameterized constructor test passed");
    } else {
        LOG("Sprite parameterized constructor test FAILED");
        SetExitCode(1);
    }
    
    LOG("Sprite tests completed.\n");
}

// Test SpriteInstance operations
void TestSpriteInstance() {
    LOG("Testing SpriteInstance operations...");
    
    SpriteInstance si1;
    si1.sprite_id = "sprite1";
    si1.position = Vec2(10, 20);
    si1.rotation = 1.5;
    si1.scale = Vec2(1.2, 1.5);
    si1.alpha = 0.8;
    si1.zindex = 5;
    
    SpriteInstance si2;
    si2.sprite_id = "sprite1";
    si2.position = Vec2(10, 20);
    si2.rotation = 1.5;
    si2.scale = Vec2(1.2, 1.5);
    si2.alpha = 0.8;
    si2.zindex = 5;
    
    SpriteInstance si3;
    si3.sprite_id = "different_sprite";
    
    // Test equality
    if (si1 == si2) {
        LOG("SpriteInstance equality test passed");
    } else {
        LOG("SpriteInstance equality test FAILED");
        SetExitCode(1);
    }
    if (si1 != si3) {
        LOG("SpriteInstance inequality test passed");
    } else {
        LOG("SpriteInstance inequality test FAILED");
        SetExitCode(1);
    }
    
    // Test defaults
    SpriteInstance si0;
    if (si0.rotation == 0) {
        LOG("SpriteInstance default rotation test passed");
    } else {
        LOG("SpriteInstance default rotation test FAILED");
        SetExitCode(1);
    }
    if (si0.scale.x == 1 && si0.scale.y == 1) {
        LOG("SpriteInstance default scale test passed");
    } else {
        LOG("SpriteInstance default scale test FAILED");
        SetExitCode(1);
    }
    if (si0.alpha == 1.0) {
        LOG("SpriteInstance default alpha test passed");
    } else {
        LOG("SpriteInstance default alpha test FAILED");
        SetExitCode(1);
    }
    if (si0.zindex == 0) {
        LOG("SpriteInstance default zindex test passed");
    } else {
        LOG("SpriteInstance default zindex test FAILED");
        SetExitCode(1);
    }
    
    LOG("SpriteInstance tests completed.\n");
}

// Test CollisionRect operations
void TestCollisionRect() {
    LOG("Testing CollisionRect operations...");
    
    CollisionRect c1;
    c1.id = "collision1";
    c1.rect = RectF(0, 0, 10, 20);
    
    CollisionRect c2;
    c2.id = "collision1";
    c2.rect = RectF(0, 0, 10, 20);
    
    CollisionRect c3;
    c3.id = "collision2";
    
    // Test equality
    if (c1 == c2) {
        LOG("CollisionRect equality test passed");
    } else {
        LOG("CollisionRect equality test FAILED");
        SetExitCode(1);
    }
    if (c1 != c3) {
        LOG("CollisionRect inequality test passed");
    } else {
        LOG("CollisionRect inequality test FAILED");
        SetExitCode(1);
    }
    
    // Test defaults
    CollisionRect c0;
    if (IsEmpty(c0.id)) {
        LOG("CollisionRect default id test passed");
    } else {
        LOG("CollisionRect default id test FAILED");
        SetExitCode(1);
    }
    
    LOG("CollisionRect tests completed.\n");
}

// Test Frame operations
void TestFrame() {
    LOG("Testing Frame operations...");
    
    Frame f1("frame1");
    f1.name = "Frame 1";
    f1.default_duration = 0.5;
    
    // Add a sprite instance
    SpriteInstance si;
    si.sprite_id = "sprite1";
    si.position = Vec2(5, 10);
    f1.sprites.Add(pick(si));
    
    // Add a collision
    CollisionRect cr;
    cr.id = "collision1";
    cr.rect = RectF(0, 0, 5, 5);
    f1.collisions.Add(pick(cr));
    
    Frame f2("frame1");
    f2.name = "Frame 1";
    f2.default_duration = 0.5;
    
    // Copy sprite instance and collision
    SpriteInstance si2;
    si2.sprite_id = "sprite1";
    si2.position = Vec2(5, 10);
    f2.sprites.Add(pick(si2));
    
    CollisionRect cr2;
    cr2.id = "collision1";
    cr2.rect = RectF(0, 0, 5, 5);
    f2.collisions.Add(pick(cr2));
    
    Frame f3("frame2");
    
    // Test equality
    if (f1 == f2) {
        LOG("Frame equality test passed");
    } else {
        LOG("Frame equality test FAILED");
        SetExitCode(1);
    }
    if (f1 != f3) {
        LOG("Frame inequality test passed");
    } else {
        LOG("Frame inequality test FAILED");
        SetExitCode(1);
    }
    
    // Test defaults
    Frame f0("test");
    if (f0.id == "test") {
        LOG("Frame id test passed");
    } else {
        LOG("Frame id test FAILED");
        SetExitCode(1);
    }
    if (f0.name.IsEmpty()) {
        LOG("Frame default name test passed");
    } else {
        LOG("Frame default name test FAILED");
        SetExitCode(1);
    }
    if (f0.default_duration == 0.1) {
        LOG("Frame default duration test passed");
    } else {
        LOG("Frame default duration test FAILED");
        SetExitCode(1);
    }
    if (f0.sprites.GetCount() == 0) {
        LOG("Frame default sprites array test passed");
    } else {
        LOG("Frame default sprites array test FAILED");
        SetExitCode(1);
    }
    if (f0.collisions.GetCount() == 0) {
        LOG("Frame default collisions array test passed");
    } else {
        LOG("Frame default collisions array test FAILED");
        SetExitCode(1);
    }
    
    LOG("Frame tests completed.\n");
}

// Test FrameRef operations
void TestFrameRef() {
    LOG("Testing FrameRef operations...");
    
    FrameRef fr1;
    fr1.frame_id = "frame1";
    fr1.has_duration = true;
    fr1.duration = 0.25;
    
    FrameRef fr2;
    fr2.frame_id = "frame1";
    fr2.has_duration = true;
    fr2.duration = 0.25;
    
    FrameRef fr3;
    fr3.frame_id = "frame2";
    
    // Test equality
    if (fr1 == fr2) {
        LOG("FrameRef equality test passed");
    } else {
        LOG("FrameRef equality test FAILED");
        SetExitCode(1);
    }
    if (fr1 != fr3) {
        LOG("FrameRef inequality test passed");
    } else {
        LOG("FrameRef inequality test FAILED");
        SetExitCode(1);
    }
    
    // Test defaults
    FrameRef fr0;
    if (fr0.has_duration == false) {
        LOG("FrameRef default has_duration test passed");
    } else {
        LOG("FrameRef default has_duration test FAILED");
        SetExitCode(1);
    }
    if (fr0.duration == 0.0) {
        LOG("FrameRef default duration test passed");
    } else {
        LOG("FrameRef default duration test FAILED");
        SetExitCode(1);
    }
    
    LOG("FrameRef tests completed.\n");
}

// Test Animation operations
void TestAnimation() {
    LOG("Testing Animation operations...");
    
    Animation a1;
    a1.id = "anim1";
    a1.name = "Animation 1";
    a1.category = "character";
    
    FrameRef fr;
    fr.frame_id = "frame1";
    fr.has_duration = true;
    fr.duration = 0.25;
    a1.frames.Add(pick(fr));
    
    Animation a2;
    a2.id = "anim1";
    a2.name = "Animation 1";
    a2.category = "character";
    
    FrameRef fr2;
    fr2.frame_id = "frame1";
    fr2.has_duration = true;
    fr2.duration = 0.25;
    a2.frames.Add(pick(fr2));
    
    Animation a3;
    a3.id = "anim2";
    
    // Test equality
    if (a1 == a2) {
        LOG("Animation equality test passed");
    } else {
        LOG("Animation equality test FAILED");
        SetExitCode(1);
    }
    if (a1 != a3) {
        LOG("Animation inequality test passed");
    } else {
        LOG("Animation inequality test FAILED");
        SetExitCode(1);
    }
    
    // Test defaults
    Animation a0;
    if (IsEmpty(a0.id)) {
        LOG("Animation default id test passed");
    } else {
        LOG("Animation default id test FAILED");
        SetExitCode(1);
    }
    if (IsEmpty(a0.name)) {
        LOG("Animation default name test passed");
    } else {
        LOG("Animation default name test FAILED");
        SetExitCode(1);
    }
    if (a0.category.IsEmpty()) {
        LOG("Animation default category test passed");
    } else {
        LOG("Animation default category test FAILED");
        SetExitCode(1);
    }
    if (a0.frames.GetCount() == 0) {
        LOG("Animation default frames array test passed");
    } else {
        LOG("Animation default frames array test FAILED");
        SetExitCode(1);
    }
    
    LOG("Animation tests completed.\n");
}

// Test AnimationProject operations
void TestAnimationProject() {
    LOG("Testing AnimationProject operations...");
    
    AnimationProject p1;
    p1.id = "project1";
    p1.name = "Test Project";
    
    // Add a sprite
    Sprite s;
    s.id = "sprite1";
    s.category = "character";
    p1.sprites.Add(pick(s));
    
    // Add a frame
    Frame f;
    f.id = "frame1";
    f.name = "Frame 1";
    p1.frames.Add(pick(f));
    
    // Add an animation
    Animation a;
    a.id = "anim1";
    a.category = "character";
    p1.animations.Add(pick(a));
    
    AnimationProject p2;
    p2.id = "project1";
    p2.name = "Test Project";
    
    // Replicate the content for p2
    Sprite s2;
    s2.id = "sprite1";
    s2.category = "character";
    p2.sprites.Add(pick(s2));
    
    Frame f2;
    f2.id = "frame1";
    f2.name = "Frame 1";
    p2.frames.Add(pick(f2));
    
    Animation a2;
    a2.id = "anim1";
    a2.category = "character";
    p2.animations.Add(pick(a2));
    
    AnimationProject p3;
    p3.id = "project2";
    
    // Test equality
    if (p1 == p2) {
        LOG("AnimationProject equality test passed");
    } else {
        LOG("AnimationProject equality test FAILED");
        SetExitCode(1);
    }
    if (p1 != p3) {
        LOG("AnimationProject inequality test passed");
    } else {
        LOG("AnimationProject inequality test FAILED");
        SetExitCode(1);
    }
    
    // Test defaults
    AnimationProject p0;
    if (IsEmpty(p0.id)) {
        LOG("AnimationProject default id test passed");
    } else {
        LOG("AnimationProject default id test FAILED");
        SetExitCode(1);
    }
    if (IsEmpty(p0.name)) {
        LOG("AnimationProject default name test passed");
    } else {
        LOG("AnimationProject default name test FAILED");
        SetExitCode(1);
    }
    if (p0.sprites.GetCount() == 0) {
        LOG("AnimationProject default sprites array test passed");
    } else {
        LOG("AnimationProject default sprites array test FAILED");
        SetExitCode(1);
    }
    if (p0.frames.GetCount() == 0) {
        LOG("AnimationProject default frames array test passed");
    } else {
        LOG("AnimationProject default frames array test FAILED");
        SetExitCode(1);
    }
    if (p0.animations.GetCount() == 0) {
        LOG("AnimationProject default animations array test passed");
    } else {
        LOG("AnimationProject default animations array test FAILED");
        SetExitCode(1);
    }
    
    // Test Find methods with a populated project
    const Sprite* foundSprite = p1.FindSprite("sprite1");
    const Frame* foundFrame = p1.FindFrame("frame1");
    const Animation* foundAnim = p1.FindAnimation("anim1");
    
    if (foundSprite != nullptr) {
        LOG("FindSprite test passed");
    } else {
        LOG("FindSprite test FAILED");
        SetExitCode(1);
    }
    if (foundFrame != nullptr) {
        LOG("FindFrame test passed");
    } else {
        LOG("FindFrame test FAILED");
        SetExitCode(1);
    }
    if (foundAnim != nullptr) {
        LOG("FindAnimation test passed");
    } else {
        LOG("FindAnimation test FAILED");
        SetExitCode(1);
    }
    
    if (foundSprite && foundSprite->id == "sprite1") {
        LOG("FindSprite correct object test passed");
    } else {
        LOG("FindSprite correct object test FAILED");
        SetExitCode(1);
    }
    if (foundFrame && foundFrame->id == "frame1") {
        LOG("FindFrame correct object test passed");
    } else {
        LOG("FindFrame correct object test FAILED");
        SetExitCode(1);
    }
    if (foundAnim && foundAnim->id == "anim1") {
        LOG("FindAnimation correct object test passed");
    } else {
        LOG("FindAnimation correct object test FAILED");
        SetExitCode(1);
    }
    
    LOG("AnimationProject tests completed.\n");
}

// Test Find methods
void TestFindMethods() {
    LOG("Testing Find methods...");
    
    AnimationProject proj;
    proj.id = "test_project";
    proj.name = "Find Test Project";
    
    // Add test objects
    Sprite s;
    s.id = "test_sprite";
    s.category = "character";
    proj.sprites.Add(pick(s));
    
    Frame f;
    f.id = "test_frame";
    f.name = "Test Frame";
    proj.frames.Add(pick(f));
    
    Animation a;
    a.id = "test_anim";
    a.name = "Test Animation";
    a.category = "character";
    proj.animations.Add(pick(a));
    
    // Test const versions
    const Sprite* cs = proj.FindSprite("test_sprite");
    const Frame* cf = proj.FindFrame("test_frame");
    const Animation* ca = proj.FindAnimation("test_anim");
    
    if (cs != nullptr) {
        LOG("Const FindSprite found test passed");
    } else {
        LOG("Const FindSprite found test FAILED");
        SetExitCode(1);
    }
    if (cf != nullptr) {
        LOG("Const FindFrame found test passed");
    } else {
        LOG("Const FindFrame found test FAILED");
        SetExitCode(1);
    }
    if (ca != nullptr) {
        LOG("Const FindAnimation found test passed");
    } else {
        LOG("Const FindAnimation found test FAILED");
        SetExitCode(1);
    }
    
    // Test not found case
    const Sprite* notFoundS = proj.FindSprite("nonexistent");
    const Frame* notFoundF = proj.FindFrame("nonexistent");
    const Animation* notFoundA = proj.FindAnimation("nonexistent");
    
    if (notFoundS == nullptr) {
        LOG("Const FindSprite not found test passed");
    } else {
        LOG("Const FindSprite not found test FAILED");
        SetExitCode(1);
    }
    if (notFoundF == nullptr) {
        LOG("Const FindFrame not found test passed");
    } else {
        LOG("Const FindFrame not found test FAILED");
        SetExitCode(1);
    }
    if (notFoundA == nullptr) {
        LOG("Const FindAnimation not found test passed");
    } else {
        LOG("Const FindAnimation not found test FAILED");
        SetExitCode(1);
    }
    
    // Test non-const versions
    Sprite* ns = proj.FindSprite("test_sprite");
    Frame* nf = proj.FindFrame("test_frame");
    Animation* na = proj.FindAnimation("test_anim");
    
    if (ns != nullptr) {
        LOG("Non-const FindSprite found test passed");
    } else {
        LOG("Non-const FindSprite found test FAILED");
        SetExitCode(1);
    }
    if (nf != nullptr) {
        LOG("Non-const FindFrame found test passed");
    } else {
        LOG("Non-const FindFrame found test FAILED");
        SetExitCode(1);
    }
    if (na != nullptr) {
        LOG("Non-const FindAnimation found test passed");
    } else {
        LOG("Non-const FindAnimation found test FAILED");
        SetExitCode(1);
    }
    
    LOG("Find methods tests completed.\n");
}

// Test serialization round-trip
void TestSerialization() {
    LOG("Testing serialization round-trip...");
    
    AnimationProject proj;
    proj.id = "serial_test_proj";
    proj.name = "Serialization Test Project";
    
    // Add a sprite
    Sprite s;
    s.id = "serial_sprite";
    s.category = "character";
    s.texture_path = "serial_texture.png";
    s.region = RectF(0, 0, 64, 64);
    s.pivot = Vec2(32, 32);
    proj.sprites.Add(pick(s));
    
    // Add a frame with collision
    Frame f;
    f.id = "serial_frame";
    f.name = "Serial Frame";
    f.default_duration = 0.2;
    
    // Add a sprite instance
    SpriteInstance si;
    si.sprite_id = "serial_sprite";
    si.position = Vec2(10, 20);
    si.rotation = 0.5;
    si.scale = Vec2(1.1, 1.1);
    si.alpha = 0.9;
    si.zindex = 1;
    f.sprites.Add(pick(si));
    
    // Add a collision rectangle
    CollisionRect cr;
    cr.id = "serial_collision";
    cr.rect = RectF(5, 5, 10, 10);
    f.collisions.Add(pick(cr));
    
    proj.frames.Add(pick(f));
    
    // Add an animation
    Animation a;
    a.id = "serial_anim";
    a.name = "Serial Animation";
    a.category = "movement";
    
    FrameRef fr;
    fr.frame_id = "serial_frame";
    fr.has_duration = true;
    fr.duration = 0.3;
    a.frames.Add(pick(fr));
    
    proj.animations.Add(pick(a));
    
    // Serialize
    String serialized = SaveProjectJson(proj);
    if (!serialized.IsEmpty()) {
        LOG("Serialization test passed - output not empty");
    } else {
        LOG("Serialization test FAILED - output is empty");
        SetExitCode(1);
    }
    
    // Deserialize
    AnimationProject deserialized;
    bool success = LoadProjectJson(deserialized, serialized);
    if (success) {
        LOG("Deserialization test passed - no errors");
    } else {
        LOG("Deserialization test FAILED - error occurred");
        SetExitCode(1);
    }
    
    // Compare original and deserialized
    if (proj == deserialized && success) {
        LOG("Round-trip equality test passed");
    } else {
        LOG("Round-trip equality test FAILED");
        SetExitCode(1);
    }
    
    LOG("Serialization tests completed.\n");
}

// Test ID generation utilities
void TestIdGeneration() {
    LOG("Testing ID generation utilities...");
    
    AnimationProject proj;
    proj.id = "test_project";
    proj.name = "ID Gen Test";
    
    // Add some existing sprites
    Sprite s1;
    s1.id = "hero_head";
    proj.sprites.Add(pick(s1));
    
    Sprite s2;
    s2.id = "hero_head_1";
    proj.sprites.Add(pick(s2));
    
    // Test GenerateSpriteId
    // Project contains: hero_head, hero_head_1
    String newSpriteId1 = GenerateSpriteId(proj, "hero_head");
    if (newSpriteId1 == "hero_head_2") {
        LOG("GenerateSpriteId test 1 passed");
    } else {
        LOG("GenerateSpriteId test 1 FAILED - expected hero_head_2, got " + newSpriteId1);
        SetExitCode(1);
    }
    
    String newSpriteId2 = GenerateSpriteId(proj, "new_sprite");
    if (newSpriteId2 == "new_sprite") {
        LOG("GenerateSpriteId test 2 passed");
    } else {
        LOG("GenerateSpriteId test 2 FAILED - expected new_sprite, got " + newSpriteId2);
        SetExitCode(1);
    }
    
    // The third call should still return hero_head_2 since we didn't add newSpriteId1 to the project
    String newSpriteId3 = GenerateSpriteId(proj, "hero_head");
    if (newSpriteId3 == "hero_head_2") {
        LOG("GenerateSpriteId test 3 passed");
    } else {
        LOG("GenerateSpriteId test 3 FAILED - expected hero_head_2, got " + newSpriteId3);
        SetExitCode(1);
    }
    
    // Add some existing frames
    Frame f1;
    f1.id = "walk_frame";
    proj.frames.Add(pick(f1));
    
    Frame f2;
    f2.id = "walk_frame_1";
    proj.frames.Add(pick(f2));
    
    // Test GenerateFrameId
    String newFrameId1 = GenerateFrameId(proj, "walk_frame");
    if (newFrameId1 == "walk_frame_2") {
        LOG("GenerateFrameId test 1 passed");
    } else {
        LOG("GenerateFrameId test 1 FAILED - expected walk_frame_2, got " + newFrameId1);
        SetExitCode(1);
    }
    
    String newFrameId2 = GenerateFrameId(proj, "run_frame");
    if (newFrameId2 == "run_frame") {
        LOG("GenerateFrameId test 2 passed");
    } else {
        LOG("GenerateFrameId test 2 FAILED - expected run_frame, got " + newFrameId2);
        SetExitCode(1);
    }
    
    // Add some existing animations
    Animation a1;
    a1.id = "idle_anim";
    proj.animations.Add(pick(a1));
    
    Animation a2;
    a2.id = "idle_anim_1";
    proj.animations.Add(pick(a2));
    
    // Test GenerateAnimationId
    String newAnimId1 = GenerateAnimationId(proj, "idle_anim");
    if (newAnimId1 == "idle_anim_2") {
        LOG("GenerateAnimationId test 1 passed");
    } else {
        LOG("GenerateAnimationId test 1 FAILED - expected idle_anim_2, got " + newAnimId1);
        SetExitCode(1);
    }
    
    String newAnimId2 = GenerateAnimationId(proj, "jump_anim");
    if (newAnimId2 == "jump_anim") {
        LOG("GenerateAnimationId test 2 passed");
    } else {
        LOG("GenerateAnimationId test 2 FAILED - expected jump_anim, got " + newAnimId2);
        SetExitCode(1);
    }
    
    // Add frame to test collision ID generation
    Frame cf;
    cf.id = "test_frame";
    proj.frames.Add(pick(cf));
    
    // Test GenerateCollisionId
    String newCollisionId1 = GenerateCollisionId(proj, "test_frame", "collision");
    if (newCollisionId1 == "test_frame_collision") {
        LOG("GenerateCollisionId test 1 passed");
    } else {
        LOG("GenerateCollisionId test 1 FAILED - expected test_frame_collision, got " + newCollisionId1);
        SetExitCode(1);
    }
    
    // Add existing collision to the frame in the project
    for(int i = 0; i < proj.frames.GetCount(); i++) {
        if(proj.frames[i].id == "test_frame") {
            CollisionRect existingCr;
            existingCr.id = "test_frame_collision";
            proj.frames[i].collisions.Add(pick(existingCr));
            break;
        }
    }
    
    String newCollisionId2 = GenerateCollisionId(proj, "test_frame", "collision");
    if (newCollisionId2 == "test_frame_collision_1") {
        LOG("GenerateCollisionId test 2 passed");
    } else {
        LOG("GenerateCollisionId test 2 FAILED - expected test_frame_collision_1, got " + newCollisionId2);
        SetExitCode(1);
    }
    
    LOG("ID generation tests completed.\n");
}

// Test validation utilities
void TestValidation() {
    LOG("Testing validation utilities...");
    
    // Test valid project
    AnimationProject validProj;
    validProj.id = "valid_proj";
    validProj.name = "Valid Project";
    
    // Add a sprite
    Sprite sprite;
    sprite.id = "valid_sprite";
    sprite.category = "character";
    sprite.texture_path = "valid_texture.png";
    sprite.region = RectF(0, 0, 10, 10);
    sprite.pivot = Vec2(5, 5);
    validProj.sprites.Add(pick(sprite));
    
    // Add a frame
    Frame frame;
    frame.id = "valid_frame";
    frame.name = "Valid Frame";
    validProj.frames.Add(pick(frame));
    
    // Add an animation
    Animation anim;
    anim.id = "valid_anim";
    anim.name = "Valid Animation";
    anim.category = "movement";
    
    FrameRef fr;
    fr.frame_id = "valid_frame";
    fr.has_duration = true;
    fr.duration = 0.15;
    anim.frames.Add(pick(fr));
    
    validProj.animations.Add(pick(anim));
    
    String errorOut;
    bool validResult = ValidateProject(validProj, errorOut);
    if (validResult) {
        LOG("Valid project validation test passed");
    } else {
        LOG("Valid project validation test FAILED");
        SetExitCode(1);
    }
    if (errorOut.IsEmpty()) {
        LOG("Valid project has no error message test passed");
    } else {
        LOG("Valid project has no error message test FAILED - got error: " + errorOut);
        SetExitCode(1);
    }
    
    // Test invalid project with missing frame reference
    AnimationProject invalidProj;
    invalidProj.id = "invalid_proj";
    invalidProj.name = "Invalid Project";
    
    Animation invalidAnim;
    invalidAnim.id = "invalid_anim";
    invalidAnim.name = "Invalid Animation";
    
    FrameRef invalidFr;
    invalidFr.frame_id = "nonexistent_frame";
    invalidFr.has_duration = true;
    invalidFr.duration = 0.15;
    invalidAnim.frames.Add(pick(invalidFr));
    
    invalidProj.animations.Add(pick(invalidAnim));
    
    bool invalidResult = ValidateProject(invalidProj, errorOut);
    if (!invalidResult) {
        LOG("Invalid project validation test passed");
    } else {
        LOG("Invalid project validation test FAILED - should have returned false");
        SetExitCode(1);
    }
    if (!errorOut.IsEmpty()) {
        LOG("Invalid project has error message test passed");
    } else {
        LOG("Invalid project has error message test FAILED - no error message");
        SetExitCode(1);
    }
    
    // Test invalid project with missing sprite reference
    AnimationProject invalidProj2;
    invalidProj2.id = "invalid_proj2";
    invalidProj2.name = "Invalid Project 2";
    
    Frame invalidFrame;
    invalidFrame.id = "invalid_frame";
    invalidFrame.name = "Invalid Frame";
    
    SpriteInstance invalidSi;
    invalidSi.sprite_id = "nonexistent_sprite";
    invalidFrame.sprites.Add(pick(invalidSi));
    
    invalidProj2.frames.Add(pick(invalidFrame));
    
    bool invalidResult2 = ValidateProject(invalidProj2, errorOut);
    if (!invalidResult2) {
        LOG("Invalid project 2 validation test passed");
    } else {
        LOG("Invalid project 2 validation test FAILED - should have returned false");
        SetExitCode(1);
    }
    if (!errorOut.IsEmpty()) {
        LOG("Invalid project 2 has error message test passed");
    } else {
        LOG("Invalid project 2 has error message test FAILED - no error message");
        SetExitCode(1);
    }
    
    // Test invalid project with invalid collision rect
    AnimationProject invalidProj3;
    invalidProj3.id = "invalid_proj3";
    invalidProj3.name = "Invalid Project 3";
    
    Frame invalidFrame2;
    invalidFrame2.id = "invalid_frame2";
    invalidFrame2.name = "Invalid Frame 2";
    
    CollisionRect invalidCr;
    invalidCr.id = "invalid_collision";
    invalidCr.rect = RectF(0, 0, -5, 10);  // Negative width
    invalidFrame2.collisions.Add(pick(invalidCr));
    
    invalidProj3.frames.Add(pick(invalidFrame2));
    
    bool invalidResult3 = ValidateProject(invalidProj3, errorOut);
    if (!invalidResult3) {
        LOG("Invalid project 3 validation test passed");
    } else {
        LOG("Invalid project 3 validation test FAILED - should have returned false");
        SetExitCode(1);
    }
    if (!errorOut.IsEmpty()) {
        LOG("Invalid project 3 has error message test passed");
    } else {
        LOG("Invalid project 3 has error message test FAILED - no error message");
        SetExitCode(1);
    }
    
    LOG("Validation tests completed.\n");
}

// Test AnimResourceRegistry
void TestResourceRegistry() {
    LOG("Testing AnimResourceRegistry...");
    
    using namespace Upp;
    Upp::AnimResourceRegistry registry;
    
    // Test adding a sprite
    Sprite sprite;
    sprite.id = "test_sprite";
    sprite.category = "character";
    sprite.texture_path = "test.png";
    sprite.region = RectF(0, 0, 32, 32);
    sprite.pivot = Vec2(16, 16);
    
    registry.AddSprite(pick(sprite));
    
    // Test getting the sprite back
    const Sprite* retrievedSprite = registry.GetSprite("test_sprite");
    if (retrievedSprite && retrievedSprite->id == "test_sprite") {
        LOG("Sprite add/get test passed");
    } else {
        LOG("Sprite add/get test FAILED");
        SetExitCode(1);
    }
    
    // Test getting all sprite IDs
    Vector<String> spriteIds = registry.GetAllSpriteIds();
    if (spriteIds.GetCount() == 1 && spriteIds[0] == "test_sprite") {
        LOG("GetAllSpriteIds test passed");
    } else {
        LOG("GetAllSpriteIds test FAILED");
        SetExitCode(1);
    }
    
    // Test adding a frame
    Frame frame;
    frame.id = "test_frame";
    frame.name = "Test Frame";
    
    SpriteInstance si;
    si.sprite_id = "test_sprite";
    si.position = Vec2(10, 10);
    frame.sprites.Add(pick(si));
    
    registry.AddFrame(pick(frame));
    
    // Test getting the frame back
    const Frame* retrievedFrame = registry.GetFrame("test_frame");
    if (retrievedFrame && retrievedFrame->id == "test_frame") {
        LOG("Frame add/get test passed");
    } else {
        LOG("Frame add/get test FAILED");
        SetExitCode(1);
    }
    
    // Test adding an animation
    Animation anim;
    anim.id = "test_anim";
    anim.name = "Test Animation";
    anim.category = "movement";
    
    FrameRef fr;
    fr.frame_id = "test_frame";
    fr.has_duration = true;
    fr.duration = 0.15;
    anim.frames.Add(pick(fr));
    
    registry.AddAnimation(pick(anim));
    
    // Test getting the animation back
    const Animation* retrievedAnim = registry.GetAnimation("test_anim");
    if (retrievedAnim && retrievedAnim->id == "test_anim") {
        LOG("Animation add/get test passed");
    } else {
        LOG("Animation add/get test FAILED");
        SetExitCode(1);
    }
    
    // Test removing a sprite
    registry.RemoveSprite("test_sprite");
    const Sprite* removedSprite = registry.GetSprite("test_sprite");
    if (!removedSprite) {
        LOG("Sprite remove test passed");
    } else {
        LOG("Sprite remove test FAILED");
        SetExitCode(1);
    }
    
    // Test clear operations
    registry.ClearProject();
    if (registry.GetAllSpriteIds().GetCount() == 0 && 
        registry.GetAllFrameIds().GetCount() == 0 && 
        registry.GetAllAnimationIds().GetCount() == 0) {
        LOG("ClearProject test passed");
    } else {
        LOG("ClearProject test FAILED");
        SetExitCode(1);
    }
    
    LOG("AnimResourceRegistry tests completed.\n");
}

CONSOLE_APP_MAIN
{
    StdLogSetup(LOG_COUT | LOG_FILE);
    
    LOG("Starting comprehensive AnimEdit unit tests...\n");
    
    TestVec2();
    TestRectF();
    TestSprite();
    TestSpriteInstance();
    TestCollisionRect();
    TestFrame();
    TestFrameRef();
    TestAnimation();
    TestAnimationProject();
    TestFindMethods();
    TestSerialization();
    TestIdGeneration();
    TestValidation();
    TestResourceRegistry();
    
    LOG("All tests completed successfully!");
}