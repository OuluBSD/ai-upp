#ifndef _EonWin_EonWin_h_
#define _EonWin_EonWin_h_

#include <Core/Core.h>

#ifdef flagWIN32
#define CY win32_CY_
#endif

#include <Core/config.h>

#include <optional>
#include <functional>
#include <memory>
#include <future>
#include <tuple>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <robuffer.h>
#include <algorithm>
#include <array>
#include <map>
#include <string>
#include <sstream>
#include <optional>

#include <winapifamily.h>

#if WINAPI_FAMILY != WINAPI_FAMILY_APP
	#error Wrong WINAPI_FAMILY
#endif

#include <windows.h>

#include <Guiddef.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <d3d11_3.h>
#include <d3d11_4.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include <stdint.h>

#include <d2d1_2.h>
#include <d3d11_4.h>
#include <DirectXColors.h>
#include <dwrite_2.h>
#include <wincodec.h>
#include <WindowsNumerics.h>
#include <Windows.Graphics.Directx.Direct3D11.Interop.h>


/*
#if WINAPI_FAMILY != WINAPI_FAMILY_APP
	#error Wrong WINAPI_FAMILY
#endif

#ifndef __WRL_NO_DEFAULT_LIB__
	#error Not defined: __WRL_NO_DEFAULT_LIB__
#endif
*/

#include <ppl.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Devices.h>
#include <winrt/Windows.Devices.haptics.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Gaming.Input.h>
#include <winrt/Windows.Graphics.Display.h>
#include <winrt/Windows.Graphics.Holographic.h>
#include <winrt/Windows.Perception.People.h>
#include <winrt/Windows.Perception.Spatial.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Input.Spatial.h>

#include <Vfs/Ecs/Ecs.h>

using namespace Upp;

template <class T, class F>
void erase_if(std::vector<T>* container, F predicate) {
	if (container)
		container->erase(std::remove_if(container->begin(), container->end(), predicate), container->end());
}

inline void fail_fast_if(bool condition, const char* msg = nullptr) {
	if (condition) {
		#ifdef flagDEBUG
		// Panic(msg ? msg : "fail_fast_if triggered");
		#else
		exit(1);
		#endif
	}
}

inline void debug_log(const char* fmt, ...) {
	#ifdef flagDEBUG
	// va_list args;
	// va_start(args, fmt);
	// VLog(VString(fmt, args));
	// va_end(args);
	#endif
}

#include <plugin/stb/stb_image.h> // not needed: #define STB_IMAGE_IMPLEMENTATION
#include <plugin/tiny_gltf/tiny_gltf.h>
#include <plugin/mikktspace/mikktspace.h>
#include "PbrResources.h"
#include "PbrCommon.h"
#include "PbrMaterial.h"
#include "PbrPrimitive.h"
#include "PbrModel.h"
#include "CommonComponents.h"
#include "PbrModelCache.h"
#include "ShaderBytecode.h"

#include "CameraResources.h"
#include "DeviceResources.h"
//#include "DDSTextureLoader.h"
#include "DirectXHelper.h"
#include <plugin/DirectXTK/PlatformHelpers.h>

#include "AppLogicSystem.h"
#include "ListenerCollection.h"
#include "HolographicScene.h"
#include "SpatialInteractionSystem.h"
#include "MotionControllerSystem.h"

#include "EasingSystem.h"
#include "ToolboxSystem.h"
#include "ToolSystem.h"
#include "PaintingSystem.h"
#include "ShootingSystem.h"
#include "ThrowingSystem.h"
#include "PaintStrokeSystem.h"

#include "ControllerRendering.h"
#include <plugin/DirectXTK/GltfHelper.h>
#include "GltfLoader.h"

#include "PhysicsSystem.h"
#include "StepTimer.h"
#include "DemoRoomMain.h"
#include "EntityPrefabs.h"

#include "Haptics.h"

#include "AppView.h"

#include "HolographicRenderer.h"
#include "TextRenderer.h"
#include "QuadRenderer.h"
#include "SkyboxRenderer.h"

#include "Physics.h"
#include "TransformUtil.h"

#ifdef flagWIN32
#undef CY
#endif

#ifdef flagUWP
#define UWPVR_APP_MAIN \
	static std::unique_ptr<Upp::ShellConnectorApp> CreateUwpVrApp__(); \
	int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) \
	{\
		winrt::init_apartment(); \
		Upp::SetUwpVrAppFactory(&CreateUwpVrApp__); \
		winrt::Windows::ApplicationModel::Core::CoreApplication::Run(Upp::AppViewSource()); \
		return 0; \
	} \
	static std::unique_ptr<Upp::ShellConnectorApp> CreateUwpVrApp__()
#else
#define UWPVR_APP_MAIN CONSOLE_APP_MAIN
#endif

#endif
