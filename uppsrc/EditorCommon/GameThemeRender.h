#ifndef _GameCommon_GameThemeRender_h_
#define _GameCommon_GameThemeRender_h_

#include <EditorCommon/GameTheme.h>
#include <EditorCommon/RecognitionData.h>
#include <Draw/Draw.h>

namespace Upp {

void ThemeApplyCardTheme(const Vector<ThemeObject>& objs, const String& theme_profile);
Image ThemeOverlayObjects(const Image& src, const Vector<ThemeObject>& objs, const GameState* state,
                          const String& project_name, const String& theme_profile);

}

#endif
