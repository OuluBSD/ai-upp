////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include "ToolSystem.h"

namespace DemoRoom
{
    struct ToolComponent : Component
    {
        ECS_COMPONENT_CTOR(ToolComponent)

        WString title;
        WString description;
        TypeCls tool_type;
    };

    ////////////////////////////////////////////////////////////////////////////////
    // ToolboxSystem
    // This system manages the ToolSystems and manages the two Entities that represent the left and right Motion Controllers
    class ToolboxSystem : 
        public System,
        public ISpatialInteractionListener
    {
    public:
        SYS_CTOR(ToolboxSystem)

        void AddToolSystem(ToolSystemBase& system);
        void RemoveToolSystem(ToolSystemBase& system);

    protected:
        bool Initialize(const WorldState& ws) override;

        // System
        bool Start() override;
        void Update(double dt) override;
        void Stop() override;

        // ISpatialInteractionListener
        void OnSourcePressed(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs& args) override;

    private:
        ArrayMap<TypeCls, Ptr<ToolSystemBase>> m_selectors;
        ArrayMap<TypeCls, EntityPtr> m_selectorObjects;

        bool m_showToolbox{ false };

        enum ControllerHand {
            Left, Right, Count
        };

        static WString ControllerHandToString(ControllerHand hand);
        static winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceHandedness ControllerHandToHandedness(ControllerHand hand);

        struct ControllerContext {
            EntityPtr Controller;
            EntityPtr DebugText;
            ControllerHand Hand;
        };

        void SwitchToolType(Entity& entity, const TypeCls& new_type);

        EntityPtr FindController(const winrt::Windows::UI::Input::Spatial::SpatialInteractionSource& source);

        std::array<ControllerContext, ControllerHand::Count> m_controllers;

        EntityPtr m_instructionalText;
        WorldState ws_at_init;
    };
}
