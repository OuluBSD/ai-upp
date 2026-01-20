////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "EonWin.h"

using namespace winrt::Windows::UI::Input::Spatial;
using namespace std::placeholders;

namespace DemoRoom {

bool SpatialInteractionSystem::Initialize(const WorldState& ws)
{
	(void)ws;
    m_spatialInteractionManager = SpatialInteractionManager::GetForCurrentView();
    BindEventHandlers();
	return true;
}

void SpatialInteractionSystem::Uninitialize()
{
    ReleaseEventHandlers();
    m_spatialInteractionManager = nullptr;
}

void SpatialInteractionSystem::BindEventHandlers()
{
    fail_fast_if(m_spatialInteractionManager == nullptr);

    m_sourceTokens[Detected] = m_spatialInteractionManager.SourceDetected(
        std::bind(&SpatialInteractionSystem::HandleSourceDetected, this, _1, _2));

    m_sourceTokens[Pressed] = m_spatialInteractionManager.SourcePressed(
        std::bind(&SpatialInteractionSystem::HandleSourcePressed, this, _1, _2));

    m_sourceTokens[Updated] = m_spatialInteractionManager.SourceUpdated(
        std::bind(&SpatialInteractionSystem::HandleSourceUpdated, this, _1, _2));

    m_sourceTokens[Released] = m_spatialInteractionManager.SourceReleased(
        std::bind(&SpatialInteractionSystem::HandleSourceReleased, this, _1, _2));

    m_sourceTokens[Lost] = m_spatialInteractionManager.SourceLost(
        std::bind(&SpatialInteractionSystem::HandleSourceLost, this, _1, _2));
}

void SpatialInteractionSystem::ReleaseEventHandlers()
{
    fail_fast_if(m_spatialInteractionManager == nullptr);

    m_spatialInteractionManager.SourceLost(m_sourceTokens[Lost]);
    m_spatialInteractionManager.SourceReleased(m_sourceTokens[Released]);
    m_spatialInteractionManager.SourceUpdated(m_sourceTokens[Updated]);
    m_spatialInteractionManager.SourcePressed(m_sourceTokens[Pressed]);
    m_spatialInteractionManager.SourceDetected(m_sourceTokens[Detected]);
}

void SpatialInteractionSystem::HandleSourceDetected(
    const SpatialInteractionManager& /*sender*/,
    const SpatialInteractionSourceEventArgs& args)
{
    for (auto* listener : m_spatialInteractionListeners.GetListeners())
    {
        listener->OnSourceDetected(args);
    }
}

void SpatialInteractionSystem::HandleSourceLost(
    const SpatialInteractionManager& /*sender*/,
    const SpatialInteractionSourceEventArgs& args)
{
    for (auto* listener : m_spatialInteractionListeners.GetListeners())
    {
        listener->OnSourceLost(args);
    }
}

void SpatialInteractionSystem::HandleSourcePressed(
    const SpatialInteractionManager& /*sender*/,
    const SpatialInteractionSourceEventArgs& args)
{
    for (auto* listener : m_spatialInteractionListeners.GetListeners())
    {
        listener->OnSourcePressed(args);
    }
}

void SpatialInteractionSystem::HandleSourceUpdated(
    const SpatialInteractionManager& /*sender*/,
    const SpatialInteractionSourceEventArgs& args)
{
    for (auto* listener : m_spatialInteractionListeners.GetListeners())
    {
        listener->OnSourceUpdated(args);
    }
}

void SpatialInteractionSystem::HandleSourceReleased(
    const SpatialInteractionManager& /*sender*/,
    const SpatialInteractionSourceEventArgs& args)
{
    for (auto* listener : m_spatialInteractionListeners.GetListeners())
    {
        listener->OnSourceReleased(args);
    }
}

} // namespace DemoRoom

