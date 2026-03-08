#ifndef _ScriptIDE_ScriptIDE_h_
#define _ScriptIDE_ScriptIDE_h_

#include <CtrlLib/CtrlLib.h>
#include <CodeEditor/CodeEditor.h>
#include <TabBar/TabBar.h>
#include <ByteVM/ByteVM.h>
#include <RichEdit/RichEdit.h>
#include <Docking/Docking.h>

namespace Upp {

#define LAYOUTFILE <ScriptIDE/ScriptIDE.lay>
#include <CtrlCore/lay.h>

/*
#define IMAGECLASS ScriptIDEImg
#define IMAGEFILE  <ScriptIDE/ScriptIDE.iml>
#include <Draw/iml_header.h>
*/

#include "Settings.h"
#include "VariableExplorer.h"
#include "PythonConsole.h"
#include "FileTree.h"
#include "CustomFileTabs.h"
#include "PythonIDE.h"

}

#endif
