#include <Core/Core.h>
#include <AnimEditLib/AnimCore.h>
#include <AnimEditLib/AnimSerialize.h>

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
}