#ifndef _MetaCtrl_MetaCtrl_h_
#define _MetaCtrl_MetaCtrl_h_

#include <Meta/Meta.h>
#include <CtrlLib/CtrlLib.h>
#include <Sound/Sound.h>
#include <SoundCtrl/SoundCtrl.h>
#include <CodeEditor/CodeEditor.h>

#ifndef flagGUI
#error GUI flag is required
#endif

#define IMAGECLASS MetaImgs
#define IMAGEFILE <MetaCtrl/Images.iml>
#include <Draw/iml_header.h>

NAMESPACE_UPP

#define LAYOUTFILE <MetaCtrl/MetaCtrl.lay>
#include <CtrlCore/lay.h>

#include "Common.h"
#include "Node.h"
#include "ProcessCtrl.h"
#include "TextDesigner.h"
#include "Entity.h"

END_UPP_NAMESPACE

#endif
