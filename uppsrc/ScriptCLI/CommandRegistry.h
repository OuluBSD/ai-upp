#ifndef _ScriptCLI_CommandRegistry_h_
#define _ScriptCLI_CommandRegistry_h_

int HandleScriptCliCommand(const Vector<String>& args);
void PrintScriptCliUsage();

static const int SCRIPTCLI_OK = 0;
static const int SCRIPTCLI_USAGE_ERROR = 1;
static const int SCRIPTCLI_RUNTIME_ERROR = 2;
static const int SCRIPTCLI_INFRA_ERROR = 3;

#endif
