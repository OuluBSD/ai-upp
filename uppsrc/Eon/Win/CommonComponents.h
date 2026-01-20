////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

namespace Pbr {
	struct Model;
}

namespace DemoRoom
{
	// These are a few commonly used components
	struct Transform : Component
	{
		ECS_COMPONENT_CTOR(Transform)

		winrt::Windows::Foundation::Numerics::float3 position = winrt::Windows::Foundation::Numerics::float3::zero();
		winrt::Windows::Foundation::Numerics::float3 scale = winrt::Windows::Foundation::Numerics::float3::one();
		winrt::Windows::Foundation::Numerics::quaternion orientation = winrt::Windows::Foundation::Numerics::quaternion::identity();

		void SetFromMatrix(const winrt::Windows::Foundation::Numerics::float4x4& matrix)
		{
			fail_fast_if(!decompose(matrix, &scale, &orientation, &position));
		}

		winrt::Windows::Foundation::Numerics::float4x4 GetMatrix() const
		{
			using namespace winrt::Windows::Foundation::Numerics;
			return make_float4x4_scale(scale)
				* make_float4x4_from_quaternion(orientation)
				* make_float4x4_translation(position);
		}
	};

	struct RigidBody : Component
	{
		ECS_COMPONENT_CTOR(RigidBody)

		winrt::Windows::Foundation::Numerics::float3 velocity = winrt::Windows::Foundation::Numerics::float3::zero();
		winrt::Windows::Foundation::Numerics::float3 acceleration = winrt::Windows::Foundation::Numerics::float3::zero();
		winrt::Windows::Foundation::Numerics::float3 angularVelocity = winrt::Windows::Foundation::Numerics::float3::zero();
		winrt::Windows::Foundation::Numerics::float3 angularAcceleration = winrt::Windows::Foundation::Numerics::float3::zero();

		float dampingFactor = 0.999f;
	};

	struct PbrRenderable : Component
	{
		ECS_COMPONENT_CTOR(PbrRenderable)

		void ResetModel(String name,
		                std::optional<winrt::Windows::Foundation::Numerics::float4x4> offset = std::nullopt)
		{
			ModelName = pick(name);
			Offset = std::move(offset);
			Model = nullptr;
			OwnedModel.Clear();
		}

		String ModelName;
		One<Pbr::Model> OwnedModel;
		Pbr::Model* Model = nullptr;
		std::optional<DirectX::XMVECTORF32> Color;
		std::optional<winrt::Windows::Foundation::Numerics::float4x4> Offset;
		std::optional<float> AlphaMultiplier;
	};

	struct TextRenderable : Component
	{
		ECS_COMPONENT_CTOR(TextRenderable)

		WString Text;
		float FontSize = 60.0f;
	};
}
