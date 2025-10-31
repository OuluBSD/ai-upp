#pragma once
#ifndef _Core_Index_h_
#define _Core_Index_h_

#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include "Core.h"

struct IndexCommon {
	enum { HIBIT = 0x80000000 };

	struct Hash : Moveable<Hash> {
		int   next; // also link for unlinked items...
		dword hash;
		int   prev;
	};
	
	int         *map;
	Hash        *hash;
	dword        mask;
	int          unlinked;

	
	static int empty[1];

	static dword Smear(hash_t h)          { return FoldHash(h) | HIBIT; }

	void Link(int& m, Hash& hh, int ii);
	void Link(int ii, dword sh);
	void Del(int& m, Hash& hh, int ii);
	void Ins(int& m, Hash& hh, int ii);

	void MakeMap(int count);
	void Remap(int count);
	void Reindex(int count);
	void GrowMap(int count);
	void FreeMap();
	void Free();

	void Set(int ii, dword h);
	
	Vector<int> GetUnlinked() const;
	
	void Clear();
	void Trim(int n, int count);
	void Sweep(int n);
	void Reserve(int n);
	void Shrink();
	void AdjustMap(int count, int alloc);
	
	void Copy(const IndexCommon& b, int count);
	void Pick(IndexCommon& b);
	void Swap(IndexCommon& b);
	
	IndexCommon();
	~IndexCommon();
};

template <class K, class T, class V> class AMap;

template <class T>
class Index : MoveableAndDeepCopyOption<Index<T>>, IndexCommon {
	Vector<T> key;

	static dword Smear(const T& k)   { return IndexCommon::Smear(GetHashValue(k)); }

	int  FindFrom(int i, dword sh, const T& k, int end) const;
	int  FindBack(int i, dword sh, const T& k, int end) const;


	void ReallocHash(int n);
	template <typename U> void GrowAdd(U&& k, dword sh);
	template <typename U> void AddS(int& m, U&& k, dword sh);
	template <typename U> void AddS(U&& k, dword sh);

	template <class OP, class U> int FindAdd(U&& k, OP add_op);
	template <class U> int FindPut0(U&& k, bool& put);

	template <typename U> int Put0(U&& k, dword sh);
	template <typename U> void Set0(int i, U&& k);

	template <typename, typename, typename> friend class AMap;
	
	void        FixHash(bool makemap = true);

public:
	void        Add(const T& k)             { AddS(k, Smear(k)); }
	void        Add(T&& k)                  { AddS(pick(k), Smear(k)); }
	Index&      operator<<(const T& x)      { Add(x); return *this; }
	Index&      operator<<(T&& x)           { Add(pick(x)); return *this; }

	int         Find(const T& k) const;
	int         FindNext(int i) const;
	int         FindLast(const T& k) const;
	int         FindPrev(int i) const;
	
	int         FindAdd(const T& k)         { return FindAdd(k, []{}); }
	int         FindAdd(T&& k)              { return FindAdd(pick(k), []{}); }

	int         Put(const T& k)             { return Put0(k, Smear(k)); }
	int         Put(T&& k)                  { return Put0(pick(k), Smear(k)); }
	int         FindPut(const T& k, bool& p){ return FindPut0(k, p); }
	int         FindPut(T&& k, bool& p)     { return FindPut0(pick(k), p); }
	int         FindPut(const T& k)         { bool p; return FindPut0(k, p); }
	int         FindPut(T&& k)              { bool p; return FindPut0(pick(k), p); }

	void        Unlink(int i);
	int         UnlinkKey(const T& k);
	bool        IsUnlinked(int i) const      { return hash[i].hash == 0; }
	bool        HasUnlinked() const          { return unlinked >= 0; }
	Vector<int> GetUnlinked() const          { return IndexCommon::GetUnlinked(); }

	void        Sweep();

	void        Set(int i, const T& k)       { Set0(i, k); }
	void        Set(int i, T&& k)            { Set0(i, pick(k)); }

	const T&    operator[](int i) const      { return key[i]; }
	int         GetCount() const             { return key.GetCount(); }
	bool        IsEmpty() const              { return key.IsEmpty(); }
	
	void        Clear()                      { key.Clear(); IndexCommon::Clear(); }

	void        Trim(int n = 0)              { IndexCommon::Trim(n, GetCount()); key.Trim(n); }
	void        Drop(int n = 1)              { Trim(GetCount() - 1); }
	const T&    Top() const                  { return key.Top(); }
	T           Pop()                        { T x = pick(Top()); Drop(); return x; }

	void        Reserve(int n);
	void        Shrink();
	int         GetAlloc() const             { return key.GetAlloc(); }

	Vector<T>        PickKeys()              { Vector<T> r = pick(key); Clear(); return r; }
	const Vector<T>& GetKeys() const         { return key; }

	void     Remove(const int *sorted_list, int count);
	void     Remove(const Vector<int>& sorted_list)         { Remove(sorted_list, sorted_list.GetCount()); }
	template <typename Pred> void RemoveIf(Pred p)          { Remove(FindAlli(key, p)); }
	
	Index()                                                 {}
	Index(Index&& s) : key(pick(s.key))                     { IndexCommon::Pick(s); }
	Index(const Index& s, int) : key(s.key, 0)              { ReallocHash(0); IndexCommon::Copy(s, key.GetCount()); } // TODO: Unlinked!
	explicit Index(Vector<T>&& s) : key(pick(s))            { FixHash(); }
	Index(const Vector<T>& s, int) : key(s, 0)              { FixHash(); }

	Index& operator=(Vector<T>&& x)                         { key = pick(x); FixHash(); return *this; }
	Index& operator=(Index<T>&& x)                          { key = pick(x.key); IndexCommon::Pick(x); return *this; }

	Index(std::initializer_list<T> init) : key(init)        { FixHash(); }

	void     Serialize(Stream& s);
	void     Xmlize(XmlIO& xio, const char *itemtag = "key");
	void     Jsonize(JsonIO& jio);
	String   ToString() const;
	template <class B> bool operator==(const B& b) const { return IsEqualRange(*this, b); }
#ifndef CPP_20
	template <class B> bool operator!=(const B& b) const { return !operator==(b); }
#endif
	template <class B> int  Compare(const B& b) const    { return CompareRanges(*this, b); }
	template <class B> bool operator<=(const B& x) const { return Compare(x) <= 0; }
	template <class B> bool operator>=(const B& x) const { return Compare(x) >= 0; }
	template <class B> bool operator<(const B& x) const  { return Compare(x) < 0; }
	template <class B> bool operator>(const B& x) const  { return Compare(x) > 0; }

// Standard container interface
	typedef ConstIteratorOf<Vector<T>> ConstIterator;
	ConstIterator begin() const                             { return key.begin(); }
	ConstIterator end() const                               { return key.end(); }

	friend void Swap(Index& a, Index& b)                    { a.IndexCommon::Swap(b); UPP::Swap(a.key, b.key); }

// deprecated:
#ifdef DEPRECATED
	T&       Insert(int i, const T& k)                      { key.Insert(i, k); FixHash(); return key[i]; }
	void     Remove(int i, int count)                       { key.Remove(i, count); FixHash(); }
	void     Remove(int i)                                  { Remove(i, 1); }
	int      RemoveKey(const T& k)                          { int i = Find(k); if(i >= 0) Remove(i); return i; }

	unsigned GetHash(int i) const                           { return hash[i].hash; }

	Index& operator<<=(const Vector<T>& s)                  { *this = clone(s); return *this; }
	typedef T                ValueType;
	typedef Vector<T>        ValueContainer;
	ConstIterator  GetIter(int pos) const                   { return key.GetIter(pos); }

	void     ClearIndex()                    {}
	void     Reindex(int n)                  {}
	void     Reindex()                       {}

	typedef T             value_type;
	typedef ConstIterator const_iterator;
	typedef const T&      const_reference;
	typedef int           size_type;
	typedef int           difference_type;
	const_iterator        Begin() const          { return begin(); }
	const_iterator        End() const            { return end(); }
	void                  clear()                { Clear(); }
	size_type             size() const           { return GetCount(); }
	bool                  empty() const          { return IsEmpty(); }
#endif

#ifdef _DEBUG
	String Dump() const;
#endif
};

// Implementation of IndexCommon methods
inline
void IndexCommon::Link(int& m, Hash& hh, int ii)
{
	if(m < 0)
		m = hh.prev = hh.next = ii;
	else {
		hh.next = m;
		hh.prev = hash[m].prev;
		hash[hh.prev].next = ii;
		hash[m].prev = ii;
	}
}

inline
void IndexCommon::Link(int ii, dword sh)
{
	Link(map[sh & mask], hash[ii], ii);
}

inline
void IndexCommon::Del(int& m, Hash& hh, int ii)
{ // unlink from m
	if(ii == m) { // this is item pointed by map
		if(hh.next == ii) { // this is the only item in the bucket
			m = -1; // bucket is now empty
			return;
		}
		m = hh.next; // move bucket pointer to the next item
	}
	hash[hh.next].prev = hh.prev; // unlink
	hash[hh.prev].next = hh.next;
}

inline
IndexCommon::IndexCommon()
{
	map = empty;
	hash = NULL;
	mask = 0;
	unlinked = -1;
}

inline
IndexCommon::~IndexCommon()
{
	Free();
}

inline
void IndexCommon::MakeMap(int count)
{
	int n = (count + 1) & ~1;
	if(n < 8)
		n = 8;
	mask = n - 1;
	map = (int *)MemoryAlloc(n * sizeof(int));
	fill(map, map + n, -1);
}

inline
void IndexCommon::Remap(int count)
{
	FreeMap();
	if(count)
		MakeMap(count);
}

inline
void IndexCommon::Reindex(int count)
{
	Remap(count);
	if(hash)
		for(int i = 0; i < count; i++)
			Link(i, hash[i].hash);
}

inline
void IndexCommon::GrowMap(int count)
{
	Remap(count);
}

inline
void IndexCommon::FreeMap()
{
	if(map != empty) {
		MemoryFree(map);
		map = empty;
	}
}

inline
void IndexCommon::Free()
{
	FreeMap();
	MemoryFree(hash);
	hash = NULL;
}

inline
void IndexCommon::Clear()
{
	Free();
	mask = 0;
	unlinked = -1;
}

inline
void IndexCommon::Trim(int n, int count)
{
	for(int i = n; i < count; i++) {
		Hash& h = hash[i];
		Del(map[h.hash & mask], h, i);
	}
}

inline
void IndexCommon::Sweep(int n)
{
	Trim(n, n);
}

inline
void IndexCommon::Reserve(int n)
{
	if(n > (int)mask)
		GrowMap(n);
}

inline
void IndexCommon::Shrink()
{
	Remap(GetCount());
}

inline
void IndexCommon::AdjustMap(int count, int alloc)
{
	if(alloc > (int)mask)
		GrowMap(alloc);
	else
	if(alloc < (int)mask && alloc < count)
		Remap(alloc);
}

inline
void IndexCommon::Copy(const IndexCommon& b, int count)
{
	Free();
	mask = b.mask;
	unlinked = b.unlinked;
	if(b.map == empty)
		map = empty;
	else {
		int n = (int)mask + 1;
		map = (int *)MemoryAlloc(n * sizeof(int));
		memcpy(map, b.map, n * sizeof(int));
	}
	if(b.hash) {
		int n = count ? count : 1;
		hash = (Hash *)MemoryAlloc(n * sizeof(Hash));
		memcpy(hash, b.hash, n * sizeof(Hash));
	}
}

inline
void IndexCommon::Pick(IndexCommon& b)
{
	Free();
	mask = b.mask;
	unlinked = b.unlinked;
	map = b.map;
	hash = b.hash;
	b.map = empty;
	b.hash = NULL;
	b.mask = 0;
	b.unlinked = -1;
}

inline
void IndexCommon::Swap(IndexCommon& b)
{
	UPP::Swap(mask, b.mask);
	UPP::Swap(unlinked, b.unlinked);
	UPP::Swap(map, b.map);
	UPP::Swap(hash, b.hash);
	if(map == b.empty)
		map = empty;
	if(b.map == empty)
		b.map = b.empty;
}

inline
Vector<int> IndexCommon::GetUnlinked() const
{
	Vector<int> r;
	if(unlinked >= 0) {
		int i = unlinked;
		do {
			r.Add(i);
			i = hash[i].next;
		}
		while(i != unlinked);
	}
	return r;
}

// Implementation of Index methods
template <class T>
void Index<T>::ReallocHash(int n)
{ // realloc hash to have the same capacity as key, copy n elements from previous alloc
	if(key.GetAlloc()) {
		size_t sz = key.GetAlloc() * sizeof(Hash);
		if(!MemoryTryRealloc(hash, sz)) {
			Hash *h = (Hash *)MemoryAlloc(sz);
			if(hash) {
				if(n)
					memcpy_t(h, hash, n);
				MemoryFree(hash);
			}
			hash = h;
		}
	}
	else {
		MemoryFree(hash);
		hash = NULL;
	}
}

template <class T>
void Index<T>::FixHash(bool makemap)
{
	ReallocHash(0);
	unlinked = -1;
	for(int i = 0; i < key.GetCount(); i++)
		hash[i].hash = Smear(key[i]);
	if(makemap)
		MakeMap(key.GetCount());
	else
		Remap(key.GetCount());
}

template <class T>
template <typename U>
void Index<T>::GrowAdd(U&& k, dword sh)
{
	int n = key.GetCount();
	key.GrowAdd(std::forward<U>(k));
	ReallocHash(n);
}

template <class T>
template <typename U>
void Index<T>::AddS(int& m, U&& k, dword sh)
{
	int ii = key.GetCount();
	if(ii >= key.GetAlloc())
		GrowAdd(std::forward<U>(k), sh);
	else
		new(key.Rdd()) T(std::forward<U>(k));
	Hash& hh = hash[ii];
	hh.hash = sh;
	if(ii >= (int)mask)
		GrowMap(key.GetCount());
	else
		Link(m, hh, ii);
}

template <class T>
template <typename U>
void Index<T>::AddS(U&& k, dword sh)
{
	AddS(map[sh & mask], std::forward<U>(k), sh);
}

template <class T>
int Index<T>::FindFrom(int i, dword sh, const T& k, int end) const
{
	if(i >= 0)
		do {
			if(key[i] == k)
				return i;
			i = hash[i].next;
		}
		while(i != end);
	return -1;
}

template <class T>
int Index<T>::Find(const T& k) const
{
	dword sh = Smear(k);
	int& m = map[sh & mask];
	return FindFrom(m, sh, k, m);
}

template <class T>
int Index<T>::FindNext(int i) const
{
	const Hash& hh = hash[i];
	int end = map[hash[i].hash & mask];
	return hh.next == end ? -1 : FindFrom(hh.next, hh.hash, key[i], end);
}

template <class T>
int Index<T>::FindBack(int i, dword sh, const T& k, int end) const
{
	do {
		const Hash& ih = hash[i];
		if(key[i] == k)
			return i;
		i = ih.prev;
	}
	while(i != end);
	return -1;
}

template <class T>
int Index<T>::FindLast(const T& k) const
{
	dword sh = Smear(k);
	int& m = map[sh & mask];
	return m < 0 ? -1 : FindBack(hash[m].prev, sh, k, hash[m].prev);
}

template <class T>
int Index<T>::FindPrev(int i) const
{
	const Hash& hh = hash[i];
	int end = map[hash[i].hash & mask];
	return hh.prev == hash[end].prev ? -1 : FindBack(hh.prev, hh.hash, key[i], hash[end].prev);
}

template <class T>
template <class OP, class U>
int Index<T>::FindAdd(U&& k, OP op) {
	dword sh = Smear(k);
	int& m = map[sh & mask];
	int i = m;
	if(i >= 0)
		do {
			if(key[i] == k)
				return i;
			i = hash[i].next;
		}
		while(i != m);
	i = key.GetCount();
	AddS(m, std::forward<U>(k), sh);
	op();
	return i;
}

template <class T>
void Index<T>::Unlink(int ii)
{
	Hash& hh = hash[ii];
	Del(map[hh.hash & mask], hh, ii);
	Link(unlinked, hh, ii);
	hh.hash = 0;
}

template <class T>
int Index<T>::UnlinkKey(const T& k)
{
	dword sh = Smear(k);
	int& m = map[sh & mask];
	int i = m;
	int n = 0;
	if(i >= 0)
		for(;;) {
			Hash& hh = hash[i];
			int ni = hh.next;
			if(key[i] == k) {
				Del(m, hh, i);
				Link(unlinked, hh, i);
				n++;
				hh.hash = 0;
				if(ni == i) // last item removed
					break;
				i = ni;
			}
			else {
				i = ni;
				if(i == m)
					break;
			}
		}
	return n;
}

template <class T>
template <typename U>
int Index<T>::Put0(U&& k, dword sh)
{
	int i;
	if(HasUnlinked()) {
		i = hash[unlinked].prev;
		Hash& hh = hash[i];
		Del(unlinked, hh, i);
		Link(map[sh & mask], hh, i);
		hh.hash = sh;
		key[i] = std::forward<U>(k);
	}
	else {
		i = GetCount();
		AddS(std::forward<U>(k), sh);
	}
	return i;
}

template <class T>
template <class U>
int Index<T>::FindPut0(U&& k, bool& put)
{
	dword sh = Smear(k);
	int& m = map[sh & mask];
	int i = m;
	put = false;
	if(i >= 0)
		do {
			if(key[i] == k)
				return i;
			i = hash[i].next;
		}
		while(i != m);
	put = true;
	return Put0(std::forward<U>(k), sh);
}

template <class T>
template <typename U>
void Index<T>::Set0(int ii, U&& k)
{
	Hash& hh = hash[ii];
	if(IsUnlinked(ii))
		Del(unlinked, hh, ii);
	else
		Del(map[hh.hash & mask], hh, ii);
	
	dword sh = Smear(k);
	hh.hash = sh;
	Link(map[sh & mask], hh, ii);
	key[ii] = std::forward<U>(k);
}

template <class T>
void Index<T>::Sweep()
{
	if(unlinked >= 0) {
		int n = key.GetCount();
		key.RemoveIf([&](int i) { return hash[i].hash == 0; });
		IndexCommon::Sweep(n);
	}
}

template <class T>
void Index<T>::Reserve(int n)
{
	int a = key.GetAlloc();
	key.Reserve(n);
	if(a != key.GetAlloc()) {
		ReallocHash(key.GetCount());
		AdjustMap(key.GetCount(), n);
	}
}

template <class T>
void Index<T>::Shrink()
{
	int a = key.GetAlloc();
	key.Shrink();
	if(a != key.GetAlloc()) {
		ReallocHash(key.GetCount());
		AdjustMap(key.GetCount(), key.GetCount());
	}
}

template <class T>
void Index<T>::Remove(const int *sorted_list, int count)
{
	if(HasUnlinked()) {
		Vector<bool> u;
		u.SetCount(GetCount());
		for(int i = 0; i < GetCount(); i++)
			u[i] = IsUnlinked(i);
		key.Remove(sorted_list, count);
		u.Remove(sorted_list, count);
		FixHash(false);
		for(int i = 0; i < GetCount(); i++)
			if(u[i])
				Unlink(i);
	}
	else {
		key.Remove(sorted_list, count);
		FixHash(false);
	}
}

template <class T>
void Index<T>::Serialize(Stream& s) {
	key.Serialize(s);
	if(s.IsLoading())
		FixHash();

	int version = 1;
	s / version;
	if(version == 0) { // support previous version
		Vector<unsigned> h;
		h.Serialize(s);
		if(s.IsLoading())
			for(int i = 0; i < h.GetCount(); i++)
				if(i < GetCount() && h[i] & 0x80000000)
					Unlink(i);
	}
	else {
		Vector<int> u = GetUnlinked();
		u.Serialize(s);
		if(s.IsLoading())
			for(int i : ReverseRange(u)) { // Reverse range to ensure the correct order of Put
				if(i >= 0 && i < GetCount())
					Unlink(i);
				else
					s.LoadError();
			}
	}
}

template <class T>
void Index<T>::Xmlize(XmlIO& xio, const char *itemtag)
{
	XmlizeIndex<T, Index<T> >(xio, itemtag, *this);
}

template <class T>
void Index<T>::Jsonize(JsonIO& jio)
{
	JsonizeIndex<Index<T>, T>(jio, *this);
}

template <class T>
String Index<T>::ToString() const
{
	return AsStringArray(*this);
}

#ifdef _DEBUG
template <class T>
String Index<T>::Dump() const
{
	String h;
	for(int i = 0; i < key.GetCount(); i++) {
		if(i)
			h << "; ";
		if(IsUnlinked(i))
			h << "#";
		h << i << ": " << key[i] << '/' << (hash[i].hash & mask) << " -> " << hash[i].prev << ":" << hash[i].next;
	}
	return h;
}
#endif

#endif
