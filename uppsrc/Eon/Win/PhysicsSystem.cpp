////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "EonWin.h"

using namespace winrt::Windows::Foundation::Numerics;

namespace DemoRoom {

const float3 PhysicsSystem::EarthGravity = { 0, -9.8f, 0 };

void PhysicsSystem::Update(double dt)
{
	auto& root = GetEngine().GetRootPool();
	auto entities = root.FindAllDeep<Entity>();
	for (auto& entity : entities) {
		auto transform = entity->val.Find<Transform>();
		auto rigid_body = entity->val.Find<RigidBody>();
		if (!transform || !rigid_body)
			continue;

		rigid_body->velocity += rigid_body->acceleration * (float)dt;
		transform->position += rigid_body->velocity * (float)dt;

		const float3 adjusted_angular = winrt::Windows::Foundation::Numerics::transform(
			rigid_body->angularVelocity, inverse(transform->orientation));

		const float angle = length(adjusted_angular);
		if (angle > 0.0f) {
			const float3 axis = adjusted_angular / angle;
			transform->orientation *= make_quaternion_from_axis_angle(axis, angle * (float)dt);
		}

		rigid_body->velocity *= rigid_body->dampingFactor;
		rigid_body->angularVelocity *= rigid_body->dampingFactor;
	}
}

} // namespace DemoRoom

