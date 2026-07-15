#ifndef _VideoTableVisualAuditReport_VideoTableVisualAuditReport_h_
#define _VideoTableVisualAuditReport_VideoTableVisualAuditReport_h_

#include <Core/Core.h>

NAMESPACE_UPP

struct VisualAuditOptions {
	String tracker_dir;
	String out_path;
	int    max_frames = 3;
	bool   help = false;
};

END_UPP_NAMESPACE

#endif
