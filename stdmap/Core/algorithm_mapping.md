# U++ to STL Mapping: Core Package Algorithms

This document provides comprehensive mapping between U++ Core package algorithm functions and their STL equivalents.

## 1. Sort ↔ std::sort and related algorithms

### U++ Declaration
```cpp
template <class Range>
void Sort(Range&& c);                               // Sort with default less comparator

template <class Range, class Less>
void Sort(Range&& c, const Less& less);            // Sort with custom comparator

template <class Range>
void StableSort(Range&& r);                        // Stable sort with default less

template <class Range, class Less>
void StableSort(Range&& r, const Less& less);      // Stable sort with custom comparator

template <class MasterRange, class Range2>
void IndexSort(MasterRange&& r, Range2&& r2);      // Sort master range, apply same changes to r2

template <class MasterRange, class Range2, class Less>
void IndexSort(MasterRange&& r, Range2&& r2, const Less& less); // IndexSort with custom comparator

template <class MasterRange, class Range2>
void StableIndexSort(MasterRange&& r, Range2&& r2); // Stable index sort

template <class MasterRange, class Range2, class Less>
void StableIndexSort(MasterRange&& r, Range2&& r2, const Less& less); // Stable index sort with custom

template <class Map>
void SortByKey(Map& map);                           // Sort map by key

template <class Map, class Less>
void SortByKey(Map& map, const Less& less);        // Sort map by key with custom comparator

template <class Map>
void SortByValue(Map& map);                        // Sort map by value

template <class Map, class Less>
void SortByValue(Map& map, const Less& less);      // Sort map by value with custom comparator

template <class Range>
Vector<int> GetSortOrder(const Range& r);          // Get indices that would sort the range

template <class Range, class Less>
Vector<int> GetSortOrder(const Range& r, const Less& less); // Get sort order with custom comparator
```

### STL Equivalent
```cpp
#include <algorithm>
template< class Iterator >
void std::sort(Iterator first, Iterator last);     // Sort with default operator<

template< class Iterator, class Compare >
void std::sort(Iterator first, Iterator last, Compare comp); // Sort with custom comparator

template< class Iterator >
void std::stable_sort(Iterator first, Iterator last); // Stable sort with default operator<

template< class Iterator, class Compare >
void std::stable_sort(Iterator first, Iterator last, Compare comp); // Stable sort with custom comparator

template< class Iterator >
void std::partial_sort(Iterator first, Iterator middle, Iterator last); // Partial sort

template< class Iterator, class Compare >
void std::partial_sort(Iterator first, Iterator middle, Iterator last, Compare comp); // Partial sort with custom

template< class Iterator >
Iterator std::is_sorted_until(Iterator first, Iterator last); // Check if sorted

template< class Iterator, class Compare >
Iterator std::is_sorted_until(Iterator first, Iterator last, Compare comp); // Check if sorted with custom

template< class Container >
auto std::ranges::sort(Container& c);              // Sort range (C++20)

template< class Container, class Compare >
auto std::ranges::sort(Container& c, Compare comp); // Sort range with custom (C++20)

template< class Range, class Compare = std::less<> >
std::vector<std::ranges::iterator_t<Range>> 
std::ranges::sort_keys(Range&& r, Compare comp = {}); // Get sort indices (not standard, custom implementation)
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| Sort(range) | std::sort(range.begin(), range.end()) | ✓ Complete | |
| Sort(range, less) | std::sort(range.begin(), range.end(), less) | ✓ Complete | |
| StableSort(range) | std::stable_sort(range.begin(), range.end()) | ✓ Complete | |
| StableSort(range, less) | std::stable_sort(range.begin(), range.end(), less) | ✓ Complete | |
| GetSortOrder(range) | Custom implementation using std::stable_sort with index pairs | ⚠️ Complex | |
| GetSortOrder(range, less) | Custom implementation using std::stable_sort with index pairs | ⚠️ Complex | |
| IndexSort(r, r2) | Sort index pairs and apply to both ranges | ⚠️ Complex | |
| IndexSort(r, r2, less) | Sort index pairs with custom comparator and apply to both ranges | ⚠️ Complex | |
| SortByKey(map) | std::sort with key comparison | ⚠️ Complex | Need custom implementation |
| SortByValue(map) | std::sort with value comparison | ⚠️ Complex | Need custom implementation |

### Conversion Notes
- U++ Sort() directly maps to std::sort() with begin/end iterators
- U++ StableSort() directly maps to std::stable_sort()
- U++ GetSortOrder() requires a custom implementation using std::stable_sort on index-value pairs
- U++ IndexSort() requires sorting index pairs and applying permutation to both ranges
- U++ SortByKey() and SortByValue() require custom implementations for map-like containers

## 2. Find ↔ std::find and related search algorithms

### U++ Declaration
```cpp
template <class Range, class V>
int FindIndex(const Range& r, const V& value, int from = 0); // Find index of value

template <class Range, class Predicate>
Vector<int> FindAll(const Range& r, Predicate match, int from = 0); // Find all matching indices

template <class Range>
int FindMin(const Range& r);                        // Find index of minimum element

template <class Range>
int FindMax(const Range& r);                        // Find index of maximum element

template <class Range, class T>
int FindBinary(const Range& r, const T& val);      // Binary search for value

template <class Range, class T, class L>
int FindBinary(const Range& r, const T& val, const L& less); // Binary search with custom comparator

template <class Range, class T>
int FindLowerBound(const Range& r, const T& val);  // Find lower bound

template <class Range, class T, class L>
int FindLowerBound(const Range& r, const T& val, const L& less); // Lower bound with comparator

template <class Range, class T>
int FindUpperBound(const Range& r, const T& val);  // Find upper bound

template <class Range, class T, class L>
int FindUpperBound(const Range& r, const T& val, const L& less); // Upper bound with comparator

template <class Range, class T>
int FindMatch(const Range& r, const T& match, int from = 0); // Find index matching predicate
```

### STL Equivalent
```cpp
#include <algorithm>
#include <vector>

template< class Iterator, class T >
Iterator std::find(Iterator first, Iterator last, const T& value); // Find element

template< class Iterator, class Predicate >
Iterator std::find_if(Iterator first, Iterator last, Predicate p); // Find if predicate matches

template< class Iterator, class Predicate >
Iterator std::find_if_not(Iterator first, Iterator last, Predicate q); // Find if not predicate

template< class Iterator, class T >
Iterator std::find_end(Iterator first, Iterator last, const T& value); // Find last occurrence

template< class Iterator, class Predicate >
std::vector<typename std::iterator_traits<Iterator>::difference_type> 
find_all(Iterator first, Iterator last, Predicate p); // Find all (custom implementation)

template< class Iterator, class Compare >
Iterator std::min_element(Iterator first, Iterator last); // Find min element

template< class Iterator, class Compare >
Iterator std::max_element(Iterator first, Iterator last); // Find max element

template< class Iterator, class T >
Iterator std::lower_bound(Iterator first, Iterator last, const T& value); // Lower bound

template< class Iterator, class T, class Compare >
Iterator std::lower_bound(Iterator first, Iterator last, const T& value, Compare comp); // Lower bound with comp

template< class Iterator, class T >
Iterator std::upper_bound(Iterator first, Iterator last, const T& value); // Upper bound

template< class Iterator, class T, class Compare >
Iterator std::upper_bound(Iterator first, Iterator last, const T& value, Compare comp); // Upper bound with comp

template< class Iterator, class T >
std::pair<Iterator, Iterator> std::equal_range(Iterator first, Iterator last, const T& value); // Equal range

template< class Iterator, class T, class Compare >
std::pair<Iterator, Iterator> std::equal_range(Iterator first, Iterator last, const T& value, Compare comp); // Equal range with comp

template< class Iterator, class T >
bool std::binary_search(Iterator first, Iterator last, const T& value); // Binary search

template< class Iterator, class T, class Compare >
bool std::binary_search(Iterator first, Iterator last, const T& value, Compare comp); // Binary search with comp
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| FindIndex(range, value) | std::find(range.begin(), range.end(), value) | ⚠️ Complex | STL returns iterator, U++ returns index |
| FindIndex(range, value, from) | std::find(range.begin()+from, range.end(), value) | ⚠️ Complex | |
| FindAll(range, predicate) | Custom implementation or std::find_if with loop | ⚠️ Complex | |
| FindMin(range) | std::min_element(range.begin(), range.end()) | ⚠️ Complex | STL returns iterator, U++ returns index |
| FindMax(range) | std::max_element(range.begin(), range.end()) | ⚠️ Complex | STL returns iterator, U++ returns index |
| FindBinary(range, value) | std::binary_search(range.begin(), range.end(), value) | ⚠️ Complex | STL returns bool, U++ returns index |
| FindLowerBound(range, value) | std::lower_bound(range.begin(), range.end(), value) | ⚠️ Complex | STL returns iterator, U++ returns index |
| FindUpperBound(range, value) | std::upper_bound(range.begin(), range.end(), value) | ⚠️ Complex | STL returns iterator, U++ returns index |

### Conversion Notes
- U++ find functions return indices while STL functions return iterators
- To map U++ behavior to STL, subtract the beginning iterator from the result: `std::find(...) - range.begin()`
- FindAll would require a custom implementation using a loop with std::find_if
- FindBinary returns an index in U++ but std::binary_search returns a boolean; to get the index, use std::lower_bound with a check

## 3. Algorithm operations ↔ STL algorithms

### U++ Declaration
```cpp
template <class T>
inline int sgn(T a) { return a > 0 ? 1 : a < 0 ? -1 : 0; } // Sign function

template <class T>
inline T tabs(T a) { return (a >= 0 ? a : -a); }    // Absolute value

template <class T>
inline int abs(T a) { return (a >= 0 ? a : -a); }   // Absolute value

template <class T>
inline int cmp(const T& a, const T& b) { return a > b ? 1 : a < b ? -1 : 0; } // Three-way comparison

template <class Range>
void Reverse(Range&& r);                            // Reverse range

template <class Range>
ValueTypeOf<Range> Sum(const Range& r, const ValueTypeOf<Range>& zero); // Sum with zero

template <class T>
ValueTypeOf<T> Sum(const T& c);                    // Sum container

template <class Range, class V>
int Count(const Range& r, const V& val);           // Count occurrences

template <class Range, class Predicate>
int CountIf(const Range& r, const Predicate& p);   // Count if predicate

template <class Range, class Pred>
int FindBest(const Range& r, const Pred& pred);    // Find best element by predicate

template <class Range>
const ValueTypeOf<Range>& Min(const Range& r);     // Get minimum element

template <class Range>
const ValueTypeOf<Range>& Min(const Range& r, const ValueTypeOf<Range>& def); // Min with default

template <class Range>
const ValueTypeOf<Range>& Max(const Range& r);     // Get maximum element

template <class Range>
const ValueTypeOf<Range>& Max(const Range& r, const ValueTypeOf<Range>& def); // Max with default

template <class Range1, class Range2>
bool IsEqualRange(const Range1& a, const Range2& b); // Check if ranges are equal

template <class Range1, class Range2>
int CompareRanges(const Range1& a, const Range2& b); // Compare ranges lexicographically

template <class Range, class C>
int FindMatch(const Range& r, const C& match, int from = 0); // Find by predicate

template <class Container, class T>
void LruAdd(Container& lru, T value, int limit = 10); // Add to LRU container

template <class C = Vector<int>, class V>
C MakeIota(V end, V start = 0, V step = 1);        // Create sequence

dword Random(dword n);                              // Random number generation

template <class Range>
void RandomShuffle(Range& r);                      // Random shuffle
```

### STL Equivalent
```cpp
#include <algorithm>
#include <numeric>
#include <functional>
#include <random>

template< class T >
constexpr int signum(T val) {                      // Sign function (not standard, custom)
    return (T(0) < val) - (val < T(0));
}

template< class T >
T abs(const T& n);                                 // Absolute value (std::abs)

template< class T, class U >
decltype(auto) cmp3way(const T& a, const U& b) {   // Three-way comparison (C++20)
    return a <=> b; 
}

template< class Iterator >
void std::reverse(Iterator first, Iterator last);  // Reverse range

template< class Iterator, class T >
T std::accumulate(Iterator first, Iterator last, T init); // Sum with initial value

template< class Iterator, class T >
T std::reduce(Iterator first, Iterator last, T init); // Reduce with initial value (C++17)

template< class Iterator, class T >
auto std::count(Iterator first, Iterator last, const T& value); // Count occurrences

template< class Iterator, class Predicate >
auto std::count_if(Iterator first, Iterator last, Predicate p); // Count if predicate

template< class Iterator, class Compare >
Iterator std::min_element(Iterator first, Iterator last); // Min element

template< class Iterator, class Compare >
Iterator std::max_element(Iterator first, Iterator last); // Max element

template< class Iterator >
std::pair<Iterator, Iterator> 
std::minmax_element(Iterator first, Iterator last); // Min and max elements

template< class Iterator1, class Iterator2 >
bool std::equal(Iterator1 first1, Iterator1 last1, Iterator2 first2); // Check if ranges are equal

template< class Iterator1, class Iterator2, class BinaryPredicate >
bool std::equal(Iterator1 first1, Iterator1 last1, Iterator2 first2, BinaryPredicate p); // Equal with predicate

template< class Iterator1, class Iterator2 >
int std::lexicographical_compare(Iterator1 first1, Iterator1 last1, Iterator2 first2, Iterator2 last2); // Lexicographical compare

template< class Iterator, class Predicate >
Iterator std::find_if(Iterator first, Iterator last, Predicate p); // Find by predicate

template< class T >
std::vector<T> std::iota(std::vector<T>& container, T start); // Fill with sequence (with modification)

template< class Generator >
int std::generate_random_int(Generator& g, int min, int max); // Random generation

template< class RandomIt >
void std::shuffle(RandomIt first, RandomIt last, std::random_device& g); // Random shuffle

template< class RandomIt, class URBG >
void std::shuffle(RandomIt first, RandomIt last, URBG&& g); // Random shuffle with generator
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| sgn(a) | std::signum(a) (custom) or (a > 0) - (a < 0) | ✓ Complete | |
| tabs(a) | std::abs(a) | ✓ Complete | |
| abs(a) | std::abs(a) | ✓ Complete | |
| cmp(a, b) | std::compare_three_way{}(a, b) (C++20) | ⚠️ Complex | |
| Reverse(range) | std::reverse(range.begin(), range.end()) | ✓ Complete | |
| Sum(range, zero) | std::accumulate(range.begin(), range.end(), zero) | ✓ Complete | |
| Count(range, value) | std::count(range.begin(), range.end(), value) | ✓ Complete | |
| CountIf(range, predicate) | std::count_if(range.begin(), range.end(), predicate) | ✓ Complete | |
| Min(range) | *std::min_element(range.begin(), range.end()) | ⚠️ Complex | STL returns iterator |
| Max(range) | *std::max_element(range.begin(), range.end()) | ⚠️ Complex | STL returns iterator |
| IsEqualRange(a, b) | std::equal(a.begin(), a.end(), b.begin()) | ✓ Complete | |
| CompareRanges(a, b) | std::lexicographical_compare() | ⚠️ Complex | Implementation needed |
| FindMatch(range, predicate) | std::find_if(range.begin(), range.end(), predicate) | ⚠️ Complex | STL returns iterator |
| MakeIota(...) | std::iota with vector creation | ⚠️ Complex | Implementation needed |
| RandomShuffle(range) | std::shuffle(range.begin(), range.end(), gen) | ✓ Complete | Need random generator |

### Conversion Notes
- U++ algorithms often return indices or values while STL algorithms often return iterators
- To map iterator returns to index returns: `iterator - container.begin()`
- U++ Sum() maps to std::accumulate() or std::reduce()
- U++ Reverse() maps directly to std::reverse()
- U++ IsEqualRange() maps to std::equal()
- U++ RandomShuffle() maps to std::shuffle() but requires a random number generator

## Summary of Algorithm Mappings

| U++ Algorithm | STL Equivalent | Notes |
|---------------|----------------|-------|
| Sort | std::sort | Direct mapping with iterators |
| StableSort | std::stable_sort | Direct mapping with iterators |
| FindIndex | std::find, etc. | STL returns iterators, U++ returns indices |
| FindMin/Max | std::min_element/max_element | STL returns iterators, U++ returns indices |
| Count/CountIf | std::count/count_if | Direct mapping |
| Reverse | std::reverse | Direct mapping |
| Sum | std::accumulate | Direct mapping |
| IsEqualRange | std::equal | Direct mapping |
| RandomShuffle | std::shuffle | Requires random generator |