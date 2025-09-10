#ifdef HAVE_WINRT
#include <winrt/Windows.Foundation.Collections.h>
using namespace winrt;
using namespace Windows::Foundation::Collections;
#include <vector>
#include <algorithm>


void WinRT01() {
	
	
	// 1. Vector
	{
	    // Use single_threaded_vector for a simple IVector-like container
	    auto v = single_threaded_vector<int>({5,3,1,4,2});
	
	
	    // Sort by copying out, sorting, and copying back
	    std::vector<int> tmp(v.Size());
	    for(uint32_t i = 0; i < v.Size(); ++i) tmp[i] = v.GetAt(i);
	    std::sort(tmp.begin(), tmp.end());
	    v.Clear();
	    for(int x : tmp) v.Append(x);
	
	    v.Append(6); v.Append(7); v.Append(8);
	
	
	    v.InsertAt(1, 5);
	
	    bool b = (v.Size() == 0); (void)b;
	
	    // No reserve
	
	    v.SetAt(0, 1);
	    v.SetAt(v.Size()-1, 10);
	
	
	    while(v.Size() < 100) v.Append(5);
	
	
	
	    v.RemoveAt(2); v.RemoveAtEnd(); // as part of edits
	    v.RemoveAt(1); v.RemoveAt(1); // Remove range 1..2
	    v.RemoveAtEnd(); // pop
	
	    int count = (int)v.Size();
	
	    v.Clear();
	}
	
}

#endif