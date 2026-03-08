#ifndef _Core_Recycler_h_
#define _Core_Recycler_h_


template <class T, bool keep_as_constructed=false>
class RecyclerPool {
	Vector<T*> pool;
	Mutex lock;
	bool destructed = false;
	
public:
	typedef RecyclerPool CLASSNAME;
	RecyclerPool() {}
	~RecyclerPool() {Clear(); destructed = true;}
	
	void Clear() {
		lock.Enter();
		auto t = pool.Begin();
		auto end = pool.End();
		while(t != end) {
			T* o = *t;
			if (keep_as_constructed)
				o->~T();
			MemoryFree(o);
			t++;
		}
		pool.Clear();
		lock.Leave();
	}
	void Reserve(int i) {
		lock.Enter();
		pool.Reserve(i);
		lock.Leave();
	}
	
	template <typename... Args>
	T* New(const Args&... args) {
		T* o;
		while (1) {
			if (pool.IsEmpty()) {
				o = (T*)MemoryAlloc(sizeof(T));
				new (o) T(args...);
			}
			else {
				lock.Enter();
				if (pool.IsEmpty()) {
					lock.Leave();
					continue;
				}
				o = pool.Pop();
				lock.Leave();
				if (!keep_as_constructed)
					new (o) T(args...);
			}
			break;
		}
		return o;
	}
	
	void Return(T* o) {
		if (destructed) {
			o->~T();
			MemoryFree(o);
			return;
		}
		if (!keep_as_constructed)
			o->~T();
		lock.Enter();
		pool.Add(o);
		lock.Leave();
	}
	
	static RecyclerPool& StaticPool() {MAKE_STATIC(RecyclerPool, pool); return pool;}
	
};


template <class T, bool keep_as_constructed>
class Recycler {
	
public:
	using Pool = RecyclerPool<T, keep_as_constructed> ;
	
private:
	Pool* pool = 0;
	One<T> o;
	
public:
	Recycler() {}
	Recycler(Pool& pool) : pool(&pool), o(pool.New()) {o->SetCount(0);}
	~Recycler() {Clear();}
	
	void Clear() {if (o && pool) pool->Return(o.Detach());}
	void Create(Pool& pool) {Clear(); this->pool = &pool; o = pool.New();}
	T& Get() {ASSERT(!o.IsEmpty()); return *o;}
	
	T& operator*() {return Get();}
	T* operator->() {return &Get();}
	
};


template <class T>
struct RecyclerRefBase : RefBase {
	using Pool = RecyclerPool<RecyclerRefBase<T>>;
	
	~RecyclerRefBase() {Clear();}
	void Clear() override {
		if (!RefBase::obj) return;
		T::Pool::StaticPool().Return((T*)RefBase::obj);
		RefBase::obj = 0;
	}
	void Delete() override {
		Pool::StaticPool().Return(this);
	}
	void SetObj(T* o) {
		(T*&)RefBase::obj = o;
	}
};


template <class T> using SharedRecycler = Shared<T, RecyclerRefBase<T>>;


template <class T, bool keep_as_constructed=false>
class BiVectorRecycler {
	BiVector<T*> q;
	RecyclerPool<T, keep_as_constructed> pool;
	
public:
	BiVectorRecycler() {}
	BiVectorRecycler(BiVectorRecycler&& s) : q(pick(s.q)) {}
	~BiVectorRecycler() { Clear(); }
	
	template <typename... Args>
	T* AddHead(const Args&... args) {
		T* o = pool.New(args...);
		q.AddHead(o);
		return o;
	}
	
	template <typename... Args>
	T* AddTail(const Args&... args) {
		T* o = pool.New(args...);
		q.AddTail(o);
		return o;
	}
	
	void DropHead() {
		if(q.GetCount() > 0) {
			T* o = q.Head();
			q.DropHead();
			pool.Return(o);
		}
	}
	
	void DropTail() {
		if(q.GetCount() > 0) {
			T* o = q.Tail();
			q.DropTail();
			pool.Return(o);
		}
	}
	
	void Clear() {
		while(q.GetCount() > 0)
			DropHead();
	}
	
	int GetCount() const { return max(0, q.GetCount()); }
	bool IsEmpty() const { return q.GetCount() <= 0; }
	
	T& Head() { return *q.Head(); }
	const T& Head() const { return *q.Head(); }
	T& Tail() { return *q.Tail(); }
	const T& Tail() const { return *q.Tail(); }
	
	T& operator[](int i) { return *q[i]; }
	const T& operator[](int i) const { return *q[i]; }
	
	// Iterator support
	typedef typename BiVector<T*>::Iterator Iterator;
	typedef typename BiVector<T*>::ConstIterator ConstIterator;
	
	Iterator begin() { return q.Begin(); }
	Iterator end() { return q.End(); }
	ConstIterator begin() const { return q.Begin(); }
	ConstIterator end() const { return q.End(); }
	
	// Transfer ownership (pick)
	friend BiVectorRecycler<T, keep_as_constructed> pick(BiVectorRecycler<T, keep_as_constructed>& s) {
		BiVectorRecycler<T, keep_as_constructed> d;
		d.q = pick(s.q);
		// Note: pool content is separate, but since q items came from s.pool, 
		// we must ensure they are returned to d.pool? 
		// Actually, RecyclerPool manages allocations. If we move q, items persist.
		// When d destroys, it returns items to d.pool. 
		// But s.pool still exists. 
		// If pools are instance members, we have a problem: items allocated by s.pool returned to d.pool.
		// If RecyclerPool uses global heap (MemoryAlloc), it's fine.
		// RecyclerPool::Return adds to its internal 'pool' vector.
		// So if we cross-return, d.pool grows, s.pool shrinks (effectively).
		// Memory management wise it's fine as long as T size matches (it does).
		return d;
	}
	
	void operator=(BiVectorRecycler<T, keep_as_constructed>&& s) {
		Clear();
		q = pick(s.q);
	}
	
	friend void Swap(BiVectorRecycler& a, BiVectorRecycler& b) {
		Upp::Swap(a.q, b.q);
	}
};


#endif