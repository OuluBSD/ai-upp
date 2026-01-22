////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "EonWin.h"

namespace winrt_num = winrt::Windows::Foundation::Numerics;

namespace DemoRoom {

void EasingSystem::Update(double dt)
{
	auto& root = GetEngine().GetRootPool();
	auto entities = root.FindAllDeep<Entity>();
	for (auto& entity : entities) {
		auto transform = entity->val.Find<Transform>();
		auto easing = entity->val.Find<Easing>();
		if (!transform || !easing)
			continue;

		transform->position = winrt_num::lerp(transform->position, easing->TargetPosition, easing->PositionEasingFactor);
		transform->orientation = winrt_num::slerp(transform->orientation, easing->TargetOrientation, easing->OrientationEasingFactor);
	}
}

} // namespace DemoRoom