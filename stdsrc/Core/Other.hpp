#pragma once
#ifndef _Core_Other_hpp_
#define _Core_Other_hpp_

#include <functional>
#include <vector>
#include "Core.h"

template <class F>
bool PackedData::GetData(int ii, F out) const
{
	int i = 0;
	const byte *s = (const byte *)ptr;
	if(s)
		for(;;) {
			int len = *s++;
			if(len == 255)
				break;
			if(len == 254) {
				memcpy(&len, s, 4);
				s += 4;
			}
			if(i == ii) {
				out((const char *)s, len);
				return true;
			}
			s += len;
			i++;
		}
	out("", 0);
	return false;
}

template <class T>
T PackedData::Get(int ii, T def) const
{
	T q = def;
	GetData(ii, [&](const char *ptr, int len) {
		if(len) {
			ASSERT(len == sizeof(T));
			memcpy(&q, ptr, sizeof(T));
		}
	});
	return q;
}

template <class T, class K>
void LRUCache<T, K>::LinkHead(int i)
{
	Item& m = data[i];
	if(head >= 0) {
		int tail = data[head].prev;
		m.next = head;
		m.prev = tail;
		data[head].prev = i;
		data[tail].next = i;
	}
	else
		m.prev = m.next = i;
	head = i;
	count++;
}


template <class T, class K>
void LRUCache<T, K>::Unlink(int i)
{
	Item& m = data[i];
	if(m.prev == i)
		head = -1;
	else {
		if(head == i)
			head = m.next;
		data[m.next].prev = m.prev;
		data[m.prev].next = m.next;
	}
	count--;
}

template <class T, class K>
T& LRUCache<T, K>::GetLRU()
{
	int tail = data[head].prev;
	return *data[tail].data;
}

template <class T, class K>
const K& LRUCache<T, K>::GetLRUKey()
{
	int tail = data[head].prev;
	return key[tail].key;
}

template <class T, class K>
void LRUCache<T, K>::DropLRU()
{
	if(head >= 0) {
		int tail = data[head].prev;
		size -= data[tail].size;
		data[tail].data.Clear();
		Unlink(tail);
		key.Unlink(tail);
	}
}

template <class T, class K>
template <class P>
void LRUCache<T, K>::AdjustSize(P getsize)
{
	size = 0;
	count = 0;
	for(int i = 0; i < data.GetCount(); i++)
		if(!key.IsUnlinked(i)) {
			int sz = getsize(*data[i].data);
			if(sz >= 0)
				data[i].size = sz + InternalSize;
			size += data[i].size;
			count++;
		}
}

template <class T, class K>
template <class P>
int LRUCache<T, K>::Remove(P predicate)
{
	int n = 0;
	int i = 0;
	while(i < data.GetCount())
		if(!key.IsUnlinked(i) && predicate(*data[i].data)) {
			size -= data[i].size;
			Unlink(i);
			key.Unlink(i);
			n++;
		}
		else
			i++;
	return n;
}

template <class T, class K>
template <class P>
bool LRUCache<T, K>::RemoveOne(P predicate)
{
	int i = head;
	if(i >= 0)
		for(;;) {
			int next = data[i].next;
			if(predicate(*data[i].data)) {
				size -= data[i].size;
				Unlink(i);
				key.Unlink(i);
				return true;
			}
			if(i == next || next == head || next < 0)
				break;
			i = next;
		}
	return false;
}

template <class T, class K>
void LRUCache<T, K>::Shrink(int maxsize, int maxcount)
{
	if(maxsize >= 0 && maxcount >= 0)
		while(count > maxcount || size > maxsize)
			DropLRU();
}

template <class T, class K>
void LRUCache<T, K>::Clear()
{
	head = -1;
	size = 0;
	count = 0;
	newsize = foundsize = 0;
	key.Clear();
	data.Clear();
}

template <class T, class K>
void LRUCache<T, K>::ClearCounters()
{
	flag = !flag;
	newsize = foundsize = 0;
}

template <class T, class K>
template <class B, class A>
T& LRUCache<T, K>::Get(const Maker& m, B before_make, A after_make, int& sz)
{
	Key k;
	k.key = m.Key();
	k.type = typeid(m).name();
	int q = key.Find(k);
	if(q < 0) {
		One<T> val;
		before_make();
		int sz = m.Make(val.Create());
		after_make();
		q = key.Put(k);
		Item& t = data.At(q);
		t.data = pick(val);
		t.size = sz + InternalSize;
		size += t.size;
		newsize += t.size;
		t.flag = flag;
		sz = t.size;
	}
	else {
		Item& t = data[q];
		sz = t.size;
		Unlink(q);
		if(t.flag != flag) {
			t.flag = flag;
			foundsize += t.size;
		}
	}
	LinkHead(q);
	return *data[q].data;
}

template <class T, class K>
T& LRUCache<T, K>::Get(const Maker& m, int& sz)
{
	return Get(m, []{}, []{}, sz);
}

template <class T, class K>
T& LRUCache<T, K>::Get(const Maker& m)
{
	int sz;
	return Get(m, sz);
}

template <class T, class K>
template <class B, class A>
T& LRUCache<T, K>::Get(const K& k, B before_make, A after_make)
{
	Key mk;
	mk.key = k;
	mk.type = typeid(K).name();
	int q = key.Find(mk);
	if(q < 0) {
		One<T> val;
		before_make();
		val.Create();
		after_make();
		q = key.Put(mk);
		Item& t = data.At(q);
		t.data = pick(val);
		t.size = sizeof(T) + InternalSize;
		size += t.size;
		newsize += t.size;
		t.flag = flag;
	}
	else {
		Item& t = data[q];
		Unlink(q);
		if(t.flag != flag) {
			t.flag = flag;
			foundsize += t.size;
		}
	}
	LinkHead(q);
	return *data[q].data;
}

template <class T, class K>
T& LRUCache<T, K>::Get(const K& k)
{
	return Get(k, []{}, []{});
}

template <class T, class K>
int LRUCache<T, K>::Find(const K& k) const
{
	Key mk;
	mk.key = k;
	mk.type = typeid(K).name();
	return key.Find(mk);
}

template <class T, class K>
bool LRUCache<T, K>::Has(const K& k) const
{
	return Find(k) >= 0;
}

template <class T, class K>
void LRUCache<T, K>::Remove(const K& k)
{
	int q = Find(k);
	if(q >= 0) {
		size -= data[q].size;
		Unlink(q);
		key.Unlink(q);
	}
}

template <class T, class K>
T& LRUCache<T, K>::operator[](const K& k)
{
	return Get(k);
}

template <class T, class K>
const T& LRUCache<T, K>::operator[](const K& k) const
{
	int q = Find(k);
	ASSERT(q >= 0);
	return *data[q].data;
}

template <class T, class K>
int LRUCache<T, K>::GetCount() const
{
	return count;
}

template <class T, class K>
bool LRUCache<T, K>::IsEmpty() const
{
	return count == 0;
}

template <class T, class K>
int LRUCache<T, K>::GetSize() const
{
	return size;
}

template <class T, class K>
void LRUCache<T, K>::Reserve(int n)
{
	key.Reserve(n);
	data.Reserve(n);
}

template <class T, class K>
void LRUCache<T, K>::Shrink()
{
	key.Shrink();
	data.Shrink();
}

template <class T, class K>
void LRUCache<T, K>::Swap(LRUCache& other)
{
	UPP::Swap(head, other.head);
	UPP::Swap(size, other.size);
	UPP::Swap(count, other.count);
	UPP::Swap(newsize, other.newsize);
	UPP::Swap(foundsize, other.foundsize);
	UPP::Swap(flag, other.flag);
	UPP::Swap(key, other.key);
	UPP::Swap(data, other.data);
}

template <class T, class K>
template <class Stream>
void LRUCache<T, K>::Serialize(Stream& s)
{
	int version = 0;
	s / version % key % data % size % count % newsize % foundsize % flag;
	if(s.IsLoading()) {
		head = -1;
		for(int i = 0; i < data.GetCount(); i++)
			if(!key.IsUnlinked(i))
				LinkHead(i);
	}
}

template <class T, class K>
template <class Stream>
void operator%(Stream& s, LRUCache<T, K>& cache)
{
	cache.Serialize(s);
}

template <class T, class K>
String LRUCache<T, K>::ToString() const
{
	return "LRUCache(count=" + AsString(count) + ", size=" + AsString(size) + ")";
}

template <class T, class K>
template <class B, class A>
void LRUCache<T, K>::Xmlize(XmlIO& xio, const char *keytag, const char *valuetag, B before_make, A after_make)
{
	if(xio.IsStoring()) {
		for(int i = 0; i < data.GetCount(); i++)
			if(!key.IsUnlinked(i)) {
				XmlIO k = xio.Add(keytag);
				XmlizeStore(k, key.GetKey(i));
				XmlIO v = xio.Add(valuetag);
				XmlizeStore(v, *data[i].data);
			}
	}
	else {
		Clear();
		int i = 0;
		while(i < xio->GetCount() - 1 && xio->Node(i).IsTag(keytag) && xio->Node(i + 1).IsTag(valuetag)) {
			K kkey;
			XmlIO k = xio.At(i++);
			Xmlize(k, kkey);
			T t;
			XmlIO v = xio.At(i++);
			Xmlize(v, t);
			Put(kkey, pick(t), before_make, after_make);
		}
	}
}

template <class T, class K>
void LRUCache<T, K>::Xmlize(XmlIO& xio, const char *keytag, const char *valuetag)
{
	Xmlize(xio, keytag, valuetag, []{}, []{});
}

template <class T, class K>
template <class B, class A>
void LRUCache<T, K>::Jsonize(JsonIO& jio, const char *keytag, const char *valuetag, B before_make, A after_make)
{
	if(jio.IsStoring()) {
		for(int i = 0; i < data.GetCount(); i++)
			if(!key.IsUnlinked(i)) {
				JsonIO k = jio.Add(keytag);
				JsonizeStore(k, key.GetKey(i));
				JsonIO v = jio.Add(valuetag);
				JsonizeStore(v, *data[i].data);
			}
	}
	else {
		Clear();
		ValueArray va = jio.GetArray();
		for(int i = 0; i < va.GetCount(); i += 2) {
			K kkey;
			JsonIO k(jio, va[i]);
			Jsonize(k, kkey);
			T t;
			JsonIO v(jio, va[i + 1]);
			Jsonize(v, t);
			Put(kkey, pick(t), before_make, after_make);
		}
	}
}

template <class T, class K>
void LRUCache<T, K>::Jsonize(JsonIO& jio, const char *keytag, const char *valuetag)
{
	Jsonize(jio, keytag, valuetag, []{}, []{});
}

template <class T, class K>
template <class B, class A>
T& LRUCache<T, K>::Put(const K& k, T&& value, B before_make, A after_make)
{
	Key mk;
	mk.key = k;
	mk.type = typeid(K).name();
	int q = key.Find(mk);
	if(q < 0) {
		One<T> val;
		val = pick(value);
		before_make();
		after_make();
		q = key.Put(mk);
		Item& t = data.At(q);
		t.data = pick(val);
		t.size = sizeof(T) + InternalSize;
		size += t.size;
		newsize += t.size;
		t.flag = flag;
	}
	else {
		Item& t = data[q];
		size -= t.size;
		t.data = pick(value);
		t.size = sizeof(T) + InternalSize;
		size += t.size;
		Unlink(q);
		if(t.flag != flag) {
			t.flag = flag;
			foundsize += t.size;
		}
	}
	LinkHead(q);
	return *data[q].data;
}

template <class T, class K>
T& LRUCache<T, K>::Put(const K& k, T&& value)
{
	return Put(k, pick(value), []{}, []{});
}

template <class T, class K>
template <class B, class A>
T& LRUCache<T, K>::Put(const K& k, const T& value, B before_make, A after_make)
{
	return Put(k, clone(value), before_make, after_make);
}

template <class T, class K>
T& LRUCache<T, K>::Put(const K& k, const T& value)
{
	return Put(k, clone(value), []{}, []{});
}

template <class T, class K>
void LRUCache<T, K>::SetMaxSize(int maxsize)
{
	max_size = maxsize;
	if(maxsize >= 0)
		while(size > maxsize && count > 0)
			DropLRU();
}

template <class T, class K>
void LRUCache<T, K>::SetMaxCount(int maxcount)
{
	max_count = maxcount;
	if(maxcount >= 0)
		while(count > maxcount && count > 0)
			DropLRU();
}

template <class T, class K>
int LRUCache<T, K>::GetMaxSize() const
{
	return max_size;
}

template <class T, class K>
int LRUCache<T, K>::GetMaxCount() const
{
	return max_count;
}

template <class T, class K>
void LRUCache<T, K>::Touch(const K& k)
{
	int q = Find(k);
	if(q >= 0) {
		Unlink(q);
		LinkHead(q);
	}
}

template <class T, class K>
const T& LRUCache<T, K>::Peek(const K& k) const
{
	int q = Find(k);
	ASSERT(q >= 0);
	return *data[q].data;
}

template <class T, class K>
bool LRUCache<T, K>::Peek(const K& k, T& value) const
{
	int q = Find(k);
	if(q >= 0) {
		value = *data[q].data;
		return true;
	}
	return false;
}

template <class T, class K>
Vector<K> LRUCache<T, K>::GetKeys() const
{
	Vector<K> keys;
	keys.SetCount(count);
	int i = 0;
	for(int j = 0; j < key.GetCount(); j++)
		if(!key.IsUnlinked(j))
			keys[i++] = key.GetKey(j);
	return keys;
}

template <class T, class K>
Vector<T> LRUCache<T, K>::GetValues() const
{
	Vector<T> values;
	values.SetCount(count);
	int i = 0;
	for(int j = 0; j < data.GetCount(); j++)
		if(!key.IsUnlinked(j))
			values[i++] = *data[j].data;
	return values;
}

template <class T, class K>
void LRUCache<T, K>::Purge()
{
	Clear();
}

template <class T, class K>
bool LRUCache<T, K>::IsFull() const
{
	return (max_size >= 0 && size >= max_size) || (max_count >= 0 && count >= max_count);
}

template <class T, class K>
double LRUCache<T, K>::GetHitRate() const
{
	if(newsize + foundsize == 0)
		return 0.0;
	return (double)foundsize / (newsize + foundsize);
}

template <class T, class K>
void LRUCache<T, K>::ResetStats()
{
	newsize = foundsize = 0;
}

template <class T, class K>
int LRUCache<T, K>::GetNewCount() const
{
	return newsize;
}

template <class T, class K>
int LRUCache<T, K>::GetFoundCount() const
{
	return foundsize;
}

template <class T, class K>
void LRUCache<T, K>::SetInternalSize(int sz)
{
	InternalSize = sz;
}

template <class T, class K>
int LRUCache<T, K>::GetInternalSize() const
{
	return InternalSize;
}

#endif