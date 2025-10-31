#pragma once
#ifndef _Core_CharFilter_h_
#define _Core_CharFilter_h_

#include <string>
#include <cctype>
#include "Core.h"

typedef int (*CharFilter)(int);

inline
int CharFilterAscii(int c)
{
	return c >= 32 && c < 256 ? c : 0;
}

inline
int CharFilterAscii128(int c)
{
	return c >= 32 && c < 128 ? c : 0;
}

inline
int CharFilterUnicode(int c)
{
	return c >= 32 && c < 0x10FFFF ? c : 0;
}

inline
int CharFilterDigit(int c)
{
	return IsDigit(c) ? c : 0;
}

inline
int CharFilterInt(int c)
{
	if(c == '+' || c == '-') return c;
	return CharFilterDigit(c);
}

inline
int CharFilterDouble(int c)
{
	if(c == ',' || c == '.') return '.';
	if(c == 'e' || c == 'E') return 'E';
	return CharFilterInt(c);
}

inline
int CharFilterWhitespace(int c)
{
	return IsSpace(c) ? c : 0;
}

inline
int CharFilterNotWhitespace(int c)
{
	return IsSpace(c) ? 0 : c;
}

inline
int CharFilterAlpha(int c)
{
	return IsAlpha(c) ? c : 0;
}

inline
int CharFilterToUpper(int c)
{
	return ToUpper(c);
}

inline
int CharFilterToLower(int c)
{
	return ToLower(c);
}

inline
int CharFilterToUpperAscii(int c)
{
	return ToUpperAscii(c);
}

inline
int CharFilterAlphaToUpper(int c)
{
	return IsAlpha(c) ? IsUpper(c) ? c : ToUpper(c) : 0;
}

inline
int CharFilterAlphaToLower(int c)
{
	return IsAlpha(c) ? IsLower(c) ? c : ToLower(c) : 0;
}

inline
int CharFilterDefaultToUpperAscii(int c)
{
	return ToUpper(ToAscii(c, CHARSET_DEFAULT));
}

inline
int CharFilterCrLf(int c)
{
	return c == '\r' || c == '\n' ? c : 0;
}

inline
int CharFilterNoCrLf(int c)
{
	return c != '\r' && c != '\n' ? c : 0;
}

inline
String Filter(const char *s, int (*filter)(int))
{
	String result;
	while(*s) {
		int c = (*filter)((byte)*s++);
		if(c) result.Cat(c);
	}
	return result;
}

inline
String FilterWhile(const char *s, int (*filter)(int))
{
	String result;
	while(*s) {
		int c = (*filter)((byte)*s++);
		if(!c) break;
		result.Cat(c);
	}
	return result;
}

inline
WString Filter(const wchar *s, int (*filter)(int))
{
	WString result;
	while(*s) {
		int c = (*filter)(*s++);
		if(c) result.Cat(c);
	}
	return result;
}

inline
WString FilterWhile(const wchar *s, int (*filter)(int))
{
	WString result;
	while(*s) {
		int c = (*filter)(*s++);
		if(!c) break;
		result.Cat(c);
	}
	return result;
}

// Additional utility functions
inline
String Filter(const std::string& s, int (*filter)(int))
{
	return Filter(s.c_str(), filter);
}

inline
String FilterWhile(const std::string& s, int (*filter)(int))
{
	return FilterWhile(s.c_str(), filter);
}

inline
WString Filter(const std::wstring& s, int (*filter)(int))
{
	return Filter(s.c_str(), filter);
}

inline
WString FilterWhile(const std::wstring& s, int (*filter)(int))
{
	return FilterWhile(s.c_str(), filter);
}

// Common character filters
inline int CharFilterNone(int c) { return c; }
inline int CharFilterAlphaNumeric(int c) { return IsAlNum(c) ? c : 0; }
inline int CharFilterHexDigit(int c) { return IsXDigit(c) ? c : 0; }
inline int CharFilterPunctuation(int c) { return IsPunct(c) ? c : 0; }
inline int CharFilterPrintable(int c) { return IsPrint(c) ? c : 0; }
inline int CharFilterGraphical(int c) { return IsGraph(c) ? c : 0; }
inline int CharFilterControl(int c) { return IsCntrl(c) ? c : 0; }

// Combined filters
inline int CharFilterAlphaNumericUpper(int c) {
	return IsAlpha(c) ? ToUpper(c) : IsDigit(c) ? c : 0;
}

inline int CharFilterAlphaNumericLower(int c) {
	return IsAlpha(c) ? ToLower(c) : IsDigit(c) ? c : 0;
}

// Filter with range
inline int CharFilterRange(int c, int min_char, int max_char) {
	return (c >= min_char && c <= max_char) ? c : 0;
}

// Filter with exclusion
inline int CharFilterExclude(int c, int exclude_char) {
	return (c != exclude_char) ? c : 0;
}

// Filter with multiple exclusions
inline int CharFilterExcludeMultiple(int c, const std::vector<int>& exclude_chars) {
	for (int ec : exclude_chars) {
		if (c == ec) return 0;
	}
	return c;
}

// Filter with inclusion
inline int CharFilterInclude(int c, const std::vector<int>& include_chars) {
	for (int ic : include_chars) {
		if (c == ic) return c;
	}
	return 0;
}

// Filter with custom predicate
template<typename Predicate>
inline int CharFilterPredicate(int c, Predicate pred) {
	return pred(c) ? c : 0;
}

// Inverse filter
inline int CharFilterInverse(int c, int (*filter)(int)) {
	return (*filter)(c) ? 0 : c;
}

// Streaming operators
template<typename Stream>
void operator%(Stream& s, CharFilter& filter) {
	// CharFilter is a function pointer, so we can't serialize it directly
	// This is just a placeholder to satisfy interface requirements
	s.LoadError();
}

// String conversion
inline std::string AsString(CharFilter filter) {
	return "CharFilter(function pointer)";
}

#endif