#ifndef _GameCommon_GameTheme_h_
#define _GameCommon_GameTheme_h_

#include <Core/Core.h>
#include <Draw/Draw.h>

namespace Upp {

struct ThemeObject : Moveable<ThemeObject> {
	String name;
	String type = "rect";
	Rect rect = RectC(100, 100, 120, 40);
	String text;
	String color = "255,200,0";
	VectorMap<String, String> props;
	bool visible = true;

	ThemeObject() {}
	ThemeObject(const ThemeObject& s) { *this = s; }
	ThemeObject(const ThemeObject& s, int) { *this = s; }
	void operator=(const ThemeObject& s) {
		name = s.name;
		type = s.type;
		rect = s.rect;
		text = s.text;
		color = s.color;
		props <<= s.props;
		visible = s.visible;
	}

	void Jsonize(JsonIO& jio);
};

struct ThemeFile {
	Vector<ThemeObject> objects;
	VectorMap<String, String> settings;
	void Jsonize(JsonIO& jio) { jio("objects", objects)("settings", settings); }
};

Color ThemeParseColor(const String& s, Color def = Color(255, 200, 0));
String ThemeGetPropString(const VectorMap<String, String>& props, const String& key, const String& def = String());
int ThemeGetPropInt(const VectorMap<String, String>& props, const String& key, int def);
bool ThemeGetPropBool(const VectorMap<String, String>& props, const String& key, bool def);
bool ThemeObjectVisibleForProfile(const ThemeObject& o, const String& profile);
String ResolveThemeAssetPath(const String& project_name, const String& asset_path);
String ResolveThemeProfile(const ThemeFile& theme, const String& explicit_profile = String());
String ResolveThemeLayoutProfile(const ThemeFile& theme, const String& platform_name = String(), const String& fallback = String());

}

#endif
