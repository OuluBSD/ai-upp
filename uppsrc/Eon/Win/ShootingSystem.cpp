////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "EonWin.h"

#include "Haptics.h"

using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::UI::Input::Spatial;
using namespace std::chrono_literals;

namespace DemoRoom {

std::wstring_view ShootingInteractionSystem::GetInstructions() const
{
	return L"Pull the trigger to fire the gun.\n\n"
	       L"You can feel controller vibrate for each bullet.\n\n";
}

std::wstring_view ShootingInteractionSystem::GetDisplayName() const
{
	return L"Shooting";
}

EntityPtr ShootingInteractionSystem::CreateToolSelector() const
{
	auto& pool = GetEngine().GetRootPool();
	auto selector = CreatePrefab<ToolSelectorPrefab>(pool, ws_at_init);

	selector->Get<PbrRenderable>().ResetModel("Gun");
	selector->Get<ToolSelectorKey>().type = GetTypeCls();

	return selector;
}

void ShootingInteractionSystem::Register(Vector<EntityPtr> entities)
{
	ToolSystem::Register(pick(entities));

	// These values were created through trial and error and would be specific to the particular 3D model.
	const float4x4 modelToControllerRotation =
		make_float4x4_from_yaw_pitch_roll(DirectX::XMConvertToRadians(180),
		                                  DirectX::XMConvertToRadians(70),
		                                  0.0f);
	const float4x4 modelToControllerTranslation = make_float4x4_translation(0, 0.05f, 0.0f);
	const float4x4 modelToController = modelToControllerRotation * modelToControllerTranslation;

	const float4x4 barrelToController =
		make_float4x4_translation(0.0f, 0.0675f, -0.22f)
		* make_float4x4_rotation_x(DirectX::XMConvertToRadians(-70));

	auto& pool = GetEngine().GetRootPool();
	for (auto& entity : m_entities) {
		if (!entity)
			continue;

		auto gun = CreatePrefab<Gun>(pool, ws_at_init);
		gun->Get<MotionControllerComponent>().requestedHandedness =
			entity->Get<MotionControllerComponent>().requestedHandedness;
		gun->Get<PbrRenderable>().Offset = modelToController;

		auto shooting = entity->val.Find<ShootingComponent>();
		if (!shooting)
			continue;

		shooting->barrelToController = barrelToController;
		shooting->gun = gun;
		shooting->SetEnabled(false);
	}
}

void ShootingInteractionSystem::Activate(Entity& entity)
{
	ToolSystem::Activate(entity);
	entity.Get<PbrRenderable>().SetEnabled(false);
}

void ShootingInteractionSystem::Deactivate(Entity& entity)
{
	entity.Get<PbrRenderable>().SetEnabled(true);
	ToolSystem::Deactivate(entity);
}

void ShootingInteractionSystem::OnSourcePressed(const SpatialInteractionSourceEventArgs& args)
{
	if (auto enabledEntity = TryGetEntityFromSource(args.State().Source())) {
		auto shooting = (*enabledEntity).Get<1>();

		if (args.PressKind() == SpatialInteractionPressKind::Select) {
			const float4x4 barrelToWorld = shooting->barrelToController * shooting->gun->Get<Transform>().GetMatrix();

			const float3 position = float4x4_util::position(barrelToWorld);
			const quaternion orientation = make_quaternion_from_rotation_matrix(barrelToWorld);

			const float3 forward = float4x4_util::forward(barrelToWorld);
			const float3 bulletVelocity = forward * shooting->bulletSpeed;

			auto& pool = GetEngine().GetRootPool();
			auto bullet = CreatePrefab<Bullet>(pool, ws_at_init);

			auto& transform = bullet->Get<Transform>();
			transform.position = position;
			transform.orientation = orientation;
			bullet->Get<RigidBody>().velocity = bulletVelocity;

			SpatialInputUtilities::Haptics::SendContinuousBuzzForDuration(args.State().Source(), 125ms);
		}
	}
}

void ShootingInteractionSystem::OnSourceUpdated(const SpatialInteractionSourceEventArgs& args)
{
	if (auto enabledEntity = TryGetEntityFromSource(args.State().Source())) {
		auto entity = (*enabledEntity).Get<0>();
		auto shooting = (*enabledEntity).Get<1>();

		// Show the controllers while we're holding grasp, to help show how the model relates to the real world object
		const bool shouldRenderController = args.State().IsGrasped();

		entity->Get<PbrRenderable>().SetEnabled(shouldRenderController);
		shooting->gun->Get<PbrRenderable>().AlphaMultiplier =
			shouldRenderController ? std::make_optional(0.25f) : std::nullopt;
	}
}

void ShootingComponent::SetEnabled(bool enable) 
{
	Enableable::SetEnabled(enable);
	if (gun)
		gun->SetEnabled(enable);
}

void ShootingComponent::Destroy() 
{
	Destroyable::Destroy();
	if (gun)
		gun->Destroy();
}

} // namespace DemoRoom
