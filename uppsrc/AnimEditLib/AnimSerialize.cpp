#include "AnimSerialize.h"

namespace Upp {

void Jsonize(JsonIO& jio, Vec2& v) {
    jio("x", v.x)("y", v.y);
}

void Jsonize(JsonIO& jio, RectF& r) {
    jio("x", r.x)("y", r.y)("cx", r.cx)("cy", r.cy);
}

void Jsonize(JsonIO& jio, Sprite& s) {
    jio("id", s.id)("name", s.name)("category", s.category)("texture_path", s.texture_path)
       ("region", s.region)("pivot", s.pivot)("tags", s.tags)("description", s.description);
}

void Jsonize(JsonIO& jio, SpriteInstance& si) {
    jio("sprite_id", si.sprite_id)("position", si.position)
       ("rotation", si.rotation)("scale", si.scale)
       ("alpha", si.alpha)("zindex", si.zindex);
}

void Jsonize(JsonIO& jio, CollisionRect& cr) {
    jio("id", cr.id)("rect", cr.rect);
}

void Jsonize(JsonIO& jio, Frame& f) {
    jio("id", f.id)("name", f.name)("sprites", f.sprites)
       ("collisions", f.collisions)("default_duration", f.default_duration);
}

void Jsonize(JsonIO& jio, FrameRef& fr) {
    jio("frame_id", fr.frame_id)("has_duration", fr.has_duration);
    if(fr.has_duration) {
        jio("duration", fr.duration);
    }
}

void Jsonize(JsonIO& jio, Animation& a) {
    jio("id", a.id)("name", a.name)("category", a.category)("frames", a.frames);
}

void Jsonize(JsonIO& jio, NamedAnimationSlot& slot) {
    jio("name", slot.name)("animation_id", slot.animation_id);
}

void Jsonize(JsonIO& jio, Entity& e) {
    jio("id", e.id)("name", e.name)("type", e.type)
       ("animation_slots", e.animation_slots)
       ("properties", e.properties);
}

void Jsonize(JsonIO& jio, AnimationProject& p) {
    jio("id", p.id)("name", p.name)
       ("sprites", p.sprites)
       ("frames", p.frames)
       ("animations", p.animations)
       ("entities", p.entities);
}

String SaveProjectJson(const AnimationProject& p) {
    return StoreAsJson(p, true); // true for pretty formatting
}

bool LoadProjectJson(AnimationProject& p, const String& json) {
    return LoadFromJson(p, json);
}

} // namespace Upp