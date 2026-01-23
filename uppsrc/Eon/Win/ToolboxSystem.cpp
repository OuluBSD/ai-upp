////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "EonWin.h"
#include <cmath>
#include <numeric>

using namespace DemoRoom;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::UI::Input::Spatial;

struct MotionControllerPrefab : EntityPrefab<Transform, PbrRenderable, MotionControllerComponent, ToolComponent>
{};

struct TextDisplay : EntityPrefab<Transform, TextRenderable>
{};

static const WString InstructionalText =
	L"Press the menu button to bring interaction objects toward you.\n\n"
	L"Grasp (grasp button) an interaction object to use it.";

namespace {
	bool HitTest(float3 positionA, float3 positionB, float diameter)
	{
		auto distance = length(positionA - positionB);
		return distance < diameter;
	}

	WString ToWString(std::wstring_view text)
	{
		return WString(text.data(), (int)text.size());
	}
}

bool ToolboxSystem::Initialize(const WorldState& ws)
{
	ws_at_init = ws;
	return true;
}

void ToolboxSystem::AddToolSystem(ToolSystemBase& system)
{
	if (m_controllers[Left].Controller && m_controllers[Right].Controller) {
		Vector<EntityPtr> entities;
		entities << m_controllers[Left].Controller << m_controllers[Right].Controller;
		system.Register(pick(entities));
	}

	m_selectorObjects.GetAdd(system.GetTypeCls()) = system.CreateToolSelector();
	m_selectors.GetAdd(system.GetTypeCls()) = &system;

	for (auto& context : m_controllers) {
		if (context.Controller)
			SwitchToolType(*context.Controller, system.GetTypeCls());
	}
}

void ToolboxSystem::RemoveToolSystem(ToolSystemBase& system)
{
	m_selectors.RemoveKey(system.GetTypeCls());
	m_selectorObjects.RemoveKey(system.GetTypeCls());
	system.Unregister();
}

bool ToolboxSystem::Start()
{
	auto& pool = GetEngine().GetRootPool();

	for (size_t i = 0; i < m_controllers.size(); ++i) {
		const ControllerHand hand = static_cast<ControllerHand>(i);

		m_controllers[i].Hand = hand;
		m_controllers[i].Controller = CreatePrefab<MotionControllerPrefab>(pool, ws_at_init);

		if (auto mc = m_controllers[i].Controller->val.Find<MotionControllerComponent>()) {
			mc->requestedHandedness = ControllerHandToHandedness(hand);
			mc->attachControllerModel = true;
		}
	}

	m_instructionalText = CreatePrefab<TextDisplay>(pool, ws_at_init);
	m_instructionalText->Get<TextRenderable>().Text = InstructionalText;
	m_instructionalText->Get<Transform>().position = { 0, 1.5f, -5.f };
	m_instructionalText->Get<Transform>().scale = float3{ 2.0f };

	m_controllers[Left].DebugText = CreatePrefab<TextDisplay>(pool, ws_at_init);
	m_controllers[Left].DebugText->Get<Transform>().position = { -2.5, 1.25f, -4.f };
	m_controllers[Left].DebugText->Get<Transform>().orientation =
		make_quaternion_from_axis_angle({ 0, 1, 0 }, DirectX::XM_PI * 0.15f);
	m_controllers[Left].DebugText->Get<Transform>().scale = float3{ 2.0f };
	m_controllers[Left].DebugText->Get<TextRenderable>().FontSize = 52.0f;

	m_controllers[Right].DebugText = CreatePrefab<TextDisplay>(pool, ws_at_init);
	m_controllers[Right].DebugText->Get<Transform>().position = { 2.5, 1.25f, -4.f };
	m_controllers[Right].DebugText->Get<Transform>().orientation =
		make_quaternion_from_axis_angle({ 0, 1, 0 }, -DirectX::XM_PI * 0.15f);
	m_controllers[Right].DebugText->Get<Transform>().scale = float3{ 2.0f };
	m_controllers[Right].DebugText->Get<TextRenderable>().FontSize = 52.0f;

	GetEngine().Get<SpatialInteractionSystem>()->AddListener(this);
	return true;
}

void ToolboxSystem::Stop()
{
	GetEngine().Get<SpatialInteractionSystem>()->RemoveListener(this);
}

void ToolboxSystem::Update(double dt)
{
	static double fps[32] = {};
	static uint32_t curr_fps = 0;
	fps[curr_fps++] = dt;
	curr_fps %= _countof(fps);

	const double avg_dt = std::accumulate(std::begin(fps), std::end(fps), 0.0) / _countof(fps);
	WString fps_text(std::to_wstring(static_cast<int>(std::round(1.0 / avg_dt))).c_str());
	m_instructionalText->Get<TextRenderable>().Text =
		fps_text + WString(L" FPS\n\n") + InstructionalText;

	if (!m_showToolbox) {
		int count = m_selectorObjects.GetCount();
		if (count > 0) {
			for (int i = 0; i < count; ++i) {
				EntityPtr selector = m_selectorObjects[i];
				if (!selector)
					continue;

				const float offset = (i - floorf(count / 2.f)) / count;
				selector->Get<Easing>().TargetPosition = float3{ offset, 1.25f, -5.f };
			}
		}

		// Update the debug text for each Controller based on the currently selected tool
		for (size_t i = 0; i < m_controllers.size(); ++i) {
			WString displayed_text = ControllerHandToString(m_controllers[i].Hand);
			displayed_text += WString(L": ");

			if (auto tool = m_controllers[i].Controller->val.Find<ToolComponent>()) {
				displayed_text += tool->title;
				displayed_text += WString(L"\n\n");
				displayed_text += tool->description;
			}

			m_controllers[i].DebugText->Get<TextRenderable>().Text = displayed_text;
		}
	}
	else {
		auto& root = GetEngine().GetRootPool();
		auto entities = root.FindAllDeep<Entity>();

		for (size_t i = 0; i < m_controllers.size(); ++i) {
			WString displayed_text = ControllerHandToString(m_controllers[i].Hand);
			displayed_text += WString(L" switch to: ");

			const float3 controller_position = m_controllers[i].Controller->Get<Transform>().position;

			for (auto& entity : entities) {
				auto transform = entity->val.Find<Transform>();
				auto selector = entity->val.Find<ToolSelectorKey>();
				if (!transform || !selector)
					continue;

				if (HitTest(controller_position, transform->position, 0.15f)) {
					int idx = m_selectors.Find(selector->type);
					if (idx >= 0) {
						displayed_text += ToWString(m_selectors[idx]->GetDisplayName());
						displayed_text += WString(L"\n\n");
						displayed_text += ToWString(m_selectors[idx]->GetInstructions());
					}
				}
			}

			m_controllers[i].DebugText->Get<TextRenderable>().Text = displayed_text;
		}
	}
}

void ToolboxSystem::OnSourcePressed(const SpatialInteractionSourceEventArgs& args)
{
	if (args.State().Source().Kind() != SpatialInteractionSourceKind::Controller)
		return;

	EntityPtr controller = FindController(args.State().Source());
	if (!controller)
		return;

	// Bring the toolbox in front of user
	if (args.PressKind() == SpatialInteractionPressKind::Menu) {
		m_showToolbox = !m_showToolbox;
		if (m_showToolbox) {
			auto holoScene = GetEngine().TryGet<HolographicScene>();
			if (!holoScene)
				return;
			if (SpatialPointerPose pointerPose =
				SpatialPointerPose::TryGetAtTimestamp(holoScene->WorldCoordinateSystem(),
				                                      holoScene->CurrentTimestamp())) {
				const float3 headPosition = pointerPose.Head().Position();
				const float3 forward = pointerPose.Head().ForwardDirection();
				const float3 headUp = pointerPose.Head().UpDirection();

				const float3 headDirection = normalize(float3{ forward.x, 0.0f, forward.z });

				float3 headRight = cross(headDirection, headUp);
				headRight.y = 0;
				headRight = normalize(headRight);

				const float3 toolkitCenter = headDirection * 0.5f;

				int count = m_selectorObjects.GetCount();
				if (count > 0) {
					for (int i = 0; i < count; ++i) {
						EntityPtr selector = m_selectorObjects[i];
						if (!selector)
							continue;

						const float offset = (i - floorf(count / 2.f)) / count;
						const float3 targetPosition =
							toolkitCenter + headPosition + headRight * offset + float3{ 0, -0.3f, 0 };
						selector->Get<Easing>().TargetPosition = targetPosition;
					}
				}
			}
		}
	}
	else if (args.PressKind() == SpatialInteractionPressKind::Grasp && m_showToolbox) {
		if (auto location = controller->Get<MotionControllerComponent>().location) {
			if (location.Position()) {
				const float3 position = location.Position().Value();

				auto& root = GetEngine().GetRootPool();
				auto entities = root.FindAllDeep<Entity>();

				for (auto& entity : entities) {
					auto transform = entity->val.Find<Transform>();
					auto selector = entity->val.Find<ToolSelectorKey>();
					if (!transform || !selector)
						continue;

					if (HitTest(position, transform->position, 0.15f)) {
						SwitchToolType(*controller, selector->type);
						m_showToolbox = false;
						break;
					}
				}
			}
		}
	}
}

WString ToolboxSystem::ControllerHandToString(ControllerHand hand)
{
	return hand == Left ? WString(L"Left") : WString(L"Right");
}

SpatialInteractionSourceHandedness ToolboxSystem::ControllerHandToHandedness(ControllerHand hand)
{
	return hand == Left ? SpatialInteractionSourceHandedness::Left : SpatialInteractionSourceHandedness::Right;
}

void ToolboxSystem::SwitchToolType(Entity& entity, const TypeCls& new_type)
{
	ToolComponent* tool = entity.val.Find<ToolComponent>();
	if (!tool)
		return;

	// Disable old tool
	int old_idx = m_selectors.Find(tool->tool_type);
	if (old_idx >= 0)
		m_selectors[old_idx]->Deactivate(entity);

	// Enable new tool
	int new_idx = m_selectors.Find(new_type);
	if (new_idx >= 0) {
		auto sys = m_selectors[new_idx];
		sys->Activate(entity);
		tool->tool_type = sys->GetTypeCls();
		tool->description = ToWString(sys->GetInstructions());
		tool->title = ToWString(sys->GetDisplayName());
	}
}

EntityPtr ToolboxSystem::FindController(const SpatialInteractionSource& source)
{
	for (auto& context : m_controllers) {
		if (!context.Controller)
			continue;
		if (auto mc = context.Controller->val.Find<MotionControllerComponent>()) {
			if (mc->IsSource(source))
				return context.Controller;
		}
	}

	return nullptr;
}
