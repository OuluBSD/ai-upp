#include "Core.h"

namespace Upp {

template <class S, class Char, class F>
Vector<S> SplitGeneric(int maxcount, const F& delim, const Char *s, bool ignoreempty = true, bool ignoreescaped = false)
{
	Vector<S> r;
	r.Reserve(min(maxcount, 8));
	const Char *t = s;
	while(*t && r.GetCount() < maxcount) {
		const Char *q = delim(t);
		if(q) {
			if (ignoreescaped && t > s && *(t-1) == '\\')
				t++;
			else {
				if (!ignoreempty || t > s)
					r.Add().Set(s, int(t - s)); // This is faster than r.Add(String(s, t))...
				t = s = q;
			}
		}
		else
			t++;
	}
	if((!ignoreempty || t > s) && r.GetCount() < maxcount)
		r.Add().Set(s, int(t - s));
	return r;
}

Vector<String> Split(int maxcount, const char *s, const char * (*text_filter)(const char *), bool ignoreempty, bool ignoreescaped)
{
	return SplitGeneric<String>(maxcount, text_filter, s, ignoreempty, ignoreescaped);
}

struct SplitDelimFilter__ {
	int (*filter)(int);
	const char *operator()(const char *s) const { return (*filter)((byte)*s) ? s + 1 : NULL; }
};

Vector<String> Split(int maxcount, const char *s, int (*filter)(int), bool ignoreempty, bool ignoreescaped)
{
	SplitDelimFilter__ delim;
	delim.filter = filter;
	return SplitGeneric<String>(maxcount, delim, s, ignoreempty, ignoreescaped);
}

struct SplitDelimChar__ {
	int chr;
	const char *operator()(const char *s) const { return *s == chr ? s + 1 : NULL; }
};

Vector<String> Split(int maxcount, const char *s, int chr, bool ignoreempty, bool ignoreescaped)
{
	SplitDelimChar__ delim;
	delim.chr = chr;
	return SplitGeneric<String>(maxcount, delim, s, ignoreempty, ignoreescaped);
}

struct SplitDelimText__ {
	const char *ds;
	int l;
	const char *operator()(const char *s) const { return strncmp(s, ds, l) == 0 ? s + l : NULL; }
};

Vector<String> Split(int maxcount, const char *s, const char *text, bool ignoreempty, bool ignoreescaped)
{
	SplitDelimText__ delim;
	delim.ds = text;
	delim.l = (int)strlen(text);
	return delim.l ? SplitGeneric<String>(maxcount, delim, s, ignoreempty, ignoreescaped) : Vector<String>();
}

Vector<String> Split(const char *s, const char * (*text_filter)(const char *), bool ignoreempty, bool ignoreescaped)
{
	return Split(INT_MAX, s, text_filter, ignoreempty, ignoreescaped);
}

Vector<String> Split(const char *s, int (*filter)(int), bool ignoreempty, bool ignoreescaped)
{
	return Split(INT_MAX, s, filter, ignoreempty, ignoreescaped);
}

Vector<String> Split(const char *s, int chr, bool ignoreempty, bool ignoreescaped)
{
	return Split(INT_MAX, s, chr, ignoreempty, ignoreescaped);
}

Vector<String> Split(const char *s, const char *text, bool ignoreempty, bool ignoreescaped)
{
	return Split(INT_MAX, s, text, ignoreempty, ignoreescaped);
}

Vector<WString> Split(int maxcount, const wchar *s, const wchar * (*text_filter)(const wchar *), bool ignoreempty, bool ignoreescaped)
{
	return SplitGeneric<WString>(maxcount, text_filter, s, ignoreempty, ignoreescaped);
}

struct SplitDelimWFilter__ {
	int (*filter)(int);
	const wchar *operator()(const wchar *s) const { return (*filter)((byte)*s) ? s + 1 : NULL; }
};

Vector<WString> Split(int maxcount, const wchar *s, int (*filter)(int), bool ignoreempty, bool ignoreescaped)
{
	SplitDelimWFilter__ delim;
	delim.filter = filter;
	return SplitGeneric<WString>(maxcount, delim, s, ignoreempty, ignoreescaped);
}

struct SplitDelimWChar__ {
	int chr;
	const wchar *operator()(const wchar *s) const { return *s == chr ? s + 1 : NULL; }
};

Vector<WString> Split(int maxcount, const wchar *s, int chr, bool ignoreempty, bool ignoreescaped)
{
	SplitDelimWChar__ delim;
	delim.chr = chr;
	return SplitGeneric<WString>(maxcount, delim, s, ignoreempty, ignoreescaped);
}

int w_strncmp(const wchar *s, const wchar *t, int n)
{
	while(*s && *t && n > 0) {
		int q = (int)(uint16)*s - (int)(uint16)*t;
		if(q)
			return q;
		s++;
		t++;
		n--;
	}
	return 0;
}

struct SplitDelimWText {
	const wchar *ds;
	int l;
	const wchar *operator()(const wchar *s) const { return w_strncmp(s, ds, l) == 0 ? s + l : NULL; }
};

Vector<WString> Split(int maxcount, const wchar *s, const wchar *text, bool ignoreempty, bool ignoreescaped)
{
	SplitDelimWText delim;
	delim.ds = text;
	delim.l = strlen__(text);
	return delim.l ? SplitGeneric<WString>(maxcount, delim, s, ignoreempty, ignoreescaped) : Vector<WString>();
}

Vector<WString> Split(const wchar *s, int (*filter)(int), bool ignoreempty, bool ignoreescaped)
{
	return Split(INT_MAX, s, filter, ignoreempty, ignoreescaped);
}

Vector<WString> Split(const wchar *s, int chr, bool ignoreempty, bool ignoreescaped)
{
	return Split(INT_MAX, s, chr, ignoreempty, ignoreescaped);
}

Vector<WString> Split(const wchar *s, const wchar *text, bool ignoreempty, bool ignoreescaped)
{
	return Split(INT_MAX, s, text, ignoreempty, ignoreescaped);
}

Vector<WString> Split(const wchar *s, const wchar * (*text_filter)(const wchar *), bool ignoreempty, bool ignoreescaped)
{
	return SplitGeneric<WString>(INT_MAX, text_filter, s, ignoreempty, ignoreescaped);
}

template <class B, class T>
T Join_(const Vector<T>& im, const T& delim, bool ignoreempty) {
	B r;
	bool next = false;
	for(int i = 0; i < im.GetCount(); i++)
		if(!ignoreempty || im[i].GetCount()) {
			if(next)
				r.Cat(delim);
			r.Cat(im[i]);
			next = true;
		}
	return T(r);
}

String Join(const Vector<String>& im, const String& delim, bool ignoreempty) {
	return Join_<StringBuffer, String>(im, delim, ignoreempty);
}

WString Join(const Vector<WString>& im, const WString& delim, bool ignoreempty) {
	return Join_<WStringBuffer, WString>(im, delim, ignoreempty);
}

}
