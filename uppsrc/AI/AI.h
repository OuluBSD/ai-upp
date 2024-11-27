#ifndef _AI_AI_h_
#define _AI_AI_h_

#include <CodeEditor/CodeEditor.h>
#include <CtrlLib/CtrlLib.h>
#include <ide/clang/clang.h>

#define LAYOUTFILE <AI/AI.lay>
#include <CtrlCore/lay.h>

#include "Common.h"

#include "MetaEnv.h"
#include "MetaEnvTree.h"
#include "ProcessBase.h"
#include "MetaCodeGenerator.h"

#include "ClangTypeResolver.h"
#include "CodeProcess.h"
#include "CodeVisitor.h"
#include "ProcessCtrl.h"


#include "MetaCodeCtrl.h"
#include "Prompt.h"
#include "RemoteTask.h"
#include "TaskCtrl.h"
#include "TaskManager.h"

NAMESPACE_UPP



END_UPP_NAMESPACE

#endif
