#pragma once


NAMESPACE_UPP


// PredictionUpdated event listener
class IPredictionUpdateListener abstract :
	public RefScopeEnabler<IPredictionUpdateListener,Machine>
{
public:
    enum PredictionUpdateReason
    {
        HolographicSpaceCreateNextFrame,
        HolographicFrameUpdatePrediction,
    };

    virtual void OnPredictionUpdated(
        PredictionUpdateReason reason,
        void* coordinateSystem,  // Platform-agnostic coordinate system
        void* prediction) = 0;  // Platform-agnostic prediction
};


// HolographicScene
// Maintains a list of our current state of VR objects, ensuring the rest of the systems
// use the same coordinate system, timestamp, etc.
// Updated to work with multiple VR platforms (OpenVR, OpenHMD, WinRT)
class HolographicScene : public System
{
public:
    using System::System;
    using Base = System;
	//RTTI_DECL1(HolographicScene, Base)

    HolographicScene(Engine& core);

    // Platform-agnostic frame and timestamp access
    void* CurrentFrame() const;
    Time CurrentTimestamp() const;

    void* WorldCoordinateSystem() const;
    void UpdateCurrentPrediction();

    void AddPredictionUpdateListener(IPredictionUpdateListener& listener);
    void RemovePredictionUpdateListener(IPredictionUpdateListener& listener);

    // Platform-agnostic camera management
    int GetCameraCount() const;
    IVRCamera* GetCamera(int index) const;
    void AddCameraListener(void* listener);
    void RemoveCameraListener(void* listener);

    // Platform management
    VRPlatform::Type GetPlatformType() const;
    void SetPlatformType(VRPlatform::Type type);

protected:
    bool Initialize(const WorldState&) override;
    void Update(double) override;
    void Uninitialize() override;

    void OnCurrentStageChanged();

    void OnPredictionChanged(IPredictionUpdateListener::PredictionUpdateReason reason);

private:
    mutable std::shared_mutex m_mutex;
    std::unique_ptr<IVRScene> m_platformScene;  // Platform-specific scene implementation

    ListenerCollection<IPredictionUpdateListener> m_predictionUpdatelisteners;
};


END_UPP_NAMESPACE
