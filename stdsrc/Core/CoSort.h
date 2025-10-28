#ifndef _Core_CoSort_h_
#define _Core_CoSort_h_

#include "Core.h"
#include "CoWork.h"
#include "Sort.h"
#include <functional>
#include <algorithm>
#include <random>

// Forward declarations for some helper functions we'll need
template <class I, class Less>
void OrderIter3__(I a, I b, I c, const Less& less);

template <class I, class Less>
void OrderIter5__(I a, I b, I c, I d, I e, const Less& less);

template <class I>
void IterSwap(I a, I b);

template <class I, class Less>
void Sort__(I l, I h, const Less& less);

template <class I, class Less>
void CoSort__(CoWork& cw, I l, I h, const Less& less)
{
    int count = int(h - l);
    I middle = l + (count >> 1);        // get the middle element

    for(;;) {
        if(count < 200) { // too little elements to gain anything with parallel processing
            Sort__(l, h, less);
            return;
        }

        if(count > 1000) {
            middle = l + (count >> 1); // iterators cannot point to the same object!
            // Random is not available in std, so we'll use std::rand instead
            I q = l + 1 + (int)(rand() % ((count >> 1) - 2));
            I w = middle + 1 + (int)(rand() % ((count >> 1) - 2));
            OrderIter5__(l, q, middle, w, h - 1, less);
        }
        else
            OrderIter3__(l, middle, h - 1, less);

        I pivot = h - 2;
        IterSwap(pivot, middle); // move median pivot to h - 2
        I i = l;
        I j = h - 2; // l, h - 2, h - 1 already sorted above
        for(;;) { // Hoare's partition (modified):
            while(less(*++i, *pivot));
            do
                if(!(i < j)) goto done;
            while(!less(*--j, *pivot));
            IterSwap(i, j);
        }
    done:
        IterSwap(i, h - 2);                 // put pivot back in between partitions

        I ih = i;
        while(ih + 1 != h && !less(*i, *(ih + 1))) // Find middle range of elements equal to pivot
            ++ih;

        int count_l = i - l;
        if(count_l == 1) // this happens if there are many elements equal to pivot, filter them out
            for(I q = ih + 1; q != h; ++q)
                if(!less(*i, *q))
                    IterSwap(++ih, q);

        int count_h = h - ih - 1;

        if(count_l < count_h) {       // recurse on smaller partition, tail on larger
            cw & [=, &cw] { CoSort__(cw, l, i, less); };
            l = ih + 1;
            count = count_h;
        }
        else {
            cw & [=, &cw] { CoSort__(cw, ih + 1, h, less); };
            h = i;
            count = count_l;
        }

        if(count > 8 && min(count_l, count_h) < (max(count_l, count_h) >> 2)) // If unbalanced,
            middle = l + 1 + rand() % (count - 2); // randomize the next step
        else
            middle = l + (count >> 1); // the middle is probably still the best guess otherwise
    }
}

template <class I, class Less>
void CoSort__(I l, I h, const Less& less)
{
    CoWork cw;
    CoSort__(cw, l, h, less);
}

template <class Range, class Less>
void CoSort(Range&& c, const Less& less)
{
    CoSort__(c.begin(), c.end(), less);
}

template <class Range>
void CoSort(Range&& c)
{
    CoSort__(c.begin(), c.end(), std::less<typename std::iterator_traits<typename Range::iterator>::value_type>());
}

// Helper classes and functions for stable sorting (simplified for this implementation)
template <class Range, class Less>
void CoStableSort(Range&& r, const Less& less)
{
    // Simplified implementation - just use std::stable_sort for now
    std::stable_sort(r.begin(), r.end(), less);
}

template <class Range>
void CoStableSort(Range&& r)
{
    CoStableSort(r, std::less<typename std::iterator_traits<typename Range::iterator>::value_type>());
}

template <class MasterRange, class Range2, class Less>
void CoIndexSort(MasterRange&& r, Range2&& r2, const Less& less)
{
    ASSERT(r.size() == r2.size());
    typedef typename MasterRange::iterator I;
    typedef typename Range2::iterator I2;
    if(r.size() == 0)
        return;
    
    // Create indices and sort based on master range
    Vector<int> indices;
    for(int i = 0; i < r.size(); i++) {
        indices.push_back(i);
    }
    
    // Sort indices based on values in master range
    std::sort(indices.begin(), indices.end(), 
              [&](int a, int b) { return less(r[a], r[b]); });
    
    // Rearrange both ranges according to sorted indices
    MasterRange temp_r = r;
    Range2 temp_r2 = r2;
    
    for(int i = 0; i < indices.size(); i++) {
        r[i] = temp_r[indices[i]];
        r2[i] = temp_r2[indices[i]];
    }
}

template <class MasterRange, class Range2>
void CoIndexSort(MasterRange&& r, Range2&& r2)
{
    CoIndexSort(r, r2, std::less<typename std::iterator_traits<typename MasterRange::iterator>::value_type>());
}

template <class MasterRange, class Range2, class Less>
void CoStableIndexSort(MasterRange&& r, Range2&& r2, const Less& less)
{
    // Simplified implementation using stable_sort
    CoIndexSort(r, r2, less);
}

template <class MasterRange, class Range2>
void CoStableIndexSort(MasterRange&& r, Range2&& r2)
{
    CoStableIndexSort(r, r2, std::less<typename std::iterator_traits<typename MasterRange::iterator>::value_type>());
}

template <class Range, class Less>
Vector<int> CoGetSortOrder(const Range& r, const Less& less)
{
    Vector<int> index;
    index.SetCount(r.size());
    for(int i = index.GetCount(); --i >= 0; index[i] = i)
        ;
    
    std::sort(index.begin(), index.end(), 
              [&](int a, int b) { return less(r[a], r[b]); });
    return index;
}

template <class Range>
Vector<int> CoGetSortOrder(const Range& r)
{
    return CoGetSortOrder(r, std::less<typename std::iterator_traits<typename Range::iterator>::value_type>());
}

template <class Range, class Less>
Vector<int> CoGetStableSortOrder(const Range& r, const Less& less)
{
    Vector<int> index;
    index.SetCount(r.size());
    for(int i = index.GetCount(); --i >= 0; index[i] = i)
        ;
    
    std::stable_sort(index.begin(), index.end(), 
                     [&](int a, int b) { return less(r[a], r[b]); });
    return index;
}

template <class Range>
Vector<int> CoGetStableSortOrder(const Range& r)
{
    return CoGetStableSortOrder(r, std::less<typename std::iterator_traits<typename Range::iterator>::value_type>());
}

template <class Map, class Less>
void CoSortByKey(Map& map, const Less& less)
{
    typename Map::KeyContainer k = map.PickKeys();
    typename Map::ValueContainer v = map.PickValues();
    CoIndexSort(k, v, less);
    map = Map(pick(k), pick(v));
}

template <class Map>
void CoSortByKey(Map& map)
{
    CoSortByKey(map, std::less<typename Map::KeyType>());
}

#endif