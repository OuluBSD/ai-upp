# CMakeLists.txt - CMake configuration for Eon/Win project

cmake_minimum_required(VERSION 3.15)
project(EonWin)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define source files
set(EONWIN_SOURCES
    AppView.cpp
    CameraResources.cpp
    ComponentStore.cpp
    DemoRoomMain.cpp
    DeviceResources.cpp
    Engine.cpp
    Entity.cpp
    EntityStore.cpp
    GltfHelper.cpp
    GltfLoader.cpp
    HolographicRenderer.cpp
    HolographicScene.cpp
    MotionControllerSystem.cpp
    PaintingSystem.cpp
    PaintStrokeSystem.cpp
    PbrCommon.cpp
    PbrMaterial.cpp
    PbrModel.cpp
    PbrModelCache.cpp
    PbrPrimitive.cpp
    PbrResources.cpp
    PhysicsSystem.cpp
    QuadRenderer.cpp
    ShootingSystem.cpp
    SkyboxRenderer.cpp
    SpatialInteractionSystem.cpp
    TextRenderer.cpp
    ThrowingSystem.cpp
    ToolboxSystem.cpp
    ToolSystem.cpp
    
    # VR Platform abstraction files
    VRPlatform.cpp
    VRPlatformOpenVR.cpp
    VRPlatformOpenHMD.cpp
    VRPlatformWinRT.cpp
)

# Define header files (for IDEs)
set(EONWIN_HEADERS
    AppView.h
    CameraResources.h
    CommonComponents.h
    Component.h
    DemoRoomMain.h
    DeviceResources.h
    DirectXHelper.h
    EasingSystem.cpp
    Engine.h
    EngineCommon.h
    EngineTypeTraits.h
    Entity.h
    EntityPrefabs.h
    GltfHelper.h
    GltfLoader.h
    Haptics.h
    HolographicRenderer.h
    HolographicScene.h
    ListenerCollection.h
    MotionControllerSystem.h
    PaintingSystem.h
    PaintStrokeSystem.h
    PbrCommon.h
    PbrMaterial.h
    PbrModel.h
    PbrModelCache.h
    PbrPrimitive.h
    PbrResources.h
    Physics.h
    PhysicsSystem.h
    QuadRenderer.h
    ShootingSystem.h
    SkyboxRenderer.h
    SpatialInteractionSystem.h
    StepTimer.h
    TextRenderer.h
    ThrowingSystem.h
    ToolboxSystem.h
    ToolSystem.h
    TransformUtil.h
    EonWin.h
    UwpCompat.h
    VRPlatform.h
)

# Find required packages
find_package(WindowsSDK REQUIRED)
find_package(directxmath CONFIG REQUIRED)

# For OpenVR support
option(ENABLE_OPENVR "Enable OpenVR support" OFF)
if(ENABLE_OPENVR)
    find_path(OPENVR_INCLUDE_DIR openvr.h PATHS ${CMAKE_CURRENT_SOURCE_DIR}/external/openvr/headers)
    find_library(OPENVR_LIBRARY openvr_api PATHS ${CMAKE_CURRENT_SOURCE_DIR}/external/openvr/lib/windows64)
    
    if(OPENVR_INCLUDE_DIR AND OPENVR_LIBRARY)
        target_include_directories(EonWin PRIVATE ${OPENVR_INCLUDE_DIR})
        target_link_libraries(EonWin PRIVATE ${OPENVR_LIBRARY})
        target_compile_definitions(EonWin PRIVATE PLATFORM_OPENVR=1)
    endif()
endif()

# For OpenHMD support
option(ENABLE_OPENHMD "Enable OpenHMD support" OFF)
if(ENABLE_OPENHMD)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(OPENHMD REQUIRED openhmd)
    
    target_include_directories(EonWin PRIVATE ${OPENHMD_INCLUDE_DIRS})
    target_link_libraries(EonWin PRIVATE ${OPENHMD_LIBRARIES})
    target_compile_definitions(EonWin PRIVATE PLATFORM_OPENHMD=1)
endif()

# Add executable
add_executable(EonWin ${EONWIN_SOURCES} ${EONWIN_HEADERS})

# Set Windows-specific properties
set_target_properties(EonWin PROPERTIES
    VS_WINRT_COMPONENT TRUE
    WINDOWS_SDK_VERSION 10.0
)

# UWP-specific settings
option(BUILD_UWP "Build for UWP" OFF)
if(BUILD_UWP)
    set_target_properties(EonWin PROPERTIES
        VS_WINRT_COMPONENT TRUE
        VS_PACKAGE_REFERENCES "Microsoft.Windows.SDK.Contracts_10.0.19041.1"
    )
    target_compile_definitions(EonWin PRIVATE UWP_ENABLED=1)
endif()

# Include directories
target_include_directories(EonWin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    # Add more include directories as needed
)

# Compile definitions
target_compile_definitions(EonWin PRIVATE
    _UNICODE
    UNICODE
    WIN32_LEAN_AND_MEAN
    # Additional definitions as needed
)

# Link libraries
target_link_libraries(EonWin PRIVATE
    dxgi
    d3d11
    dxguid
    windowscodecs
    RuntimeObject
)

# Set output directory
set_target_properties(EonWin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy required DLLs after build (for OpenVR, OpenHMD if enabled)
if(ENABLE_OPENVR)
    add_custom_command(TARGET EonWin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/external/openvr/bin/win64/openvr_api.dll"
        $<TARGET_FILE_DIR:EonWin>
    )
endif()

if(ENABLE_OPENHMD)
    add_custom_command(TARGET EonWin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${OPENHMD_LIBRARIES}"
        $<TARGET_FILE_DIR:EonWin>
    )
endif()

# Installation rules
install(TARGETS EonWin
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Example usage:
# cmake -G "Visual Studio 16 2019" -A x64 -DENABLE_OPENVR=ON -DBUILD_UWP=ON ..