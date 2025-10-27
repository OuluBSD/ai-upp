// VRPlatform.h - Abstraction layer for different VR platforms
// This allows the Eon ECS engine to work with OpenVR, OpenHMD, and WinRT

#ifndef VR_PLATFORM_H
#define VR_PLATFORM_H

#include "Core.h"
#include "EonWin.h"

NAMESPACE_UPP

// Forward declarations for platform interfaces
class IVRPlatform;
class IVRRenderer;
class IVRScene;
class IVRCamera;

// VR Platform abstraction
class VRPlatform {
private:
    static std::unique_ptr<IVRPlatform> s_platform;

public:
    enum class Type {
        WinRT,    // Windows Holographic
        OpenVR,   // Valve OpenVR
        OpenHMD   // OpenHMD
    };

    static bool Initialize(Type type);
    static void Uninitialize();
    static IVRPlatform* Get() { return s_platform.get(); }
    static Type GetPlatformType();
};

// Interface for VR platform operations
class IVRPlatform {
public:
    virtual ~IVRPlatform() = default;

    virtual bool Initialize() = 0;
    virtual void Uninitialize() = 0;
    virtual void Update(double dt) = 0;
    virtual void Render() = 0;

    // Create platform-specific implementations
    virtual std::unique_ptr<IVRScene> CreateScene() = 0;
    virtual std::unique_ptr<IVRRenderer> CreateRenderer() = 0;

    // Platform-specific capabilities
    virtual bool IsInitialized() const = 0;
    virtual bool IsConnected() const = 0;
    virtual int GetDeviceCount() const = 0;
    virtual String GetDeviceName(int index) const = 0;
    virtual String GetPlatformName() const = 0;
};

// Interface for VR rendering
class IVRRenderer {
public:
    virtual ~IVRRenderer() = default;

    virtual bool Initialize() = 0;
    virtual void Uninitialize() = 0;
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void Submit() = 0;

    // Rendering parameters
    virtual Size GetRenderTargetSize() const = 0;
    virtual Rect GetViewBounds() const = 0;
};

// Interface for VR scene management
class IVRScene {
public:
    virtual ~IVRScene() = default;

    virtual bool Initialize() = 0;
    virtual void Uninitialize() = 0;
    virtual void Update(double dt) = 0;

    // Frame handling
    virtual void ProcessFrame() = 0;
    virtual Time GetCurrentTimestamp() const = 0;

    // Coordinate system management
    virtual void* GetWorldCoordinateSystem() const = 0;
    virtual void* GetCurrentFrame() const = 0;

    // Camera management
    virtual int GetCameraCount() const = 0;
    virtual IVRCamera* GetCamera(int index) const = 0;
    virtual void AddCameraListener(void* listener) = 0;
    virtual void RemoveCameraListener(void* listener) = 0;
};

// Interface for VR camera
class IVRCamera {
public:
    virtual ~IVRCamera() = 0;

    virtual void* GetCameraHandle() const = 0;
    virtual Matrix4 GetViewMatrix() const = 0;
    virtual Matrix4 GetProjectionMatrix() const = 0;
    virtual Size GetRenderTargetSize() const = 0;
    virtual Point GetPosition() const = 0;
    virtual double GetNearClip() const = 0;
    virtual double GetFarClip() const = 0;
};

// Matrix4 helper that's used across platforms
class Matrix4 {
public:
    double m[4][4];

    Matrix4() {
        // Initialize as identity matrix
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                m[i][j] = (i == j) ? 1.0 : 0.0;
            }
        }
    }

    static Matrix4 Identity() {
        return Matrix4();
    }

    Matrix4 operator*(const Matrix4& other) const {
        Matrix4 result;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i][j] = 0.0;
                for (int k = 0; k < 4; k++) {
                    result.m[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }
        return result;
    }
};

// Utility functions for coordinate system conversions
struct VRVector3 {
    double x, y, z;

    VRVector3(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
    
    VRVector3 operator+(const VRVector3& other) const {
        return VRVector3(x + other.x, y + other.y, z + other.z);
    }
    
    VRVector3 operator-(const VRVector3& other) const {
        return VRVector3(x - other.x, y - other.y, z - other.z);
    }
    
    VRVector3 operator*(double scale) const {
        return VRVector3(x * scale, y * scale, z * scale);
    }
    
    double Length() const {
        return std::sqrt(x*x + y*y + z*z);
    }
};

struct VRQuaternion {
    double x, y, z, w;

    VRQuaternion(double x = 0, double y = 0, double z = 0, double w = 1) 
        : x(x), y(y), z(z), w(w) {}
};

// Platform-agnostic types that map to WinRT/Windows.Graphics.Holographic types
struct VRFramePrediction {
    // Placeholder until we have platform-specific implementations
    void* platform_handle;
};

struct VRCoordinateSystem {
    // Placeholder until we have platform-specific implementations
    void* platform_handle;
};

struct VRRenderParameters {
    // Placeholder until we have platform-specific implementations
    void* platform_handle;
};

struct VRCameraPose {
    // Placeholder until we have platform-specific implementations
    void* platform_handle;
};

END_UPP_NAMESPACE

#endif // VR_PLATFORM_H