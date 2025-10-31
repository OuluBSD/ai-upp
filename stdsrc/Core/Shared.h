#pragma once
#ifndef _Core_Shared_h_
#define _Core_Shared_h_

#include <memory>
#include <atomic>
#include <mutex>
#include <vector>
#include <utility>
#include "Core.h"

struct WeakBase {
	virtual void SetDeleted() = 0;
};

template <class T> class Weak;

struct RefBase {
	void* obj = nullptr;
	std::vector<WeakBase*> weaks;
	std::atomic<int> refs{1};
	
	virtual ~RefBase() {ASSERT(!obj);} // deleting obj requires known type, use RefTemplate
	virtual void Clear() = 0;
	virtual void Delete() = 0;
	void Inc() {refs.fetch_add(1);}
	void Dec() {
		int old_refs = refs.fetch_sub(1);
		if (old_refs <= 1) {
			for(WeakBase* w : weaks) {
				w->SetDeleted();
			}
			Clear();
			Delete();
		}
	}
	void IncWeak(WeakBase* w) {
		std::lock_guard<std::mutex> lock(weaks_mutex);
		weaks.push_back(w);
	}
	void DecWeak(WeakBase* w) {
		std::lock_guard<std::mutex> lock(weaks_mutex);
		for(auto it = weaks.begin(); it != weaks.end(); ++it) {
			if (*it == w) {
				weaks.erase(it);
				break;
			}
		}
	}
	int GetRefCount() const {return refs.load();}
	
protected:
	std::mutex weaks_mutex;
	RefBase() = default;
};

template <class T>
struct RefTemplate : RefBase {
	~RefTemplate() {Clear();}
	void Clear() override {
		if (!obj) return;
		delete static_cast<T*>(obj);
		obj = nullptr;
	}
	void Delete() override {
		delete this;
	}
};

template <class T, class Base=RefTemplate<T>>
class Shared : Moveable<Shared<T>> {
	
protected:
	friend class Weak<T>;
	RefBase* r = nullptr;
	T* o = nullptr;

public:
	using S = Shared<T,Base>;
	using Class = T;
	template <class K> using SharedT = Shared<K,Base>;
	
	Shared() = default;
	Shared(Shared&& s) noexcept : r(s.r), o(s.o) {s.r = nullptr; s.o = nullptr;}
	Shared(const Shared& s) : r(s.r), o(s.o) {
		if (r) r->Inc();
	}
	Shared(T* o, RefBase* r) : o(o), r(r) {
		ASSERT(r->GetRefCount() > 0);
	}
	explicit Shared(T* o) : o(o) {
		Base* r = new Base;
		r->obj = o;
		ASSERT(r->GetRefCount() > 0);
		this->r = r;
	}
	Shared(const Nuller&) {}
	~Shared() { Clear(); }
	
	template <class C> C& Create() {
		static_assert(std::is_base_of_v<T,C>, "Class is not the base of T");
		Clear(); 
		auto* new_r = new RefTemplate<C>();
		C* c = new C();
		o = c; 
		r = new_r;
		r->obj = o;
		return *c;
	}
	void Create() { 
		Clear(); 
		auto* new_r = new RefTemplate<T>();
		o = new T(); 
		r = new_r;
		r->obj = o;
	}
	template<class K> void CreateAbstract() { 
		Clear(); 
		auto* new_r = new RefTemplate<T>();
		o = new K(); 
		r = new_r;
		r->obj = o;
	}
	void Clear() { 
		if (r) { 
			r->Dec(); 
			r = nullptr; 
			o = nullptr;
		} 
	}
	void operator=(const Shared& s) {
		if (r == s.r) return; 
		Shared tmp;
		Swap(*this, tmp); // don't unref until new ref
		o = s.o;
		r = s.r;
		if (r) r->Inc();
	}
	void operator=(Shared&& s) {
		if (this != &s) {
			Clear();
			r = s.r;
			s.r = nullptr;
			o = s.o;
			s.o = nullptr;
		}
	}
	
	bool IsEmpty() const { return r == nullptr; }
	int GetRefCount() const {return r ? r->GetRefCount() : 0;}
	T* operator->() {return o;}
	const T* operator->() const {return o;}
	T* Get() const {return o;}
	operator bool() const {return !IsEmpty();}
	T& operator*() const {return *Get();}
	bool operator==(const T* ptr) const {return o == ptr;}
	bool operator!=(const T* ptr) const {return o != ptr;}
	bool operator==(const Shared& s) const {return o == s.o;}
	bool operator!=(const Shared& s) const {return o != s.o;}
	template <class K> SharedT<K> As() {
		static_assert(std::is_base_of_v<T, K> || std::is_base_of_v<K, T>, "K -> T or T -> K inheritance is required");
		
		if (o) {
			K* ptr = dynamic_cast<K*>(o);
			if (ptr) {
				SharedT<K> s;
				s.SetPtr(ptr, r);
				return s;
			}
		}
		return SharedT<K>();
	}
	void SetPtr(T* o, RefBase* r) {
		Shared tmp;
		Swap(*this, tmp); // don't unref until new ref
		this->o = o;
		this->r = r;
		if (r) r->Inc();
	}
	Shared& WrapObject(T* obj) {
		Clear();
		if (obj) {
			r = new RefTemplate<T>();
			r->obj = obj;
			o = obj;
		}
		return *this;
	}
	const Base* GetBase() const {return static_cast<const Base*>(r);}
	String ToString() const {return r ? o->ToString() : "";}
	
	// std::shared_ptr interface compatibility methods
	template <class Y>
	explicit Shared(Y* ptr) : o(ptr) {
		Base* r = new Base;
		r->obj = ptr;
		ASSERT(r->GetRefCount() > 0);
		this->r = r;
	}
	
	void reset() { Clear(); }
	
	template <class Y>
	void reset(Y* ptr) {
		Clear();
		if (ptr) {
			Base* r = new Base;
			r->obj = ptr;
			o = ptr;
			ASSERT(r->GetRefCount() > 0);
		}
	}
	
	// Note: Full deleter support is not implemented in this U++ Shared<T> implementation
	// The standard reset(Y* ptr, Deleter d) method is only provided for interface compatibility
	template <class Y, class Deleter>
	void reset(Y* ptr, Deleter d) {
		// For full compatibility with std::shared_ptr, this should use the deleter
		// but the current U++ implementation doesn't support custom deleters
		Clear();
		if (ptr) {
			// Using default behavior - the U++ framework handles deletion through RefTemplate
			Base* r = new Base;
			r->obj = ptr;
			o = ptr;
			ASSERT(r->GetRefCount() > 0);
		}
	}

	long use_count() const { return r ? r->GetRefCount() : 0; }
	
	bool unique() const { return use_count() == 1; }
	
	T* get() const { return Get(); }
	
	void swap(Shared& other) {
		std::swap(r, other.r);
		std::swap(o, other.o);
	}
	
	template <class U>
	Shared static_pointer_cast(const Shared<U>& other) {
		T* ptr = static_cast<T*>(other.Get());
		if (ptr) {
			return Shared(ptr, other.r);
		}
		return Shared();
	}
	
	template <class U>
	Shared dynamic_pointer_cast(const Shared<U>& other) {
		T* ptr = dynamic_cast<T*>(other.Get());
		if (ptr) {
			return Shared(ptr, other.r);
		}
		return Shared();
	}
};

template <class T> inline Shared<T> MakeShared() {return Shared<T>(new T());}
template <class T, class P> inline Shared<T> MakeSharedBase() {Shared<P> s; s.Create(); return s.template As<T>();}

template <class T, class A0> inline Shared<T> MakeShared(A0 a0) {return Shared<T>(new T(a0));}
template <class T, class A0, class A1> inline Shared<T> MakeShared(A0 a0, A1 a1) {return Shared<T>(new T(a0, a1));}
template <class T, class A0, class A1, class A2> inline Shared<T> MakeShared(A0 a0, A1 a1, A2 a2) {return Shared<T>(new T(a0, a1, a2));}
template <class T, class A0, class A1, class A2, class A3> inline Shared<T> MakeShared(A0 a0, A1 a1, A2 a2, A3 a3) {return Shared<T>(new T(a0, a1, a2, a3));}
template <class T, class A0, class A1, class A2, class A3, class A4> inline Shared<T> MakeShared(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) {return Shared<T>(new T(a0, a1, a2, a3, a4));}
template <class T, class A0, class A1, class A2, class A3, class A4, class A5> inline Shared<T> MakeShared(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {return Shared<T>(new T(a0, a1, a2, a3, a4, a5));}
template <class T, class A0, class A1, class A2, class A3, class A4, class A5, class A6> inline Shared<T> MakeShared(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) {return Shared<T>(new T(a0, a1, a2, a3, a4, a5, a6));}

template <class T>
class Weak : public WeakBase {
	RefBase* r = nullptr;
	T* o = nullptr;
	
public:
	Weak() = default;
	Weak(const Weak& w) : r(w.r), o(w.o) {if (r) r->IncWeak(this);}
	Weak(const Shared<T>& s) : r(s.r), o(s.o) {if (r) r->IncWeak(this);}
	~Weak() {Clear();}
	
	void SetDeleted() override {r = nullptr; o = nullptr;}
	void Clear() {if (r) { r->DecWeak(this); r = nullptr; o = nullptr; }}
	void operator=(const Shared<T>& s) { 
		Clear(); 
		r = s.r; 
		o = s.o; 
		if (r) r->IncWeak(this);
	}
	void operator=(Weak&& p) { 
		Clear(); 
		r = p.r; 
		o = p.o; 
		p.r = nullptr; 
		p.o = nullptr; 
		if (r) r->IncWeak(this);
	}
	void operator=(const Weak& p) { 
		Clear(); 
		r = p.r; 
		o = p.o; 
		if (r) r->IncWeak(this);
	}
	bool IsEmpty() const { return r == nullptr; }
	T* operator->() {return o;}
	T* Get() {return o;}
	operator bool() {return !IsEmpty();}
	Shared<T> Enter() const {
		Shared<T> s; 
		s.r = r; 
		s.o = o; 
		if (s.r) s.r->Inc(); 
		return s;
	}
	
	// std::weak_ptr interface compatibility methods
	void reset() { Clear(); }
	
	Shared<T> lock() const { return Enter(); }
	
	long use_count() const { 
		if (r && o) return r->GetRefCount();
		return 0;
	}
	
	bool expired() const { return use_count() == 0; }
	
	void swap(Weak& other) {
		std::swap(r, other.r);
		std::swap(o, other.o);
	}
};

template <class T>
class EnableSharedFromThis {
	mutable Weak<T> weak;
	
public:
	virtual ~EnableSharedFromThis() = default;
	
	void InitWeak(const Shared<T>& s) {
		ASSERT(weak.IsEmpty()); 
		weak = s;
	}
	
	bool HasWeak() const {return !weak.IsEmpty();}
	
	Shared<T> GetSharedFromThis() const {
		ASSERT(!weak.IsEmpty());
		return weak.Enter();
	}
	
	template <class V>
	Shared<V> AsShared() const {return GetSharedFromThis().template As<V>();}
};

#endif