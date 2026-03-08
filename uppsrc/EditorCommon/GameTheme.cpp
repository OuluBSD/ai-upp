#include <EditorCommon/GameTheme.h>
#include <EditorCommon/EditorCommon.h>
#include <climits>

namespace Upp {

void ThemeObject::Jsonize(JsonIO& jio)
{
	bool loading = jio.IsLoading();
	int x = loading ? INT_MIN : rect.left;
	int y = loading ? INT_MIN : rect.top;
	int w = loading ? INT_MIN : rect.GetWidth();
	int h = loading ? INT_MIN : rect.GetHeight();
	jio("name", name)
	   ("type", type)
	   ("rect", rect)
	   ("x", x)
	   ("y", y)
	   ("w", w)
	   ("h", h)
	   ("text", text)
	   ("color", color)
	   ("props", props)
	   ("visible", visible);
	if (loading && x != INT_MIN && y != INT_MIN && w != INT_MIN && h != INT_MIN)
		rect = RectC(x, y, max(1, w), max(1, h));
}

Color ThemeParseColor(const String& s, Color def)
{
	Vector<String> p = Split(TrimBoth(s), ',');
	if (p.GetCount() < 3)
		return def;
	int r = minmax(StrInt(TrimBoth(p[0])), 0, 255);
	int g = minmax(StrInt(TrimBoth(p[1])), 0, 255);
	int b = minmax(StrInt(TrimBoth(p[2])), 0, 255);
	return Color(r, g, b);
}

String ThemeGetPropString(const VectorMap<String, String>& props, const String& key, const String& def)
{
	int q = props.Find(key);
	if (q < 0)
		return def;
	String v = TrimBoth(props[q]);
	return v.IsEmpty() ? def : v;
}

int ThemeGetPropInt(const VectorMap<String, String>& props, const String& key, int def)
{
	int q = props.Find(key);
	if (q < 0)
		return def;
	String v = TrimBoth(props[q]);
	if (v.IsEmpty())
		return def;
	return StrInt(v);
}

bool ThemeGetPropBool(const VectorMap<String, String>& props, const String& key, bool def)
{
	String v = ToLower(ThemeGetPropString(props, key, def ? "1" : "0"));
	return v == "1" || v == "true" || v == "yes" || v == "on";
}

bool ThemeObjectVisibleForProfile(const ThemeObject& o, const String& profile)
{
	if (!o.visible)
		return false;
	String p = TrimBoth(ThemeGetPropString(o.props, "profiles"));
	if (p.IsEmpty())
		return true;
	Vector<String> list = Split(p, ',');
	String want = ToLower(TrimBoth(profile));
	for (const String& it : list) {
		if (ToLower(TrimBoth(it)) == want)
			return true;
	}
	return false;
}

String ResolveThemeAssetPath(const String& project_name, const String& asset_path)
{
	String a = TrimBoth(asset_path);
	if (a.IsEmpty())
		return String();
	if (IsFullPath(a))
		return a;
	return AppendFileName(GetProjectDirPath(project_name), a);
}

String ResolveThemeProfile(const ThemeFile& theme, const String& explicit_profile)
{
	String p = TrimBoth(explicit_profile);
	if (!p.IsEmpty())
		return p;
	int q = theme.settings.Find("profiles");
	if (q >= 0) {
		Vector<String> v = Split(theme.settings[q], ',');
		if (!v.IsEmpty())
			return TrimBoth(v[0]);
	}
	return "default";
}

String ResolveThemeLayoutProfile(const ThemeFile& theme, const String& platform_name, const String& fallback)
{
	int q = theme.settings.Find("layout_profile");
	if (q >= 0) {
		String v = TrimBoth(theme.settings[q]);
		if (!v.IsEmpty())
			return v;
	}
	String fb = TrimBoth(fallback);
	if (!fb.IsEmpty())
		return fb;
	if (ToLower(TrimBoth(platform_name)) == "texas-holdem")
		return "texas-holdem-legacy-pokertable";
	return "texas-holdem-classic";
}

}
