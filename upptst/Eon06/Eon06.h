#ifndef _Eon06_Eon06_h_
#define _Eon06_Eon06_h_

#include <Shell/Shell.h>

NAMESPACE_UPP

void Run06aToyShaderSinglePassClouds(Engine& eng, int method);
void Run06bToyShaderMultiPass(Engine& eng, int method);
void Run06cToyShaderWithTexture(Engine& eng, int method);
void Run06dToyShaderWithBuffers(Engine& eng, int method);
void Run06eToyShaderAudio(Engine& eng, int method);
void Run06fToyShaderVoronoi(Engine& eng, int method);
void Run06gToyShaderFractal(Engine& eng, int method);
void Run06hToyShaderAnimation(Engine& eng, int method);
void Run06iToyShaderRaymarching(Engine& eng, int method);
void Run06jToyShaderWater(Engine& eng, int method);
void Run06kToyShaderFire(Engine& eng, int method);
void Run06lToyShaderRgbTest(Engine& eng, int method);

END_UPP_NAMESPACE

#endif