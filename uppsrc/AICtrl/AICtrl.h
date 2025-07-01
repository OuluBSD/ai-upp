#ifndef _AICtrl_AICtrl_h_
#define _AICtrl_AICtrl_h_

#include <AICore2/AICore.h>
#include <CtrlLib/CtrlLib.h>
#ifdef flagAUDIO
	#include <SoundCtrl/SoundCtrl.h>
#endif
#include <MetaCtrl/MetaCtrl.h>

#ifndef flagGUI
#error GUI flag is required
#endif

/*
#include <CodeEditor/CodeEditor.h>
#include <CtrlLib/CtrlLib.h>
#include <ide/clang/clang.h>
#include <ide/Shell/Shell.h>
*/

NAMESPACE_UPP

#define LAYOUTFILE <AICtrl/TextCtrl.lay>
#include <CtrlCore/lay.h>

#define LAYOUTFILE <AICtrl/AI.lay>
#include <CtrlCore/lay.h>

END_UPP_NAMESPACE

#include "Fn.h"

#include "TaskCtrl.h"
#include "OmniCtrl.h"
#include "ImgGen.h"
#include "ImgAspectFixer.h"
#include "VfsProgram.h"
#include "Playground.h"



#include "ProjectWizard.h"

#include "SourceText.h"
#include "Ai.h"

#include "Owner.h"
#include "Notepad.h"
#include "Biography.h"
#include "BiographyPerspectives.h"

#include "Profile.h"
#include "Release.h"
#include "Perspective.h"
#include "Artist.h"
#include "Briefing.h"
#include "BiographyPlatform.h"
#include "PlatformManager.h"
#include "CoverImage.h"

#include "Male.h"

#include "Female.h"


#include "Factory.h"

#include "Consumer.h"

#include "LeadSourceCtrl.h"
#include "LeadTemplateCtrl.h"
#include "LeadPublisherCtrl.h"

#include "AudioTranscript.h"
#include "Composition.h"

#include "LyricContent.h"
#include "LyricRef.h"
#include "LyricsEditor.h"
#include "Song.h"
#include "Reasoning.h"
#include "Transcript.h"
#include "Script.h"

#include "VideoSourceFile.h"
#include "VideoPrompt.h"
#include "VideoStoryboard.h"

#include "DialogueBuilder.h"

NAMESPACE_UPP



END_UPP_NAMESPACE


#endif
