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

struct AnimationProject : public Moveable<AnimationProject> {
    String id;
    String name;

    Vector<Sprite>     sprites;
    Vector<Frame>      frames;
    Vector<Animation>  animations;

    const Sprite*    FindSprite(const String&) const;
    Sprite*          FindSprite(const String&);

    const Frame*     FindFrame(const String&) const;
    Frame*           FindFrame(const String&);

    const Animation* FindAnimation(const String&) const;
    Animation*       FindAnimation(const String&);
    
    bool operator==(const AnimationProject& other) const { 
        return id == other.id && name == other.name && 
               sprites == other.sprites && frames == other.frames &&
               animations == other.animations; 
    }
    bool operator!=(const AnimationProject& other) const { return !(*this == other); }
    
    void Swap(AnimationProject& other) {
        Upp::Swap(id, other.id);
        Upp::Swap(name, other.name);
        Upp::Swap(sprites, other.sprites);
        Upp::Swap(frames, other.frames);
        Upp::Swap(animations, other.animations);
    }
};

} // namespace Upp

#endif