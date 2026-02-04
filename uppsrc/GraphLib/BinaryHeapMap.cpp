#include "GraphLib.h"

namespace GraphLib {

BinaryMinHeap::BinaryMinHeap(Vector<Node>& nodes) {
	for(int i = 0; i < nodes.GetCount(); i++)
		heap.Add(&nodes[i]);
	Heapify();
}

void BinaryMinHeap::Heapify() {
	for(int i = heap.GetCount() / 2 - 1; i >= 0; i--) {
		int parent = i;
		while(true) {
			int left = 2 * parent + 1;
			int right = 2 * parent + 2;
			int smallest = parent;
			
			if(left < heap.GetCount() && heap[left]->distance < heap[smallest]->distance)
				smallest = left;
			if(right < heap.GetCount() && heap[right]->distance < heap[smallest]->distance)
				smallest = right;
				
			if(smallest == parent) break;
			
			Swap(heap[parent], heap[smallest]);
			parent = smallest;
		}
	}
}

void BinaryMinHeap::DecreaseKey(Node* n) {
	int idx = -1;
	for(int i = 0; i < heap.GetCount(); i++)
		if(heap[i] == n) { idx = i; break; }
		
	if(idx < 0) return;
	
	while(idx > 0) {
		int parent = (idx - 1) / 2;
		if(heap[idx]->distance >= heap[parent]->distance) break;
		
		Swap(heap[idx], heap[parent]);
		idx = parent;
	}
}

Node* BinaryMinHeap::ExtractMin() {
	if(heap.IsEmpty()) return nullptr;
	
	Node* minNode = heap[0];
	heap[0] = heap.Top();
	heap.Drop();
	
	if(!heap.IsEmpty()) {
		int parent = 0;
		while(true) {
			int left = 2 * parent + 1;
			int right = 2 * parent + 2;
			int smallest = parent;
			
			if(left < heap.GetCount() && heap[left]->distance < heap[smallest]->distance)
				smallest = left;
			if(right < heap.GetCount() && heap[right]->distance < heap[smallest]->distance)
				smallest = right;
				
			if(smallest == parent) break;
			
			Swap(heap[parent], heap[smallest]);
			parent = smallest;
		}
	}
	
	return minNode;
}

}
