////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "EonWin.h"

namespace DemoRoom {

ToolSelectorPrefab::Components ToolSelectorPrefab::Make(Entity& e, const WorldState& ws)
{
    auto components = e.CreateComponents<Transform, PbrRenderable, ToolSelectorKey, RigidBody, Easing>(ws);

    components.Get<Ptr<RigidBody>>()->angularVelocity = { 0.0f, -3.0f, 0.0f }; // Spin in place
    components.Get<Ptr<RigidBody>>()->dampingFactor = 1.0f;
    components.Get<Ptr<Easing>>()->PositionEasingFactor = 0.1f;

    return components;
}

} // namespace DemoRoom
