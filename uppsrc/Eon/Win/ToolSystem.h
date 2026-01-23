////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

namespace DemoRoom
{
    ////////////////////////////////////////////////////////////////////////////////
    // ToolSystemBase
    // Base abstract class for all ToolSystems
    class ToolSystemBase abstract : public System
    {
    public:
        SYS_CTOR(ToolSystemBase)

        virtual std::wstring_view GetInstructions() const = 0;
        virtual std::wstring_view GetDisplayName() const = 0;

        virtual EntityPtr CreateToolSelector() const = 0;

        virtual void Register(Vector<EntityPtr> entities) = 0;
        virtual void Unregister() = 0;
        virtual void Activate(Entity& entity) = 0;
        virtual void Deactivate(Entity& entity) = 0;

    protected:
        bool Initialize(const WorldState& ws) override
        {
            ws_at_init = ws;
            return true;
        }

        WorldState ws_at_init;
    };

    struct ToolSelectorKey : Component
    {
        ECS_COMPONENT_CTOR(ToolSelectorKey)

        TypeCls type;
    };

    struct ToolSelectorPrefab : EntityPrefab<Transform, PbrRenderable, ToolSelectorKey, RigidBody, Easing>
    {
        static Components Make(Entity& e, const WorldState& ws);
    };

    // CRTP implementation helper
    // Usage: class MyToolSystem : ToolSystem<MyToolSystem> { /* functions + data members */ };
    // Adds functionality to automatically register to listeners and helpers to access entities 
    // that actually have the associated ToolComponent attached and enabled
    template<typename T, typename ToolComponent>
    class ToolSystem abstract : 
        public ToolSystemBase, 
        public ISpatialInteractionListener
    {
    public:
        using ToolSystemBase::ToolSystemBase;

    protected:
        // System
        bool Start() override
        {
            GetEngine().Get<ToolboxSystem>()->AddToolSystem(*this);
            return true;
        }

        void Stop() override
        {
            GetEngine().Get<ToolboxSystem>()->RemoveToolSystem(*this);
        }

        // ToolSystemBase
        void Register(Vector<EntityPtr> entities) override
        {
            m_entities = pick(entities);

            for (auto& entity : m_entities)
            {
                if (!entity)
                    continue;
                auto comp = entity->val.Find<ToolComponent>();
                if (!comp) {
                    auto tuple = entity->CreateComponents<ToolComponent>(ws_at_init);
                    comp = tuple.Get<Ptr<ToolComponent>>();
                }
                comp->SetEnabled(false);
            }

            GetEngine().Get<SpatialInteractionSystem>()->AddListener(this);
        }

        void Unregister() override 
        {
            GetEngine().Get<SpatialInteractionSystem>()->RemoveListener(this);

            for (auto& entity : m_entities)
            {
                if (entity)
                    entity->val.RemoveAllShallow<ToolComponent>();
            }

            m_entities.Clear();
        }

        void Activate(Entity& entity) override
        {
            if (auto comp = entity.val.Find<ToolComponent>())
                comp->SetEnabled(true);
        }

        void Deactivate(Entity& entity) override
        {
            if (auto comp = entity.val.Find<ToolComponent>())
                comp->SetEnabled(false);
        }

        // Internal helpers
        Vector<Tuple<Entity*, ToolComponent*>> GetEnabledEntities() const
        {
            Vector<Tuple<Entity*, ToolComponent*>> entities;

            for (auto& entity : m_entities)
            {
                if (!entity)
                    continue;
                auto comp = entity->val.Find<ToolComponent>();
                if (comp && comp->IsEnabled())
                    entities.Add(MakeTuple(~entity, comp));
            }
            return entities;
        }

        std::optional<Tuple<Entity*, ToolComponent*>> TryGetEntityFromSource(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSource& source) const
        {
            auto entities = GetEnabledEntities();
            for (auto& entity : entities)
            {
                auto ent = entity.Get<0>();
                auto controller = ent->val.Find<MotionControllerComponent>();
                if (controller && controller->IsSource(source))
                    return entity;
            }
            return std::nullopt;
        }

    protected:
        Vector<EntityPtr> m_entities;
    };
}
