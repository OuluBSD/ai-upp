#include "AnimCore.h"

namespace Upp {

const Sprite* AnimationProject::FindSprite(const String& sid) const {
    for(const Sprite& s : sprites)
        if(s.id == sid) return &s;
    return nullptr;
}

Sprite* AnimationProject::FindSprite(const String& sid) {
    for(Sprite& s : sprites)
        if(s.id == sid) return &s;
    return nullptr;
}

const Frame* AnimationProject::FindFrame(const String& fid) const {
    for(const Frame& f : frames)
        if(f.id == fid) return &f;
    return nullptr;
}

Frame* AnimationProject::FindFrame(const String& fid) {
    for(Frame& f : frames)
        if(f.id == fid) return &f;
    return nullptr;
}

const Animation* AnimationProject::FindAnimation(const String& aid) const {
    for(const Animation& a : animations)
        if(a.id == aid) return &a;
    return nullptr;
}

Animation* AnimationProject::FindAnimation(const String& aid) {
    for(Animation& a : animations)
        if(a.id == aid) return &a;
    return nullptr;
}

const Entity* AnimationProject::FindEntity(const String& eid) const {
    for(const Entity& e : entities)
        if(e.id == eid) return &e;
    return nullptr;
}

Entity* AnimationProject::FindEntity(const String& eid) {
    for(Entity& e : entities)
        if(e.id == eid) return &e;
    return nullptr;
}

}