#include "EonApiEditor.h"

NAMESPACE_UPP


void InterfaceBuilder::AddHal() {
	Package("Hal", "Hal");
	SetColor(0, 128, 0);
	Dependency("api/Graphics");
	Library("SDL2 SDL2_ttf SDL2_image", "SDL2 & (!WIN32 | MSC)");
	Library("D3D11 DXGI D2d1 Dwrite", "UWP&DX12");
	Library("SDL2main SDL2 SDL2_image SDL2_ttf glew32 freetype harfbuzz Dwrite SetupAPI jpeg jxl png tiff webp Imm32 Version OleAut32 hwy brotlidec brotlicommon graphite2 Rpcrt4  jbig deflate lzma zstd Lerc sharpyuv z", "SDL2 & WIN32 & !MSC");
	Library("Ole32", "SDL2 WIN32");
	Library("glew32 libglew32.dll.a OpenGL32 iconv", "SDL2 WIN32 (GCC | CLANG)");
	HaveRecvFinalize();
	HaveUpdate();
	HaveIsReady();
	HaveContextFunctions();
	
	Interface("AudioSinkDevice", "HAL & AUDIO");
	Interface("CenterScreenSinkDevice", "HAL & SCREEN");
	Interface("CenterFboSinkDevice", "HAL & FBO & SCREEN");
	Interface("OglScreenSinkDevice", "HAL & OGL & SCREEN");
	Interface("D12ScreenSinkDevice", "HAL & DX12 & SCREEN");
	Interface("ContextBase","HAL");
	Interface("EventsBase","HAL");
	Interface("GuiSinkBase","HAL");
	Interface("GuiFileSrc","HAL");

	Vendor("Upp", "GUI");
	Vendor("Sdl", "SDL2");
	Vendor("Holo", "UWP&DX12"); // Microsoft Hololens
	
}


END_UPP_NAMESPACE
