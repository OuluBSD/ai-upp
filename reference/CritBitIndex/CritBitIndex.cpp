#include <Core/Core.h>

using namespace Upp;

// Simple GetHash functor (optional for merges/derivations)
struct GetHashInt {
	uint64 operator()(const int& v) const { return (uint64)(uint32)v; }
};

CONSOLE_APP_MAIN
{
	SetLanguage(LNG_ENGLISH);

	CritBitIndex<int, GetHashInt, 64> idx;

	Vector<uint64> keys;
	keys << 42 << 100 << 7 << 9999999 << 42; // includes duplicate 42

	Vector<int*> out;
	idx.PutBulk(keys, out);

	// Assign values matching order (duplicate should reuse pointer)
	for(int i = 0; i < out.GetCount(); i++)
		*out[i] = i + 1;

	DUMP(idx.GetCount()); // should be 4 (duplicate excluded)

	if(int* p = idx.Find(100))
		LOG("100 -> " << *p);

	if(int* p = idx.Find(42))
		LOG("42 -> " << *p);

	// Remove and optimize
	bool ok = idx.Remove(7);
	DUMP(ok);
	idx.Optimize();
	DUMP(idx.GetCount());

	// Merge example
	CritBitIndex<int, GetHashInt, 64> other;
	other.Put(5, 555);
	other.Put(100, 1000); // conflict with keepThis
	idx.Merge(other, true);
	DUMP(idx.GetCount());
	if(int* p = idx.Find(100)) LOG("100(after merge keepThis) -> " << *p);
}

