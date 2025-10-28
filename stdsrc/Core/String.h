#pragma once
// U++-compatible String wrapper implemented on top of std::string
// This header is aggregated and wrapped into namespace Upp by Core.h

class WString; // forward declaration

class String : public std::string {
public:
    using Base = std::string;
    using Base::basic_string; // inherit std::string constructors

    // Interop with std::string
    String(const std::string& s) : Base(s) {}
    String(std::string&& s) noexcept : Base(std::move(s)) {}

    // U++-style helpers
    void Clear();
    int  GetLength() const;
    bool IsEmpty() const;
    bool Is() const;

    const char* Begin() const;
    const char* End() const;

    int operator[](int i) const;

    // Append
    void Cat(int c);
    void Cat(const char* s, int len);
    void Cat(const char* s);
    void Cat(const String& s);
    void Cat(int c, int count);

    // Operator sugar
    String& operator+=(char c);
    String& operator+=(const char* s);
    String& operator+=(const String& s);

    // Insert
    void Insert(int pos, int c);
    void Insert(int pos, const char* s, int count);
    void Insert(int pos, const String& s);

    // Trim and reserve
    void Trim(int pos);
    void TrimLast(int count);
    void Reserve(int r);
    void Shrink();

    // Compare / equality
    int  Compare(const String& s) const;
    int  Compare(const char* s) const;
    bool IsEqual(const String& s) const;
    bool IsEqual(const char* s) const;

    // Slices
    String Mid(int pos, int length) const;
    String Mid(int pos) const;
    String Right(int count) const;
    String Left(int count) const;

    // Find helpers
    int Find(int chr, int from = 0) const;
    int ReverseFind(int chr, int from) const;
    int ReverseFind(int chr) const;

    int Find(const char* s, int from = 0) const;
    int Find(const String& s, int from = 0) const;
    int FindAfter(const char* s, int from = 0) const;
    int FindAfter(const String& s, int from = 0) const;

    bool StartsWith(const char* s, int len) const;
    bool StartsWith(const char* s) const;
    bool StartsWith(const String& s) const;

    bool EndsWith(const char* s, int len) const;
    bool EndsWith(const char* s) const;
    bool EndsWith(const String& s) const;

    // Conversions
    std::string ToStd() const;
    const String& ToString() const;
    WString ToWString() const;
    int GetCharCount() const;
    static String GetVoid();
    bool IsVoid() const;
};
