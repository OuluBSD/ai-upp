// HolographicScene.cpp - Updated to support multiple VR platforms

#include "EcsWin.h"
#include "VRPlatform.h"


NAMESPACE_UPP


// Platform-agnostic HolographicScene implementation
HolographicScene::HolographicScene(Engine& core) :
    System(core)
{
    // Initialize platform-agnostic scene
}

bool HolographicScene::Initialize(const WorldState& ws)
{
    // Initialize the VR platform based on system selection
    // For now, we'll default to WinRT, but this could be configurable
    if (!VRPlatform::Initialize(VRPlatform::Type::WinRT)) {
        return false;
    }

    // Create the platform-specific scene
    auto platform = VRPlatform::Get();
    if (platform) {
        m_platformScene = platform->CreateScene();
        if (m_platformScene) {
            return m_platformScene->Initialize();
        }
    }

    return false;
}

void HolographicScene::Update(double dt)
{
    // Update the platform-specific scene
    if (m_platformScene) {
        m_platformScene->Update(dt);
    }

    // Call the original prediction update logic
    OnPredictionChanged(IPredictionUpdateListener::PredictionUpdateReason::HolographicSpaceCreateNextFrame);
}

void HolographicScene::Uninitialize()
{
    // Uninitialize platform scene
    if (m_platformScene) {
        m_platformScene->Uninitialize();
        m_platformScene.reset();
    }

    // Uninitialize the VR platform
    VRPlatform::Uninitialize();
}

void HolographicScene::UpdateCurrentPrediction()
{
    // In the platform-agnostic version, this would update the platform's prediction
    // For now, we'll just call the prediction change handlers
    OnPredictionChanged(IPredictionUpdateListener::PredictionUpdateReason::HolographicFrameUpdatePrediction);
}

void HolographicScene::OnCurrentStageChanged()
{
    // Handle stage changes in platform-agnostic way
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    // Update coordinate system reference based on platform
}

void HolographicScene::OnPredictionChanged(IPredictionUpdateListener::PredictionUpdateReason reason)
{
    // Get prediction from platform
    auto platform = VRPlatform::Get();
    if (!platform || !m_platformScene) {
        return;
    }

    // Get coordinate system and prediction from platform
    void* coordinateSystem = m_platformScene->GetWorldCoordinateSystem();
    void* prediction = m_platformScene->GetCurrentFrame();

    // Call all listeners with platform-agnostic data
    for (auto& listener : m_predictionUpdatelisteners.PurgeAndGetListeners()) {
        // We would need to adapt platform-specific types to our platform-agnostic interface
        // For now, this is a simplified version
        listener->OnPredictionUpdated(reason, coordinateSystem, prediction);
    }
}

void HolographicScene::AddPredictionUpdateListener(IPredictionUpdateListener& listener)
{
    m_predictionUpdatelisteners.Add(listener);
}

void HolographicScene::RemovePredictionUpdateListener(IPredictionUpdateListener& listener)
{
    m_predictionUpdatelisteners.Remove(listener);
}

void* HolographicScene::WorldCoordinateSystem() const
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (m_platformScene) {
        return m_platformScene->GetWorldCoordinateSystem();
    }
    return nullptr;
}

void* HolographicScene::CurrentFrame() const
{
    if (!m_platformScene) {
        fail_fast();
    }
    return m_platformScene->GetCurrentFrame();
}

Time HolographicScene::CurrentTimestamp() const
{
    if (!m_platformScene) {
        fail_fast();
    }
    return m_platformScene->GetCurrentTimestamp();
}

// Platform-agnostic methods
int HolographicScene::GetCameraCount() const
{
    if (m_platformScene) {
        return m_platformScene->GetCameraCount();
    }
    return 0;
}

IVRCamera* HolographicScene::GetCamera(int index) const
{
    if (m_platformScene) {
        return m_platformScene->GetCamera(index);
    }
    return nullptr;
}

void HolographicScene::AddCameraListener(void* listener)
{
    if (m_platformScene) {
        m_platformScene->AddCameraListener(listener);
    }
}

void HolographicScene::RemoveCameraListener(void* listener)
{
    if (m_platformScene) {
        m_platformScene->RemoveCameraListener(listener);
    }
}

VRPlatform::Type HolographicScene::GetPlatformType() const
{
    return VRPlatform::GetPlatformType();
}

void HolographicScene::SetPlatformType(VRPlatform::Type type)
{
    if (VRPlatform::GetPlatformType() != type) {
        VRPlatform::Uninitialize();
        VRPlatform::Initialize(type);
        
        // Recreate scene for new platform
        auto platform = VRPlatform::Get();
        if (platform) {
            m_platformScene = platform->CreateScene();
            if (m_platformScene) {
                m_platformScene->Initialize();
            }
        }
    }
}


END_UPP_NAMESPACE