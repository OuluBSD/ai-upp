// Minimal U++-style VectorMap<K,T> implemented on top of std::vector + std::unordered_map
// This header is aggregated and wrapped into namespace Upp by Core.h

template <class K, class T>
class VectorMap {
    std::vector<std::pair<K, T>> items;      // preserves order
    std::map<K, int>              pos;       // key -> index

    void rebuild() {
        pos.clear();
        pos.reserve(items.size());
        for (int i = 0; i < (int)items.size(); ++i)
            pos[items[(size_t)i].first] = i;
    }
public:
    // Basic info
    int  GetCount() const { return static_cast<int>(items.size()); }
    bool IsEmpty() const { return items.empty(); }
    void Clear() { items.clear(); pos.clear(); }
    void Reserve(int n) { items.reserve(static_cast<size_t>(n)); pos.reserve(static_cast<size_t>(n)); }
    void Shrink() { items.shrink_to_fit(); }

    // Access by index
    T&       operator[](int i) { return items[(size_t)i].second; }
    const T& operator[](int i) const { return items[(size_t)i].second; }
    const K& GetKey(int i) const { return items[(size_t)i].first; }
    T&       GetValue(int i) { return items[(size_t)i].second; }
    const T& GetValue(int i) const { return items[(size_t)i].second; }

    // Access by key (inserts default if missing)
    T& operator[](const K& k) { return GetAdd(k); }

    // Find
    int Find(const K& k) const {
        auto it = pos.find(k);
        return it == pos.end() ? -1 : it->second;
    }
    bool Has(const K& k) const { return pos.find(k) != pos.end(); }

    // Add
    T& Add(const K& k) {
        int i = GetCount();
        items.emplace_back(k, T{});
        pos[items.back().first] = i;
        return items.back().second;
    }
    T& Add(const K& k, const T& v) {
        int i = GetCount();
        items.emplace_back(k, v);
        pos[items.back().first] = i;
        return items.back().second;
    }
    T& Add(K&& k, T&& v) {
        int i = GetCount();
        items.emplace_back(std::move(k), std::move(v));
        pos[items.back().first] = i;
        return items.back().second;
    }

    // Put: insert if missing, otherwise overwrite value; returns position
    int Put(const K& k, const T& v) {
        int i = Find(k);
        if(i < 0) { Add(k, v); return GetCount() - 1; }
        items[(size_t)i].second = v; return i;
    }
    int Put(K&& k, T&& v) {
        int i = Find(k);
        if(i < 0) { Add(std::move(k), std::move(v)); return GetCount() - 1; }
        items[(size_t)i].second = std::move(v); return i;
    }

    // Get with auto-add
    T& GetAdd(const K& k) {
        int i = Find(k);
        if(i >= 0) return items[(size_t)i].second;
        return Add(k);
    }
    T& GetAdd(const K& k, const T& init) {
        int i = Find(k);
        if(i >= 0) return items[(size_t)i].second;
        return Add(k, init);
    }

    const T& Get(const K& k, const T& def) const {
        int i = Find(k);
        return i >= 0 ? items[(size_t)i].second : def;
    }

    // Remove
    int RemoveKey(const K& k) {
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

    // Iteration over pairs
    auto begin() { return items.begin(); }
    auto end() { return items.end(); }
    auto begin() const { return items.begin(); }
    auto end() const { return items.end(); }
};
