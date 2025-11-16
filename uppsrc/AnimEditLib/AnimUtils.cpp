#include "AnimUtils.h"

namespace Upp {

// Helper function to check if a String already exists in a Vector<String>
bool ContainsString(const Vector<String>& container, const String& str) {
    for(int i = 0; i < container.GetCount(); i++) {
        if(container[i] == str) {
            return true;
        }
    }
    return false;
}

// Helper function to check if an ID already exists in a container of objects with .id field
template<typename Container>
bool ContainsId(const Container& container, const String& id) {
    for(int i = 0; i < container.GetCount(); i++) {
        if(container[i].id == id) {
            return true;
        }
    }
    return false;
}

// Helper function to generate a unique ID
String GenerateUniqueId(const Vector<String>& existingIds, const String& base) {
    if (!ContainsString(existingIds, base)) {
        return base;
    }
    
    int counter = 1;
    while(true) {
        String candidate = base + "_" + IntStr(counter);
        if (!ContainsString(existingIds, candidate)) {
            return candidate;
        }
        counter++;
    }
}

String GenerateSpriteId(const AnimationProject& p, const String& base) {
    Vector<String> existingIds;
    for(int i = 0; i < p.sprites.GetCount(); i++) {
        existingIds.Add(p.sprites[i].id);
    }
    return GenerateUniqueId(existingIds, base);
}

String GenerateFrameId(const AnimationProject& p, const String& base) {
    Vector<String> existingIds;
    for(int i = 0; i < p.frames.GetCount(); i++) {
        existingIds.Add(p.frames[i].id);
    }
    return GenerateUniqueId(existingIds, base);
}

String GenerateAnimationId(const AnimationProject& p, const String& base) {
    Vector<String> existingIds;
    for(int i = 0; i < p.animations.GetCount(); i++) {
        existingIds.Add(p.animations[i].id);
    }
    return GenerateUniqueId(existingIds, base);
}

String GenerateCollisionId(const AnimationProject& p, const String& frameId, const String& base) {
    const Frame* frame = p.FindFrame(frameId);
    if (!frame) {
        // If frame doesn't exist, just use a generic pattern
        Vector<String> allCollisionIds;
        for(int i = 0; i < p.frames.GetCount(); i++) {
            const Frame& f = p.frames[i];
            for(int j = 0; j < f.collisions.GetCount(); j++) {
                allCollisionIds.Add(f.collisions[j].id);
            }
        }
        return GenerateUniqueId(allCollisionIds, frameId + "_" + base);
    }
    
    Vector<String> existingIds;
    for(int i = 0; i < frame->collisions.GetCount(); i++) {
        existingIds.Add(frame->collisions[i].id);
    }
    
    return GenerateUniqueId(existingIds, frameId + "_" + base);
}



Vector<String> FindDanglingSpriteReferences(const AnimationProject& p) {
    Vector<String> danglingRefs;
    
    // Get all sprite IDs in the project
    Vector<String> allSpriteIds;
    for (int i = 0; i < p.sprites.GetCount(); i++) {
        allSpriteIds.Add(p.sprites[i].id);
    }
    
    // Check each frame for sprite references
    for (int i = 0; i < p.frames.GetCount(); i++) {
        const Frame& frame = p.frames[i];
        for (int j = 0; j < frame.sprites.GetCount(); j++) {
            const SpriteInstance& si = frame.sprites[j];
            // If the sprite ID is not in the list of existing sprites, it's dangling
            if (!ContainsString(allSpriteIds, si.sprite_id)) {
                // Create a unique identifier for this dangling reference
                String refId = "Frame: " + frame.id + " -> SpriteID: " + si.sprite_id;
                if (!ContainsString(danglingRefs, refId)) { // Only add if not already there
                    danglingRefs.Add(refId);
                }
            }
        }
    }
    
    return danglingRefs;
}

// Validation helpers
bool ValidateProject(const AnimationProject& p, String& errorOut) {
    // Check for empty project ID
    if (p.id.IsEmpty()) {
        errorOut = "Project ID cannot be empty";
        return false;
    }

    // Check for empty project name
    if (p.name.IsEmpty()) {
        errorOut = "Project name cannot be empty";
        return false;
    }

    // Validate sprites
    for(int i = 0; i < p.sprites.GetCount(); i++) {
        const Sprite& s = p.sprites[i];
        if (s.id.IsEmpty()) {
            errorOut = "Sprite at index " + IntStr(i) + " has an empty ID";
            return false;
        }
        
        // Check for duplicate sprite IDs
        for(int j = i + 1; j < p.sprites.GetCount(); j++) {
            if (p.sprites[j].id == s.id) {
                errorOut = "Duplicate sprite ID found: " + s.id;
                return false;
            }
        }
    }

    // Validate frames
    for(int i = 0; i < p.frames.GetCount(); i++) {
        const Frame& f = p.frames[i];
        if (f.id.IsEmpty()) {
            errorOut = "Frame at index " + IntStr(i) + " has an empty ID";
            return false;
        }
        
        // Check for duplicate frame IDs
        for(int j = i + 1; j < p.frames.GetCount(); j++) {
            if (p.frames[j].id == f.id) {
                errorOut = "Duplicate frame ID found: " + f.id;
                return false;
            }
        }
        
        // Validate frame content
        if (!ValidateSpriteLinks(p, f, errorOut)) {
            return false;
        }
        
        // Check frame duration
        if (f.default_duration <= 0) {
            errorOut = "Frame '" + f.id + "' has invalid duration (must be > 0)";
            return false;
        }
    }

    // Validate animations
    for(int i = 0; i < p.animations.GetCount(); i++) {
        const Animation& a = p.animations[i];
        if (a.id.IsEmpty()) {
            errorOut = "Animation at index " + IntStr(i) + " has an empty ID";
            return false;
        }
        
        // Check for duplicate animation IDs
        for(int j = i + 1; j < p.animations.GetCount(); j++) {
            if (p.animations[j].id == a.id) {
                errorOut = "Duplicate animation ID found: " + a.id;
                return false;
            }
        }
        
        // Validate animation content
        if (!ValidateFrameLinks(p, a, errorOut)) {
            return false;
        }
    }

    // All validations passed
    return true;
}

bool ValidateAnimation(const AnimationProject& p, const Animation& a, String& errorOut) {
    if (a.id.IsEmpty()) {
        errorOut = "Animation has an empty ID";
        return false;
    }
    
    if (!ValidateFrameLinks(p, a, errorOut)) {
        return false;
    }
    
    return true;
}

bool ValidateFrameLinks(const AnimationProject& p, const Animation& a, String& errorOut) {
    for(int i = 0; i < a.frames.GetCount(); i++) {
        const FrameRef& fr = a.frames[i];
        if (fr.frame_id.IsEmpty()) {
            errorOut = "Animation '" + a.id + "' has frame reference with empty ID at index " + IntStr(i);
            return false;
        }
        
        // Check if the referenced frame exists
        if (!p.FindFrame(fr.frame_id)) {
            errorOut = "Animation '" + a.id + "' references non-existent frame: " + fr.frame_id;
            return false;
        }
        
        // Check duration if duration override is specified
        if (fr.has_duration && fr.duration <= 0) {
            errorOut = "Animation '" + a.id + "' has invalid duration (must be > 0) for frame reference: " + fr.frame_id;
            return false;
        }
    }
    
    return true;
}

bool ValidateSpriteLinks(const AnimationProject& p, const Frame& f, String& errorOut) {
    for(int i = 0; i < f.sprites.GetCount(); i++) {
        const SpriteInstance& si = f.sprites[i];
        if (si.sprite_id.IsEmpty()) {
            errorOut = "Frame '" + f.id + "' has sprite instance with empty sprite ID at index " + IntStr(i);
            return false;
        }
        
        // Check if the referenced sprite exists
        if (!p.FindSprite(si.sprite_id)) {
            errorOut = "Frame '" + f.id + "' references non-existent sprite: " + si.sprite_id;
            return false;
        }
    }
    
    // Validate collision rectangles
    for(int i = 0; i < f.collisions.GetCount(); i++) {
        const CollisionRect& cr = f.collisions[i];
        if (cr.id.IsEmpty()) {
            errorOut = "Frame '" + f.id + "' has collision rectangle with empty ID at index " + IntStr(i);
            return false;
        }
        
        // Check if collision rectangle has valid positive size
        if (cr.rect.cx <= 0 || cr.rect.cy <= 0) {
            errorOut = "Frame '" + f.id + "' has collision rectangle with invalid size (must be > 0): " + cr.id;
            return false;
        }
    }
    
    return true;
}

bool ValidateAnimationBlendParams(const AnimationBlendParams& params, String& errorOut) {
    if (params.weight < 0.0 || params.weight > 1.0) {
        errorOut = "Invalid blend weight (must be between 0.0 and 1.0): " + DoubleStr(params.weight);
        return false;
    }
    
    if (params.transition_time < 0.0) {
        errorOut = "Invalid transition time (must be >= 0.0): " + DoubleStr(params.transition_time);
        return false;
    }
    
    return true;
}

bool ValidateEntity(const AnimationProject& p, const Entity& e, String& errorOut) {
    if (e.id.IsEmpty()) {
        errorOut = "Entity has an empty ID";
        return false;
    }

    if (!ValidateEntityLinks(p, e, errorOut)) {
        return false;
    }

    return true;
}

bool ValidateEntityLinks(const AnimationProject& p, const Entity& e, String& errorOut) {
    for(int i = 0; i < e.animation_slots.GetCount(); i++) {
        const NamedAnimationSlot& slot = e.animation_slots[i];
        if (slot.name.IsEmpty()) {
            errorOut = "Entity '" + e.id + "' has animation slot with empty name at index " + IntStr(i);
            return false;
        }

        if (slot.animation_id.IsEmpty()) {
            errorOut = "Entity '" + e.id + "' has animation slot '" + slot.name + "' with empty animation ID at index " + IntStr(i);
            return false;
        }

        // Check if the referenced animation exists
        if (!p.FindAnimation(slot.animation_id)) {
            errorOut = "Entity '" + e.id + "' references non-existent animation: " + slot.animation_id;
            return false;
        }
        
        // Validate blend parameters
        if (slot.blend_params.weight < 0.0 || slot.blend_params.weight > 1.0) {
            errorOut = "Entity '" + e.id + "' has animation slot '" + slot.name + "' with invalid blend weight (must be between 0.0 and 1.0)";
            return false;
        }
        
        if (slot.blend_params.transition_time < 0.0) {
            errorOut = "Entity '" + e.id + "' has animation slot '" + slot.name + "' with invalid transition time (must be >= 0.0)";
            return false;
        }
    }

    return true;
}

} // namespace Upp