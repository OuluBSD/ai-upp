////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

namespace Pbr {
	struct PrimitiveBuilder;
	struct Resources;
}

namespace DemoRoom 
{
    struct PaintStrokeComponent : Component
    {
        ECS_COMPONENT_CTOR(PaintStrokeComponent)

        struct Square
        {
            winrt::Windows::Foundation::Numerics::float3 TopLeft, TopRight, BottomLeft, BottomRight;
        };

        std::vector<Square> squares;
        bool strokeChanged{ true };

        void AddPoint(const winrt::Windows::Foundation::Numerics::float4x4& transformationMatrix, float width);
        Pbr::PrimitiveBuilder GetPrimitiveData();
    };

    ////////////////////////////////////////////////////////////////////////////////
    // PaintStrokeSystem
    // This System manages the PaintStrokeComponents and automatically generates the 3D mesh for each stroke
    class PaintStrokeSystem : public System
    {
    public:
        CLASSTYPE(PaintStrokeSystem)
        PaintStrokeSystem(VfsValue& v, Pbr::Resources& pbr_resources);
        ~PaintStrokeSystem() = default;

    protected:
        void Update(double dt) override;

    private:
        Pbr::Resources* pbr_resources = nullptr;
    };

}
