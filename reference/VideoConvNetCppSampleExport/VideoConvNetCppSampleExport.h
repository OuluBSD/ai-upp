#ifndef _VideoConvNetCppSampleExport_VideoConvNetCppSampleExport_h_
#define _VideoConvNetCppSampleExport_VideoConvNetCppSampleExport_h_

#include <Core/Core.h>
#include <Draw/Draw.h>
#include <plugin/jpg/jpg.h>

NAMESPACE_UPP

struct ExportOptions {
	String candidates, review, out_dir;
	bool help = false;
};

END_UPP_NAMESPACE

#endif
