#ifndef _Core_TOON_h_
#define _Core_TOON_h_

Value ParseTOON(const char *s, bool strict = true, int indentSize = 2, bool expandPaths = false);
String AsTOON(const Value& v, int indentSize = 2, int delimiter = ',', bool keyFolding = false, int flattenDepth = INT_MAX);

template <class T>
String StoreAsTOON(const T& var, int indentSize = 2, int delimiter = ',', bool keyFolding = false, int flattenDepth = INT_MAX)
{
	return AsTOON(StoreAsJsonValue(var), indentSize, delimiter, keyFolding, flattenDepth);
}

template <class T>
bool LoadFromTOON(T& var, const char *toon, bool strict = true, int indentSize = 2, bool expandPaths = false)
{
	try {
		Value v = ParseTOON(toon, strict, indentSize, expandPaths);
		if(v.IsError())
			return false;
		LoadFromJsonValue(var, v);
	}
	catch(ValueTypeError) {
		return false;
	}
	catch(JsonizeError) {
		return false;
	}
	return true;
}

#endif
