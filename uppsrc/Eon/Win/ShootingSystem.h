////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include "ToolSystem.h"

namespace DemoRoom
{
    struct ShootingComponent : Component
    {
        ECS_COMPONENT_CTOR(ShootingComponent)

        void SetEnabled(bool enable) override;
        void Destroy() override;

        EntityPtr gun;

        float bulletSpeed = 20.0f;
        winrt::Windows::Foundation::Numerics::float4x4 barrelToController;
    };

    ////////////////////////////////////////////////////////////////////////////////
    // ShootingInteractionSystem
    // This ToolSystem manages the Gun tool which allows you to shoot balls in the 3D scene
    
    class ShootingInteractionSystem : public ToolSystem<ShootingInteractionSystem, ShootingComponent>
    {
    public:
        using ToolSystem::ToolSystem;

    protected:
        // ToolSystemBase
        std::wstring_view GetInstructions() const override;
        std::wstring_view GetDisplayName() const override;
        EntityPtr CreateToolSelector() const override;

        void Register(Vector<EntityPtr> entities) override;
        void Activate(Entity& entity) override;
        void Deactivate(Entity& entity) override;

        // ISpatialInteractionListener
        void OnSourcePressed(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs& args) override;

        void OnSourceUpdated(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs& args) override;
    };
}
