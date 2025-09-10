#ifdef HAVE_WX
 #include <wx/dynarray.h>
#include <vector>
#include <algorithm>




void Wx01() {
	
	// 1. Vector
	{
		// wxArrayInt is a common dynamic array type in wxWidgets
		wxArrayInt v;
		int init[] = {5,3,1,4,2};
		for(int x : init) v.Add(x);
		
		// Sort requires copying to std::vector or manual sort (wxArrayInt lacks sort)
		std::vector<int> tmp(v.begin(), v.end());
		std::sort(tmp.begin(), tmp.end());
		v.Clear();
		for(int x : tmp) v.Add(x);
		
		v.Add(6);
		v.Add(7);
		v.Add(8);
		
		v.Insert(5, 1);
		
		bool b = v.IsEmpty(); (void)b;
		
		// No explicit reserve
		
		v[0] = 1;
		v[v.size()-1] = 10;
		
		// No direct resize with fill; simulate grow
		while(v.size() < 100) v.Add(5);
		
		// No shrink_to_fit
		
		v.Erase(v.begin() + 2);
		v.Erase(v.begin() + 1, v.begin() + 3);
		v.RemoveAt(v.size()-1);
		
		int count = (int)v.size(); (void)count;
		
		v.Clear();
	}
	
}

#endif
