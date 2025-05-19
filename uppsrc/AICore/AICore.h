#ifndef _AICore_AICore_h_
#define _AICore_AICore_h_

#include <Meta/Meta.h>
#include <Sound/Sound.h>
#include <plugin/bz2/bz2.h>
#include <plugin/png/png.h>
#include <plugin/jpg/jpg.h>
#include <plugin/openai/openai.h>
#include <ide/AiProvider.h>
#include <Esc/Esc.h>

#ifndef flagAI
#error AI flag is not set
#endif


struct Ide;
struct CurrentFileClang;
struct CurrentFileContext;

NAMESPACE_UPP

#include "Defs.h"
#include "Enums.h"
#include "Types.h"

#include "Common.h"
#include "ProcessBase.h"
#include "CodeProcess.h"
#include "Prompt.h"
#include "RemoteTask.h"
#include "TaskManager.h"

#include "Omni.h"

#include "Container.h"
#include "Fn.h"
#include "Phoneme.h"
#include "NatLang.h"

#include "ProcessFramework.h"


// Asset
#include "ProjectWizard.h"

// Disposable
#include "Data.h"
#include "Browser.h"
#include "SourceText.h"
#include "AiCompletion.h"
#include "AiChat.h"

// Private
#include "Owner.h"
#include "Notepad.h"
#include "Biography.h"
#include "BiographySummary.h"
#include "ImageBiography.h"
#include "ImageBiographySummary.h"
#include "BiographyPlatform.h"

// Public
#include "Profile.h"
#include "Release.h"
#include "Perspective.h"
#include "Artist.h"
#include "ReleaseBriefing.h"
#include "Audience.h"
#include "Platform.h"
#include "PlatformProfile.h"
#include "CoverImage.h"
#include "Justice.h"
#include "Lobbying.h"

// Male
#include "Male.h"

// Female
#include "Female.h"

// Buyer

// Seller
#include "Factory.h"

// Consumer
#include "Consumer.h"

// Marketer
#include "LeadData.h"
#include "LeadDataTemplate.h"

// Sound
#include "AudioTranscript.h"
#include "Composition.h"

// Text
#include "Lyrical.h"
#include "LyricsSolver.h"
#include "LyricStructSolver.h"
#include "Song.h"
#include "Reasoning.h"
#include "Transcript.h"
#include "ScriptText.h"

// Photo
#include "Layer.h"
#include "AspectFixer.h"

// Video
#include "VideoSourceFile.h"
#include "VideoPrompt.h"
#include "VideoStoryboard.h"

END_UPP_NAMESPACE

#endif
