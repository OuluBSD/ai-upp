////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

namespace DemoRoom::KnownModelNames 
{
    constexpr auto UnitSphere = "UnitSphere_LowPoly";
    constexpr auto UnitCube = "UnitCube";
    constexpr auto Baseball = "Baseball";
    constexpr auto PaintBrush = "PaintBrush";
    constexpr auto Gun = "Gun";
}

namespace DemoRoom
{
    struct FloorPrefab : EntityPrefab<Transform, PbrRenderable>
    {
        static Components Make(Entity& e, const WorldState& ws)
        {
            auto components = EntityPrefab::Make(e, ws);

            components.Get<Ptr<Transform>>()->scale = { 10.0f, 0.00001f, 10.0f };
            components.Get<Ptr<PbrRenderable>>()->ResetModel(DemoRoom::KnownModelNames::UnitCube);
            components.Get<Ptr<PbrRenderable>>()->Color = DirectX::XMVECTORF32{ 0.15f, 0.15f, 0.15f, 1.0f };

            return components;
        }
    };

    struct Baseball : EntityPrefab<Transform, PbrRenderable, RigidBody>
    {
        static Components Make(Entity& e, const WorldState& ws)
        {
            auto components = EntityPrefab::Make(e, ws);

            components.Get<Ptr<RigidBody>>()->acceleration = PhysicsSystem::EarthGravity;
            components.Get<Ptr<PbrRenderable>>()->ResetModel(DemoRoom::KnownModelNames::Baseball);

            return components;
        }
    };

    struct Bullet : EntityPrefab<Transform, PbrRenderable, RigidBody>
    {
        static Components Make(Entity& e, const WorldState& ws)
        {
            auto components = EntityPrefab::Make(e, ws);

            components.Get<Ptr<RigidBody>>()->acceleration = PhysicsSystem::EarthGravity;
            components.Get<Ptr<PbrRenderable>>()->ResetModel(DemoRoom::KnownModelNames::UnitSphere);
            components.Get<Ptr<PbrRenderable>>()->Color = DirectX::Colors::Blue;
            components.Get<Ptr<Transform>>()->scale = winrt::Windows::Foundation::Numerics::float3{ 0.025f };

            return components;
        }
    };

    struct PaintBrush : EntityPrefab<Transform, PbrRenderable, DemoRoom::MotionControllerComponent>
    {
        static Components Make(Entity& e, const WorldState& ws)
        {
            auto components = EntityPrefab::Make(e, ws);

            components.Get<Ptr<PbrRenderable>>()->ResetModel(DemoRoom::KnownModelNames::PaintBrush);

            return components;
        }
    };

    struct Gun : EntityPrefab<Transform, PbrRenderable, DemoRoom::MotionControllerComponent>
    {
        static Components Make(Entity& e, const WorldState& ws)
        {
            auto components = EntityPrefab::Make(e, ws);

            components.Get<Ptr<PbrRenderable>>()->ResetModel(DemoRoom::KnownModelNames::Gun);

            return components;
        }
    };

    struct PaintStroke : EntityPrefab<Transform, PbrRenderable, DemoRoom::PaintStrokeComponent>
    {
        static Components Make(Entity& e, const WorldState& ws)
        {
            auto components = EntityPrefab::Make(e, ws);

            auto pbr = components.Get<Ptr<PbrRenderable>>();
            pbr->OwnedModel.Create();
            pbr->Model = pbr->OwnedModel.Get();

            return components;
        }
    };

    struct StaticSphere : EntityPrefab<Transform, PbrRenderable>
    {
        static Components Make(Entity& e, const WorldState& ws)
        {
            auto components = EntityPrefab::Make(e, ws);

            components.Get<Ptr<PbrRenderable>>()->ResetModel(DemoRoom::KnownModelNames::UnitSphere);
            components.Get<Ptr<PbrRenderable>>()->Color = DirectX::Colors::Gray;

            return components;
        }
    };

    struct StaticCube : EntityPrefab<Transform, PbrRenderable>
    {
        static Components Make(Entity& e, const WorldState& ws)
        {
            auto components = EntityPrefab::Make(e, ws);

            components.Get<Ptr<PbrRenderable>>()->ResetModel(DemoRoom::KnownModelNames::UnitCube);
            components.Get<Ptr<PbrRenderable>>()->Color = DirectX::Colors::Gray;

            return components;
        }
    };
}

