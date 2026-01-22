////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "EonWin.h"

using namespace concurrency;
using namespace DirectX;
using namespace std::placeholders;

namespace winrt_num = winrt::Windows::Foundation::Numerics;
namespace winrt_gaming = winrt::Windows::Gaming::Input;
namespace winrt_holo = winrt::Windows::Graphics::Holographic;
namespace winrt_spatial = winrt::Windows::Perception::Spatial;
namespace winrt_input = winrt::Windows::UI::Input::Spatial;

namespace DemoRoom {

// Loads and initializes application assets when the application is loaded.
DemoRoomMain::DemoRoomMain()
{
	device_resources.Create();
}

void DemoRoomMain::SetHolographicSpace(winrt_holo::HolographicSpace const& holographicSpace)
{
	if (engine && engine->IsStarted()) {
		engine->Stop();
		Engine::Uninstall(true, engine);
		engine = nullptr;
	}

	if (holographicSpace == nullptr)
		return;

	device_resources->SetHolographicSpace(holographicSpace);

	pbr_resources.Create(device_resources->GetD3DDevice());

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> diffuseEnvironmentMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> specularEnvironmentMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brdlutTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skyboxTexture;

	auto resourceLoadingTask = std::async(std::launch::async, [&]
	{
		auto diffuseTextureFuture = DX::LoadDDSTextureAsync(
			device_resources->GetD3DDevice(), L"ms-appx:///Media/Environment/DiffuseHDR.dds");
		auto specularTextureFuture = DX::LoadDDSTextureAsync(
			device_resources->GetD3DDevice(), L"ms-appx:///Media/Environment/SpecularHDR.dds");
		auto skyboxTextureFuture = DX::LoadDDSTextureAsync(
			device_resources->GetD3DDevice(), L"ms-appx:///Media/Environment/EnvHDR.dds");
		auto brdfLutFileDataFuture = DX::ReadDataAsync(L"ms-appx:///PBR/brdf_lut.png");

		diffuseEnvironmentMap = diffuseTextureFuture.get();
		specularEnvironmentMap = specularTextureFuture.get();
		skyboxTexture = skyboxTextureFuture.get();
		std::vector<byte> brdfLutFileData = brdfLutFileDataFuture.get();

		// Read the BRDF Lookup Table used by the PBR system into a DirectX texture.
		brdlutTexture = Pbr::Texture::LoadImage(
			device_resources->GetD3DDevice(),
			brdfLutFileData.data(),
			static_cast<uint32_t>(brdfLutFileData.size()));
	});

	resourceLoadingTask.wait();

	pbr_resources->SetBrdfLut(brdlutTexture.Get());
	pbr_resources->SetEnvironmentMap(
		device_resources->GetD3DDeviceContext(),
		specularEnvironmentMap.Get(),
		diffuseEnvironmentMap.Get());

	engine = &MetaEnv().root.GetAdd<Engine>("UwpDemoRoom");

	engine->Add<HolographicScene>(holographicSpace);
	engine->Add<EasingSystem>();
	engine->Add<PhysicsSystem>();
	engine->Add<PbrModelCache>(*pbr_resources);
	engine->Add<SpatialInteractionSystem>();
	engine->Add<MotionControllerSystem>();
	engine->Add<AppLogicSystem>();
	engine->Add<ToolboxSystem>();
	engine->Add<ShootingInteractionSystem>();
	engine->Add<PaintingInteractionSystem>();
	engine->Add<ThrowingInteractionSystem>();
	engine->Add<PaintStrokeSystem>(*pbr_resources);
	engine->Add<HolographicRenderer>(*device_resources, *pbr_resources, skyboxTexture.Get());

	if (!engine->Start()) {
		LOG("DemoRoomMain: engine failed to start");
		return;
	}

	// Seed model cache
	auto pbrModelCache = engine->Get<PbrModelCache>();

	// Register a low poly sphere model.
	{
		Pbr::Primitive spherePrimitive(
			*pbr_resources,
			Pbr::PrimitiveBuilder().AddSphere(1.0f, 3),
			Pbr::Material::CreateFlat(*pbr_resources, DirectX::Colors::White, 0.15f));

		One<Pbr::Model> sphereModel;
		sphereModel.Create();
		sphereModel->AddPrimitive(std::move(spherePrimitive));
		pbrModelCache->RegisterModel(KnownModelNames::UnitSphere, pick(sphereModel));
	}

	// Register a cube model.
	{
		Pbr::Primitive cubePrimitive(
			*pbr_resources,
			Pbr::PrimitiveBuilder().AddCube(1.0f),
			Pbr::Material::CreateFlat(*pbr_resources, DirectX::Colors::White, 0.15f));

		One<Pbr::Model> cubeModel;
		cubeModel.Create();
		cubeModel->AddPrimitive(std::move(cubePrimitive));
		pbrModelCache->RegisterModel(KnownModelNames::UnitCube, pick(cubeModel));
	}

	// Register glb models.
	auto loadGLBModels = [this](
		std::wstring_view path,
		String name,
		std::optional<DirectX::XMFLOAT4X4> transform = std::nullopt,
		std::optional<DirectX::XMFLOAT4> color = std::nullopt) -> std::future<void>
	{
		return std::async(std::launch::async, [this, path = std::wstring(path), name, transform, color] {
			auto pbrModelCache = engine->Get<PbrModelCache>();

			std::vector<byte> fileData = DX::ReadDataAsync(path).get();

			const DirectX::XMMATRIX modelTransform = transform.has_value()
				? DirectX::XMLoadFloat4x4(&transform.value())
				: DirectX::XMMatrixIdentity();

			std::shared_ptr<Pbr::Model> pbrModel = Gltf::FromGltfBinary(
				*pbr_resources,
				fileData.data(),
				(uint32_t)fileData.size(),
				modelTransform);

			if (color) {
				for (uint32_t i = 0; i < pbrModel->GetPrimitiveCount(); ++i) {
					pbrModel->GetPrimitive(i).GetMaterial()->Parameters.Set([&](Pbr::Material::ConstantBufferData& data) {
						data.BaseColorFactor = color.value();
					});
				}
			}

			debug_log("Loaded Model: %s", name.Begin());

			One<Pbr::Model> owned;
			owned.Create(*pbrModel);
			pbrModelCache->RegisterModel(name, pick(owned));
		});
	};

	DirectX::XMFLOAT4X4 baseballScale;
	DirectX::XMStoreFloat4x4(&baseballScale, DirectX::XMMatrixScaling(0.15f, 0.15f, 0.15f));
	loadGLBModels(L"ms-appx:///Media/Models/Baseball.glb", KnownModelNames::Baseball, baseballScale,
	              DirectX::XMFLOAT4{ 2.0f, 2.0f, 2.0f, 1.0f });

	DirectX::XMFLOAT4X4 gunScale;
	DirectX::XMStoreFloat4x4(&gunScale, DirectX::XMMatrixScaling(0.35f, 0.35f, 0.35f));
	loadGLBModels(L"ms-appx:///Media/Models/Gun.glb", KnownModelNames::Gun, gunScale);

	loadGLBModels(L"ms-appx:///Media/Models/PaintBrush.glb", KnownModelNames::PaintBrush);

	// Keep floor entity around forever.
	{
		WorldState ws;
		CreatePrefab<FloorPrefab>(engine->GetRootPool(), ws);
	}

	m_timer.ResetElapsedTime();
}

DemoRoomMain::~DemoRoomMain()
{
	if (engine) {
		engine->Stop();
		Engine::Uninstall(true, engine);
		engine = nullptr;
	}
}

// Updates the application state once per frame.
void DemoRoomMain::Update()
{
	m_timer.Tick([&]
	{
		if (engine)
			engine->Update(m_timer.GetElapsedSeconds());
	});
}

void DemoRoomMain::SaveAppState()
{
	device_resources->Trim();

	if (engine)
		engine->Suspend();
}

void DemoRoomMain::LoadAppState()
{
	if (engine)
		engine->Resume();
}

} // namespace DemoRoom