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

void Jsonize(JsonIO& jio, AnimationEvent& event) {
    jio("id", event.id)("name", event.name)("type", event.type)("frame_index", event.frame_index)("parameters", event.parameters);
}

void Jsonize(JsonIO& jio, AnimationTransition& transition) {
    jio("from_animation_id", transition.from_animation_id)
       ("to_animation_id", transition.to_animation_id)
       ("transition_time", transition.transition_time)
       ("condition", transition.condition)
       ("trigger_event", transition.trigger_event);
}

void Jsonize(JsonIO& jio, EntityAnimationParams& params) {
    jio("speed_multiplier", params.speed_multiplier)
       ("time_offset", params.time_offset)
       ("is_looping", params.is_looping);
}

void Jsonize(JsonIO& jio, AnimationBlendParams& params) {
    jio("weight", params.weight)("transition_time", params.transition_time)("is_active", params.is_active)("events", params.events);
}

void Jsonize(JsonIO& jio, NamedAnimationSlot& slot) {
    jio("name", slot.name)("animation_id", slot.animation_id)("blend_params", slot.blend_params);
}

void Jsonize(JsonIO& jio, Entity& e) {
    jio("id", e.id)("name", e.name)("type", e.type)
       ("animation_slots", e.animation_slots)
       ("animation_transitions", e.animation_transitions)
       ("anim_params", e.anim_params)
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