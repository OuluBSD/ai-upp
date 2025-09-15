// Simple text I/O helpers using Stream

class TextReader {
    Stream* in = nullptr;
public:
    explicit TextReader(Stream& s) : in(&s) {}
    bool GetLine(String& out) {
        out.Clear();
        if(!in) return false;
        char ch;
        while(true){
            unsigned char c;
            size_t n = in->Get(&c, 1);
            if(n == 0) break;
            if(c == '\n') break;
            if(c == '\r') { // handle CRLF or CR
                unsigned char next;
                size_t m = in->Get(&next, 1);
                if(m && next != '\n') in->Seek(in->Tell() - 1);
                break;
            }
            out.Cat((int)c);
        }
        return out.GetLength() > 0 || !in->IsEof();
    }
};

class TextWriter {
    Stream* out = nullptr;
public:
    explicit TextWriter(Stream& s) : out(&s) {}
    void PutLine(const String& s) {
        if(!out) return;
        out->Put(s.Begin(), s.GetLength());
        char nl = '\n'; out->Put(&nl, 1);
    }
};

inline bool SaveFileAsString(const char* path, const String& content) {
    FileStream fs(path, FileStream::WRITE);
    if(!fs.IsEof() || true) { fs.Put(content.Begin(), content.GetLength()); return true; }
    return false;
}

inline String LoadFileAsString(const char* path) {
    FileStream fs(path, FileStream::READ);
    if(!fs.IsEof() || true) {
        size_t sz = fs.GetSize(); String out; out.Reserve((int)sz);
        std::vector<char> tmp(sz);
        size_t n = fs.Get(tmp.data(), sz);
        out.Cat(tmp.data(), (int)n);
        return out;
    }
    return String();
}

