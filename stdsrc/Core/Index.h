// Minimal U++-style Index<T> implemented over std::vector + std::unordered_map
// This header is aggregated and wrapped into namespace Upp by Core.h

template <class T>
class Index {
    std::vector<T> items;
    std::unordered_map<T, int> pos; // key -> index

    void rebuild() {
        pos.clear();
        pos.reserve(items.size());
        for (int i = 0; i < (int)items.size(); ++i)
            pos[items[(size_t)i]] = i;
    }
public:
    // Basic info
    int  GetCount() const { return static_cast<int>(items.size()); }
    bool IsEmpty() const { return items.empty(); }
    void Clear() { items.clear(); pos.clear(); }
    void Reserve(int n) { items.reserve(static_cast<size_t>(n)); pos.reserve(static_cast<size_t>(n)); }
    void Shrink() { items.shrink_to_fit(); }

    // Access
    const T& operator[](int i) const { return items[(size_t)i]; }
    T&       operator[](int i)       { return items[(size_t)i]; }
    const T& Top() const { return items.back(); }
    T        Pop() { T x = std::move(items.back()); items.pop_back(); rebuild(); return x; }

    // Find
    int Find(const T& k) const {
        auto it = pos.find(k);
        return it == pos.end() ? -1 : it->second;
    }
    bool Has(const T& k) const { return pos.find(k) != pos.end(); }

    // Add/Put
    void Add(const T& k) {
        if (pos.find(k) != pos.end()) return; // keep unique
        int i = GetCount();
        items.push_back(k);
        pos.emplace(items.back(), i);
    }
    void Add(T&& k) {
        if (pos.find(k) != pos.end()) return;
        int i = GetCount();
        items.push_back(std::move(k));
        pos.emplace(items.back(), i);
    }
    int FindAdd(const T& k) { int i = Find(k); if(i >= 0) return i; Add(k); return GetCount() - 1; }
    int FindAdd(T&& k) { int i = Find(k); if(i >= 0) return i; Add(std::move(k)); return GetCount() - 1; }
    int Put(const T& k) { return FindAdd(k); }
    int Put(T&& k) { return FindAdd(std::move(k)); }

    // Modify
    void Set(int i, const T& k) {
        if (i < 0 || i >= GetCount()) return;
        items[(size_t)i] = k;
        rebuild();
    }
    void Set(int i, T&& k) {
        if (i < 0 || i >= GetCount()) return;
        items[(size_t)i] = std::move(k);
        rebuild();
    }

    // Remove
    int RemoveKey(const T& k) {
        int i = Find(k);
        if(i < 0) return -1;
        Remove(i);
        return i;
    }
    void Remove(int i, int count = 1) {
        if (count <= 0) return;
        int n = GetCount();
        if (i < 0) i = 0; if (i > n) return; if (i + count > n) count = n - i;
        items.erase(items.begin() + i, items.begin() + i + count);
        rebuild();
    }
    void Trim(int n = 0) {
        if (n < 0) n = 0; if (n >= GetCount()) return;
        items.resize(static_cast<size_t>(n));
        rebuild();
    }
    void Drop(int n = 1) { Trim(GetCount() - n); }

    // Iteration
    auto begin() { return items.begin(); }
    auto end() { return items.end(); }
    auto begin() const { return items.begin(); }
    auto end() const { return items.end(); }
};

