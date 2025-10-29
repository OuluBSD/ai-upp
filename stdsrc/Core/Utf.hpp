#pragma once
#ifndef _Core_Utf_hpp_
#define _Core_Utf_hpp_

#include <string>
#include <vector>
#include "Core.h"

NAMESPACE_UPP

// UTF-8 utility functions
std::string ToUtf8(int codepoint);
std::vector<int> FromUtf8(const std::string& utf8);
std::string ToUtf8(const std::vector<int>& codepoints);
int GetUtf8Length(const std::string& utf8);
bool IsValidUtf8(const std::string& utf8);

// Helper functions for UTF-8 character processing
int Utf8Length(const char* utf8, int max_bytes);
int Utf8Length(const std::string& utf8);
std::string Utf8Substr(const std::string& utf8, int start, int length);
int Utf8Next(const char* utf8, int* byte_pos, int max_bytes);

// Character type detection
bool IsAlpha(int codepoint);
bool IsDigit(int codepoint);
bool IsAlNum(int codepoint);
bool IsSpace(int codepoint);

END_UPP_NAMESPACE

#endif