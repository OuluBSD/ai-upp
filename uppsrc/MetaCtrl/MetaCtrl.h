#ifndef _MetaCtrl_MetaCtrl_h_
#define _MetaCtrl_MetaCtrl_h_

#include <Meta/Meta.h>
#include <CtrlLib/CtrlLib.h>
#include <Sound/Sound.h>
#include <SoundCtrl/SoundCtrl.h>

#define IMAGECLASS MetaImgs
#define IMAGEFILE <MetaCtrl/Images.iml>
#include <Draw/iml_header.h>

#define LAYOUTFILE <MetaCtrl/MetaCtrl.lay>
#include <CtrlCore/lay.h>

NAMESPACE_UPP

#include "Common.h"
#include "Node.h"
#include "MetaEnvTree.h"
#include "ProcessCtrl.h"
#include "MetaCodeCtrl.h"
#include "TextDesigner.h"
#include "Entity.h"
#include "Env.h"

END_UPP_NAMESPACE

#endif
