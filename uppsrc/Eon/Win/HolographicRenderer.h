#pragma once


NAMESPACE_UPP

using namespace ::Upp::Ecs;

class HolographicScene;
class TextRenderer;
class QuadRenderer;
class SkyboxRenderer;


// HolographicRenderer
// A stereoscopic 3D rendering system, manages rendering everything in the scene
// Updated to work with multiple VR platforms (OpenVR, OpenHMD, WinRT)
class HolographicRenderer :
	public System,
	public IDeviceNotify
{
public:
	using System::System;
	using Base = System;
	//RTTI_DECL2(HolographicRenderer, Base, IDeviceNotify)
	
    HolographicRenderer(
        Engine& core,
        std::shared_ptr<DeviceResources> deviceResources,
        std::shared_ptr<Pbr::Resources> pbrResources,
        ID3D11ShaderResourceView* skyboxTexture);

    ~HolographicRenderer();

    std::shared_ptr<Pbr::Resources> GetPbrResources();
    std::shared_ptr<DeviceResources> GetDeviceResources();

    void OnDeviceLost() override;
    void OnDeviceRestored() override;

protected:
    bool Initialize(const WorldState&) override;
    bool Start() override;
    void Update(double) override;
    void Stop() override;
    void Uninitialize() override;

private:
    //EntityStorePtr m_entityStore;
    
    Ptr<HolographicScene> m_holoScene;

    std::unique_ptr<SkyboxRenderer> m_skyboxRenderer{ nullptr };

    std::unordered_map<float, std::unique_ptr<TextRenderer>> m_textRenderers;
    std::unique_ptr<QuadRenderer> m_quadRenderer{ nullptr };

    std::shared_ptr<Pbr::Resources> m_pbrResources{ nullptr };

    std::shared_ptr<DeviceResources> m_deviceResources{ nullptr };

    TextRenderer* GetTextRendererForFontSize(float fontSize);

    // Platform-agnostic rendering method
    bool RenderAtCameraPose(
        IVRCamera* camera,
        void* coordinateSystem,
        void* prediction);

    // Platform-specific handlers that work through VRPlatform abstraction
    void BindEventHandlers();
    void ReleaseEventHandlers();
};


END_UPP_NAMESPACE
