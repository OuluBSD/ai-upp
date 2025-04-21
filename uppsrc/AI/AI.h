#ifndef _AI_AI_h_
#define _AI_AI_h_

#include <CodeEditor/CodeEditor.h>
#include <CtrlLib/CtrlLib.h>
#include <ide/clang/clang.h>
#include <ide/Shell/Shell.h>
#include <Sound/Sound.h>
#include <SoundCtrl/SoundCtrl.h>

#define LAYOUTFILE <AI/AI.lay>
#include <CtrlCore/lay.h>

#define IMAGECLASS AIImages
#define IMAGEFILE <AI/Images.iml>
#include <Draw/iml_header.h>


#include "Common.h"

#include "MetaEnv.h"
#include "MetaEnvTree.h"
#include "PkgEnv.h"
#include "ProcessBase.h"
#include "MetaCodeGenerator.h"

#include "ClangTypeResolver.h"
#include "CodeProcess.h"
#include "CodeVisitor.h"
#include "ProcessCtrl.h"
#include "Omni.h"
#include "OmniCtrl.h"

#include "ImgGen.h"
#include "ImgAspectFixer.h"

#include "MetaCodeCtrl.h"
#include "Prompt.h"
#include "RemoteTask.h"
#include "TaskCtrl.h"
#include "TaskManager.h"

#include "Playground.h"

NAMESPACE_UPP



END_UPP_NAMESPACE

#endif
