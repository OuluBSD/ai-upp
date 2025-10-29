#pragma once
#ifndef _Core_St_h_
#define _Core_St_h_

#include <string>
#include <vector>
#include "Core.h"

NAMESPACE_UPP

using String = std::string;
using StringVector = std::vector<String>;

// String utility functions
StringVector Split(const String& text, char delimiter);
StringVector Split(const String& text, const String& delimiter);
String Join(const StringVector& parts, const String& delimiter);
String Trim(const String& text);
String ToLower(const String& text);
String ToUpper(const String& text);
bool StartsWith(const String& text, const String& prefix);
bool EndsWith(const String& text, const String& suffix);
String Replace(const String& text, const String& from, const String& to);

// String conversion functions
int ToInt(const String& text, int default_value = 0);
double ToDouble(const String& text, double default_value = 0.0);
String ToString(int value);
String ToString(double value);

END_UPP_NAMESPACE

#endif