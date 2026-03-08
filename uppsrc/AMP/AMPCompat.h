#ifndef _AMP_AMPCompat_h_
#define _AMP_AMPCompat_h_

#if defined flagMSC && defined flagWIN32 && defined flagAMP
	#define PARALLEL restrict(cpu,amp)
	#define PARALLEL_AMP restrict(amp)
	#define HAVE_SYSTEM_AMP
	#include <amp.h>
	#include <amp_math.h>
	#include <cstdlib>












#define AMPASSERT(x)
#define AMPASSERT_(x, y)

inline float AmpTanh(float d) PARALLEL {return ::concurrency::fast_math::tanh(d);}
inline float AmpSqrt(float d) PARALLEL {return ::concurrency::fast_math::sqrt(d);}

inline int GetAmpDeviceMemory() {
	concurrency::accelerator defdev;
	if (defdev.is_emulated)
		return 1024*1000*1000;
	return (int)defdev.get_dedicated_memory() * 1000;
}

inline String GetAmpDevices() {
	String out;
	
	std::vector<concurrency::accelerator> accls = concurrency::accelerator::get_all();
	
	if (accls.empty()) {
		out << "No accelerators found that are compatible with C++ AMP";
		return out;
	}
	
	out << "Show all AMP Devices (";
	#if defined(_DEBUG)
	out << "DEBUG";
	#else
	out << "RELEASE";
	#endif
	out <<  " build)
";
	
	out << "Found " << accls.size()
		<< " accelerator device(s) that are compatible with C++ AMP:
";
	
	/*if (old_format) {
		std::for_each(accls.cbegin(), accls.cend(), [=, &n] (const concurrency::accelerator& a) {
			out << "  " << ++n << ": " << a.description
				<< ", has_display=" << (a.has_display ? "true" : "false")
				<< ", is_emulated=" << (a.is_emulated ? "true" : "false") << "
";
		});
		out << "
";
		return out;
	}*/
	
	for(int i = 0; i < accls.size(); i++) {
		concurrency::accelerator& a = accls[i];
		out << "  " << i << ": " << a.description.c_str() << " "
			<< "
       device_path                       = " << a.device_path.c_str()
			<< "
       dedicated_memory                  = " << DblStr((a.dedicated_memory) / (1024.0f * 1024.0f)) << " Gb"
			<< "
       has_display                       = " << (a.has_display ? "true" : "false")
			<< "
       is_debug                          = " << (a.is_debug ? "true" : "false")
			<< "
       is_emulated                       = " << (a.is_emulated ? "true" : "false")
			<< "
       supports_double_precision         = " << (a.supports_double_precision ? "true" : "false")
			<< "
       supports_limited_double_precision = " << (a.supports_limited_double_precision ? "true" : "false")
			<< "
";
	}
	out << "
";
	return out;
}

/*inline int AmpAtomicSet(int& i, int value) PARALLEL {return concurrency::atomic_exchange(&i, value);}
inline int AmpAtomicInc(int& i) PARALLEL {return concurrency::atomic_fetch_inc(&i);}
inline int AmpAtomicDec(int& i) PARALLEL {return concurrency::atomic_fetch_dec(&i);}*/

#else
	#define PARALLEL
	#define PARALLEL_AMP

#define AMPASSERT(x) ASSERT(x)
#define AMPASSERT_(x, y) ASSERT_(x, y)

namespace concurrency {

struct accelerator {
	
	void wait() {}
};

/*template <int I> struct tiled_index {
	typedef tiled_index<I> idx;
	
	int i;
	
	tiled_index(int i) : i(i) {}
	tiled_index(const idx& src) : i(src.i) {}
	int operator[] (int i) const {AMPASSERT(i == 0); return this->i;}
};*/

template <int I> struct index {
	typedef index<I> idx;
	
	int value[I];
	
	index(int i) {AMPASSERT(I == 1); value[0] = i;}
	index(int x, int y) {AMPASSERT(I == 2); value[0] = x; value[1] = y;}
	index(const idx& src) {*this = src;}
	void operator=(const idx& src) {for(int i = 0; i < I; i++) value[i] = src.value[i];}
	int operator[] (int i) const {AMPASSERT(i >= 0 && i < I); return value[i];}
};

/*template <int I> struct tiled_extent {
	typedef tiled_extent<I> ext;
	typedef tiled_index<I> TileType;
	
	int i;
	
	tiled_extent(int i) : i(i) {}
	tiled_extent(const ext& src) : i(src.i) {;}
	int operator[] (int i) const {AMPASSERT(i == 0); return this->i;}
	int size() const {return i;} // total size of elements
	
};*/

template <int I> struct extent {
	typedef extent<I> ext;
	typedef index<I> TileType;
	static const int dimensions = I;
	
	int value[I];
	
	extent(int i) {AMPASSERT(I == 1); value[0] = i;}
	extent(int w, int h) {AMPASSERT(I == 2); value[0] = w; value[1] = h;}
	extent(const ext& src) {*this = src;}
	void operator=(const ext& src)  {for(int i = 0; i < I; i++) value[i] = src.value[i];}
	int operator[] (int i) const {AMPASSERT(i >= 0 && i < I); return value[i];}
	int size() const {
		int j = 1;
		for(int i = 0; i < I; i++)
			j *= value[i];
		return j;
	}
	//template <int J> tiled_extent<J> tile() {return i;}
	
};

template <class T, int I> struct array_view {
	typedef array_view<T,I> thiscls;
	T* data;
	int count;
	concurrency::extent<I> extent;

	array_view(int count, T* data) : data(data), count(count), extent(count) {
		AMPASSERT(I == 1);
		
	}
	
	array_view(int w, int h, T* data) : data(data), count(w*h), extent(w, h) {
		AMPASSERT(I == 2);
		
	}
	
	array_view(const thiscls& src) : extent(src.extent) {
		data = src.data;
		count = src.count;
	}
	
	T& operator[] (index<1> idx) const {
		AMPASSERT(I == 1);
		int i = idx.value[0];
		ASSERT(i >= 0 && i < count);
		return data[i];
	}
	
	T& operator[] (index<2> idx) const {
		AMPASSERT(I == 2);
		AMPASSERT(idx.value[0] >= 0 && idx.value[0] < extent.value[0]);
		AMPASSERT(idx.value[1] >= 0 && idx.value[1] < extent.value[1]);
		int i = idx.value[0] * extent.value[1] + idx.value[1];
		ASSERT(i >= 0 && i < count);
		return data[i];
	}
	
	/*template <int J>
	T& operator[] (tiled_index<J> idx) const {
		ASSERT(J == count);
		int i = idx.i;
		ASSERT(i >= 0 && i < count);
		return data[i];
	}*/
	
	T& operator[] (int i) const {
		ASSERT(i >= 0 && i < count);
		return data[i];
	}
	
	int size() const {return count;}
	
	void synchronize() {}
	void discard_data() const {}
	accelerator get_source_accelerator_view() {return accelerator();}
	
};

template <class T, int I> struct array {
	typedef array<T,I> thiscls;
	T* data;
	int count;

	array(int count, T* data) : data(data), count(count), extent(count) {
		
	}
	
	array(const thiscls& src) : extent(src.extent) {
		data = src.data;
		count = src.count;
	}
	
	T& operator[] (index<1> idx) const {
		int i = idx.i;
		ASSERT(i >= 0 && i < count);
		return data[i];
	}
	
	T& operator[] (int i) const {
		ASSERT(i >= 0 && i < count);
		return data[i];
	}
	
	int size() const {return count;}
	
	void synchronize() {}
	void discard_data() const {}
	
	struct concurrency::extent<I> extent;
};

struct critical_section {
	Mutex m;
	void lock() {m.Enter();}
	void unlock() {m.Leave();}
};

template <class T, class CB, int I>
struct WorkerManager {
	static const int MAX_I = 10;
	
	int cursor[MAX_I], count[MAX_I];
	int not_stopped = 0;
	bool running = false;
	SpinLock spin;
	Mutex waiter;
	CB cb;
	
	typedef WorkerManager CLASSNAME;
	WorkerManager(int w, CB cb) :cb(cb) {ASSERT(I == 1); count[0] = w; cursor[0] = 0;}
	WorkerManager(int w, int h, CB cb) :cb(cb) {ASSERT(I == 2); count[0] = w; count[1] = h; cursor[0] = 0; cursor[1] = 0;}
	void Start() {
		int count = CPU_Cores() - 1;
		not_stopped = count;
		running = true;
		for(int i = 0; i < count; i++)
			Thread::Start(THISBACK1(Process, i));
		Sleep(10);
	}
	void Process(int i) {
		if (!i) waiter.Enter();
		while (running) {
			spin.Enter();
			int args[MAX_I];
			bool valid = running;
			if (running) {
				if (I == 1) {
					if (cursor[0] < count[0]) {
						args[0] = cursor[0]++;
						valid = true;
					}
					if (cursor[0] >= count[0])
						running = false;
				}
				else if (I == 2) {
					if (cursor[0] < count[0]) {
						args[0] = cursor[0];
						args[1] = cursor[1];
						cursor[0]++;
						if (cursor[0] >= count[0]) {
							cursor[0] = 0;
							cursor[1]++;
							if (cursor[1] >= count[1])
								running = false;
						}
						valid = true;
					}
				}
				else Panic("Not implemented");
			}
			spin.Leave();
			
			if (valid) {
				if (I == 1) {
					cb(typename T::TileType(args[0]));
				}
				else if (I == 2) {
					cb(typename T::TileType(args[0], args[1]));
				}
			}
		}
		spin.Enter();
		not_stopped--;
		spin.Leave();
		if (!i) waiter.Leave();
	}
	void Wait() {
		waiter.Enter();
		waiter.Leave();
		while (not_stopped)
			Sleep(1);
	}
};

template <class T, class CB> void parallel_for_each(T extent, CB cb) {
	if (extent.dimensions == 1) {
		WorkerManager<T, CB, 1> mgr(extent.value[0], cb);
		mgr.Start();
		mgr.Wait();
	}
	else if (extent.dimensions == 2) {
		WorkerManager<T, CB, 2> mgr(extent.value[0], extent.value[1], cb);
		mgr.Start();
		mgr.Wait();
	}
	else Panic("Dimensions over 2 have not yet been implemented");
}

inline void TestCompatAMP() {
	Vector<int> ints;
	for(int i = 0; i < 16; i++) ints.Add(1000 + i);
	
	array_view<int, 1>  ints_view(ints.GetCount(), ints.Begin());
	
	parallel_for_each(ints_view.extent, [=](index<1> idx) PARALLEL
    {
        COUTLOG("View: " << (int)idx[0] << ",	Value: " << ints_view[idx]);
    });
    
    ints_view.synchronize();
}

inline int GetAmpDeviceMemory() {return 1024*1000*1000;}
inline String GetAmpDevices() {return "Fake AMP";}
inline double AmpTanh(double d) PARALLEL {return tanh(d);}
inline double AmpSqrt(double d) PARALLEL {return sqrt(d);}

inline int AmpAtomicSet(int& i, int value) {
	static Mutex l;
	l.Enter();
	int j = i;
	i = value;
	l.Leave();
	return j;
}

inline int AmpAtomicInc(int& i, int value) {
	static Mutex l;
	l.Enter();
	int j = i++;
	l.Leave();
	return j;
}

inline int AmpAtomicDec(int& i, int value) {
	static Mutex l;
	l.Enter();
	int j = i--;
	l.Leave();
	return j;
}

struct runtime_exception {
	String msg;
	
	runtime_exception(String msg) : msg(msg) {}
	String what() {return msg;}
};

inline bool atomic_compare_exchange(int* dest, int* exp_value, int value) {
	#ifdef flagPOSIX
	return __atomic_compare_exchange_n(dest, exp_value, value, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
	#else
	//dword prev = InterlockedCompareExchange((dword*)dest, (dword)*exp_value, (dword)value);
	//return prev == (dword)*exp_value;
	INTERLOCKED {
		if (*dest == *exp_value) {
			*dest = value;
			return true;
		}
		return false;
	}
	#endif
}

}

#endif

template <class T>
inline T AmpFabs(T d) PARALLEL {if (d >= 0) return +d; else return -d;}












#endif