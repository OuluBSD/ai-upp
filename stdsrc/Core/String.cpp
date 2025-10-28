// STL-backed Core API implementation

#include "String.h"
#include "WString.h"
#include <cstring>

namespace Upp {

// U++-style helpers
void String::Clear() { 
    this->clear(); 
}
int String::GetLength() const { 
    return static_cast<int>(this->size()); 
}
bool String::IsEmpty() const { 
    return this->empty(); 
}
bool String::Is() const { 
    return !this->empty(); 
}

const char* String::Begin() const { 
    return this->data(); 
}
const char* String::End() const { 
    return this->data() + this->size(); 
}

int String::operator[](int i) const { 
    return static_cast<unsigned char>(Base::operator[](static_cast<size_t>(i))); 
}

// Append
void String::Cat(int c) { 
    this->push_back(static_cast<char>(c)); 
}
void String::Cat(const char* s, int len) { 
    if(len > 0) this->append(s, static_cast<size_t>(len)); 
}
void String::Cat(const char* s) { 
    if(s) this->append(s); 
}
void String::Cat(const String& s) { 
    this->append(s.data(), s.size()); 
}
void String::Cat(int c, int count) { 
    if(count > 0) this->append(static_cast<size_t>(count), static_cast<char>(c)); 
}

// Operator sugar
String& String::operator+=(char c) { 
    this->push_back(c); 
    return *this; 
}
String& String::operator+=(const char* s) { 
    if(s) this->append(s); 
    return *this; 
}
String& String::operator+=(const String& s) { 
    this->append(s); 
    return *this; 
}

// Insert
void String::Insert(int pos, int c) { 
    this->insert(this->begin() + static_cast<ptrdiff_t>(pos), static_cast<char>(c)); 
}
void String::Insert(int pos, const char* s, int count) { 
    this->insert(static_cast<size_t>(pos), s, static_cast<size_t>(count)); 
}
void String::Insert(int pos, const String& s) { 
    this->insert(static_cast<size_t>(pos), s); 
}

// Trim and reserve
void String::Trim(int pos) { 
    if(pos < 0) pos = 0; 
    if(static_cast<size_t>(pos) < this->size()) this->resize(static_cast<size_t>(pos)); 
}
void String::TrimLast(int count) { 
    if(count <= 0) return; 
    if(this->size() >= static_cast<size_t>(count)) this->resize(this->size() - static_cast<size_t>(count)); 
    else this->clear(); 
}
void String::Reserve(int r) { 
    this->reserve(static_cast<size_t>(r)); 
}
void String::Shrink() { 
    this->shrink_to_fit(); 
}

// Compare / equality
int String::Compare(const String& s) const { 
    return this->compare(s); 
}
int String::Compare(const char* s) const { 
    return this->compare(s ? s : ""); 
}
bool String::IsEqual(const String& s) const { 
    return *this == s; 
}
bool String::IsEqual(const char* s) const { 
    return this->compare(s ? s : "") == 0; 
}

// Slices
String String::Mid(int pos, int length) const { 
    return String(this->substr(static_cast<size_t>(pos), static_cast<size_t>(length))); 
}
String String::Mid(int pos) const { 
    return String(this->substr(static_cast<size_t>(pos))); 
}
String String::Right(int count) const { 
    return count <= 0 ? String() : String(this->substr(this->size() - static_cast<size_t>(count))); 
}
String String::Left(int count) const { 
    return count <= 0 ? String() : String(this->substr(0, static_cast<size_t>(count))); 
}

// Find helpers
int String::Find(int chr, int from) const {
    size_t p = this->find(static_cast<char>(chr), static_cast<size_t>(from));
    return p == Base::npos ? -1 : static_cast<int>(p);
}
int String::ReverseFind(int chr, int from) const {
    if(from < 0) return -1;
    if(from >= GetLength()) from = GetLength() - 1;
    size_t p = this->rfind(static_cast<char>(chr), static_cast<size_t>(from));
    return p == Base::npos ? -1 : static_cast<int>(p);
}
int String::ReverseFind(int chr) const {
    size_t p = this->rfind(static_cast<char>(chr));
    return p == Base::npos ? -1 : static_cast<int>(p);
}

int String::Find(const char* s, int from) const {
    size_t p = this->find(s ? s : "", static_cast<size_t>(from));
    return p == Base::npos ? -1 : static_cast<int>(p);
}
int String::Find(const String& s, int from) const {
    size_t p = this->find(s, static_cast<size_t>(from));
    return p == Base::npos ? -1 : static_cast<int>(p);
}
int String::FindAfter(const char* s, int from) const {
    if(!s) return -1;
    int n = static_cast<int>(std::char_traits<char>::length(s));
    int q = Find(s, from);
    return q < 0 ? -1 : q + n;
}
int String::FindAfter(const String& s, int from) const {
    int q = Find(s, from);
    return q < 0 ? q + static_cast<int>(s.size()) : -1;
}

bool String::StartsWith(const char* s, int len) const {
    if(!s) return false; if(len < 0) return false; if(static_cast<size_t>(len) > this->size()) return false;
    return this->compare(0, static_cast<size_t>(len), s, static_cast<size_t>(len)) == 0;
}
bool String::StartsWith(const char* s) const {
    if(!s) return false; size_t len = std::char_traits<char>::length(s);
    return StartsWith(s, static_cast<int>(len));
}
bool String::StartsWith(const String& s) const { 
    return StartsWith(s.data(), static_cast<int>(s.size())); 
}

bool String::EndsWith(const char* s, int len) const {
    if(!s) return false; if(len < 0) return false; if(static_cast<size_t>(len) > this->size()) return false;
    return this->compare(this->size() - static_cast<size_t>(len), static_cast<size_t>(len), s, static_cast<size_t>(len)) == 0;
}
bool String::EndsWith(const char* s) const {
    if(!s) return false; size_t len = std::char_traits<char>::length(s);
    return EndsWith(s, static_cast<int>(len));
}
bool String::EndsWith(const String& s) const { 
    return EndsWith(s.data(), static_cast<int>(s.size())); 
}

// Conversions
std::string String::ToStd() const { 
    return *this; 
}
const String& String::ToString() const { 
    return *this; 
}
WString String::ToWString() const {
    // This is a placeholder implementation
    return WString();
}
int String::GetCharCount() const { 
    return GetLength(); 
}
String String::GetVoid() { 
    return String(); 
}
bool String::IsVoid() const { 
    return false; 
}

}