////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "EonWin.h"

#include "PbrModel.h"
#include "Physics.h"

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::UI::Input::Spatial;

namespace DemoRoom {

constexpr float BallHoldingDistance = 0.075f;

std::wstring_view ThrowingInteractionSystem::GetInstructions() const
{
	return L"Press and hold trigger to spawn a baseball.\n\n"
	       L"Release trigger to throw the baseball.";
}

std::wstring_view ThrowingInteractionSystem::GetDisplayName() const
{
	return L"Throwing";
}

EntityPtr ThrowingInteractionSystem::CreateToolSelector() const
{
	auto& pool = GetEngine().GetRootPool();
	auto selector = CreatePrefab<ToolSelectorPrefab>(pool, ws_at_init);

	selector->Get<PbrRenderable>().ResetModel("Baseball");
	selector->Get<ToolSelectorKey>().type = GetTypeCls();

	return selector;
}

void ThrowingInteractionSystem::Update(double dt)
{
	for (auto& enabledEntity : GetEnabledEntities()) {
		auto entity = enabledEntity.Get<0>();
		auto throwing = enabledEntity.Get<1>();

		if (throwing->ballObject) {
			if (const SpatialInteractionSourceLocation location =
					entity->Get<MotionControllerComponent>().location) {
				if (const SpatialPointerInteractionSourcePose pointerPose = location.SourcePointerPose()) {
					auto& transform = throwing->ballObject->Get<Transform>();
					transform.position = pointerPose.Position() + pointerPose.ForwardDirection() * BallHoldingDistance;
					transform.orientation = pointerPose.Orientation();

					if (transform.scale.x < 1.0f)
						transform.scale += float3{ 2.0f * (float)dt };
				}
			}
		}
	}
}

void ThrowingInteractionSystem::OnSourcePressed(const SpatialInteractionSourceEventArgs& args)
{
	if (args.PressKind() == SpatialInteractionPressKind::Select) {
		if (auto enabledEntity = TryGetEntityFromSource(args.State().Source())) {
			auto throwing = (*enabledEntity).Get<1>();

			auto& pool = GetEngine().GetRootPool();
			auto ball = CreatePrefab<Baseball>(pool, ws_at_init);
			ball->Get<Transform>().scale = float3{ throwing->scale };
			ball->Get<RigidBody>().SetEnabled(false);
			throwing->ballObject = ball;
		}
	}
}

void ThrowingInteractionSystem::OnSourceReleased(const SpatialInteractionSourceEventArgs& args)
{
	if (args.PressKind() == SpatialInteractionPressKind::Select) {
		if (auto enabledEntity = TryGetEntityFromSource(args.State().Source())) {
			auto throwing = (*enabledEntity).Get<1>();
			fail_fast_if(!throwing);

			if (throwing->ballObject) {
				// We no longer need to keep a reference to the thrown ball.
				auto ball = throwing->ballObject;
				throwing->ballObject = nullptr;

				// If the controller has no motion, release the ball with no initial velocity.
				auto& rigid = ball->Get<RigidBody>();
				rigid.SetEnabled(true);
				rigid.velocity = {};
				rigid.angularVelocity = {};

				// If controller has motion, use velocity and angular velocity at ball's holding distances.
				const SpatialCoordinateSystem coordinateSystem =
					GetEngine().Get<HolographicScene>()->WorldCoordinateSystem();
				if (const SpatialInteractionSourceLocation graspLocation =
						args.State().Properties().TryGetLocation(coordinateSystem)) {
					if (const SpatialPointerInteractionSourcePose pointerPose = graspLocation.SourcePointerPose()) {
						if (const IReference<float3> graspAngularVelocity = graspLocation.AngularVelocity()) {
							const float3 ballPosition =
								pointerPose.Position() + (pointerPose.ForwardDirection() * BallHoldingDistance);

							if (const std::optional<float3> ballVelocity =
									SpatialInputUtilities::Physics::GetVelocityNearSourceLocation(graspLocation, ballPosition)) {
								auto& transform = ball->Get<Transform>();
								transform.position = ballPosition;
								transform.orientation = pointerPose.Orientation();
								rigid.velocity = ballVelocity.value();
								rigid.angularVelocity = graspAngularVelocity.Value();
							}
						}
					}
				}
			}
		}
	}
}

void ThrowingComponent::SetEnabled(bool enable) 
{
	Enableable::SetEnabled(enable);
	if (ballObject)
		ballObject->SetEnabled(enable);
}

void ThrowingComponent::Destroy() 
{
	Destroyable::Destroy();
	if (ballObject)
		ballObject->Destroy();
}

} // namespace DemoRoom
