////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

namespace DX {
    class DeviceResources;
    class StepTimer;
}
namespace Pbr {
    struct Resources;
}
namespace Upp {
    class Engine;
}

// Updates, renders, and presents holographic content using Direct3D.
namespace DemoRoom
{
    class DemoRoomMain
    {
    public:
        DemoRoomMain();
        ~DemoRoomMain();

        // Sets the holographic space. This is our closest analogue to setting a new window
        // for the app.
        void SetHolographicSpace(
            winrt::Windows::Graphics::Holographic::HolographicSpace const& holographicSpace);

        // Starts the holographic frame and updates the content.
        void Update();

        // Handle saving and loading of app state owned by AppMain.
        void SaveAppState();
        void LoadAppState();

    private:
        Ptr<Upp::Engine> engine;
        One<DX::DeviceResources> device_resources;
        One<Pbr::Resources> pbr_resources;
        DX::StepTimer m_timer;
    };
}

