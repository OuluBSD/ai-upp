#include "Core.h"
#include <unordered_map>
#include <memory>
#include <mutex>

NAMESPACE_UPP

template<typename K, typename V>
class ValueCache {
private:
    std::unordered_map<K, std::shared_ptr<V>> cache;
    mutable std::mutex cache_mutex;
    size_t max_size;
    
public:
    explicit ValueCache(size_t max_cache_size = 1000) : max_size(max_cache_size) {}
    
    // Get value from cache, returns nullptr if not found
    std::shared_ptr<V> Get(const K& key) const {
        std::lock_guard<std::mutex> lock(cache_mutex);
        auto it = cache.find(key);
        return (it != cache.end()) ? it->second : nullptr;
    }
    
    // Put value in cache
    void Put(const K& key, const std::shared_ptr<V>& value) {
        std::lock_guard<std::mutex> lock(cache_mutex);
        
        // If cache is at max size, remove first element
        if (cache.size() >= max_size) {
            cache.erase(cache.begin());
        }
        
        cache[key] = value;
    }
    
    // Check if key exists in cache
    bool Has(const K& key) const {
        std::lock_guard<std::mutex> lock(cache_mutex);
        return cache.find(key) != cache.end();
    }
    
    // Remove key from cache
    bool Remove(const K& key) {
        std::lock_guard<std::mutex> lock(cache_mutex);
        return cache.erase(key) > 0;
    }
    
    // Clear cache
    void Clear() {
        std::lock_guard<std::mutex> lock(cache_mutex);
        cache.clear();
    }
    
    // Get current cache size
    size_t Size() const {
        std::lock_guard<std::mutex> lock(cache_mutex);
        return cache.size();
    }
    
    // Get max cache size
    size_t MaxSize() const {
        return max_size;
    }
    
    // Set max cache size
    void SetMaxSize(size_t new_max_size) {
        max_size = new_max_size;
    }
};

// Specific ValueCache implementations
using ValueCacheType = ValueCache<std::string, Value>;

static ValueCacheType global_value_cache(10000); // Default cache with 10000 entries

Value GetCachedValue(const std::string& key) {
    auto ptr = global_value_cache.Get(key);
    return ptr ? *ptr : Value();
}

void SetCachedValue(const std::string& key, const Value& value) {
    auto ptr = std::make_shared<Value>(value);
    global_value_cache.Put(key, ptr);
}

bool HasCachedValue(const std::string& key) {
    return global_value_cache.Has(key);
}

void RemoveCachedValue(const std::string& key) {
    global_value_cache.Remove(key);
}

void ClearValueCache() {
    global_value_cache.Clear();
}

size_t GetValueCacheSize() {
    return global_value_cache.Size();
}

END_UPP_NAMESPACE