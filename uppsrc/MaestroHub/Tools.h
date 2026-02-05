#ifndef _MaestroHub_Tools_h_
#define _MaestroHub_Tools_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

NAMESPACE_UPP

bool CreateIssueTaskFile(const String& root, const MaestroIssue& iss, const String& title, String& task_path);

END_UPP_NAMESPACE

#endif
