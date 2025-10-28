#pragma once
// U++-compatible WString wrapper implemented on top of std::wstring
// This header is aggregated and wrapped into namespace Upp by Core.h

class WString : public std::wstring {
public:
    using Base = std::wstring;
    using Base::basic_string; // inherit std::wstring constructors
    

    // Interop with std::wstring
    WString(const std::wstring& s) : Base(s) {}
    WString(std::wstring&& s) noexcept : Base(std::move(s)) {}

    // U++-style helpers
    void Clear();
    int  GetLength() const;
    bool IsEmpty() const;
    bool Is() const;

    const wchar_t* Begin() const;
    const wchar_t* End() const;

    int operator[](int i) const;

    // Append
    void Cat(int c);
    void Cat(const wchar_t* s, int len);
    void Cat(const wchar_t* s);
    void Cat(const WString& s);
    void Cat(int c, int count);

    // Operator sugar
    WString& operator+=(wchar_t c);
    WString& operator+=(const wchar_t* s);
    WString& operator+=(const WString& s);

    // Insert
    void Insert(int pos, int c);
    void Insert(int pos, const wchar_t* s, int count);
    void Insert(int pos, const WString& s);

    // Trim and reserve
    void Trim(int pos);
    void TrimLast(int count);
    void Reserve(int r);
    void Shrink();

    // Compare / equality
    int  Compare(const WString& s) const;
    int  Compare(const wchar_t* s) const;
    bool IsEqual(const WString& s) const;
    bool IsEqual(const wchar_t* s) const;

    // Slices
    WString Mid(int pos, int length) const;
    WString Mid(int pos) const;
    WString Right(int count) const;
    WString Left(int count) const;

    // Find helpers
    int Find(int chr, int from = 0) const;
    int ReverseFind(int chr, int from) const;
    int ReverseFind(int chr) const;

    int Find(const wchar_t* s, int from = 0) const;
    int Find(const WString& s, int from = 0) const;
    int FindAfter(const wchar_t* s, int from = 0) const;
    int FindAfter(const WString& s, int from = 0) const;

    bool StartsWith(const wchar_t* s, int len) const;
    bool StartsWith(const wchar_t* s) const;
    bool StartsWith(const WString& s) const;

    bool EndsWith(const wchar_t* s, int len) const;
    bool EndsWith(const wchar_t* s) const;
    bool EndsWith(const WString& s) const;

    // Conversions (UTF-8 <-> wide)
    // Helpers to convert between UTF-8 and wide using platform facilities
    static std::string WideToUtf8(const std::wstring& ws);
    static std::wstring Utf8ToWide(const char* s, size_t len);

    String ToString() const;
};

// Define String::ToWString now that WString is complete
inline WString String::ToWString() const {
    return WString(WString::Utf8ToWide(Begin(), (size_t)GetLength()));
}
