#ifndef _ide_Shell_Shell_h_
#define _ide_Shell_Shell_h_


#include <CtrlLib/CtrlLib.h>
#include <CodeEditor/CodeEditor.h>
#include <ide/Core/Core.h>
#include <AICtrl/AICtrl.h>

#define LAYOUTFILE <ide/Shell/Shell.lay>
#include <CtrlCore/lay.h>

#define KEYGROUPNAME "Shell"
#define KEYNAMESPACE Shell
#define KEYFILE      <ide/Shell/Shell.key>
#include             <CtrlLib/key_header.h>

#include "EscCmds.h"
#include "IdeShell.h"
#include "Widget.h"
#include "Console.h"
#include "SmallWidgets.h"

NAMESPACE_UPP



END_UPP_NAMESPACE

#endif
