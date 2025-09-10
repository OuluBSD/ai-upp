#if defined(HAVE_MFC)
#include <afx.h>
#include <vector>
#include <algorithm>




void Mfc01() {
	
	// 1. Vector
	{
	    // Using MFC CArray as an example container
	    CArray<int, int> v;
	    v.Add(5); v.Add(3); v.Add(1); v.Add(4); v.Add(2);
	
	
	    // No direct sort on CArray; copy to std::vector for sorting as a practical approach
	    std::vector<int> tmp;
	    for(int i = 0; i < v.GetSize(); i++) tmp.push_back(v[i]);
	    std::sort(tmp.begin(), tmp.end());
	    v.RemoveAll();
	    for(int x : tmp) v.Add(x);
	    v.Add(6);
	    v.Add(7);
	    v.Add(8);
	
	    v.InsertAt(1, 5);
	
	    BOOL b = v.IsEmpty();
	
	    // CArray has no explicit reserve
	
	    v[0] = 1;
	    v[v.GetSize()-1] = 10;
	
	
	    v.SetSize(100, 5);
	    
	    // No shrink
	
	    v.RemoveAt(2);
	    v.RemoveAt(1, 2);
	    v.RemoveAt(v.GetSize()-1); // pop
	
	    int count = (int)v.GetSize(); (void)count;
	
	    v.RemoveAll();
	}
	
}

#endif
