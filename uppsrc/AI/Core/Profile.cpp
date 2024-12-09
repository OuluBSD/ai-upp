#include "Core.h"

NAMESPACE_UPP


BiographySnapshot* Profile::FindSnapshotRevision(int i) {
	for (auto& snap : snapshots)
		if (snap.revision == i)
			return &snap;
	return 0;
}


END_UPP_NAMESPACE
