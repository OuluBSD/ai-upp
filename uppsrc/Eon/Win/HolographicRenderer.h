////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

namespace Pbr {
    struct Resources;
}

namespace DX { class DeviceResources; }

namespace DemoRoom
{
	class HolographicScene;
	class TextRenderer;
	class QuadRenderer;
	class SkyboxRenderer;

	////////////////////////////////////////////////////////////////////////////////
	// HolographicRenderer
	// A stereoscopic 3D rendering system, manages rendering everything in the scene
	// through DirectX 11 and Windows::Perception APIs
	class HolographicRenderer : public System, public DX::IDeviceNotify
	{
	public:
		CLASSTYPE(HolographicRenderer)
		HolographicRenderer(
			VfsValue& v,
			DX::DeviceResources& deviceResources,
			Pbr::Resources& pbrResources,
			ID3D11ShaderResourceView* skyboxTexture);

		~HolographicRenderer();

		Pbr::Resources* GetPbrResources();
		DX::DeviceResources* GetDeviceResources();

		void OnDeviceLost() override;
		void OnDeviceRestored() override;

	protected:
		bool Initialize(const WorldState& ws) override;
		bool Start() override;
		void Update(double dt) override;
		void Stop() override;
		void Uninitialize() override;

		void BindEventHandlers(
			const winrt::Windows::Graphics::Holographic::HolographicSpace& holographicSpace);

		void ReleaseEventHandlers(
			const winrt::Windows::Graphics::Holographic::HolographicSpace& holographicSpace);

	private:
		Ptr<HolographicScene> holo_scene;
		One<SkyboxRenderer> skybox_renderer;
		ArrayMap<float, One<TextRenderer>> text_renderers;
		One<QuadRenderer> quad_renderer;
		Pbr::Resources* pbr_resources = nullptr;
		DX::DeviceResources* device_resources = nullptr;

		winrt::event_token camera_added_token{};
		winrt::event_token camera_removed_token{};

		TextRenderer* GetTextRendererForFontSize(float fontSize);

		bool RenderAtCameraPose(
			DX::CameraResources *pCameraResources,
			winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const& coordinateSystem,
			winrt::Windows::Graphics::Holographic::HolographicFramePrediction& prediction,
			winrt::Windows::Graphics::Holographic::HolographicCameraRenderingParameters const& renderingParameters,
			winrt::Windows::Graphics::Holographic::HolographicCameraPose const& cameraPose);

		void OnCameraAdded(
			winrt::Windows::Graphics::Holographic::HolographicSpace const& sender,
			winrt::Windows::Graphics::Holographic::HolographicSpaceCameraAddedEventArgs const& args);

		void OnCameraRemoved(
			winrt::Windows::Graphics::Holographic::HolographicSpace const& sender,
			winrt::Windows::Graphics::Holographic::HolographicSpaceCameraRemovedEventArgs const& args);
	};
}
