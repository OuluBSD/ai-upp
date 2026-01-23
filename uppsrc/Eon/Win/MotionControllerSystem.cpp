////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "EonWin.h"

#include "ControllerRendering.h"
#include "PbrModel.h"

using namespace DemoRoom;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Windows::UI::Input::Spatial;

namespace 
{
    String ControllerModelKeyToString(const std::tuple<uint16_t, uint16_t, uint16_t, SpatialInteractionSourceHandedness>& tuple)
    {
        return Format("MotionController_%d_%d_%d_%d",
            std::get<0>(tuple),
            std::get<1>(tuple),
            std::get<2>(tuple),
            static_cast<uint16_t>(std::get<3>(tuple)));
    }

    std::future<void> LoadAndCacheModel(
        const SpatialInteractionSource& source,
        Engine& engine)
    {
        return std::async(std::launch::async, [&source, &engine] {
            const auto controllerModelName = ControllerModelKeyToString(ControllerRendering::GetControllerModelKey(source));

            auto pbrModelCache = engine.Get<PbrModelCache>();
            if (!pbrModelCache->ModelExists(controllerModelName))
            {
                auto pbrResourcesPtr = engine.Get<HolographicRenderer>()->GetPbrResources();
                const auto model = ControllerRendering::TryLoadRenderModelAsync(*pbrResourcesPtr, source).get();

                if (model)
                {
                    auto clone = model->Clone(*pbrResourcesPtr);
                    if (clone) {
                        One<Pbr::Model> owned;
                        owned.Create(*clone);
                        pbrModelCache->RegisterModel(controllerModelName, pick(owned));
                    }
                }
                else
                {
                    debug_log("Failed to load model for source %d", source.Id());
                }
            }
        });
    }
}

namespace DemoRoom {

bool MotionControllerSystem::Start()
{
    GetEngine().Get<HolographicScene>()->AddPredictionUpdateListener(this);
    GetEngine().Get<SpatialInteractionSystem>()->AddListener(this);
    return true;
}

void MotionControllerSystem::OnPredictionUpdated(
    IPredictionUpdateListener::PredictionUpdateReason /*reason*/,
    const SpatialCoordinateSystem& coordinateSystem,
    const HolographicFramePrediction& prediction)
{
    // Update the positions of the controllers based on the current timestamp.
    for (auto& sourceState : GetEngine().Get<SpatialInteractionSystem>()
		->GetInteractionManager().GetDetectedSourcesAtTimestamp(prediction.Timestamp())) {
        RefreshComponentsForSource(sourceState.Source());

        auto& root = GetEngine().GetRootPool();
        auto entities = root.FindAllDeep<Entity>();
        for (auto& entity : entities) {
            auto transform = entity->val.Find<Transform>();
            auto controller = entity->val.Find<MotionControllerComponent>();
            if (!transform || !controller)
                continue;

            if (controller->IsSource(sourceState.Source())) {
                const SpatialInteractionSourceLocation location =
                    sourceState.Properties().TryGetLocation(coordinateSystem);
                controller->location = location;

                if (location) {
                    transform->position = location_util::position(location);
                    transform->orientation = location_util::orientation(location);
                }
            }
        }
    }
}

void MotionControllerSystem::Stop()
{
    GetEngine().Get<HolographicScene>()->RemovePredictionUpdateListener(this);
    GetEngine().Get<SpatialInteractionSystem>()->RemoveListener(this);
}

void MotionControllerSystem::RefreshComponentsForSource(const SpatialInteractionSource& source)
{
    auto& root = GetEngine().GetRootPool();
    auto entities = root.FindAllDeep<Entity>();
    for (auto& entity : entities) {
        auto controller = entity->val.Find<MotionControllerComponent>();
        if (!controller)
            continue;

        fail_fast_if(controller->requestedHandedness == SpatialInteractionSourceHandedness::Unspecified,
            "Unspecified is not supported yet");

        if (controller->source == nullptr && source.Handedness() == controller->requestedHandedness) {
            controller->source = source;
            debug_log("Attached source id %d to entity with requested handedness %d",
                      source.Id(), static_cast<uint32_t>(controller->requestedHandedness));
        }
    }
}

void MotionControllerSystem::OnSourceUpdated(const SpatialInteractionSourceEventArgs& args)
{
    if (args.State().Source().Kind() == SpatialInteractionSourceKind::Controller)
    {
        auto& root = GetEngine().GetRootPool();
        auto entities = root.FindAllDeep<Entity>();
        for (auto& entity : entities) {
            auto pbr = entity->val.Find<PbrRenderable>();
            auto controller = entity->val.Find<MotionControllerComponent>();
            if (!pbr || !controller)
                continue;

            if (controller->IsSource(args.State().Source()) && controller->attachControllerModel) {
                // If we don't have a model yet, set the ModelName so PbrModelCache will update the model
                if (!pbr->Model) {
                    pbr->ResetModel(ControllerModelKeyToString(
                        ControllerRendering::GetControllerModelKey(controller->source)));
                }
                else {
                    ControllerRendering::ArticulateControllerModel(
                        ControllerRendering::GetArticulateValues(args.State()), *pbr->Model);
                }
            }
        }
    }
}

void MotionControllerSystem::OnSourceDetected(const SpatialInteractionSourceEventArgs& args)
{
    if (args.State().Source().Kind() == SpatialInteractionSourceKind::Controller)
    {
        // Attempt to load any controller models into the PbrModelCache
        (void)LoadAndCacheModel(args.State().Source(), GetEngine());

        // Update any components with their new Source 
        RefreshComponentsForSource(args.State().Source());
    }
}

void MotionControllerSystem::OnSourceLost(const SpatialInteractionSourceEventArgs& args)
{
    if (args.State().Source().Kind() == SpatialInteractionSourceKind::Controller)
    {
        auto& root = GetEngine().GetRootPool();
        auto entities = root.FindAllDeep<Entity>();
        for (auto& entity : entities) {
            auto controller = entity->val.Find<MotionControllerComponent>();
            if (!controller)
                continue;
            if (controller->IsSource(args.State().Source())) {
                controller->source = nullptr;
                controller->location = nullptr;
            }
        }
    }
}

bool MotionControllerComponent::IsSource(const SpatialInteractionSource& rhs) const 
{
    return (this->source && rhs) ? this->source.Id() == rhs.Id() : false;
}

} // namespace DemoRoom
