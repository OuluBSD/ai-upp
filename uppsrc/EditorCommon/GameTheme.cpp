#include <EditorCommon/GameTheme.h>
#include <EditorCommon/EditorCommon.h>
#include <climits>

namespace Upp {

static Value UnwrapTypedValue(const Value& in)
{
	Value v = in;
	for (;;) {
		if (!v.Is<ValueMap>())
			break;
		ValueMap m = v;
		int qt = m.Find("type");
		int qv = m.Find("value");
		if (qt < 0 || qv < 0)
			break;
		String t = ToLower(AsString(m.GetValue(qt)));
		if (t == "string" || t == "wstring" || t == "int" || t == "int64" ||
		    t == "double" || t == "bool" || t == "valuemap" || t == "valuearray") {
			v = m.GetValue(qv);
			continue;
		}
		break;
	}
	return v;
}

static String DecodeTypedScalar(const Value& v)
{
	Value s = UnwrapTypedValue(v);
	if (s.Is<String>())
		return s;
	return AsString(s);
}

static void LoadThemeSettingsFromValue(const Value& src, VectorMap<String, String>& dst)
{
	dst.Clear();
	Value raw = UnwrapTypedValue(src);
	if (raw.Is<ValueMap>()) {
		ValueMap m = raw;
		const Index<Value>& keys = m.GetKeys();
		ValueArray vals = m.GetValues();
		for (int i = 0; i < keys.GetCount() && i < vals.GetCount(); i++) {
			String k = AsString(keys[i]);
			String v = DecodeTypedScalar(vals[i]);
			dst.GetAdd(k) = v;
		}
		return;
	}
	if (raw.Is<ValueArray>()) {
		ValueArray arr = raw;
		for (const Value& it : arr) {
			Value row_value = UnwrapTypedValue(it);
			if (!row_value.Is<ValueMap>())
				continue;
			ValueMap row = row_value;
			int qk = row.Find("key");
			int qv = row.Find("value");
			if (qk < 0 || qv < 0)
				continue;
			String k = DecodeTypedScalar(row.GetValue(qk));
			String v = DecodeTypedScalar(row.GetValue(qv));
			dst.GetAdd(k) = v;
		}
	}
}

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

void ThemeFile::Jsonize(JsonIO& jio)
{
	jio("objects", objects);
	if (jio.IsLoading()) {
		Value raw = jio.Get("settings");
		LoadThemeSettingsFromValue(raw, settings);
	}
	else {
		ValueMap out;
		for (int i = 0; i < settings.GetCount(); i++)
			out.Add(settings.GetKey(i), settings[i]);
		jio.Set("settings", out);
	}
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
