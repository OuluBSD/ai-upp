#include <Core/Core.h>

using namespace Upp;

struct GetHashInt {
    uint64 operator()(const int& v) const { return (uint64)(uint32)v; }
};

static void LogTreeNoDoubleNl(const String& s)
{
    if(!s.IsEmpty() && s[s.GetCount() - 1] == '\n')
        LOG(s.Left(s.GetCount() - 1));
    else
        LOG(s);
}

CONSOLE_APP_MAIN
{
    StdLogSetup(LOG_FILE|LOG_COUT);

    // Basic build and iterator sum
    CritBitIndex<int, GetHashInt, 64> idx;
    Vector<uint64> build_keys;
    build_keys << 0x1F20A4B300000012ULL << 0x1F20A4B3FFFF0012ULL << 0x9C00EE7700ABCDEFULL;
    Vector<int*> out;
    idx.PutBulk(build_keys, out);
    for(int i = 0; i < out.GetCount(); i++)
        *out[i] = i + 1; // {1,2,3}
    int sum = 0;
    for(auto& v : idx) sum += v;
    DUMP(sum); // 1+2+3=6

    // 1) Tree dump basics
    CritBitIndex<int, GetHashInt, 64> t;
    LogTreeNoDoubleNl(t.GetTreeString());
    int* p = t.Put(0x12);
    *p = 1;
    LogTreeNoDoubleNl(t.GetTreeString());
    t.Put(0x80000000000000FFULL, 2);
    LogTreeNoDoubleNl(t.GetTreeString());
    LogTreeNoDoubleNl(t.GetTreeString(4));

    // 2) Tree dump after Remove/Optimize
    bool ok = t.Remove(0x80000000000000FFULL);
    DUMP(ok);
    t.Optimize();
    LogTreeNoDoubleNl(t.GetTreeString());

    // 3) Iterator: basic / 4) edge (works when empty)
    int count = 0, sum2 = 0;
    for(auto& v : t) { count++; sum2 += v; }
    DUMP(count);
    DUMP(sum2);
    const auto& ct = t;
    int csum = 0;
    for(const auto& v : ct) csum += v;
    DUMP(csum);
    CritBitIndex<int, GetHashInt, 64> empty;
    int count2 = 0;
    for(auto& v : empty) { (void)v; count2++; }
    DUMP(count2);

    // 5) Iterator & duplicates
    int* p0 = t.Put(0x12);
    int* p1 = t.Put(0x12);
    LOG(String().Cat() << "dup_ok = " << (p0 == p1));
    LOG(String().Cat() << "final_count = " << t.GetCount());

    CheckLogEtalon();
}
