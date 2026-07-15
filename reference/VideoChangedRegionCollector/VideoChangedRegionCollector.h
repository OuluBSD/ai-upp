#ifndef _VideoChangedRegionCollector_VideoChangedRegionCollector_h_
#define _VideoChangedRegionCollector_VideoChangedRegionCollector_h_

#include <Core/Core.h>
#include <Draw/Draw.h>
#include <plugin/jpg/jpg.h>

NAMESPACE_UPP

struct ChangedRegionCollectorOptions {
	Vector<String> tracker_dirs;
	String out_dir;
	bool help = false;
};

END_UPP_NAMESPACE

#endif
