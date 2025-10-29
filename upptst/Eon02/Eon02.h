#ifndef _Eon02_Eon02_h_
#define _Eon02_Eon02_h_

#include <Shell/Shell.h>

NAMESPACE_UPP

void Run02aAudioTest(Engine& eng, int method);
void Run02bAudioTest2(Engine& eng, int method);
void Run02cFluidsynth(Engine& eng, int method);
void Run02dSoftinstru(Engine& eng, int method);
void Run02eFmsynth(Engine& eng, int method);
void Run02fCoreaudioInstru(Engine& eng, int method);
void Run02gCoreaudioFilter(Engine& eng, int method);
void Run02lPortmidiToFluidsynth(Engine& eng, int method);
void Run02mPortmidiToCoreaudio(Engine& eng, int method);

END_UPP_NAMESPACE

#endif
