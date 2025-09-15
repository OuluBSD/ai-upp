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
    void Clear() { this->clear(); }
    int  GetLength() const { return static_cast<int>(this->size()); }
    bool IsEmpty() const { return this->empty(); }
    bool Is() const { return !this->empty(); }

    const wchar_t* Begin() const { return this->data(); }
    const wchar_t* End() const { return this->data() + this->size(); }

    int operator[](int i) const { return static_cast<unsigned int>(Base::operator[](static_cast<size_t>(i))); }

    // Append
    void Cat(int c) { this->push_back(static_cast<wchar_t>(c)); }
    void Cat(const wchar_t* s, int len) { if(len > 0) this->append(s, static_cast<size_t>(len)); }
    void Cat(const wchar_t* s) { if(s) this->append(s); }
    void Cat(const WString& s) { this->append(s.data(), s.size()); }
    void Cat(int c, int count) { if(count > 0) this->append(static_cast<size_t>(count), static_cast<wchar_t>(c)); }

    // Operator sugar
    WString& operator+=(wchar_t c) { this->push_back(c); return *this; }
    WString& operator+=(const wchar_t* s) { if(s) this->append(s); return *this; }
    WString& operator+=(const WString& s) { this->append(s); return *this; }

    // Insert
    void Insert(int pos, int c) { this->insert(this->begin() + static_cast<ptrdiff_t>(pos), static_cast<wchar_t>(c)); }
    void Insert(int pos, const wchar_t* s, int count) { this->insert(static_cast<size_t>(pos), s, static_cast<size_t>(count)); }
    void Insert(int pos, const WString& s) { this->insert(static_cast<size_t>(pos), s); }

    // Trim and reserve
    void Trim(int pos) { if(pos < 0) pos = 0; if(static_cast<size_t>(pos) < this->size()) this->resize(static_cast<size_t>(pos)); }
    void TrimLast(int count = 1) { if(count <= 0) return; if(this->size() >= static_cast<size_t>(count)) this->resize(this->size() - static_cast<size_t>(count)); else this->clear(); }
    void Reserve(int r) { this->reserve(static_cast<size_t>(r)); }
    void Shrink() { this->shrink_to_fit(); }

    // Compare / equality
    int  Compare(const WString& s) const { return this->compare(s); }
    int  Compare(const wchar_t* s) const { return this->compare(s ? s : L""); }
    bool IsEqual(const WString& s) const { return *this == s; }
    bool IsEqual(const wchar_t* s) const { return this->compare(s ? s : L"") == 0; }

    // Slices
    WString Mid(int pos, int length) const { return WString(this->substr(static_cast<size_t>(pos), static_cast<size_t>(length))); }
    WString Mid(int pos) const { return WString(this->substr(static_cast<size_t>(pos))); }
    WString Right(int count) const { return count <= 0 ? WString() : WString(this->substr(this->size() - static_cast<size_t>(count))); }
    WString Left(int count) const { return count <= 0 ? WString() : WString(this->substr(0, static_cast<size_t>(count))); }

    // Find helpers
    int Find(int chr, int from = 0) const {
        size_t p = this->find(static_cast<wchar_t>(chr), static_cast<size_t>(from));
        return p == Base::npos ? -1 : static_cast<int>(p);
    }
    int ReverseFind(int chr, int from) const {
        if(from < 0) return -1;
        if(from >= GetLength()) from = GetLength() - 1;
        size_t p = this->rfind(static_cast<wchar_t>(chr), static_cast<size_t>(from));
        return p == Base::npos ? -1 : static_cast<int>(p);
    }
    int ReverseFind(int chr) const {
        size_t p = this->rfind(static_cast<wchar_t>(chr));
        return p == Base::npos ? -1 : static_cast<int>(p);
    }

    int Find(const wchar_t* s, int from = 0) const {
        size_t p = this->find(s ? s : L"", static_cast<size_t>(from));
        return p == Base::npos ? -1 : static_cast<int>(p);
    }
    int Find(const WString& s, int from = 0) const {
        size_t p = this->find(s, static_cast<size_t>(from));
        return p == Base::npos ? -1 : static_cast<int>(p);
    }
    int FindAfter(const wchar_t* s, int from = 0) const {
        if(!s) return -1;
        int n = static_cast<int>(std::wcslen(s));
        int q = Find(s, from);
        return q < 0 ? -1 : q + n;
    }
    int FindAfter(const WString& s, int from = 0) const {
        int q = Find(s, from);
        return q < 0 ? -1 : q + static_cast<int>(s.size());
    }

    bool StartsWith(const wchar_t* s, int len) const {
        if(!s) return false; if(len < 0) return false; if(static_cast<size_t>(len) > this->size()) return false;
        return this->compare(0, static_cast<size_t>(len), s, static_cast<size_t>(len)) == 0;
    }
    bool StartsWith(const wchar_t* s) const {
        if(!s) return false; size_t len = std::wcslen(s);
        return StartsWith(s, static_cast<int>(len));
    }
    bool StartsWith(const WString& s) const { return StartsWith(s.data(), static_cast<int>(s.size())); }

    bool EndsWith(const wchar_t* s, int len) const {
        if(!s) return false; if(len < 0) return false; if(static_cast<size_t>(len) > this->size()) return false;
        return this->compare(this->size() - static_cast<size_t>(len), static_cast<size_t>(len), s, static_cast<size_t>(len)) == 0;
    }
    bool EndsWith(const wchar_t* s) const {
        if(!s) return false; size_t len = std::wcslen(s);
        return EndsWith(s, static_cast<int>(len));
    }
    bool EndsWith(const WString& s) const { return EndsWith(s.data(), static_cast<int>(s.size())); }

    // Conversions (UTF-8 <-> wide)
    String ToString() const {
        // NOTE: std::wstring_convert is deprecated in C++17, but still widely available.
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return String(conv.to_bytes(*this).c_str());
    }
};
