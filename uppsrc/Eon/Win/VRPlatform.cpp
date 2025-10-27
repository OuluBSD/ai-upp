// VRPlatform.cpp - Implementation of VR platform abstraction

#include "VRPlatform.h"

NAMESPACE_UPP

// Static member definition
std::unique_ptr<IVRPlatform> VRPlatform::s_platform = nullptr;

bool VRPlatform::Initialize(Type type) {
    Uninitialize(); // Clean up any existing platform
    
    switch (type) {
#ifdef PLATFORM_WIN32
        case Type::OpenVR:
            s_platform = std::make_unique<OpenVRPlatform>();
            break;
            
        case Type::OpenHMD:
            s_platform = std::make_unique<OpenHMDPlatform>();
            break;
            
        case Type::WinRT:
            s_platform = std::make_unique<WinRTPlatform>();
            break;
#endif
        default:
            return false;
    }
    
    if (s_platform) {
        return s_platform->Initialize();
    }
    
    return false;
}

void VRPlatform::Uninitialize() {
    if (s_platform) {
        s_platform->Uninitialize();
        s_platform.reset();
    }
}

VRPlatform::Type VRPlatform::GetPlatformType() {
    if (!s_platform) {
        return Type::WinRT; // Default fallback
    }
    
#ifdef PLATFORM_WIN32
    if (dynamic_cast<OpenVRPlatform*>(s_platform.get())) {
        return Type::OpenVR;
    } else if (dynamic_cast<OpenHMDPlatform*>(s_platform.get())) {
        return Type::OpenHMD;
    } else if (dynamic_cast<WinRTPlatform*>(s_platform.get())) {
        return Type::WinRT;
    }
#endif
    
    return Type::WinRT; // Default fallback
}

END_UPP_NAMESPACE