#ifndef _VideoChangedRegionReview_VideoChangedRegionReview_h_
#define _VideoChangedRegionReview_VideoChangedRegionReview_h_

#include <Core/Core.h>
#include <Draw/Draw.h>
#include <plugin/jpg/jpg.h>

NAMESPACE_UPP

struct ReviewOptions {
	String manifest;
	String out_dir;
	bool help = false;
};

END_UPP_NAMESPACE

#endif
