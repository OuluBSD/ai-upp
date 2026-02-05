#ifndef _GraphLib_BinaryHeapMap_h_
#define _GraphLib_BinaryHeapMap_h_

#include "Graph.h"

namespace GraphLib {

class BinaryMinHeap {
	Vector<Node*> heap;
	
public:
	BinaryMinHeap(Array<Node>& nodes);
	
	bool Empty() const { return heap.IsEmpty(); }
	
	Node* GetMin() { return Empty() ? nullptr : heap[0]; }
	
	void DecreaseKey(Node* n);
	void Heapify();
	
	Node* ExtractMin();
};

}

#endif
