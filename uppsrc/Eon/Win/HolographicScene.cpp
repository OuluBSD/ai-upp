////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "EonWin.h"

namespace winrt_holo = winrt::Windows::Graphics::Holographic;
namespace winrt_perception = winrt::Windows::Perception;
namespace winrt_spatial = winrt::Windows::Perception::Spatial;

namespace DemoRoom {

HolographicScene::HolographicScene(
    VfsValue& v,
    winrt_holo::HolographicSpace holographic_space) :
    System(v),
    m_holographicSpace(std::move(holographic_space))
{}

bool HolographicScene::Initialize(const WorldState&)
{
    m_stageFrameOfReference = winrt_spatial::SpatialStageFrameOfReference::Current();

    // Create a fallback frame of reference 1.5 meters under the HMD when we start-up
    m_stationaryFrameOfReference = winrt_spatial::SpatialLocator::GetDefault().CreateStationaryFrameOfReferenceAtCurrentLocation(
        winrt::Windows::Foundation::Numerics::float3{0.0f, -1.5f, 0.0f});

    m_spatialStageCurrentChanged = winrt_spatial::SpatialStageFrameOfReference::CurrentChanged(
        std::bind(&HolographicScene::OnCurrentStageChanged, this));

    return true;
}

void HolographicScene::Update(double)
{
    m_currentFrame = m_holographicSpace.CreateNextFrame();

    OnPredictionChanged(IPredictionUpdateListener::PredictionUpdateReason::HolographicSpaceCreateNextFrame);
}

void HolographicScene::Uninitialize()
{
    winrt_spatial::SpatialStageFrameOfReference::CurrentChanged(m_spatialStageCurrentChanged);

    m_currentFrame = nullptr;
    m_stationaryFrameOfReference = nullptr;
    m_stageFrameOfReference = nullptr;
}

void HolographicScene::UpdateCurrentPrediction()
{
    m_currentFrame.UpdateCurrentPrediction();

    OnPredictionChanged(IPredictionUpdateListener::PredictionUpdateReason::HolographicFrameUpdatePrediction);
}

void HolographicScene::OnCurrentStageChanged()
{
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_stageFrameOfReference = winrt_spatial::SpatialStageFrameOfReference::Current();
}

void HolographicScene::OnPredictionChanged(IPredictionUpdateListener::PredictionUpdateReason reason)
{
    const winrt_holo::HolographicFramePrediction prediction = m_currentFrame.CurrentPrediction();
    const winrt_spatial::SpatialCoordinateSystem coordinateSystem = WorldCoordinateSystem();

    for (const auto& listener : m_predictionUpdateListeners.GetListeners()) {
        listener->OnPredictionUpdated(reason, coordinateSystem, prediction);
    }
}

void HolographicScene::AddPredictionUpdateListener(IPredictionUpdateListener* listener)
{
    m_predictionUpdateListeners.Add(listener);
}

void HolographicScene::RemovePredictionUpdateListener(IPredictionUpdateListener* listener)
{
    m_predictionUpdateListeners.Remove(listener);
}

winrt_spatial::SpatialCoordinateSystem HolographicScene::WorldCoordinateSystem() const
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (m_stageFrameOfReference)
    {
        return m_stageFrameOfReference.CoordinateSystem();
    }
    else
    {
        return m_stationaryFrameOfReference.CoordinateSystem();
    }
}

winrt_perception::PerceptionTimestamp HolographicScene::CurrentTimestamp() const
{
    return CurrentFrame().CurrentPrediction().Timestamp();
}

winrt_holo::HolographicFrame HolographicScene::CurrentFrame() const
{
    fail_fast_if(m_currentFrame == nullptr);
    return m_currentFrame;
}

winrt_holo::HolographicSpace HolographicScene::HolographicSpace() const
{
    fail_fast_if(m_holographicSpace == nullptr);
    return m_holographicSpace;
}

} // namespace DemoRoom