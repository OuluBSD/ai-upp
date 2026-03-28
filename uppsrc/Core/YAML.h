Value  ParseYAML(CParser& p, bool strict=true);
Value  ParseYAML(const char *s, bool strict=true);

inline String AsYAML(int i)             { return IsNull(i) ? String("null") : AsString(i); }
inline String AsYAML(double n)          { return IsNull(n) ? String("null") : AsString(n); }
inline String AsYAML(float f)           { return IsNull(f) ? String("null") : AsString(f); }
inline String AsYAML(bool b)            { return b ? "true" : "false"; }
inline String AsYAML(const String& s)   { return s.FindFirstOf("\n\r\t: #{}[]-,") >= 0 || s.IsEmpty() ? AsCString(s, INT_MAX, NULL, ASCSTRING_JSON) : s; }
inline String AsYAML(const WString& s)  { return AsCString(s.ToString(), INT_MAX, NULL, ASCSTRING_JSON); }
inline String AsYAML(const char *s)     { return AsCString(s, INT_MAX, NULL, ASCSTRING_JSON); }
String AsYAML(Time tm);
String AsYAML(Date dt);

String AsYAML(const Value& v, const String& indent, bool pretty);
String AsYAML(const Value& v, bool pretty = false);

class YamlArray;

class Yaml {
	String text;
	int    indent_level;

	void   Indent();

public:
	Yaml& CatRaw(const char *key, const String& val);

	String ToString() const                                     { return text; }
	String operator~() const                                    { return ToString(); }
	operator String() const                                     { return ToString(); }

	operator bool() const                                       { return text.GetCount(); }

	Yaml& operator()(const char *key, const Value& value)       { return CatRaw(key, AsYAML(value)); }
	Yaml& operator()(const char *key, int i)                    { return CatRaw(key, AsYAML(i)); }
	Yaml& operator()(const char *key, double n)                 { return CatRaw(key, AsYAML(n)); }
	Yaml& operator()(const char *key, float f)                  { return CatRaw(key, AsYAML(f)); }
	Yaml& operator()(const char *key, bool b)                   { return CatRaw(key, AsYAML(b)); }
	Yaml& operator()(const char *key, Date d)                   { return CatRaw(key, AsYAML(d)); }
	Yaml& operator()(const char *key, Time t)                   { return CatRaw(key, AsYAML(t)); }
	Yaml& operator()(const char *key, const String& s)          { return CatRaw(key, AsYAML(s)); }
	Yaml& operator()(const char *key, const WString& s)         { return CatRaw(key, AsYAML(s)); }
	Yaml& operator()(const char *key, const char *s)            { return CatRaw(key, AsYAML(s)); }
	Yaml& operator()(const char *key, const Yaml& object)       { return CatRaw(key, ~object); }
	Yaml& operator()(const char *key, const YamlArray& array);

	Yaml() : indent_level(0) {}
	Yaml(const char *key, const Value& value) : indent_level(0) { CatRaw(key, AsYAML(value)); }
	Yaml(const char *key, int i) : indent_level(0)              { CatRaw(key, AsYAML(i)); }
	Yaml(const char *key, double n) : indent_level(0)           { CatRaw(key, AsYAML(n)); }
	Yaml(const char *key, float f) : indent_level(0)            { CatRaw(key, AsYAML(f)); }
	Yaml(const char *key, bool b) : indent_level(0)             { CatRaw(key, AsYAML(b)); }
	Yaml(const char *key, Date d) : indent_level(0)             { CatRaw(key, AsYAML(d)); }
	Yaml(const char *key, Time t) : indent_level(0)             { CatRaw(key, AsYAML(t)); }
	Yaml(const char *key, const String& s) : indent_level(0)    { CatRaw(key, AsYAML(s)); }
	Yaml(const char *key, const WString& s) : indent_level(0)   { CatRaw(key, AsYAML(s)); }
	Yaml(const char *key, const char *s) : indent_level(0)      { CatRaw(key, AsYAML(s)); }
	Yaml(const char *key, const Yaml& object) : indent_level(0) { CatRaw(key, ~object); }
	Yaml(const char *key, const YamlArray& array) : indent_level(0) { operator()(key, array); }
};

class YamlArray {
	String text;
	int    indent_level;

	void   Indent();

public:
	YamlArray& CatRaw(const String& val);

	String ToString() const                                     { return text; }
	String operator~() const                                    { return ToString(); }
	operator String() const                                     { return ToString(); }

	operator bool() const                                       { return text.GetCount(); }

	YamlArray& operator<<(const Value& value)                   { return CatRaw(AsYAML(value)); }
	YamlArray& operator<<(int i)                                { return CatRaw(AsYAML(i)); }
	YamlArray& operator<<(double n)                             { return CatRaw(AsYAML(n)); }
	YamlArray& operator<<(float f)                              { return CatRaw(AsYAML(f)); }
	YamlArray& operator<<(bool b)                               { return CatRaw(AsYAML(b)); }
	YamlArray& operator<<(Date d)                               { return CatRaw(AsYAML(d)); }
	YamlArray& operator<<(Time t)                               { return CatRaw(AsYAML(t)); }
	YamlArray& operator<<(const String& s)                      { return CatRaw(AsYAML(s)); }
	YamlArray& operator<<(const WString& s)                     { return CatRaw(AsYAML(s)); }
	YamlArray& operator<<(const char *s)                        { return CatRaw(AsYAML(s)); }
	YamlArray& operator<<(const Yaml& object)                   { return CatRaw(~object); }
	YamlArray& operator<<(const YamlArray& array)               { return CatRaw(~array); }

	YamlArray() : indent_level(0) {}
};

inline Yaml& Yaml::operator()(const char *key, const YamlArray& array)
{
	return CatRaw(key, array);
}

class YamlIO {
	const Value   *src;
	One<ValueMap>  map;
	Value          tgt;

public:
	bool IsLoading() const                       { return src; }
	bool IsStoring() const                       { return !src; }

	const Value& Get() const                     { ASSERT(IsLoading()); return *src; }
	void         Set(const Value& v)             { ASSERT(IsStoring() && !map); tgt = v; }

	Value        Get(const char *key)            { ASSERT(IsLoading()); return (*src)[key]; }
	void         Set(const char *key, const Value& v);

	void         Put(Value& v)                   { ASSERT(IsStoring()); if(map) v = *map; else v = tgt; }
	Value        GetResult() const               { ASSERT(IsStoring()); return map ? Value(*map) : tgt; }

	template <class T>
	YamlIO& operator()(const char *key, T& value);

	template <class T>
	YamlIO& operator()(const char *key, T& value, const T& defvalue);

	template <class T>
	YamlIO& List(const char *key, const char *, T& var) { return operator()(key, var); }

	template <class T, class X>
	YamlIO& Var(const char *key, T& value, X item_yamlize);

	template <class T, class X>
	YamlIO& Array(const char *key, T& value, X item_yamlize, const char * = NULL, void* arg = NULL);

	YamlIO(const Value& src) : src(&src)         {}
	YamlIO()                                     { src = NULL; }
};

struct YamlizeError : Exc {
	YamlizeError(const String& s) : Exc(s) {}
};

template <class T>
void Yamlize(YamlIO& io, T& var)
{
	// Default implementation converts via Value since YAML and JSON represent the same data
	// Types can override by providing their own Yamlize method
	if(io.IsLoading()) {
		const Value& v = io.Get();
		if(!v.IsVoid()) {
			JsonIO jio(v);
			var.Jsonize(jio);
		}
	}
	else {
		JsonIO jio;
		var.Jsonize(jio);
		io.Set(jio.GetResult());
	}
}

template <class T>
YamlIO& YamlIO::operator()(const char *key, T& value)
{
	if(IsLoading()) {
		const Value& v = (*src)[key];
		if(!v.IsVoid()) {
			YamlIO yio(v);
			Yamlize(yio, value);
		}
	}
	else {
		ASSERT(tgt.IsVoid());
		if(!map)
			map.Create();
		YamlIO yio;
		Yamlize(yio, value);
		if(yio.map)
			map->Add(key, *yio.map);
		else
			map->Add(key, yio.tgt);
	}
	return *this;
}

template <class T, class X>
YamlIO& YamlIO::Var(const char *key, T& value, X yamlize)
{
	if(IsLoading()) {
		const Value& v = (*src)[key];
		if(!v.IsVoid()) {
			YamlIO yio(v);
			yamlize(yio, value);
		}
	}
	else {
		ASSERT(tgt.IsVoid());
		if(!map)
			map.Create();
		YamlIO yio;
		yamlize(yio, value);
		if(yio.map)
			map->Add(key, *yio.map);
		else
			map->Add(key, yio.tgt);
	}
	return *this;
}


template <class T, class X>
void YamlizeArray(YamlIO& io, T& array, X item_yamlize, void* arg=0)
{
	if(io.IsLoading()) {
		const Value& va = io.Get();
		array.SetCount(va.GetCount());
		for(int i = 0; i < va.GetCount(); i++) {
			YamlIO yio(va[i]);
			item_yamlize(yio, array[i]);
		}
	}
	else {
		Vector<Value> va;
		va.SetCount(array.GetCount());
		for(int i = 0; i < array.GetCount(); i++) {
			YamlIO yio;
			item_yamlize(yio, array[i]);
			yio.Put(va[i]);
		}
		io.Set(ValueArray(pick(va)));
	}
}

template <class T, class X> YamlIO& YamlIO::Array(const char *key, T& value, X item_yamlize, const char *, void* arg)
{
	if(IsLoading()) {
		const Value& v = (*src)[key];
		if(!v.IsVoid()) {
			YamlIO yio(v);
			YamlizeArray(yio, value, item_yamlize, arg);
		}
	}
	else {
		ASSERT(tgt.IsVoid());
		if(!map)
			map.Create();
		YamlIO yio;
		YamlizeArray(yio, value, item_yamlize, arg);
		if(yio.map)
			map->Add(key, *yio.map);
		else
			map->Add(key, yio.tgt);
	}
	return *this;
}

template <class T>
YamlIO& YamlIO::operator()(const char *key, T& value, const T& defvalue)
{
	if(IsLoading()) {
		const Value& v = (*src)[key];
		if(v.IsVoid())
			value = defvalue;
		else {
			YamlIO yio(v);
			Yamlize(yio, value);
		}
	}
	else {
		ASSERT(tgt.IsVoid());
		if(!map)
			map.Create();
		YamlIO yio;
		Yamlize(yio, value);
		if(yio.map)
			map->Add(key, *yio.map);
		else
			map->Add(key, yio.tgt);
	}
	return *this;
}

template <class T>
Value StoreAsYamlValue(const T& var)
{
	YamlIO io;
	Yamlize(io, const_cast<T&>(var));
	return io.GetResult();
}

template <class T>
void LoadFromYamlValue(T& var, const Value& x)
{
	YamlIO yio(x);
	Yamlize(yio, var);
}

template <class T>
String StoreAsYaml(const T& var, bool pretty = false)
{
	return AsYAML(StoreAsYamlValue(var), pretty);
}

template <class T>
bool LoadFromYaml(T& var, const char *yaml)
{
	try {
		Value yv = ParseYAML(yaml);
		if(yv.IsError())
			return false;
		LoadFromYamlValue(var, yv);
	}
	catch(ValueTypeError) {
		return false;
	}
	catch(YamlizeError) {
		return false;
	}
	return true;
}

String sYamlFile(const char *file);

template <class T>
bool StoreAsYamlFile(const T& var, const char *file = NULL, bool pretty = false)
{
	return SaveFile(sYamlFile(file), StoreAsYaml(var, pretty));
}

template <class T>
bool LoadFromYamlFile(T& var, const char *file = NULL)
{
	return LoadFromYaml(var, LoadFile(sYamlFile(file)));
}

template<> void Yamlize(YamlIO& io, int& var);
template<> void Yamlize(YamlIO& io, uint32& var);
template<> void Yamlize(YamlIO& io, byte& var);
template<> void Yamlize(YamlIO& io, int16& var);
template<> void Yamlize(YamlIO& io, int64& var);
template<> void Yamlize(YamlIO& io, double& var);
template<> void Yamlize(YamlIO& io, float& var);
template<> void Yamlize(YamlIO& io, bool& var);
template<> void Yamlize(YamlIO& io, String& var);
template<> void Yamlize(YamlIO& io, WString& var);
template<> void Yamlize(YamlIO& io, Date& var);
template<> void Yamlize(YamlIO& io, Time& var);
template<> void Yamlize(YamlIO& io, hash_t& var);

template <class T>
void YamlizeArray(YamlIO& io, T& array)
{
	YamlizeArray(io, array, [](YamlIO& io, ValueTypeOf<T>& item) { Yamlize(io, item); });
}

template <class T, class K, class V>
void YamlizeMap(YamlIO& io, T& map, const char *keyid, const char *valueid)
{
	if(io.IsLoading()) {
		map.Clear();
		const Value& va = io.Get();
		map.Reserve(va.GetCount());
		for(int i = 0; i < va.GetCount(); i++) {
			K key;
			V value;
			LoadFromYamlValue(key, va[i][keyid]);
			LoadFromYamlValue(value, va[i][valueid]);
			map.Add(key, pick(value));
		}
	}
	else {
		Vector<Value> va;
		va.SetCount(map.GetCount());
		for(int i = 0; i < map.GetCount(); i++)
			if(!map.IsUnlinked(i)) {
				ValueMap item;
				item.Add(keyid, StoreAsYamlValue(map.GetKey(i)));
				item.Add(valueid, StoreAsYamlValue(map[i]));
				va[i] = item;
			}
		io.Set(ValueArray(pick(va)));
	}
}

template <class T, class K, class V>
void YamlizeSortedMap(YamlIO& io, T& map, const char *keyid, const char *valueid)
{
	if(io.IsLoading()) {
		map.Clear();
		const Value& va = io.Get();
		for(int i = 0; i < va.GetCount(); i++) {
			K key;
			V value;
			LoadFromYamlValue(key, va[i][keyid]);
			LoadFromYamlValue(value, va[i][valueid]);
			map.Add(key, pick(value));
		}
	}
	else {
		Vector<Value> va;
		va.SetCount(map.GetCount());
		for(int i = 0; i < map.GetCount(); i++) {
			ValueMap item;
			item.Add(keyid, StoreAsYamlValue(map.GetKey(i)));
			item.Add(valueid, StoreAsYamlValue(map[i]));
			va[i] = item;
		}
		io.Set(ValueArray(pick(va)));
	}
}

template <class T, class K, class V>
void YamlizeStringMap(YamlIO& io, T& map)
{
	if(io.IsLoading()) {
		map.Clear();
		const ValueMap& va = io.Get();
		map.Reserve(va.GetCount());
		for(int i = 0; i < va.GetCount(); i++) {
			V value;
			String key = va.GetKey(i);
			LoadFromYamlValue(key, va.GetKey(i));
			LoadFromYamlValue(value, va.GetValue(i));
			map.Add(key, pick(value));
		}
	}
	else {
		Index<Value>  index;
		Vector<Value> values;
		index.Reserve(map.GetCount());
		values.Reserve(map.GetCount());
		for (int i=0; i<map.GetCount(); ++i)
		{
			index.Add(StoreAsYamlValue(map.GetKey(i)));
			values.Add(StoreAsYamlValue(map[i]));
		}
		ValueMap vm(pick(index), pick(values));
		io.Set(vm);
	}
}

template <class K, class V>
void StringMap(YamlIO& io, VectorMap<K, V>& map)
{
	YamlizeStringMap<VectorMap<K, V>, K, V>(io, map);
}

template <class K, class V>
void StringMap(YamlIO& io, ArrayMap<K, V>& map)
{
	YamlizeStringMap<ArrayMap<K, V>, K, V>(io, map);
}

template <class T, class V>
void YamlizeIndex(YamlIO& io, T& index)
{
	if(io.IsLoading()) {
		const Value& va = io.Get();
		index.Reserve(va.GetCount());
		for(int i = 0; i < va.GetCount(); i++) {
			V v;
			LoadFromYamlValue(v, va[i]);
			index.Add(pick(v));
		}
	}
	else {
		Vector<Value> va;
		for(int i = 0; i < index.GetCount(); i++)
			if(!index.IsUnlinked(i))
				va.Add(StoreAsYamlValue(index[i]));
		io.Set(ValueArray(pick(va)));
	}
}

template <class T>
void YamlizeBySerialize(YamlIO& yio, T& x)
{
	String h;
	if(yio.IsStoring())
	   h = HexString(StoreAsString(x));
	yio("data", h);
	if(yio.IsLoading())
		try {
			LoadFromString(x, ScanHexString(h));
		}
		catch(LoadingError) {
			throw YamlizeError("yamlize by serialize error");
		}
}
