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


END_UPP_NAMESPACE

#endif
