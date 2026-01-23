////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

namespace DemoRoom
{
	struct Easing : Component
	{
		ECS_COMPONENT_CTOR(Easing)

		winrt::Windows::Foundation::Numerics::float3 TargetPosition{ 0,0,0 };
		winrt::Windows::Foundation::Numerics::quaternion TargetOrientation =
			winrt::Windows::Foundation::Numerics::quaternion::identity();
		float PositionEasingFactor = 0;
		float OrientationEasingFactor = 0;
	};

	////////////////////////////////////////////////////////////////////////////////
	// EasingSystem
	// Manages the Easing component, which allows objects with a Transform component to be
	// interpolated to new position/orientations.
	class EasingSystem : public System
	{
	public:
		SYS_CTOR(EasingSystem)

	protected:
		void Update(double dt) override;
	};
}
