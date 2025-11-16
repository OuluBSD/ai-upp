#ifndef _AnimEditLib_AnimCore_h_
#define _AnimEditLib_AnimCore_h_

#include <Core/Core.h>

namespace Upp {

static const int ANIMEDITLIB_VERSION_MAJOR = 0;
static const int ANIMEDITLIB_VERSION_MINOR = 1;

struct Vec2 : public Moveable<Vec2> {
    double x = 0.0, y = 0.0;
    Vec2() {}
    Vec2(double x, double y) : x(x), y(y) {}
    
    bool operator==(const Vec2& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Vec2& other) const { return !(*this == other); }
    
    void Swap(Vec2& other) {
        Upp::Swap(x, other.x);
        Upp::Swap(y, other.y);
    }
};



struct RectF : public Moveable<RectF> {
    double x = 0.0, y = 0.0, cx = 0.0, cy = 0.0;
    RectF() {}
    RectF(double x, double y, double cx, double cy)
        : x(x), y(y), cx(cx), cy(cy) {}
    
    bool operator==(const RectF& other) const { 
        return x == other.x && y == other.y && cx == other.cx && cy == other.cy; 
    }
    bool operator!=(const RectF& other) const { return !(*this == other); }
    
    void Swap(RectF& other) {
        Upp::Swap(x, other.x);
        Upp::Swap(y, other.y);
        Upp::Swap(cx, other.cx);
        Upp::Swap(cy, other.cy);
    }
};

struct Sprite : public Moveable<Sprite> {
    String id;
    String name;
    String category;
    String texture_path;
    RectF  region;
    Vec2   pivot;
    Vector<String> tags;        // Metadata tags
    String description;         // Description of the sprite

    Sprite() = default;
    Sprite(const String& id)
        : id(id), name(id), category("default"), pivot(0,0) {}  // Default name to id
    
    bool operator==(const Sprite& other) const { 
        return id == other.id && name == other.name && category == other.category && 
               texture_path == other.texture_path && region == other.region && 
               pivot == other.pivot && tags == other.tags && description == other.description; 
    }
    bool operator!=(const Sprite& other) const { return !(*this == other); }
    
    void Swap(Sprite& other) {
        Upp::Swap(id, other.id);
        Upp::Swap(name, other.name);
        Upp::Swap(category, other.category);
        Upp::Swap(texture_path, other.texture_path);
        Upp::Swap(region, other.region);
        Upp::Swap(pivot, other.pivot);
        Upp::Swap(tags, other.tags);
        Upp::Swap(description, other.description);
    }
};

struct SpriteInstance : public Moveable<SpriteInstance> {
    String sprite_id;
    Vec2   position;
    double rotation = 0;
    Vec2   scale = Vec2(1,1);
    double alpha = 1.0;
    int    zindex = 0;
    
    bool operator==(const SpriteInstance& other) const { 
        return sprite_id == other.sprite_id && position == other.position &&
               rotation == other.rotation && scale == other.scale &&
               alpha == other.alpha && zindex == other.zindex; 
    }
    bool operator!=(const SpriteInstance& other) const { return !(*this == other); }
    
    void Swap(SpriteInstance& other) {
        Upp::Swap(sprite_id, other.sprite_id);
        Upp::Swap(position, other.position);
        Upp::Swap(rotation, other.rotation);
        Upp::Swap(scale, other.scale);
        Upp::Swap(alpha, other.alpha);
        Upp::Swap(zindex, other.zindex);
    }
};

struct CollisionRect : public Moveable<CollisionRect> {
    String id;
    RectF  rect;
    
    bool operator==(const CollisionRect& other) const { 
        return id == other.id && rect == other.rect; 
    }
    bool operator!=(const CollisionRect& other) const { return !(*this == other); }
    
    void Swap(CollisionRect& other) {
        Upp::Swap(id, other.id);
        Upp::Swap(rect, other.rect);
    }
};

struct Frame : public Moveable<Frame> {
    String id;
    String name;
    Frame() = default;
    Frame(const String& id) : id(id) {}

    Vector<SpriteInstance> sprites;
    Vector<CollisionRect>  collisions;
    double default_duration = 0.1;
    
    bool operator==(const Frame& other) const { 
        return id == other.id && name == other.name && 
               sprites == other.sprites && collisions == other.collisions &&
               default_duration == other.default_duration; 
    }
    bool operator!=(const Frame& other) const { return !(*this == other); }
    
    void Swap(Frame& other) {
        Upp::Swap(id, other.id);
        Upp::Swap(name, other.name);
        Upp::Swap(sprites, other.sprites);
        Upp::Swap(collisions, other.collisions);
        Upp::Swap(default_duration, other.default_duration);
    }
};

struct FrameRef : public Moveable<FrameRef> {
    String frame_id;
    bool   has_duration = false;
    double duration = 0.0;
    
    bool operator==(const FrameRef& other) const { 
        return frame_id == other.frame_id && has_duration == other.has_duration &&
               duration == other.duration; 
    }
    bool operator!=(const FrameRef& other) const { return !(*this == other); }
    
    void Swap(FrameRef& other) {
        Upp::Swap(frame_id, other.frame_id);
        Upp::Swap(has_duration, other.has_duration);
        Upp::Swap(duration, other.duration);
    }
};

struct Animation : public Moveable<Animation> {
    String id;
    String name;
    String category;
    Vector<FrameRef> frames;
    
    bool operator==(const Animation& other) const { 
        return id == other.id && name == other.name && 
               category == other.category && frames == other.frames; 
    }
    bool operator!=(const Animation& other) const { return !(*this == other); }
    
    void Swap(Animation& other) {
        Upp::Swap(id, other.id);
        Upp::Swap(name, other.name);
        Upp::Swap(category, other.category);
        Upp::Swap(frames, other.frames);
    }
};

struct AnimationEvent : public Moveable<AnimationEvent> {
    String id;
    String name;
    String type;           // Type of event (e.g., "sound", "particle", "callback", "trigger")
    int frame_index;       // Frame at which the event should trigger
    ValueMap parameters;   // Additional parameters specific to the event type

    AnimationEvent() : frame_index(0) {}
    AnimationEvent(const String& id, const String& name, const String& type, int frame_idx)
        : id(id), name(name), type(type), frame_index(frame_idx) {}

    bool operator==(const AnimationEvent& other) const {
        return id == other.id && name == other.name && type == other.type && 
               frame_index == other.frame_index && parameters == other.parameters;
    }
    bool operator!=(const AnimationEvent& other) const { return !(*this == other); }

    void Swap(AnimationEvent& other) {
        Upp::Swap(id, other.id);
        Upp::Swap(name, other.name);
        Upp::Swap(type, other.type);
        Upp::Swap(frame_index, other.frame_index);
        Upp::Swap(parameters, other.parameters);
    }
};

struct AnimationBlendParams : public Moveable<AnimationBlendParams> {
    double weight = 1.0;     // Blend weight (0.0 to 1.0)
    double transition_time = 0.0;  // Transition time in seconds
    bool is_active = false;  // Whether this animation is currently active
    Vector<AnimationEvent> events; // Events that occur during this animation

    AnimationBlendParams() = default;
    
    bool operator==(const AnimationBlendParams& other) const {
        return weight == other.weight && transition_time == other.transition_time && 
               is_active == other.is_active && events == other.events;
    }
    bool operator!=(const AnimationBlendParams& other) const { return !(*this == other); }

    void Swap(AnimationBlendParams& other) {
        Upp::Swap(weight, other.weight);
        Upp::Swap(transition_time, other.transition_time);
        Upp::Swap(is_active, other.is_active);
        Upp::Swap(events, other.events);
    }
};

struct NamedAnimationSlot : public Moveable<NamedAnimationSlot> {
    String name;
    String animation_id;
    AnimationBlendParams blend_params;  // Parameters for animation blending

    NamedAnimationSlot() = default;
    NamedAnimationSlot(const String& name, const String& animation_id)
        : name(name), animation_id(animation_id) {}

    bool operator==(const NamedAnimationSlot& other) const {
        return name == other.name && animation_id == other.animation_id && blend_params == other.blend_params;
    }
    bool operator!=(const NamedAnimationSlot& other) const { return !(*this == other); }

    void Swap(NamedAnimationSlot& other) {
        Upp::Swap(name, other.name);
        Upp::Swap(animation_id, other.animation_id);
        Upp::Swap(blend_params, other.blend_params);
    }
};

struct Entity : public Moveable<Entity> {
    String id;
    String name;
    String type;
    Vector<NamedAnimationSlot> animation_slots;
    ValueMap properties;  // Simple key-value store for behavior parameters

    Entity() = default;
    Entity(const String& id)
        : id(id), name(id), type("default") {}

    bool operator==(const Entity& other) const {
        return id == other.id && name == other.name && type == other.type &&
               animation_slots == other.animation_slots && properties == other.properties;
    }
    bool operator!=(const Entity& other) const { return !(*this == other); }

    void Swap(Entity& other) {
        Upp::Swap(id, other.id);
        Upp::Swap(name, other.name);
        Upp::Swap(type, other.type);
        Upp::Swap(animation_slots, other.animation_slots);
        Upp::Swap(properties, other.properties);
    }
};

struct AnimationProject : public Moveable<AnimationProject> {
    String id;
    String name;

    Vector<Sprite>     sprites;
    Vector<Frame>      frames;
    Vector<Animation>  animations;
    Vector<Entity>     entities;

    const Sprite*    FindSprite(const String&) const;
    Sprite*          FindSprite(const String&);

    const Frame*     FindFrame(const String&) const;
    Frame*           FindFrame(const String&);

    const Animation* FindAnimation(const String&) const;
    Animation*       FindAnimation(const String&);

    const Entity*    FindEntity(const String&) const;
    Entity*          FindEntity(const String&);
    
    bool operator==(const AnimationProject& other) const { 
        return id == other.id && name == other.name && 
               sprites == other.sprites && frames == other.frames &&
               animations == other.animations && entities == other.entities; 
    }
    bool operator!=(const AnimationProject& other) const { return !(*this == other); }
    
    void Swap(AnimationProject& other) {
        Upp::Swap(id, other.id);
        Upp::Swap(name, other.name);
        Upp::Swap(sprites, other.sprites);
        Upp::Swap(frames, other.frames);
        Upp::Swap(animations, other.animations);
        Upp::Swap(entities, other.entities);
    }
};

} // namespace Upp

#endif