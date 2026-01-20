////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "EonWin.h"
#include <limits>
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Windows::Perception;
using namespace winrt::Windows::Perception::Spatial;
using namespace DirectX;

namespace DemoRoom {


////////////////////////////////////////////////////////////////////////////////
// Holographic Renderer 
////////////////////////////////////////////////////////////////////////////////
HolographicRenderer::HolographicRenderer(
    VfsValue& v,
    DX::DeviceResources& deviceResources,
    Pbr::Resources& pbrResources,
    ID3D11ShaderResourceView* skyboxTexture) :
    System(v),
    pbr_resources(&pbrResources),
    device_resources(&deviceResources)
{
    quad_renderer.Create(deviceResources);
    skybox_renderer.Create(deviceResources, skyboxTexture);
}

HolographicRenderer::~HolographicRenderer() = default;

Pbr::Resources* HolographicRenderer::GetPbrResources()
{
    fail_fast_if(pbr_resources == nullptr);
    return pbr_resources;
}

DX::DeviceResources* HolographicRenderer::GetDeviceResources()
{
    fail_fast_if(device_resources == nullptr);
    return device_resources;
}

void HolographicRenderer::OnDeviceLost()
{
    pbr_resources->ReleaseDeviceDependentResources();
    for (int i = 0; i < text_renderers.GetCount(); ++i)
        text_renderers[i]->ReleaseDeviceDependentResources();
    quad_renderer->ReleaseDeviceDependentResources();
    skybox_renderer->ReleaseDeviceDependentResources();
}

void HolographicRenderer::OnDeviceRestored()
{
    pbr_resources->CreateDeviceDependentResources(device_resources->GetD3DDevice());
    for (int i = 0; i < text_renderers.GetCount(); ++i)
        text_renderers[i]->CreateDeviceDependentResources();
    quad_renderer->CreateDeviceDependentResources();
    skybox_renderer->CreateDeviceDependentResources();
}

bool HolographicRenderer::Initialize(const WorldState&)
{
    device_resources->RegisterDeviceNotify(this);
    pbr_resources->SetLight(XMVECTORF32{ 0.0f, 0.7071067811865475f, -0.7071067811865475f }, DirectX::Colors::White);
    return true;
}

void HolographicRenderer::Uninitialize()
{
    device_resources->RegisterDeviceNotify(nullptr);
}

bool HolographicRenderer::Start()
{
    holo_scene = GetEngine().Get<HolographicScene>();
    BindEventHandlers(holo_scene->HolographicSpace());
    return true;
}

void HolographicRenderer::Stop()
{
    ReleaseEventHandlers(holo_scene->HolographicSpace());
    holo_scene = nullptr;
}

void HolographicRenderer::Update(double)
{
    auto holographicFrame = holo_scene->CurrentFrame();

    device_resources->EnsureCameraResources(holographicFrame, holographicFrame.CurrentPrediction());

    const bool shouldPresent = device_resources->UseHolographicCameraResources<bool>(
        [this, holographicFrame](std::map<UINT32, std::unique_ptr<DX::CameraResources>>& cameraResourceMap)
    {
        // Up-to-date frame predictions enhance the effectiveness of image stablization and
        // allow more accurate positioning of holograms.
        holo_scene->UpdateCurrentPrediction();

        HolographicFramePrediction prediction = holographicFrame.CurrentPrediction();
        SpatialCoordinateSystem coordinateSystem = holo_scene->WorldCoordinateSystem();

        bool atLeastOneCameraRendered = false;
        for (HolographicCameraPose const& cameraPose : prediction.CameraPoses())
        {
            // This represents the device-based resources for a HolographicCamera.
            DX::CameraResources* pCameraResources = cameraResourceMap[cameraPose.HolographicCamera().Id()].get();

            if (RenderAtCameraPose(pCameraResources, coordinateSystem, prediction, holographicFrame.GetRenderingParameters(cameraPose), cameraPose))
            {
                atLeastOneCameraRendered = true;
            }
        }

        return atLeastOneCameraRendered;
    });

    if (shouldPresent)
    {
        device_resources->Present(holographicFrame);
    }
}

TextRenderer* HolographicRenderer::GetTextRendererForFontSize(float fontSize)
{
    int idx = text_renderers.Find(fontSize);
    if (idx < 0) {
        One<TextRenderer> renderer;
        renderer.Create(*device_resources, 1024u, 1024u, fontSize);
        text_renderers.Add(fontSize, pick(renderer));
        idx = text_renderers.GetCount() - 1;
    }

    return text_renderers[idx].Get();
}

bool HolographicRenderer::RenderAtCameraPose(
    DX::CameraResources *pCameraResources,
    winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const& coordinateSystem,
    winrt::Windows::Graphics::Holographic::HolographicFramePrediction& prediction,
    winrt::Windows::Graphics::Holographic::HolographicCameraRenderingParameters const& renderingParameters,
    winrt::Windows::Graphics::Holographic::HolographicCameraPose const& cameraPose)
{
    // Get the device context.
    const auto context = device_resources->GetD3DDeviceContext();
    const auto depthStencilView = pCameraResources->GetDepthStencilView();

    // Set render targets to the current holographic camera.
    ID3D11RenderTargetView *const targets[1] = { pCameraResources->GetBackBufferRenderTargetView() };
    context->OMSetRenderTargets(1, targets, depthStencilView);

    // TODO don't need to do this if we have a skybox
    // Clear the back buffer and depth stencil view.
    context->ClearRenderTargetView(targets[0], DirectX::Colors::Transparent);
    context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // The view and projection matrices for each holographic camera will change
    // every frame. This function will return false when positional tracking is lost.
    HolographicStereoTransform coordinateSystemToView;
    HolographicStereoTransform viewToProjection;
    bool cameraActive = pCameraResources->GetViewProjectionTransform(device_resources, cameraPose, coordinateSystem, &coordinateSystemToView, &viewToProjection);

    // Only render world-locked content when positional tracking is active.
    if (cameraActive)
    {
        ////////////////////////////////////////////////////////////////////////////////
        // Pbr Rendering
        pbr_resources->SetViewProjection(
            XMLoadFloat4x4(&coordinateSystemToView.Left),
            XMLoadFloat4x4(&coordinateSystemToView.Right),
            XMLoadFloat4x4(&viewToProjection.Left),
            XMLoadFloat4x4(&viewToProjection.Right));

        pbr_resources->Bind(device_resources->GetD3DDeviceContext());

        auto& root = GetEngine().GetRootPool();
        auto entities = root.FindAllDeep<Entity>();
        for (auto& entity : entities) {
            auto transform = entity->val.Find<Transform>();
            auto pbr = entity->val.Find<PbrRenderable>();
            if (!transform || !pbr || !pbr->Model)
                continue;

            float4x4 transformMtx = transform->GetMatrix();
            if (pbr->Offset)
                transformMtx = *pbr->Offset * transformMtx;

            pbr->Model->GetNode(Pbr::RootNodeIndex).SetTransform(XMLoadFloat4x4(&transformMtx));
            pbr->Model->Render(*pbr_resources, device_resources->GetD3DDeviceContext());
        }

        ////////////////////////////////////////////////////////////////////////////////
        // Text Rendering
        quad_renderer->SetViewProjection(
            coordinateSystemToView.Left,
            viewToProjection.Left,
            coordinateSystemToView.Right,
            viewToProjection.Right);

        quad_renderer->Bind();

        float prevFontSize = std::numeric_limits<float>::quiet_NaN();
        TextRenderer* textRenderer = nullptr;

        for (auto& entity : entities) {
            auto transform = entity->val.Find<Transform>();
            auto textRenderable = entity->val.Find<TextRenderable>();
            if (!transform || !textRenderable)
                continue;

            if (prevFontSize != textRenderable->FontSize) {
                prevFontSize = textRenderable->FontSize;
                textRenderer = GetTextRendererForFontSize(prevFontSize);
            }

            textRenderer->RenderTextOffscreen(textRenderable->Text.ToStd());
            quad_renderer->Render(transform->GetMatrix(), textRenderer->GetTexture());
        }

        quad_renderer->Unbind();

        ////////////////////////////////////////////////////////////////////////////////
        // Skybox Rendering
        float4x4 cameraToCoordinateSystem = float4x4::identity();
        if (auto location = SpatialLocator::GetDefault().TryLocateAtTimestamp(prediction.Timestamp(), coordinateSystem))
        {
            cameraToCoordinateSystem = make_float4x4_translation(location.Position());
        }

        skybox_renderer->SetViewProjection(
            cameraToCoordinateSystem * coordinateSystemToView.Left,  viewToProjection.Left,
            cameraToCoordinateSystem * coordinateSystemToView.Right, viewToProjection.Right);

        skybox_renderer->Bind();
        skybox_renderer->Render();
        skybox_renderer->Unbind();

        pCameraResources->CommitDirect3D11DepthBuffer(renderingParameters);
    }

    return true;
}

void HolographicRenderer::BindEventHandlers(
    const winrt::Windows::Graphics::Holographic::HolographicSpace& holographicSpace)
{
    fail_fast_if(holographicSpace == nullptr);

    camera_added_token = holographicSpace.CameraAdded(
        std::bind(&HolographicRenderer::OnCameraAdded, this, std::placeholders::_1, std::placeholders::_2));

    camera_removed_token = holographicSpace.CameraRemoved(
        std::bind(&HolographicRenderer::OnCameraRemoved, this, std::placeholders::_1, std::placeholders::_2));
}

void HolographicRenderer::ReleaseEventHandlers(
    const winrt::Windows::Graphics::Holographic::HolographicSpace& holographicSpace)
{
    fail_fast_if(holographicSpace == nullptr);

    holographicSpace.CameraRemoved(camera_removed_token);
    holographicSpace.CameraAdded(camera_added_token);
}

// Asynchronously creates resources for new holographic cameras.
void HolographicRenderer::OnCameraAdded(
    winrt::Windows::Graphics::Holographic::HolographicSpace const& sender,
    winrt::Windows::Graphics::Holographic::HolographicSpaceCameraAddedEventArgs const& args)
{
    winrt::Windows::Foundation::Deferral deferral = args.GetDeferral();
    HolographicCamera holographicCamera = args.Camera();
    concurrency::create_task([this, deferral, holographicCamera]()
    {
        //
        // TODO: Allocate resources for the new camera and load any content specific to
        //       that camera. Note that the render target size (in pixels) is a property
        //       of the HolographicCamera object, and can be used to create off-screen
        //       render targets that match the resolution of the HolographicCamera.
        //

        // Create device-based resources for the holographic camera and add it to the list of
        // cameras used for updates and rendering. Notes:
        //   * Since this function may be called at any time, the AddHolographicCamera function
        //     waits until it can get a lock on the set of holographic camera resources before
        //     adding the new camera. At 60 frames per second this wait should not take long.
        //   * A subsequent Update will take the back buffer from the RenderingParameters of this
        //     camera's CameraPose and use it to create the ID3D11RenderTargetView for this camera.
        //     Content can then be rendered for the HolographicCamera.
        device_resources->AddHolographicCamera(holographicCamera);

        // Holographic frame predictions will not include any information about this camera until
        // the deferral is completed.
        deferral.Complete();
    });
}

// Synchronously releases resources for holographic cameras that are no longer
// attached to the system.

void HolographicRenderer::OnCameraRemoved(
    winrt::Windows::Graphics::Holographic::HolographicSpace const& sender,
    winrt::Windows::Graphics::Holographic::HolographicSpaceCameraRemovedEventArgs const& args)
{
    concurrency::create_task([this]()
    {
        //
        // TODO: Asynchronously unload or deactivate content resources (not back buffer
        //       resources) that are specific only to the camera that was removed.
        //
    });

    // Before letting this callback return, ensure that all references to the back buffer
    // are released.
    // Since this function may be called at any time, the RemoveHolographicCamera function
    // waits until it can get a lock on the set of holographic camera resources before
    // deallocating resources for this camera. At 60 frames per second this wait should
    // not take long.
    device_resources->RemoveHolographicCamera(args.Camera());
}

} // namespace DemoRoom
