////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "EonWin.h"
#include <cmath>

#include "ControllerRendering.h"
#include "Haptics.h"
#include "PbrModel.h"

using namespace DirectX;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::UI::Input::Spatial;
using namespace std::literals::chrono_literals;

namespace DemoRoom {

bool PaintingInteractionSystem::Start()
{
	return ToolSystem::Start();
}

void PaintingInteractionSystem::Stop()
{
	ToolSystem::Stop();
}

std::wstring_view PaintingInteractionSystem::GetInstructions() const 
{
	return L"Press and hold trigger to paint.\n\n"
	       L"Touch and press touchpad to choose brush color.\n\n"
	       L"Hold grasp button to move strokes around. While holding grasp tilt thumbstick forward/backward to translate strokes.\n\n"
	       L"Push thumbstick down to delete strokes.\n\n";
}

std::wstring_view PaintingInteractionSystem::GetDisplayName() const
{
	return L"Painting";
}

EntityPtr PaintingInteractionSystem::CreateToolSelector() const
{
	auto& pool = GetEngine().GetRootPool();
	auto selector = CreatePrefab<ToolSelectorPrefab>(pool, ws_at_init);

	selector->Get<PbrRenderable>().ResetModel("PaintBrush");
	selector->Get<Transform>().orientation = make_quaternion_from_axis_angle({ 1, 0, 0 }, DirectX::XM_PI / 1.5f);
	selector->Get<ToolSelectorKey>().type = GetTypeCls();

	return selector;
}

void PaintingInteractionSystem::Register(Vector<EntityPtr> entities)
{
	ToolSystem::Register(pick(entities));

	auto& pool = GetEngine().GetRootPool();

	for (auto& entity : m_entities) {
		if (!entity)
			continue;

		const auto& selectedColor = m_colors[0];

		auto paintBrush = CreatePrefab<PaintBrush>(pool, ws_at_init);
		paintBrush->Get<PbrRenderable>().Color = selectedColor;

		paintBrush->Get<MotionControllerComponent>().requestedHandedness =
			entity->Get<MotionControllerComponent>().requestedHandedness;

		auto touchpadIndicator = CreatePrefab<StaticSphere>(pool, ws_at_init);
		touchpadIndicator->Get<Transform>().scale = { 0.005f, 0.005f, 0.005f };
		touchpadIndicator->Get<PbrRenderable>().Color = DirectX::Colors::Gray;

		Vector<EntityPtr> color_picker_objects;
		for (auto color : m_colors) {
			auto colorPicker = CreatePrefab<StaticSphere>(pool, ws_at_init);
			colorPicker->Get<Transform>().scale = { 0.01f, 0.01f, 0.01f };
			colorPicker->Get<PbrRenderable>().Color = color;
			color_picker_objects.Add(colorPicker);
		}

		auto beam = CreatePrefab<StaticCube>(pool, ws_at_init);
		beam->Get<Transform>().scale = { 0.005f, 0.005f, 10.0f };
		beam->Get<PbrRenderable>().Color = DirectX::Colors::Aquamarine;

		auto paint = entity->val.Find<PaintComponent>();
		if (!paint)
			continue;

		paint->selectedColor = selectedColor;
		paint->paintBrush = paintBrush;
		paint->touchpadIndicator = touchpadIndicator;
		paint->colorPickerObjects = pick(color_picker_objects);
		paint->beam = beam;
		paint->SetEnabled(false);
	}
}

void PaintingInteractionSystem::Activate(Entity& entity)
{
	ToolSystem::Activate(entity);
	entity.Get<PbrRenderable>().SetEnabled(false);
}

void PaintingInteractionSystem::Deactivate(Entity& entity)
{
	entity.Get<PbrRenderable>().SetEnabled(true);

	auto paint = entity.val.Find<PaintComponent>();
	if (!paint)
		return;

	// Copy out the strokes from the component so they can persist in the world.
	if (paint->strokeInProgress) {
		paint->strokes.Add(paint->strokeInProgress);
		paint->strokeInProgress = nullptr;
	}

	if (!paint->strokes.IsEmpty())
		m_persistentStrokes.Add(pick(paint->strokes));

	ToolSystem::Deactivate(entity);
}

void PaintingInteractionSystem::OnSourcePressed(const SpatialInteractionSourceEventArgs& args)
{
	if (args.PressKind() == SpatialInteractionPressKind::Thumbstick) {
		// Destroy all the paint strokes currently active
		for (auto& enabledEntity : GetEnabledEntities()) {
			auto entity = enabledEntity.Get<0>();
			auto paint = enabledEntity.Get<1>();

			for (auto& stroke : paint->strokes) {
				if (stroke)
					stroke->Destroy();
			}

			paint->strokes.Clear();

			if (paint->strokeInProgress) {
				paint->strokeInProgress->Destroy();
				paint->strokeInProgress = nullptr;
			}
		}

		// Destroy all the persistent strokes
		for (auto& strokeGroup : m_persistentStrokes) {
			for (auto& stroke : strokeGroup) {
				if (stroke)
					stroke->Destroy();
			}
		}

		m_persistentStrokes.Clear();
	}
}

void PaintingInteractionSystem::OnSourceUpdated(const SpatialInteractionSourceEventArgs& args)
{
	const auto& sourceState = args.State();
	const auto& source = sourceState.Source();

	if (auto enabledEntity = TryGetEntityFromSource(source)) {
		bool newStrokeStarted = false;
		auto entity = (*enabledEntity).Get<0>();
		auto paint = (*enabledEntity).Get<1>();

		auto paintBrushModel = paint->paintBrush ? paint->paintBrush->Get<PbrRenderable>().Model : nullptr;
		if (paintBrushModel && !paint->brushTipOffsetFromHoldingPose) {
			std::optional<Pbr::NodeIndex_t> touchNode = paintBrushModel->FindFirstNode("PaintTip");
			if (touchNode) {
				// Calculate paint tip offset from holding pose
				const auto brushTipWorldTransform = paintBrushModel->GetNodeWorldTransform(touchNode.value());
				const auto paintBrushWorldTransform = paintBrushModel->GetNode(Pbr::RootNodeIndex).GetTransform();
				paint->brushTipOffsetFromHoldingPose =
					brushTipWorldTransform * XMMatrixInverse(nullptr, paintBrushWorldTransform);
			}
		}

		const auto& controller = entity->Get<MotionControllerComponent>();
		if (controller.IsSource(source)) {
			const auto& controllerProperties = sourceState.ControllerProperties();

			paint->touchpadX = static_cast<float>(controllerProperties.TouchpadX());
			paint->touchpadY = static_cast<float>(controllerProperties.TouchpadY());

			paint->thumbstickX = static_cast<float>(controllerProperties.ThumbstickX());
			paint->thumbstickY = static_cast<float>(controllerProperties.ThumbstickY());

			if (paint->currentState == PaintComponent::State::Idle) {
				if (sourceState.IsSelectPressed()) {
					paint->currentState = PaintComponent::State::Painting;
					newStrokeStarted = true;
				}
				else if (sourceState.IsGrasped()) {
					paint->currentState = PaintComponent::State::Manipulating;
				}
				else if (controllerProperties.IsTouchpadTouched()) {
					paint->currentState = PaintComponent::State::ColorSelection;
				}
			}
			else if (paint->currentState == PaintComponent::State::Painting) {
				if (!sourceState.IsSelectPressed())
					paint->currentState = PaintComponent::State::Idle;
			}
			else if (paint->currentState == PaintComponent::State::Manipulating) {
				if (!sourceState.IsGrasped()) {
					paint->currentState = PaintComponent::State::Idle;
					paint->previousManipulationLocation = nullptr;
				}
			}
			else if (paint->currentState == PaintComponent::State::ColorSelection) {
				if (!paint->waitForTouchpadRelease) {
					if (controllerProperties.IsTouchpadPressed()) {
						paint->waitForTouchpadRelease = true;
						paint->selectedColor = SelectColor(paint->touchpadX, paint->touchpadY);
						SpatialInputUtilities::Haptics::SendContinuousBuzzForDuration(
							sourceState.Source(), 100ms);
					}
				}

				if (!controllerProperties.IsTouchpadPressed())
					paint->waitForTouchpadRelease = false;

				if (!controllerProperties.IsTouchpadTouched())
					paint->currentState = PaintComponent::State::Idle;
			}

			if (paint->currentState == PaintComponent::State::Painting) {
				// Start new stroke
				if (newStrokeStarted) {
					auto& pool = GetEngine().GetRootPool();
					paint->strokeInProgress = CreatePrefab<PaintStroke>(pool, ws_at_init);
					paint->strokeInProgress->Get<PbrRenderable>().Color = paint->selectedColor;
					paint->strokes.Add(paint->strokeInProgress);
				}

				auto properties = sourceState.Properties();

				// We generate stroke points in source updated using the arguments provided by the event
				// This will result in a smoother paint stroke
				if (auto location = properties.TryGetLocation(
						GetEngine().Get<HolographicScene>()->WorldCoordinateSystem())) {
					if (paint->brushTipOffsetFromHoldingPose && paint->strokeInProgress) {
						float4x4 paintToWorld;
						XMStoreFloat4x4(
							&paintToWorld,
							*paint->brushTipOffsetFromHoldingPose *
								XMLoadFloat4x4(&location_util::matrix(location)));

						paint->strokeInProgress->Get<PaintStrokeComponent>().AddPoint(
							float4x4_util::remove_scale(paintToWorld), PaintTipThickness);
					}
				}
			}
		}
	}
}

void PaintingInteractionSystem::OnSourceReleased(const SpatialInteractionSourceEventArgs& args)
{
	(void)args;
}

void PaintingInteractionSystem::Update(double dt)
{
	for (auto& enabledEntity : GetEnabledEntities()) {
		auto entity = enabledEntity.Get<0>();
		auto paint = enabledEntity.Get<1>();

		const auto& controller = entity->Get<MotionControllerComponent>();

		paint->beam->Get<PbrRenderable>().SetEnabled(
			paint->currentState == PaintComponent::State::Manipulating);

		// Set properties required for rendering
		paint->touchpadIndicator->Get<PbrRenderable>().SetEnabled(
			paint->currentState == PaintComponent::State::ColorSelection);
		for (auto& go : paint->colorPickerObjects) {
			if (go)
				go->Get<PbrRenderable>().SetEnabled(
					paint->currentState == PaintComponent::State::ColorSelection);
		}

		const bool showController = paint->currentState == PaintComponent::State::Manipulating;
		entity->Get<PbrRenderable>().SetEnabled(showController);
		paint->paintBrush->Get<PbrRenderable>().SetEnabled(!showController);

		if (auto location = controller.location) {
			const float3 position = location_util::position(location);
			const quaternion orientation = location_util::orientation(location);

			const DirectX::XMVECTORF32 paintTipColor =
				paint->currentState == PaintComponent::State::ColorSelection
					? SelectColor(paint->touchpadX, paint->touchpadY)
					: paint->selectedColor;
			paint->paintBrush->Get<PbrRenderable>().Color = paintTipColor;

			if (paint->currentState == PaintComponent::State::Manipulating) {
				// Update the paint strokes based on the change in location
				if (paint->previousManipulationLocation) {
					const float3 previousPosition = location_util::position(paint->previousManipulationLocation);
					const quaternion previousOrientation = location_util::orientation(paint->previousManipulationLocation);

					const quaternion orientationDelta = orientation * inverse(previousOrientation);

					const float4x4 manipulationTransform =
						make_float4x4_translation(-previousPosition)
						* make_float4x4_from_quaternion(orientationDelta)
						* make_float4x4_translation(position);

					for (auto& stroke : paint->strokes) {
						if (stroke) {
							auto& strokeTransform = stroke->Get<Transform>();
							strokeTransform.SetFromMatrix(strokeTransform.GetMatrix() * manipulationTransform);
						}
					}
				}

				paint->previousManipulationLocation = location;

				// Move the paint strokes based on manipulation changes
				constexpr double ThumbstickMovementThresholdPercent = 0.2f;
				constexpr float MovementSpeedInMetersPerSecond = 2.5f;

				if (auto pointerPose = location.SourcePointerPose()) {
					const float3 pos = pointerPose.Position();
					const float3 forward = pointerPose.ForwardDirection();

					if (fabs(paint->thumbstickY) > ThumbstickMovementThresholdPercent) {
						const float3 forwardMovement =
							forward * paint->thumbstickY * MovementSpeedInMetersPerSecond * (float)dt;

						// Move all paintings along beam path
						for (auto& stroke : paint->strokes) {
							if (stroke)
								stroke->Get<Transform>().position += forwardMovement;
						}
					}

					auto& beamTransform = paint->beam->Get<Transform>();
					beamTransform.position = pos + forward * (beamTransform.scale.z * 0.5f);
					beamTransform.orientation = pointerPose.Orientation();
				}
			}
			else if (paint->currentState == PaintComponent::State::ColorSelection) {
				constexpr float colorpickerDiameter = 0.025f;
				constexpr float colorpickerHeight = 0.015f;

				const float4x4 paintBrushToWorld = paint->paintBrush->Get<Transform>().GetMatrix();

				const float3 touchpadIndicatorOnPaintBrush =
					{ paint->touchpadX * colorpickerDiameter, colorpickerHeight,
					  paint->touchpadY * colorpickerDiameter * -1 };
				const float3 touchpadIndicatorInWorld = transform(touchpadIndicatorOnPaintBrush, paintBrushToWorld);

				paint->touchpadIndicator->Get<Transform>().position = touchpadIndicatorInWorld;

				// Color picker plane defined as slightly above the touchpad with the same orientation as the touchpad
				const int numColors = paint->colorPickerObjects.GetCount();

				for (int i = 0; i < numColors; ++i) {
					const float angle =
						(static_cast<float>(i * -1 - 1) / static_cast<float>(numColors))
							* (2 * DirectX::XM_PI)
						- DirectX::XM_PI;
					const float nextAngle =
						(static_cast<float>((i + 1) * -1 - 1) / static_cast<float>(numColors))
							* (2 * DirectX::XM_PI)
						- DirectX::XM_PI;
					const float angleDelta = (nextAngle - angle) / 2;
					const float finalAngle = angle - angleDelta;

					const float3 colorIndicatorOnPaintBrush =
						{ cos(finalAngle) * colorpickerDiameter, colorpickerHeight,
						  sin(finalAngle) * colorpickerDiameter };
					const float3 colorIndicatorInWorld = transform(colorIndicatorOnPaintBrush, paintBrushToWorld);

					if (paint->colorPickerObjects[i])
						paint->colorPickerObjects[i]->Get<Transform>().position = colorIndicatorInWorld;
				}
			}
		}
	}
}

DirectX::XMVECTORF32 PaintingInteractionSystem::SelectColor(double x, double y)
{
	if (x == 0 && y == 0)
		return m_colors.back();

	constexpr double min = -DirectX::XM_PI;
	constexpr double max = +DirectX::XM_PI;
	const double angle = std::atan2(y, x);
	const int index = static_cast<int>(std::round((angle - min) / (max - min) * (m_colors.size() - 1)));
	return m_colors[index];
}

void PaintComponent::SetEnabled(bool enable) 
{
	Enableable::SetEnabled(enable);

	for (auto& colorPicker : colorPickerObjects) {
		if (colorPicker)
			colorPicker->SetEnabled(enable);
	}

	if (touchpadIndicator)
		touchpadIndicator->SetEnabled(enable);

	if (paintBrush)
		paintBrush->SetEnabled(enable);

	if (beam)
		beam->SetEnabled(enable);
}

void PaintComponent::Destroy() 
{
	Destroyable::Destroy();

	for (auto& colorPicker : colorPickerObjects) {
		if (colorPicker)
			colorPicker->Destroy();
	}

	if (touchpadIndicator)
		touchpadIndicator->Destroy();

	if (paintBrush)
		paintBrush->Destroy();

	if (beam)
		beam->Destroy();
}

} // namespace DemoRoom
