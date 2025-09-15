// Minimal printf-like formatting

inline String VFormat(const char* fmt, va_list ap) {
    char buf[1024];
    va_list ap2; va_copy(ap2, ap);
    int n = std::vsnprintf(buf, sizeof(buf), fmt, ap2);
    va_end(ap2);
    if(n >= 0 && (size_t)n < sizeof(buf))
        return String(buf, n);
    // allocate dynamically
    size_t size = n > 0 ? (size_t)n + 1 : 2048;
    std::vector<char> tmp(size);
    va_copy(ap2, ap);
    n = std::vsnprintf(tmp.data(), tmp.size(), fmt, ap2);
    va_end(ap2);
    if(n < 0) return String();
    return String(tmp.data(), n);
}

inline String Format(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    String s = VFormat(fmt, ap);
    va_end(ap);
    return s;
}

