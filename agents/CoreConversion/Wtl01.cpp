#ifdef HAVE_WTL
#include <atlcoll.h>
#include <vector>
#include <algorithm>




void Wtl01() {
	
	// 1. Vector
	{
	    // Using ATL CAtlArray as an example
	    CAtlArray<int> v;
	    int init[] = {5,3,1,4,2};
	    for(int x : init) v.Add(x);
	
	    // Sort ascending order
	    std::vector<int> tmp;
	    for(size_t i = 0; i < v.GetCount(); i++) tmp.push_back(v[(INT_PTR)i]);
	    std::sort(tmp.begin(), tmp.end());
	    v.RemoveAll();
	    for(int x : tmp) v.Add(x);
	
	    v.Add(6); v.Add(7); v.Add(8);
	    
	    
	    v.InsertAt(1, 5);
	
	    bool b = (v.GetCount() == 0); (void)b;
	
	    // No reserve
	    
	    v[0] = 1;
	    v[v.GetCount()-1] = 10;
	
	
	    v.SetCount(100);
	    for(size_t i = tmp.size(); i < 100; ++i) v[(INT_PTR)i] = 5;
	
	
	    v.RemoveAt(2);
	    v.RemoveAt(1, 2);
	    v.RemoveAt(v.GetCount()-1);
	
	    int count = (int)v.GetCount();
	
	    v.RemoveAll();
	}
	
}

#endif
