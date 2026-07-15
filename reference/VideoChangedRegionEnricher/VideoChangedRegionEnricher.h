#ifndef _VideoChangedRegionEnricher_VideoChangedRegionEnricher_h_
#define _VideoChangedRegionEnricher_VideoChangedRegionEnricher_h_

#include <Core/Core.h>
#include <Draw/Draw.h>
#include <plugin/jpg/jpg.h>

NAMESPACE_UPP

struct EnricherOptions {
	String manifest;
	String out_dir;
	bool help = false;
};

END_UPP_NAMESPACE

#endif
