#ifndef _VideoEventAuditReport_VideoEventAuditReport_h_
#define _VideoEventAuditReport_VideoEventAuditReport_h_

#include <Core/Core.h>

NAMESPACE_UPP

struct AuditReportOptions {
	String tracker_dir;
	String out_path;
	bool   help = false;
};

END_UPP_NAMESPACE

#endif

