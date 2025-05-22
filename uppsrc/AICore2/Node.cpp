#include "AICore.h"

NAMESPACE_UPP

void GenerateTree(MetaNode& root, int total, int branching_low, int branching_high, Callback1<MetaNode&> set_value) {
	root.sub.Clear();
	Vector<MetaNode*> next_level;
	
	set_value(root);
	next_level.Add(&root);
	
	int branching_range = branching_high - branching_low + 1;
	
	int count = 0;
	while (count < total) {
		Vector<MetaNode*> this_level;
		this_level <<= next_level;
		next_level.Clear();
		for(int i = 0; i < this_level.GetCount(); i++) {
			MetaNode& n = *this_level[i];
			int remaining = total-count;
			if (remaining <= 0) break;
			int sub_node_count = branching_low + Random(branching_range);
			if (sub_node_count > remaining) sub_node_count = remaining;
			n.sub.SetCount(sub_node_count);
			count += sub_node_count;
			for(int j = 0; j < sub_node_count; j++) {
				MetaNode& ptr = n.sub[j];
				ptr.id = IntStr(j);
				next_level.Add(&ptr);
				set_value(ptr);
			}
		}
	}
}

END_UPP_NAMESPACE
