#ifndef _AI_TextCore_Fn_h_
#define _AI_TextCore_Fn_h_

NAMESPACE_UPP

struct CallbackInhibitor {
	Event<> cb;
	Event<>& ref;
	
	CallbackInhibitor(Event<>& other) : cb(other), ref(other) {other.Clear();}
	~CallbackInhibitor() {ref = cb;}
};

template <class T, class PTR>
int VectorFindPtr(PTR* p, T& arr) {
	int i = 0;
	for (auto& r : arr) {
		if (&r == p)
			return i;
		i++;
	}
	return -1;
}

String KeyToName(String s);
const Color& GetGroupColor(int i);
int GetColorGroupCount();
const Color& GetGroupColor(int i);
String GetColorString(int i);
int GetColorGroup(const Color& clr);
void RgbHsv(float r, float g, float b, float& h, float& s, float& v);
int GetColorDistance(const Color& a, const Color& b);

struct ColorDistanceSorter {
	Color cmp;
	bool operator()(const Color& a, const Color& b) const {
		int dist0 = GetColorDistance(a, cmp);
		int dist1 = GetColorDistance(b, cmp);
		return dist0 < dist1;
	}
};

struct TextColorDistanceSorter {
	Color cmp;
	Vector<String>* str = 0;
	Vector<Color>* clr = 0;
	bool operator()(const int& ai, const int& bi) const;
	void Sort();
};


String Capitalize(String s);

extern const char* AttrKeys[ATTR_COUNT+1][4];

int FindAttrGroup(const char* group);
int FindAttrGroupByValue(const char* value);
bool FindAttrValue(int group_i, const char* value);

template <class T>
int VectorFindAdd(Vector<T>& v, const T& o0) {
	int i = 0;
	for (const T& o1 : v) {
		if (o1 == o0)
			return i;
		i++;
	}
	i = v.GetCount();
	v.Add(o0);
	return i;
}

template <class T>
int VectorFind(Vector<T>& v, const T& o0) {
	int i = 0;
	for (const T& o1 : v) {
		if (o1 == o0)
			return i;
		i++;
	}
	return -1;
}

template <class T>
void VectorRemoveKey(Vector<T>& v, const T& o0) {
	for(int i = 0; i < v.GetCount(); i++) {
		const T& o1 = v[i];
		if (o1 == o0) {
			v.Remove(i--);
		}
	}
}

template <class T, class K>
typename T::ValueType& MapGetAdd(T& map, const K& k, int& pos) {
	pos = map.Find(k);
	if (pos >= 0) return map[pos];
	pos = map.GetCount();
	return map.Add(k);
}

template <class T, class K, class V>
V& MapGetAdd(T& map, const K& k, const V& v, int& pos) {
	pos = map.Find(k);
	if (pos >= 0) return map[pos];
	pos = map.GetCount();
	return map.Add(k);
}

template <class T>
int FixedIndexFindAdd(T* values, int max_value_count, int& value_count, const T& new_value) {
	ASSERT(value_count >= 0 && value_count <= max_value_count);
	T* it = values;
	T* end = it + value_count;
	int i = 0;
	while (it != end) {
		if (*it == new_value)
			return i;
		it++;
		i++;
	}
	ASSERT(value_count < max_value_count);
	*it = new_value;
	value_count++;
	return i;
}

END_UPP_NAMESPACE

#endif
