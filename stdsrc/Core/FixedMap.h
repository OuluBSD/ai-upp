#pragma once
#ifndef _Core_FixedMap_h_
#define _Core_FixedMap_h_

#include "Core.h"
#include <vector>
#include <algorithm>
#include <functional>

template <class K, class T, class V, class Less>
class FixedAMap {
protected:
	std::vector<K> key;
	V value;

public:
	T&       Add(const K& k, const T& x)       { key.push_back(k); return value.Add(x); }
	T&       AddPick(const K& k, T&& x)        { key.push_back(k); return value.AddPick(std::move(x)); }
	T&       Add(const K& k)                   { key.push_back(k); return value.Add(); }
	
	void     Finish()                          { 
		IndexSort(key, value, Less()); 
		Shrink(); 
	}

	int      Find(const K& k) const            { 
		auto it = std::lower_bound(key.begin(), key.end(), k, Less());
		if (it != key.end() && !Less()(*it, k) && !Less()(k, *it)) {
			return static_cast<int>(it - key.begin());
		}
		return -1; 
	}
	int      FindNext(int i) const             { 
		return i + 1 < static_cast<int>(key.size()) && key[i] == key[i + 1] ? i + 1 : -1; 
	}

	T&       Get(const K& k)                   { return value[Find(k)]; }
	const T& Get(const K& k) const             { return value[Find(k)]; }
	const T& Get(const K& k, const T& d) const { int i = Find(k); return i >= 0 ? value[i] : d; }

	T       *FindPtr(const K& k)               { int i = Find(k); return i >= 0 ? &value[i] : nullptr; }
	const T *FindPtr(const K& k) const         { int i = Find(k); return i >= 0 ? &value[i] : nullptr; }

	const T& operator[](int i) const           { return value[i]; }
	T&       operator[](int i)                 { return value[i]; }
	int      GetCount() const                  { return static_cast<int>(value.GetCount()); }
	bool     IsEmpty() const                   { return value.IsEmpty(); }
	void     Clear()                           { key.clear(); value.Clear(); }
	void     Shrink()                          { value.Shrink(); key.shrink_to_fit(); }
	void     Reserve(int xtra)                 { value.Reserve(xtra); key.reserve(key.size() + xtra); }
	int      GetAlloc() const                  { return static_cast<int>(value.GetAlloc()); }

	const K& GetKey(int i) const               { return key[i]; }

#ifdef UPP
	void     Serialize(Stream& s);
	void     Xmlize(XmlIO& xio);
	void     Jsonize(JsonIO& jio);
	String   ToString() const;
#endif

	void     Swap(FixedAMap& x)                { std::swap(value, x.value); std::swap(key, x.key); }

	const std::vector<K>& GetKeys() const      { return key; }
	std::vector<K>        PickKeys()           { 
		std::vector<K> result = std::move(key);
		key.clear();
		return result;
	}

	const V&         GetValues() const         { return value; }
	V&               GetValues()               { return value; }
	V                PickValues()              { return pick(value); }
	
	FixedAMap& operator()(const K& k, const T& v)       { Add(k, v); return *this; }

	FixedAMap()                                         {}
	FixedAMap(const FixedAMap& s, int) : key(s.key), value(s.value, 0) {}
	FixedAMap(std::vector<K>&& key_, V&& val) : key(std::move(key_)), value(std::move(val)) {}

	typedef typename V::const_iterator  ConstIterator;
	typedef typename V::iterator        Iterator;

	Iterator         begin()                             { return value.begin(); }
	Iterator         end()                               { return value.end(); }
	ConstIterator    begin() const                       { return value.begin(); }
	ConstIterator    end() const                         { return value.end(); }

#ifdef DEPRECATED
	typedef V                          ValueContainer;
	typedef T                          ValueType;

	typedef std::vector<K> KeyContainer;
	typedef K         KeyType;
	typedef typename std::vector<K>::const_iterator KeyConstIterator;

	KeyConstIterator KeyBegin() const                    { return key.begin(); }
	KeyConstIterator KeyEnd() const                      { return key.end(); }
	KeyConstIterator KeyGetIter(int pos) const           { 
		return key.begin() + pos; 
	}

	Iterator         GetIter(int pos)                    { 
		return value.begin() + pos; 
	}
	ConstIterator    GetIter(int pos) const              { 
		return value.begin() + pos; 
	}
#endif
};

template <class K, class T, class Less = std::less<K> >
class FixedVectorMap : public MoveableAndDeepCopyOption<FixedVectorMap<K, T, Less> >,
                       public FixedAMap< K, T, Vector<T>, Less > {
    typedef FixedAMap< K, T, Vector<T>, Less > B;
public:
	FixedVectorMap(const FixedVectorMap& s, int) : FixedAMap<K, T, Vector<T>, Less>(s, 1) {}
	FixedVectorMap(std::vector<K>&& key, std::vector<T>&& val) : FixedAMap<K, T, Vector<T>, Less>(std::move(key), Vector<T>(val)) {}
	FixedVectorMap()                                                       {}

	friend void    Swap(FixedVectorMap& a, FixedVectorMap& b)      { a.B::Swap(b); }

	typedef typename FixedAMap< K, T, Vector<T>, Less >::ConstIterator ConstIterator; 
	typedef typename FixedAMap< K, T, Vector<T>, Less >::Iterator      Iterator; 
	STL_MAP_COMPATIBILITY(FixedVectorMap<K, T, Less>)
};

template <class K, class T, class Less = std::less<K> >
class FixedArrayMap : public MoveableAndDeepCopyOption< FixedArrayMap<K, T, Less> >,
                      public FixedAMap< K, T, Array<T>, Less > {
	typedef FixedAMap< K, T, Array<T>, Less > B;
public:
	T&        Add(const K& k, const T& x)          { return B::Add(k, x); }
	T&        Add(const K& k)                      { return B::Add(k); }
	T&        Add(const K& k, T *newt)             { B::key.push_back(k); return B::value.Add(newt); }
	template <class TT, class... Args>
	TT&       Create(const K& k, Args&&... args)   { 
		TT *q = new TT(std::forward<Args>(args)...); 
		B::key.push_back(k); 
		return static_cast<TT&>(B::value.Add(q)); 
	}

	FixedArrayMap(const FixedArrayMap& s, int) : FixedAMap<K, T, Array<T>, Less>(s, 1) {}
	FixedArrayMap(std::vector<K>&& ndx, Array<T>&& val) : FixedAMap<K, T, Array<T>, Less>(std::move(ndx), std::move(val)) {}
	FixedArrayMap() {}

	friend void    Swap(FixedArrayMap& a, FixedArrayMap& b)        { a.B::Swap(b); }

	typedef typename FixedAMap< K, T, Array<T>, Less >::ConstIterator ConstIterator; 
	typedef typename FixedAMap< K, T, Array<T>, Less >::Iterator      Iterator; 
	STL_MAP_COMPATIBILITY(FixedArrayMap<K, T, Less>)
};

#endif