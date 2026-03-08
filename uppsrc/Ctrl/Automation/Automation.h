#ifndef _Ctrl_Automation_Automation_h_
#define _Ctrl_Automation_Automation_h_

#include <CtrlLib/CtrlLib.h>
#include <ByteVM/ByteVM.h>
#include <CtrlLib/GuiAutomation.h>

NAMESPACE_UPP

typedef void (*AddMockFn)(const String& regex, const String& response);
void SetAddMockFn(AddMockFn fn);

typedef Value (*EvalHook)(const String& script);
typedef void (*NavigateHook)(const String& url);
void SetAutomationHooks(EvalHook eval, NavigateHook navigate);

void RegisterAutomationBindings(PyVM& vm);

// Event System
void SetCurrentVM(PyVM* vm);
void TriggerEvent(const String& event);

END_UPP_NAMESPACE

#endif
