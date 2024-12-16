#include "Core.h"

NAMESPACE_UPP

BiographyPerspectives* Profile::FindSnapshotRevision(int i) {
	TODO
	#if 0
	for (auto& snap : snapshots)
		if (snap.revision == i)
			return &snap;
	#endif
	return 0;
}






// TODO: move to file
const VectorMap<String, Vector<String>>& GetMarketplaceSections() {
	static VectorMap<String, Vector<String>> m;
	if (!m.IsEmpty()) return m;
	{
		auto& v = m.Add("TODO");
		v.Add("TODO");
	}
	return m;
}


INITIALIZER_COMPONENT(Profile)

END_UPP_NAMESPACE
