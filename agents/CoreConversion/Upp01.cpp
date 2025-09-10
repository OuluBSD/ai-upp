#include <Core/Core.h>







void Upp01() {
	using namespace Upp;
	
	// 1. Vector
	{
		Vector<int> v = {5, 3, 1, 4, 2};
	
	
	
	    // Ascending order
	    Sort(v, StdLess<int>());
	    
	    
	    
	    
	    v.Add(6); // add function
	    v << 7; // operator
	    v.Add() = 8; // return value is reference
	    
	    v.Insert(1, 5);
	    
	    bool b = v.IsEmpty();
	    
	    v.Reserve(100);
	    
	    v[0] = 1;
	    v.Top() = 10;
	    
	    
	    v.SetCount(100, 5);
	    
	    v.Shrink();
	    
	    v.Remove(2);
	    v.Remove(1, 2); // count of 2
	    int value = v.Pop();
	    
	    int count = v.GetCount();
	    
	    v.Clear();
	}
	
	// 2. String
	
}
