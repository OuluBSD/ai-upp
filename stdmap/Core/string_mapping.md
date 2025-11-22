# U++ to STL Mapping: Core Package Strings

This document provides comprehensive mapping between U++ Core package string types and their STL equivalents.

## 1. String ↔ std::string

### U++ Declaration
```cpp
class String : Moveable<String>, public AString<String0> {
    // Implementation uses small string optimization with different storage tiers
    enum { //
        KIND = 14,    // chr[KIND] is String tier flag, 0 - small, 31 - medium, 32..254 ref alloc, 255 - read alloc from Ref
        SLEN = 15,    // chr[SLEN] stores the length of small tier strings (up to 14 bytes)
        LLEN = 2,     // chr[LLEN] stores the length of medium (< 32) and large tier strings
        SPECIAL = 13, // chr[SPECIAL]: stores Value type
    };
    
public:
    int  GetLength() const;                           // Get string length
    int  GetCount() const;                            // Get character count
    bool IsEmpty() const;                             // Check if empty
    bool Is() const;                                  // Check if non-empty
    const char *End() const;                          // End iterator
    const char *end() const;                          // End iterator
    const char *Last() const;                         // Last character pointer
    const char *GetIter(int i) const;                 // Get iterator at position
    int operator*() const;                            // Dereference first character
    int operator[](int i) const;                      // Access character at index
    operator const char *() const;                    // Implicit conversion to C-string
    const char *operator~() const;                    // Access as C-string
    operator const byte *() const;                    // Implicit conversion to byte array
    operator const void *() const;                    // Implicit conversion to void pointer
    void Insert(int pos, int c);                      // Insert character at position
    void Insert(int pos, const char *s, int count);   // Insert substring at position
    void Insert(int pos, const String& s);            // Insert string at position
    void Insert(int pos, const char *s);              // Insert C-string at position
    void TrimLast(int count = 1);                     // Trim from end
    void Cat(int c);                                  // Concatenate character
    void Cat(const char *s, int len);                 // Concatenate string with length
    void Cat(const char *s);                          // Concatenate C-string
    void Cat(const String& s);                        // Concatenate string
    void Cat(int c, int count);                       // Concatenate character multiple times
    void Cat(const char *s, const char *lim);         // Concatenate from range
    void Cat(const String& s, int len);               // Concatenate string with length limit
    String& Cat();                                    // Return reference after cat
    int Compare(const String& s) const;               // Compare with other string
    int Compare(const char *s) const;                 // Compare with C-string
    bool IsEqual(const String& s) const;              // Check equality with string
    bool IsEqual(const char *s) const;                // Check equality with C-string
    String Mid(int pos, int length) const;            // Extract substring
    String Mid(int pos) const;                        // Extract substring from position
    String Right(int count) const;                    // Extract right part
    String Left(int count) const;                     // Extract left part
    int Find(int chr, int from = 0) const;            // Find character
    int ReverseFind(int chr, int from) const;         // Find character from end with start pos
    int ReverseFind(int chr) const;                   // Find character from end
    int Find(int len, const char *s, int from) const; // Find substring with length
    int Find(const char *s, int from = 0) const;      // Find C-string
    int Find(const String& s, int from = 0) const;    // Find string
    int FindAfter(const char *s, int from = 0) const; // Find after substring
    int FindAfter(const String& s, int from = 0) const; // Find after string
    int ReverseFind(int len, const char *s, int from) const; // Find from end with length
    int ReverseFind(const char *s, int from) const;   // Find from end
    int ReverseFind(const String& s, int from) const; // Find string from end
    int ReverseFind(const char *s) const;             // Find from end (end of string)
    int ReverseFind(const String& s) const;           // Find string from end (end of string)
    int ReverseFindAfter(int len, const char *s, int from) const; // Find after from end with length
    int ReverseFindAfter(const char *s, int from) const; // Find after from end
    int ReverseFindAfter(const String& s, int from) const; // Find after string from end
    int ReverseFindAfter(const char *s) const;        // Find after from end (end of string)
    int ReverseFindAfter(const String& s) const;      // Find after string from end (end of string)
    void Replace(const char *find, int findlen, const char *replace, int replacelen); // Replace substring
    void Replace(const String& find, const String& replace); // Replace string
    void Replace(const char *find, const char *replace); // Replace C-strings
    void Replace(const String& find, const char *replace); // Replace with mixed types
    void Replace(const char *find, const String& replace); // Replace with mixed types
    bool StartsWith(const char *s, int len) const;    // Check starts with
    bool StartsWith(const char *s) const;             // Check starts with C-string
    bool StartsWith(const String& s) const;           // Check starts with string
    bool TrimStart(const char *s, int len) const;     // Trim start with length
    bool TrimStart(const char *s) const;              // Trim start C-string
    bool TrimStart(const String& s) const;            // Trim start string
    bool EndsWith(const char *s, int len) const;      // Check ends with
    bool EndsWith(const char *s) const;               // Check ends with C-string
    bool EndsWith(const String& s) const;             // Check ends with string
    bool TrimEnd(const char *s, int len) const;       // Trim end with length
    bool TrimEnd(const char *s) const;                // Trim end C-string
    bool TrimEnd(const String& s) const;              // Trim end string
    int FindFirstOf(int len, const char *set, int from = 0) const; // Find first of char set with length
    int FindFirstOf(const char *set, int from = 0) const; // Find first of char set
    int FindFirstOf(const String& set, int from = 0) const; // Find first of string set
    bool operator<(const String& a, const String& b); // Less than comparison
    bool operator<=(const String& a, const String& b); // Less or equal comparison
    bool operator>(const String& a, const String& b); // Greater than comparison
    bool operator>=(const String& a, const String& b); // Greater or equal comparison
    bool operator==(const String& a, const String& b); // Equality comparison
    bool operator!=(const String& a, const String& b); // Inequality comparison
    String operator+(const String& a, const String& b); // String concatenation
    const String& operator+=(const String& s);       // Concatenate and assign
    String& operator=(const char *s);                // Assignment from C-string
    String& operator=(const String& s);              // Assignment from string
    String& operator=(String&& s);                   // Move assignment
    String& operator=(StringBuffer& b);              // Assignment from buffer
    void Shrink();                                   // Minimize memory usage
    int GetCharCount() const;                        // Get character count
    String();                                        // Default constructor
    String(const String& s);                         // Copy constructor
    String(String&& s);                              // Move constructor
    String(const char *s);                           // Constructor from C-string
    String(const String& s, int n);                  // Constructor with length limit
    String(const char *s, int n);                    // Constructor with C-string and length
    String(const byte *s, int n);                    // Constructor with byte array and length
    String(const char *s, const char *lim);          // Constructor from range
    String(int chr, int count);                      // Constructor from repeated character
    String(StringBuffer& b);                         // Constructor from buffer
    WString ToWString() const;                       // Convert to wide string
    const String& ToString() const;                  // Self reference as string
    static String GetVoid();                         // Get void string
    bool IsVoid() const;                             // Check if void string
    static String Make(int alloc, Maker m);          // Create with specific allocator
    std::string ToStd() const;                       // Convert to std::string
    String(const std::string& s);                    // Constructor from std::string
};
```

### STL Equivalent
```cpp
class std::string {
    char* data;
    size_t size;
    size_t capacity;

public:
    size_type length() const;                         // Get string length
    size_type size() const;                           // Get character count
    bool empty() const;                               // Check if empty
    const char* c_str() const;                        // Get C-string
    const char* data() const;                         // Get data pointer
    const_iterator end() const;                       // End iterator
    iterator end();                                   // End iterator
    const_reverse_iterator rbegin() const;            // Reverse begin iterator
    const_reverse_iterator rend() const;              // Reverse end iterator
    void insert(size_type pos, size_type count, char ch); // Insert repeated character
    void insert(size_type pos, const char* s);        // Insert C-string
    void insert(size_type pos, const std::string& str); // Insert string
    void insert(const_iterator pos, size_type count, char ch); // Insert at iterator
    void insert(const_iterator pos, char ch);          // Insert single character
    void pop_back();                                  // Remove last character
    void append(size_type count, char ch);            // Append repeated character
    std::string& append(const std::string& str);      // Append string
    std::string& append(const std::string& str, size_type pos, size_type count); // Append substring
    std::string& append(const char* s);               // Append C-string
    std::string& append(const char* s, size_type count); // Append C-string with length
    std::string& operator+=(const std::string& str);  // Concatenate and assign
    int compare(const std::string& str) const;        // Compare with string
    int compare(const char* s) const;                 // Compare with C-string
    int compare(size_type pos1, size_type count1, const std::string& str) const; // Compare substring
    bool starts_with(const std::string& x) const;     // Check starts with (C++20)
    bool starts_with(const char* x) const;            // Check starts with (C++20)
    bool starts_with(char x) const;                   // Check starts with (C++20)
    bool ends_with(const std::string& x) const;       // Check ends with (C++20)
    bool ends_with(const char* x) const;              // Check ends with (C++20)
    bool ends_with(char x) const;                     // Check ends with (C++20)
    size_type find(const std::string& str, size_type pos = 0) const; // Find string
    size_type find(const char* s, size_type pos = 0) const; // Find C-string
    size_type find(char ch, size_type pos = 0) const; // Find character
    size_type rfind(const std::string& str, size_type pos = npos) const; // Find last string
    size_type rfind(const char* s, size_type pos = npos) const; // Find last C-string
    size_type rfind(char ch, size_type pos = npos) const; // Find last character
    size_type find_first_of(const std::string& str, size_type pos = 0) const; // Find first of
    size_type find_first_of(const char* s, size_type pos = 0) const; // Find first of C-string chars
    size_type find_first_of(char ch, size_type pos = 0) const; // Find first of character
    std::string substr(size_type pos = 0, size_type count = npos) const; // Extract substring
    std::string& replace(size_type pos, size_type count, const std::string& str); // Replace
    std::string& replace(const_iterator first, const_iterator last, const std::string& str); // Replace range
    bool operator<(const std::string& str) const;     // Less than comparison
    bool operator<=(const std::string& str) const;    // Less or equal comparison
    bool operator>(const std::string& str) const;     // Greater than comparison
    bool operator>=(const std::string& str) const;    // Greater or equal comparison
    bool operator==(const std::string& str) const;    // Equality comparison
    bool operator!=(const std::string& str) const;    // Inequality comparison
    std::string operator+(const std::string& lhs, const std::string& rhs); // Concatenation
    std::string& operator=(const char* s);            // Assignment from C-string
    std::string& operator=(const std::string& str);   // Assignment from string
    std::string& operator=(std::string&& str);        // Move assignment
    void shrink_to_fit();                             // Minimize memory usage
    size_type max_size() const;                       // Max possible size
    const char& at(size_type pos) const;              // Access with bounds check
    char& at(size_type pos);                          // Access with bounds check
    const char& operator[](size_type pos) const;      // Access without bounds check
    char& operator[](size_type pos);                  // Access without bounds check
    std::string();                                    // Default constructor
    std::string(const std::string& other);            // Copy constructor
    std::string(std::string&& other);                 // Move constructor
    std::string(const char* s);                       // Constructor from C-string
    std::string(const char* s, size_type count);      // Constructor with length
    std::string(size_type count, char ch);            // Constructor with repeated char
    std::string(const std::string& str, size_type pos, size_type count = npos); // Constructor from substring
    template< class InputIt > std::string(InputIt first, InputIt last); // Constructor from range
    std::string& assign(const std::string& str);      // Assignment
    std::string& assign(std::string&& str);           // Move assignment
    std::string& assign(const char* s);               // Assign from C-string
    std::string& assign(size_type count, char ch);    // Assign repeated character
    template< class InputIt > std::string& assign(InputIt first, InputIt last); // Assign from range
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| String | std::string | ✓ Complete | |
| String::GetLength() | std::string::length() | ✓ Complete | |
| String::GetCount() | std::string::size() | ✓ Complete | |
| String::IsEmpty() | std::string::empty() | ✓ Complete | |
| String::operator~() | std::string::c_str() | ✓ Complete | |
| String::operator const char *() | std::string::c_str() | ✓ Complete | |
| String::operator[](int i) | std::string::operator[](size_type pos) | ✓ Complete | |
| String::Mid(pos, length) | std::string::substr(pos, length) | ✓ Complete | |
| String::Left(count) | std::string::substr(0, count) | ✓ Complete | |
| String::Right(count) | std::string::substr(size()-count, count) | ✓ Complete | |
| String::Find(chr, from) | std::string::find(chr, from) | ✓ Complete | |
| String::Find(const char *, from) | std::string::find(const char *, from) | ✓ Complete | |
| String::Find(const String&, from) | std::string::find(const std::string&, from) | ✓ Complete | |
| String::ReverseFind(chr) | std::string::rfind(chr) | ✓ Complete | |
| String::ReverseFind(const char *) | std::string::rfind(const char *) | ✓ Complete | |
| String::ReverseFind(const String&) | std::string::rfind(const std::string&) | ✓ Complete | |
| String::StartsWith(const char *) | std::string::starts_with(const char *) (C++20) | ✓ Complete | |
| String::StartsWith(const String&) | std::string::starts_with(const std::string&) (C++20) | ✓ Complete | |
| String::EndsWith(const char *) | std::string::ends_with(const char *) (C++20) | ✓ Complete | |
| String::EndsWith(const String&) | std::string::ends_with(const std::string&) (C++20) | ✓ Complete | |
| String::Replace(find, replace) | std::string::replace() | ⚠️ Complex | Requires manual implementation |
| String::Compare(s) | std::string::compare(s) | ✓ Complete | |
| String::IsEqual(s) | std::string::operator==(s) | ✓ Complete | |
| String::Cat(x) | std::string::append(x) or operator+= | ✓ Complete | |
| String::operator+=(x) | std::string::operator+=(x) | ✓ Complete | |
| String::Insert(pos, chr) | std::string::insert(pos, 1, chr) | ✓ Complete | |
| String::Insert(pos, const char *, count) | std::string::insert(pos, const char *, count) | ✓ Complete | |
| String::operator+(a, b) | std::string::operator+(a, b) | ✓ Complete | |
| String::Clear() | std::string::clear() | ✓ Complete | |
| String::Shrink() | std::string::shrink_to_fit() | ✓ Complete | |
| String(const char *) | std::string(const char *) | ✓ Complete | |
| String(const String&) | std::string(const std::string&) | ✓ Complete | |
| String(String&&) | std::string(std::string&&) | ✓ Complete | |
| String& operator=(const char *) | std::string& operator=(const char *) | ✓ Complete | |
| String& operator=(const String&) | std::string& operator=(const std::string&) | ✓ Complete | |
| String& operator=(String&&) | std::string& operator=(std::string&&) | ✓ Complete | |

### Conversion Notes
- U++ String uses Small String Optimization (SSO) which std::string also typically implements
- String::GetLength() and String::GetCount() both map to std::string::size() or length()
- U++ uses "Find" for search operations while STL uses "find"
- U++ uses "ReverseFind" while STL uses "rfind" (reverse find)
- U++ String supports a "void" state which std::string doesn't have directly
- String::ToStd() and the std::string constructor from String provide bidirectional conversion
- Most operations map directly between U++ String and std::string

## 2. WString ↔ std::wstring

### U++ Declaration
```cpp
class WString : Moveable<WString>, public AString<WString0> {
public:
    UPP::String ToString() const;                     // Convert to String
    const WString& operator+=(wchar c);               // Concatenate wide character
    const WString& operator+=(const wchar *s);        // Concatenate wide C-string
    const WString& operator+=(const WString& s);      // Concatenate wide string
    WString& operator<<(wchar c);                     // Concatenate with operator
    WString& operator<<(const WString& s);           // Concatenate with operator
    WString& operator<<(const wchar *s);             // Concatenate with operator
    WString& operator=(const wchar *s);              // Assignment from wide C-string
    WString& operator=(const WString& s);            // Assignment from wide string
    WString& operator=(const Nuller& n);             // Assignment from null
    WString& operator=(WString&& s);                 // Move assignment
    WString& operator=(WStringBuffer& b);            // Assignment from buffer
    void Shrink();                                   // Minimize memory usage
    WString();                                       // Default constructor
    WString(const Nuller&);                          // Constructor from null
    WString(const WString& s);                      // Copy constructor
    WString(WString&& s);                           // Move constructor
    WString(const wchar *s);                        // Constructor from wide C-string
    WString(const WString& s, int n);               // Constructor with length limit
    WString(const wchar *s, int n);                 // Constructor with wide C-string and length
    WString(const wchar *s, const wchar *lim);       // Constructor from range
    WString(int chr, int count);                    // Constructor from repeated character
    WString(WStringBuffer& b);                      // Constructor from buffer
    WString(const char *s);                         // Constructor from char string
    WString(const char *s, int n);                  // Constructor from char string with length
    WString(const char *s, const char *lim);        // Constructor from char range
    WString(const char16 *s);                       // Constructor from char16 string
    static WString GetVoid();                       // Get void wide string
    bool IsVoid() const;                            // Check if void wide string
    #ifndef _HAVE_NO_STDWSTRING
    WString(const std::wstring& s);                 // Constructor from std::wstring
    operator std::wstring() const;                   // Convert to std::wstring
    std::wstring ToStd() const;                     // Convert to std::wstring
    #endif
};
```

### STL Equivalent
```cpp
class std::wstring {
    wchar_t* data;
    size_t size;
    size_t capacity;

public:
    size_type length() const;                         // Get string length
    size_type size() const;                           // Get character count
    bool empty() const;                               // Check if empty
    const wchar_t* c_str() const;                     // Get C-string
    const wchar_t* data() const;                      // Get data pointer
    const_iterator end() const;                       // End iterator
    iterator end();                                   // End iterator
    void append(size_type count, wchar_t ch);         // Append repeated character
    std::wstring& append(const std::wstring& str);    // Append string
    std::wstring& append(const wchar_t* s);           // Append C-string
    std::wstring& append(const wchar_t* s, size_type count); // Append C-string with length
    std::wstring& operator+=(const std::wstring& str); // Concatenate and assign
    int compare(const std::wstring& str) const;       // Compare with string
    size_type find(const std::wstring& str, size_type pos = 0) const; // Find string
    size_type find(const wchar_t* s, size_type pos = 0) const; // Find C-string
    size_type find(wchar_t ch, size_type pos = 0) const; // Find character
    size_type rfind(const std::wstring& str, size_type pos = npos) const; // Find last string
    size_type rfind(const wchar_t* s, size_type pos = npos) const; // Find last C-string
    size_type rfind(wchar_t ch, size_type pos = npos) const; // Find last character
    std::wstring substr(size_type pos = 0, size_type count = npos) const; // Extract substring
    bool operator<(const std::wstring& str) const;    // Less than comparison
    bool operator<=(const std::wstring& str) const;   // Less or equal comparison
    bool operator>(const std::wstring& str) const;    // Greater than comparison
    bool operator>=(const std::wstring& str) const;   // Greater or equal comparison
    bool operator==(const std::wstring& str) const;   // Equality comparison
    bool operator!=(const std::wstring& str) const;   // Inequality comparison
    std::wstring operator+(const std::wstring& lhs, const std::wstring& rhs); // Concatenation
    std::wstring& operator=(const wchar_t* s);        // Assignment from C-string
    std::wstring& operator=(const std::wstring& str); // Assignment from string
    std::wstring& operator=(std::wstring&& str);      // Move assignment
    void shrink_to_fit();                             // Minimize memory usage
    const wchar_t& at(size_type pos) const;           // Access with bounds check
    wchar_t& at(size_type pos);                       // Access with bounds check
    const wchar_t& operator[](size_type pos) const;   // Access without bounds check
    wchar_t& operator[](size_type pos);               // Access without bounds check
    std::wstring();                                   // Default constructor
    std::wstring(const std::wstring& other);          // Copy constructor
    std::wstring(std::wstring&& other);               // Move constructor
    std::wstring(const wchar_t* s);                   // Constructor from C-string
    std::wstring(const wchar_t* s, size_type count);  // Constructor with length
    std::wstring(size_type count, wchar_t ch);        // Constructor with repeated char
    std::wstring(const std::wstring& str, size_type pos, size_type count = npos); // Constructor from substring
    template< class InputIt > std::wstring(InputIt first, InputIt last); // Constructor from range
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| WString | std::wstring | ✓ Complete | |
| WString::ToString() | std::wstring conversion to UPP::String | ⚠️ Implementation needed | |
| WString::operator+=(wchar c) | std::wstring::operator+=(wchar_t c) | ✓ Complete | |
| WString::operator+=(const wchar *s) | std::wstring::operator+=(const wchar_t *s) | ✓ Complete | |
| WString::operator+=(const WString& s) | std::wstring::operator+=(const std::wstring& s) | ✓ Complete | |
| WString::operator=(const wchar *s) | std::wstring::operator=(const wchar_t *s) | ✓ Complete | |
| WString::operator=(const WString& s) | std::wstring::operator=(const std::wstring& s) | ✓ Complete | |
| WString::operator=(WString&& s) | std::wstring::operator=(std::wstring&& s) | ✓ Complete | |
| WString::GetLength() | std::wstring::length() | ✓ Complete | Inherited from AString |
| WString::GetCount() | std::wstring::size() | ✓ Complete | Inherited from AString |
| WString::IsEmpty() | std::wstring::empty() | ✓ Complete | Inherited from AString |
| WString::operator~() | std::wstring::c_str() | ✓ Complete | Inherited from AString |
| WString::operator const wchar *() | std::wstring::c_str() | ✓ Complete | Inherited from AString |
| WString::Mid(pos, length) | std::wstring::substr(pos, length) | ✓ Complete | Inherited from AString |
| WString::Find(chr, from) | std::wstring::find(chr, from) | ✓ Complete | Inherited from AString |
| WString::Compare(s) | std::wstring::compare(s) | ✓ Complete | Inherited from AString |
| WString::Shrink() | std::wstring::shrink_to_fit() | ✓ Complete | |
| WString(const wchar *s) | std::wstring(const wchar_t *s) | ✓ Complete | |
| WString(const WString&) | std::wstring(const std::wstring&) | ✓ Complete | |
| WString(WString&&) | std::wstring(std::wstring&&) | ✓ Complete | |
| std::wstring ToStd() | Direct conversion | ✓ Complete | |
| WString(const std::wstring& s) | Direct conversion | ✓ Complete | |

### Conversion Notes
- WString corresponds directly to std::wstring for wide character string handling
- All functionality from AString applies to WString, providing a full set of string operations
- WString can convert to and from std::wstring using the ToStd() method and constructor
- Both types handle wide character strings (typically UTF-16 or UTF-32 depending on platform)

## 3. StringBuffer ↔ std::string (for building) or std::ostringstream

### U++ Declaration
```cpp
class StringBuffer : NoCopy {
    char   *pbegin;                                  // Beginning of buffer
    char   *pend;                                    // End of used buffer
    char   *limit;                                   // End of allocated buffer
    char    buffer[256];                             // Small buffer on stack

public:
    char *Begin();                                   // Get beginning pointer
    char *begin();                                   // Get beginning pointer (STL compatibility)
    char *End();                                     // Get end pointer
    char *end();                                     // Get end pointer (STL compatibility)
    char& operator*();                               // Dereference first element
    char& operator[](int i);                         // Access element
    operator char*();                                // Implicit conversion to char*
    operator byte*();                                // Implicit conversion to byte*
    operator void*();                                // Implicit conversion to void*
    char *operator~();                               // Same as Begin()
    void SetLength(int l);                           // Set length of buffer
    void SetCount(int l);                            // Set count (alias for SetLength)
    int  GetLength() const;                          // Get current length
    int  GetCount() const;                           // Get current count
    void Strlen();                                  // Set length based on null terminator
    void Clear();                                   // Clear buffer
    void Reserve(int r);                            // Reserve additional space
    void Shrink();                                  // Minimize memory usage
    void Cat(int c);                                // Concatenate character
    void Cat(int c, int count);                     // Concatenate repeated character
    void Cat(const char *s, int l);                 // Concatenate string with length
    void Cat(const char *s, const char *e);         // Concatenate from range
    void Cat(const char *s);                        // Concatenate C-string
    void Cat(const String& s);                      // Concatenate String
    int  GetAlloc() const;                          // Get allocated size
    void operator=(String& s);                      // Assignment to String
    StringBuffer();                                 // Default constructor
    StringBuffer(String& s);                        // Constructor from String
    StringBuffer(int len);                          // Constructor with initial length
    ~StringBuffer();                                // Destructor
};
```

### STL Equivalent
```cpp
// Primary equivalent: std::string (for building strings efficiently)
class std::string {
    // ... same as above

    void push_back(char ch);                         // Add character at end
    void append(size_type count, char ch);           // Append repeated character
    std::string& append(const char* s);              // Append C-string
    std::string& append(const char* s, size_type count); // Append C-string with length
    void reserve(size_type new_cap);                 // Reserve capacity
    size_type capacity() const;                      // Get capacity
};

// Alternative equivalent: std::ostringstream (for formatted output)
#include <sstream>
class std::ostringstream : public std::ostream {
    std::string str() const;                         // Get resulting string
    void str(const std::string& s);                  // Set internal string
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| StringBuffer | std::string or std::ostringstream | ✓ Complete | Use std::string for building, ostringstream for formatting |
| StringBuffer::Cat(c) | std::string::push_back(c) or operator+= | ✓ Complete | |
| StringBuffer::Cat(c, count) | std::string::append(count, c) | ✓ Complete | |
| StringBuffer::Cat(const char *, int) | std::string::append(const char *, size_type) | ✓ Complete | |
| StringBuffer::Cat(const char *) | std::string::append(const char *) | ✓ Complete | |
| StringBuffer::Cat(const String&) | std::string::append(const std::string&) | ✓ Complete | |
| StringBuffer::GetLength() | std::string::size() or length() | ✓ Complete | |
| StringBuffer::operator~() | std::string::data() or c_str() | ✓ Complete | |
| StringBuffer::Clear() | std::string::clear() | ✓ Complete | |
| StringBuffer::Reserve(r) | std::string::reserve(r) | ✓ Complete | |
| StringBuffer::Shrink() | std::string::shrink_to_fit() | ✓ Complete | |
| StringBuffer(String& s) | std::string constructor | ✓ Complete | |
| operator<<(StringBuffer& s, const char *x) | std::ostringstream << operator | ✓ Complete | |

### Conversion Notes
- StringBuffer is primarily for efficient string building, similar to std::string but optimized for appends
- For formatted output, std::ostringstream is also a good equivalent
- StringBuffer uses a small buffer optimization similar to std::string's SSO
- StringBuffer's overloaded operators with << correspond to both std::string append operations and std::ostringstream stream operations

## 4. StringStream ↔ std::stringstream

### U++ Declaration
```cpp
class StringStream : public Stream {
protected:
    virtual void _Put(int w);                        // Internal put operation
    virtual int  _Term();                            // Internal terminal operation
    virtual int  _Get();                             // Internal get operation
    virtual void _Put(const void *data, dword size); // Internal put block
    virtual dword _Get(void *data, dword size);      // Internal get block

public:
    virtual void  Seek(int64 pos);                   // Seek to position
    virtual int64 GetSize() const;                   // Get total size
    virtual void  SetSize(int64 size);               // Set size
    virtual bool  IsOpen() const;                    // Check if open

protected:
    bool           writemode;                        // Write mode flag
    String         data;                             // String data in read mode
    StringBuffer   wdata;                            // Buffer in write mode
    dword          size;                             // Current size
    int            limit;                            // Size limit

public:
    void        Open(const String& data);           // Open in read mode
    void        Create();                           // Create in write mode
    void        Reserve(int n);                     // Reserve space
    
    String      GetResult();                        // Get resulting string
    operator    String();                           // Convert to String
    
    void        Limit(int sz);                      // Set limit
    
    StringStream();                                 // Default constructor
    StringStream(const String& data);               // Constructor with data
};
```

### STL Equivalent
```cpp
#include <sstream>
class std::stringstream : public std::iostream {
public:
    std::string str() const;                         // Get content
    void str(const std::string& s);                  // Set content
    // Inherits from std::iostream which provides:
    std::iostream& seekg(std::streampos pos);        // Seek get pointer
    std::streampos tellg();                          // Get get pointer position
    std::iostream& seekp(std::streampos pos);        // Seek put pointer
    std::streampos tellp();                          // Get put pointer position
    int get();                                      // Get character
    std::istream& get(char& c);                     // Get character
    std::istream& get(char* s, std::streamsize n);   // Get C-string
    std::istream& read(char* s, std::streamsize n);  // Read characters
    std::ostream& put(char c);                      // Put character
    std::ostream& write(const char* s, std::streamsize n); // Write characters
    // And all other stream operations
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| StringStream | std::stringstream | ✓ Complete | |
| StringStream(const String& data) | std::stringstream constructor with std::string | ✓ Complete | |
| StringStream::Open(data) | std::stringstream::str(data) | ✓ Complete | |
| StringStream::Create() | std::stringstream constructor | ✓ Complete | |
| StringStream::GetResult() | std::stringstream::str() | ✓ Complete | |
| StringStream::operator String() | std::stringstream::str() | ✓ Complete | |
| StringStream::GetSize() | std::stringstream::str().size() | ✓ Complete | |
| StringStream::Seek(pos) | std::stringstream::seekg(pos) and seekp(pos) | ✓ Complete | |
| StringStream::Get() | std::stringstream::get() | ✓ Complete | |
| StringStream::Put(c) | std::stringstream::put(c) | ✓ Complete | |
| StringStream::Get(void *data, int size) | std::stringstream::read() | ✓ Complete | |
| StringStream::Put(const void *data, int size) | std::stringstream::write() | ✓ Complete | |

### Conversion Notes
- StringStream provides both input and output operations like std::stringstream
- StringStream::Open() and StringStream::Create() control the mode of operation
- StringStream::GetResult() maps directly to std::stringstream::str() for getting the content
- Both provide stream-like operations with buffering

## Summary of String Mappings

| U++ String Type | STL Equivalent | Notes |
|----------------|----------------|-------|
| String | std::string | Direct mapping with similar functionality |
| WString | std::wstring | Wide character equivalent |
| StringBuffer | std::string or std::ostringstream | For efficient string building |
| StringStream | std::stringstream | String-based stream operations |