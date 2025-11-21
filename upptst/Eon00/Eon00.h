#ifndef _Eon00_Eon00_h_
#define _Eon00_Eon00_h_

#include <Shell/Shell.h>

NAMESPACE_UPP

void Run00aAudioGen(Engine& eng, int method);
void Run00bAudioGen(Engine& eng, int method);
void Run00cAudioGen(Engine& eng, int method);
void Run00dAudioGenNet(Engine& eng, int method);
void Run00eForkNet(Engine& eng, int method);
void Run00fDiamondNet(Engine& eng, int method);
void Run00gBranchNet(Engine& eng, int method);
void Run00hRouterFlow(Engine& eng, int method);
void Run00iRouterPerf(Engine& eng, int method);

END_UPP_NAMESPACE

#endif
