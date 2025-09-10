#ifdef HAVE_JUCE
#include <juce_core/juce_core.h>
using namespace juce;
#include <vector>
#include <algorithm>



void Juce01() {
	
	// 1. Vector
	{
	    Array<int> v;
	    v.addArray({5,3,1,4,2});
		
		
		
	    // Sort via std::sort on raw pointer range
	    std::sort(v.begin(), v.end());
		
		
		
		
	    v.add(6);
	    v.add(7);
	    v.add(8);
	
	    v.insert(1, 5);
	
	    bool b = v.isEmpty(); (void)b;
	
	    // No explicit reserve in juce::Array
	
	    v.set(0, 1);
	    v.set(v.size()-1, 10);
	    
	    // Resize with fill: grow manually
	    while(v.size() < 100) v.add(5);
	    // No shrink_to_fit; trimToSize reduces capacity to size
	    v.trimToSize();
	
	    v.remove(2);
	    v.removeRange(1, 2);
	    v.removeLast();
	
	    int count = v.size();
	
	    v.clear();
	}
}

#endif
