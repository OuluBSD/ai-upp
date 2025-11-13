#ifndef _AnimEditLib_AnimUtils_h_
#define _AnimEditLib_AnimUtils_h_

#include <Core/Core.h>
#include "AnimCore.h"

namespace Upp {

// ID Generation utilities
String GenerateSpriteId(const AnimationProject& p, const String& base);
String GenerateFrameId(const AnimationProject& p, const String& base);
String GenerateAnimationId(const AnimationProject& p, const String& base);
String GenerateCollisionId(const AnimationProject& p, const String& frameId, const String& base);

// Validation helpers
bool ValidateProject(const AnimationProject& p, String& errorOut);
bool ValidateAnimation(const AnimationProject& p, const Animation& a, String& errorOut);
bool ValidateFrameLinks(const AnimationProject& p, const Animation& a, String& errorOut);
bool ValidateSpriteLinks(const AnimationProject& p, const Frame& f, String& errorOut);

} // namespace Upp

#endif