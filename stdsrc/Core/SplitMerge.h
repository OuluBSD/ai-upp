#pragma once
#ifndef _Core_SplitMerge_h_
#define _Core_SplitMerge_h_

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#include <cctype>
#include "Core.h"
#include "String.h"

Vector<String> Split(int maxcount, const char *s, const char * (*text_filter)(const char *), bool ignoreempty = true, bool ignoreescaped = false);
Vector<String> Split(int maxcount, const char *s, int (*filter)(int), bool ignoreempty = true, bool ignoreescaped = false);
Vector<String> Split(int maxcount, const char *s, int chr, bool ignoreempty = true, bool ignoreescaped = false);
Vector<String> Split(int maxcount, const char *s, const char *text, bool ignoreempty = true, bool ignoreescaped = false);
Vector<String> Split(const char *s, const char * (*text_filter)(const char *), bool ignoreempty = true, bool ignoreescaped = false);
Vector<String> Split(const char *s, int (*filter)(int), bool ignoreempty = true, bool ignoreescaped = false);
Vector<String> Split(const char *s, int chr, bool ignoreempty = true, bool ignoreescaped = false);
Vector<String> Split(const char *s, const char *text, bool ignoreempty = true, bool ignoreescaped = false);

Vector<WString> Split(int maxcount, const wchar *s, const wchar * (*text_filter)(const wchar *), bool ignoreempty = true, bool ignoreescaped = false);
Vector<WString> Split(int maxcount, const wchar *s, int (*filter)(int), bool ignoreempty = true, bool ignoreescaped = false);
Vector<WString> Split(int maxcount, const wchar *s, int chr, bool ignoreempty = true, bool ignoreescaped = false);
Vector<WString> Split(int maxcount, const wchar *s, const wchar *text, bool ignoreempty = true, bool ignoreescaped = false);
Vector<WString> Split(const wchar *s, const wchar * (*text_filter)(const wchar *), bool ignoreempty = true, bool ignoreescaped = false);
Vector<WString> Split(const wchar *s, int (*filter)(int), bool ignoreempty = true, bool ignoreescaped = false);
Vector<WString> Split(const wchar *s, int chr, bool ignoreempty = true, bool ignoreescaped = false);
Vector<WString> Split(const wchar *s, const wchar *text, bool ignoreempty = true, bool ignoreescaped = false);

String  Join(const Vector<String>& im, const String& delim, bool ignoreempty = false);
WString Join(const Vector<WString>& im, const WString& delim, bool ignoreempty = false);

template <typename... Args>
bool SplitTo(const char *s, int delim, bool ignoreempty, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim, ignoreempty), args...);
}

template <typename... Args>
bool SplitTo(const char *s, int delim, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim), args...);
}

template <typename... Args>
bool SplitTo(const char *s, int (*delim)(int), bool ignoreempty, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim, ignoreempty), args...);
}

template <typename... Args>
bool SplitTo(const char *s, int (*delim)(int), Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim), args...);
}

template <typename... Args>
bool SplitTo(const char *s, const char *delim, bool ignoreempty, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim, ignoreempty), args...);
}

template <typename... Args>
bool SplitTo(const char *s, const char *delim, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim), args...);
}

template <typename... Args>
void MergeWith(String& dest, const char *delim, const Args& ...args)
{
	foreach_arg([&](const String& arg) {
		if(arg.GetCount()) {
			if(dest.GetCount())
				dest << delim;
			dest << arg;
		}
	}, args...);
}

template <typename... Args>
String Merge(const char *delim, const Args& ...args)
{
	String r;
	MergeWith(r, delim, args...);
	return r;
}

template <typename... Args>
bool SplitTo(const wchar *s, int delim, bool ignoreempty, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim, ignoreempty), args...);
}

template <typename... Args>
bool SplitTo(const wchar *s, int delim, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim), args...);
}

template <typename... Args>
bool SplitTo(const wchar *s, int (*delim)(int), bool ignoreempty, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim, ignoreempty), args...);
}

template <typename... Args>
bool SplitTo(const wchar *s, int (*delim)(int), Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim), args...);
}

template <typename... Args>
bool SplitTo(const wchar *s, const wchar *delim, bool ignoreempty, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim, ignoreempty), args...);
}

template <typename... Args>
bool SplitTo(const wchar *s, const wchar *delim, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim), args...);
}

template <typename... Args>
void MergeWith(WString& dest, const wchar *delim, const Args& ...args)
{
	foreach_arg([&](const WString& arg) {
		if(arg.GetCount()) {
			if(dest.GetCount())
				dest << delim;
			dest << arg;
		}
	}, args...);
}

template <typename... Args>
WString Merge(const wchar *delim, const Args& ...args)
{
	WString r;
	MergeWith(r, delim, args...);
	return r;
}

// Implementation of the Split functions
inline Vector<String> Split(const char *s, int chr, bool ignoreempty = true, bool ignoreescaped = false) {
	Vector<String> result;
	if (!s) return result;
	
	String current;
	const char *start = s;
	while (*s) {
		if (ignoreescaped && *s == '\\' && *(s+1)) {
			current.Cat(*s);
			s++;
			current.Cat(*s);
		} else if (*s == chr) {
			if (!ignoreempty || current.GetCount() > 0) {
				result.Add(current);
			}
			current.Clear();
		} else {
			current.Cat(*s);
		}
		s++;
	}
	
	if (!ignoreempty || current.GetCount() > 0) {
		result.Add(current);
	}
	
	return result;
}

inline Vector<String> Split(int maxcount, const char *s, int chr, bool ignoreempty = true, bool ignoreescaped = false) {
	Vector<String> result;
	if (!s) return result;
	
	String current;
	const char *start = s;
	while (*s && (maxcount < 0 || result.GetCount() < maxcount - 1)) {
		if (ignoreescaped && *s == '\\' && *(s+1)) {
			current.Cat(*s);
			s++;
			current.Cat(*s);
		} else if (*s == chr) {
			if (!ignoreempty || current.GetCount() > 0) {
				result.Add(current);
			}
			current.Clear();
		} else {
			current.Cat(*s);
		}
		s++;
	}
	
	// Add the remainder if we reached maxcount
	if (*s) {
		current.Cat(s);
	}
	
	if (!ignoreempty || current.GetCount() > 0) {
		result.Add(current);
	}
	
	return result;
}

inline Vector<String> Split(const char *s, const char *delim, bool ignoreempty = true, bool ignoreescaped = false) {
	Vector<String> result;
	if (!s || !delim) return result;
	
	String current;
	int delim_len = strlen(delim);
	
	while (*s) {
		bool found = true;
		for (int i = 0; i < delim_len; i++) {
			if (s[i] != delim[i]) {
				found = false;
				break;
			}
		}
		
		if (found) {
			if (!ignoreempty || current.GetCount() > 0) {
				result.Add(current);
			}
			current.Clear();
			s += delim_len;
		} else {
			current.Cat(*s);
			s++;
		}
	}
	
	if (!ignoreempty || current.GetCount() > 0) {
		result.Add(current);
	}
	
	return result;
}

inline String Join(const Vector<String>& im, const String& delim, bool ignoreempty = false) {
	String result;
	for (int i = 0; i < im.GetCount(); i++) {
		if (ignoreempty && im[i].IsEmpty()) {
			continue;
		}
		if (result.GetCount() > 0) {
			result << delim;
		}
		result << im[i];
	}
	return result;
}

// Additional Split implementations for other parameter types
inline Vector<String> Split(const char *s, int (*filter)(int), bool ignoreempty = true, bool ignoreescaped = false) {
	Vector<String> result;
	if (!s) return result;
	
	String current;
	while (*s) {
		if (filter(*s)) {
			if (!ignoreempty || current.GetCount() > 0) {
				result.Add(current);
			}
			current.Clear();
		} else {
			current.Cat(*s);
		}
		s++;
	}
	
	if (!ignoreempty || current.GetCount() > 0) {
		result.Add(current);
	}
	
	return result;
}

// WString implementations
inline Vector<WString> Split(const wchar *s, int chr, bool ignoreempty = true, bool ignoreescaped = false) {
	Vector<WString> result;
	if (!s) return result;
	
	WString current;
	while (*s) {
		if (*s == chr) {
			if (!ignoreempty || current.GetCount() > 0) {
				result.Add(current);
			}
			current.Clear();
		} else {
			current.Cat(*s);
		}
		s++;
	}
	
	if (!ignoreempty || current.GetCount() > 0) {
		result.Add(current);
	}
	
	return result;
}

inline WString Join(const Vector<WString>& im, const WString& delim, bool ignoreempty = false) {
	WString result;
	for (int i = 0; i < im.GetCount(); i++) {
		if (ignoreempty && im[i].IsEmpty()) {
			continue;
		}
		if (result.GetCount() > 0) {
			result << delim;
		}
		result << im[i];
	}
	return result;
}

#endif