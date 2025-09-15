// Minimal U++-style One<T> wrapper implemented via std::unique_ptr
// This header is aggregated and wrapped into namespace Upp by Core.h

template <class T>
class One {
    std::unique_ptr<T> p;
public:
    // Basic ops
    void  Attach(T* t) { p.reset(t); }
    T*    Detach() { return p.release(); }
    void  Clear() { p.reset(); }
    bool  IsEmpty() const { return !p; }
    explicit operator bool() const { return static_cast<bool>(p); }

    // Access
    T*       Get() { return p.get(); }
    const T* Get() const { return p.get(); }
    T&       operator*() { return *p; }
    const T& operator*() const { return *p; }
    T*       operator->() { return p.get(); }
    const T* operator->() const { return p.get(); }

    // Create helpers
    template <class... Args>
    T& Create(Args&&... args) {
        p = std::make_unique<T>(std::forward<Args>(args)...);
        return *p;
    }

    // Ctors/assignment
    One() = default;
    explicit One(T* t) : p(t) {}
    One(One&& o) noexcept : p(std::move(o.p)) {}
    One& operator=(One&& o) noexcept { if(this != &o) p = std::move(o.p); return *this; }

    One(const One&) = delete;
    One& operator=(const One&) = delete;
};

