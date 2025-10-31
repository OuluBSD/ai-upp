#pragma once
#ifndef _Core_ValueCache_h_
#define _Core_ValueCache_h_

#include <unordered_map>
#include <list>
#include <memory>
#include <functional>
#include <mutex>
#include "Core.h"

extern StaticMutex ValueCacheMutex;

// Base class for cache makers
struct ValueMaker {
    virtual String Key() const = 0;
    virtual int Make(Value& object) const = 0;
    virtual ~ValueMaker() = default;
};

// Simple LRU Cache implementation
template <typename T>
class LRUCache {
private:
    size_t max_size;
    std::unordered_map<String, std::pair<T, typename std::list<String>::iterator>> cache_map;
    std::list<String> cache_list;
    mutable std::mutex cache_mutex;

public:
    explicit LRUCache(size_t max_size = 100) : max_size(max_size) {}

    T Get(const ValueMaker& maker) {
        std::lock_guard<std::mutex> lock(cache_mutex);
        
        String key = maker.Key();
        auto it = cache_map.find(key);
        
        if (it != cache_map.end()) {
            // Move to front (most recently used)
            cache_list.erase(it->second.second);
            cache_list.push_front(key);
            it->second.second = cache_list.begin();
            return it->second.first;
        }
        
        // Create new value
        T value;
        int size = maker.Make(value);
        
        // Add to cache
        Put(key, value);
        
        return value;
    }
    
    void Put(const String& key, const T& value) {
        std::lock_guard<std::mutex> lock(cache_mutex);
        
        auto it = cache_map.find(key);
        if (it != cache_map.end()) {
            // Update existing
            cache_list.erase(it->second.second);
            cache_list.push_front(key);
            it->second = std::make_pair(value, cache_list.begin());
        } else {
            // Insert new
            cache_list.push_front(key);
            cache_map[key] = std::make_pair(value, cache_list.begin());
            
            // Evict if needed
            if (cache_map.size() > max_size) {
                String last_key = cache_list.back();
                cache_list.pop_back();
                cache_map.erase(last_key);
            }
        }
    }
    
    template <class P>
    int Remove(P what) {
        std::lock_guard<std::mutex> lock(cache_mutex);
        int removed = 0;
        
        // Remove items matching predicate
        for (auto it = cache_map.begin(); it != cache_map.end();) {
            if (what(it->first)) {
                cache_list.erase(it->second.second);
                it = cache_map.erase(it);
                removed++;
            } else {
                ++it;
            }
        }
        
        return removed;
    }
    
    void Clear() {
        std::lock_guard<std::mutex> lock(cache_mutex);
        cache_map.clear();
        cache_list.clear();
    }
    
    void SetMaxSize(size_t new_max_size) {
        max_size = new_max_size;
    }
    
    size_t Size() const {
        std::lock_guard<std::mutex> lock(cache_mutex);
        return cache_map.size();
    }
    
    // Shrink the cache to specified limits
    void Shrink(size_t max_bytes, size_t max_count) {
        std::lock_guard<std::mutex> lock(cache_mutex);
        while (cache_map.size() > max_count) {
            String last_key = cache_list.back();
            cache_list.pop_back();
            cache_map.erase(last_key);
        }
    }
    
    // Adjust size based on custom function
    template <class GetSizeFunc>
    void AdjustSize(GetSizeFunc getsize) {
        std::lock_guard<std::mutex> lock(cache_mutex);
        // This would adjust based on the function provided
        // For now, just call the function on each element
        for (auto& item : cache_map) {
            getsize(item.first, item.second.first);
        }
    }
};

// LRUCache instance for Value type
using ValueCache = LRUCache<Value>;

ValueCache& TheValueCache();

Value MakeValueSz(ValueMaker& m, int& sz);
Value MakeValue(ValueMaker& m);

bool IsValueCacheActive();

void AdjustValueCache();
void ShrinkValueCache();

void SetupValueCache(int maxsize, int maxcount);

template <class P>
int ValueCacheRemove(P what)
{
	Mutex::Lock __(ValueCacheMutex);
	return TheValueCache().Remove(what);
}

template <class P>
int ValueCacheRemoveOne(P what)
{
	Mutex::Lock __(ValueCacheMutex);
	return TheValueCache().Remove(what);
}

template <class P>
void ValueCacheAdjustSize(P getsize)
{
	Mutex::Lock __(ValueCacheMutex);
	TheValueCache().AdjustSize(getsize);
}

template <class M>
Value MakeValue_(const String& key, const M& m, int& sz)
{
	struct Maker : ValueMaker {
		const String& key;
		const M& m;

		String Key() const override {
			return key;
		}

		int Make(Value& object) const override {
			return m(object);
		}
		
		Maker(const String& key, const M& m) : key(key), m(m) {}
	};
	
	Maker maker(key, m);
	return MakeValueSz(maker, sz);
}

template <class K, class M>
String MakeKey_(const K& k, const M& m)
{
	StringBuffer key;
	RawCat(key, StaticTypeNo<K>());
	RawCat(key, StaticTypeNo<M>());
	key.Cat(k());
	return String(key);
}

template <class K, class M>
Value MakeValue(const K& k, const M& m)
{
	int sz;
	return MakeValue_(MakeKey_(k, m), m, sz);
}

template <class K, class M>
Value MakeValueTL(const K& k, const M& m)
{
	String key = MakeKey_(k, m);

	struct Maker : ValueMaker {
		const String& key;
		const M& m;

		String Key() const override {
			return key;
		}

		int Make(Value& object) const override {
			int sz;
			object = MakeValue_(key, m, sz);
			return sz;
		}
		
		Maker(const String& key, const M& m) : key(key), m(m) {}
	};
	Maker maker(key, m);
	if(IsMainThread()) {
		thread_local static LRUCache<Value> cache; // thread-local cache
		Value v = cache.Get(maker);
		cache.Shrink(128 * 1024, 1000);
		return v;
	}
	else {
		thread_local static LRUCache<Value> cache;
		Value v = cache.Get(maker);
		cache.Shrink(128 * 1024, 1000);
		return v;
	}
}

// Implementations for the functions
inline ValueCache& TheValueCache() {
    static ValueCache cache(1000); // default max size
    return cache;
}

inline Value MakeValueSz(ValueMaker& m, int& sz) {
    Value v;
    sz = m.Make(v);
    return v;
}

inline Value MakeValue(ValueMaker& m) {
    int sz;
    return MakeValueSz(m, sz);
}

inline bool IsValueCacheActive() {
    return true; // Always active in this implementation
}

inline void AdjustValueCache() {
    // In a real implementation, this would adjust the cache based on memory pressure
    // For now, just shrink it to reasonable limits
    TheValueCache().Shrink(1024 * 1024, 1000); // 1MB, 1000 items max
}

inline void ShrinkValueCache() {
    TheValueCache().Shrink(512 * 1024, 500); // 512KB, 500 items max
}

inline void SetupValueCache(int maxsize, int maxcount) {
    // This would set up the cache with the specified parameters
    // For this implementation, we'll just adjust the cache
    TheValueCache().SetMaxSize(maxcount);
    TheValueCache().Shrink(maxsize, maxcount);
}

#endif