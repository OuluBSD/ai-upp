#pragma once
#ifndef _Core_Heap_h_
#define _Core_Heap_h_

#include <cstdlib>
#include <new>
#include <cstddef>
#include <cstdint>
#include "Core.h"

struct MemoryOptions { // sizes are in KB
	int master_block; // master block size
	int sys_block_limit; // > that this: allocate directly from the system
	int master_reserve; // free master blocks kept in reserve
	int large_reserve; // free large blocks kept in reserve
	int small_reserve; // free formatted small block pages kept in reserve
	
	MemoryOptions(); // loads default options
	~MemoryOptions(); // sets options
};

enum {
	UPP_HEAP_ALIGNMENT = 16,
	UPP_HEAP_MINBLOCK = 32,
};

#ifdef UPP_HEAP

// If UPP_HEAP is defined, we'll use the U++ heap implementation
void *MemoryAllocPermanent(size_t size);

void *MemoryAllocSz(size_t& size);
void *MemoryAlloc(size_t size);
void  MemoryFree(void *ptr);
void *MemoryAlloc32();
void  MemoryFree32(void *ptr);
void  MemoryCheck();
void  MemoryDumpLarge();
void  MemoryDumpHuge();
int   MemoryUsedKb();
int   MemoryUsedKbMax();
void  MemoryLimitKb(int kb);

size_t GetMemoryBlockSize(void *ptr);

bool MemoryTryRealloc__(void *ptr, size_t& newsize);

#ifdef _DEBUG
inline // in DEBUG test for small block is moved inside, because debug adds diagnostics header
bool  MemoryTryRealloc(void *ptr, size_t& newsize) {
	return MemoryTryRealloc__(ptr, newsize);
}
#else
inline
bool  MemoryTryRealloc(void *ptr, size_t& newsize) {
	return ((((uint32_t)(uintptr_t)ptr) & 16) != 0) && MemoryTryRealloc__(ptr, newsize);
}
#endif

void  MemoryBreakpoint(uint32_t serial);

void  MemoryInitDiagnostics();
void  MemoryDumpLeaks();

#ifdef HEAPDBG
void  MemoryIgnoreLeaksBegin();
void  MemoryIgnoreLeaksEnd();

void  MemoryCheckDebug();
#else
inline void  MemoryIgnoreLeaksBegin() {}
inline void  MemoryIgnoreLeaksEnd() {}

inline void  MemoryCheckDebug() {}
#endif

struct MemoryProfile {
	int    allocated[1024]; // active small blocks (index is size in bytes)
	int    fragments[1024]; // unallocated small blocks (index is size in bytes)
	int    freepages; // empty 4KB pages (can be recycled)
	int    large_count; // count of large (~ 1 - 64KB) active blocks
	size_t large_total; // ^ total size
	int    large_fragments_count; // count of unused large blocks
	size_t large_fragments_total; // ^ total size
	int    large_fragments[2048]; // * 256
	int    huge_count; // bigger blocks managed by U++ heap (<= 32MB)
	size_t huge_total; // ^ total size
	int    huge_fragments_count; // count of unused large blocks
	size_t huge_fragments_total; // ^ total size
	int    huge_fragments[65536]; // * 256
	int    sys_count; // blocks directly allocated from the system (>32MB
	size_t sys_total; // ^total size
	int    master_chunks; // master blocks

	MemoryProfile();
};

MemoryProfile *PeakMemoryProfile();

enum {
	KLASS_8 = 17,
	KLASS_16 = 18,
	KLASS_24 = 19,
	KLASS_32 = 0,
	KLASS_40 = 20,
	KLASS_48 = 21,
	KLASS_56 = 22,
};

inline
int TinyKlass__(int sz) { // we suppose that this gets resolved at compile time....
	if(sz <= 8) return KLASS_8;
	if(sz <= 16) return KLASS_16;
	if(sz <= 24) return KLASS_24;
	if(sz <= 32) return KLASS_32;
	if(sz <= 40) return KLASS_40;
	if(sz <= 48) return KLASS_48;
	if(sz <= 56) return KLASS_56;
	return -1;
}

void *MemoryAllok__(int klass);
void  MemoryFreek__(int klass, void *ptr);

inline
void *TinyAlloc(int size) {
	int k = TinyKlass__(size);
	if(k < 0) return MemoryAlloc(size);
	return MemoryAllok__(k);
}

inline
void TinyFree(int size, void *ptr)
{
	int k = TinyKlass__(size);
	if(k < 0)
		MemoryFree(ptr);
	else
		MemoryFreek__(k, ptr);
}

#else

// Standard heap implementation using STL/new and malloc
inline MemoryOptions::MemoryOptions() : master_block(0), sys_block_limit(0), master_reserve(0), large_reserve(0), small_reserve(0) {}
inline MemoryOptions::~MemoryOptions() {}

inline void  *MemoryAllocPermanent(size_t size) { 
	static std::vector<void*> permanent_blocks;
	void* ptr = malloc(size);
	if (ptr) permanent_blocks.push_back(ptr);
	return ptr; 
}

inline void  *MemoryAlloc(size_t size) { 
	if (size == 0) size = 1; // Ensure non-zero allocation
	return new(std::nothrow) byte[size]; 
}

inline void  *MemoryAllocSz(size_t &size) { 
	if (size == 0) size = 1; // Ensure non-zero allocation
	return new(std::nothrow) byte[size]; 
}

inline void   MemoryFree(void *p) { 
	if (p) delete[] static_cast<byte*>(p); 
}

inline void  *MemoryAlloc32() { return new(std::nothrow) byte[32]; }
inline void   MemoryFree32(void *ptr) { 
	if (ptr) delete[] static_cast<byte*>(ptr); 
}

inline void  *MemoryAlloc48() { return new(std::nothrow) byte[48]; }
inline void   MemoryFree48(void *ptr) { 
	if (ptr) delete[] static_cast<byte*>(ptr); 
}

inline void   MemoryInitDiagnostics() {}
inline void   MemoryCheck() {}
inline void   MemoryCheckDebug() {}
inline int    MemoryUsedKb() { return 0; } // Placeholder
inline int    MemoryUsedKbMax() { return 0; } // Placeholder

inline void   MemoryIgnoreLeaksBegin() {}
inline void   MemoryIgnoreLeaksEnd() {}

inline size_t GetMemoryBlockSize(void *ptr) { 
	// Standard malloc doesn't provide this. Placeholder implementation.
	return 0; 
}

inline bool   MemoryTryRealloc(void *ptr, size_t& newsize) { 
	// Standard new/delete don't support reallocation. Placeholder implementation.
	return false; 
}

struct MemoryProfile {
	int empty__;
};

inline MemoryProfile *PeakMemoryProfile() { return nullptr; }

inline void *TinyAlloc(int size) { 
	if (size <= 0) size = 1; // Ensure non-zero allocation
	return MemoryAlloc(size); 
}

inline void TinyFree(int, void *ptr) { 
	MemoryFree(ptr); 
}

#endif

uint32_t MemoryGetCurrentSerial();

void  MemoryIgnoreNonMainLeaks();
void  MemoryIgnoreNonUppThreadsLeaks();

struct MemoryIgnoreLeaksBlock {
	MemoryIgnoreLeaksBlock()  { MemoryIgnoreLeaksBegin(); }
	~MemoryIgnoreLeaksBlock() { MemoryIgnoreLeaksEnd(); }
};

template <class T, class... Args>
T *tiny_new(Args... args)
{
	return new(TinyAlloc(sizeof(T))) T(args...);
}

template <class T>
void tiny_delete(T *ptr)
{
	if (ptr) {
		ptr->~T();
		TinyFree(sizeof(T), ptr);
	}
}

#endif