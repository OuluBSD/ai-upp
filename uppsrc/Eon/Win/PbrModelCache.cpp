////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "EonWin.h"

#include "PbrModel.h"
#include <stdexcept>

namespace DemoRoom {

PbrModelCache::PbrModelCache(VfsValue& v, Pbr::Resources& pbr_resources)
	: System(v), pbr_resources(&pbr_resources)
{
}

void PbrModelCache::RegisterModel(String name, One<Pbr::Model> model)
{
	if (name.IsEmpty())
		throw std::invalid_argument("Cannot register model with empty name");

	model_map.GetAdd(name) = pick(model);
}

static One<Pbr::Model> CloneModel(const Pbr::Model& model, Pbr::Resources& resources)
{
	One<Pbr::Model> cloned;
	auto shared = model.Clone(resources);
	if (!shared)
		return cloned;

	cloned.Create(*shared);
	return cloned;
}

PbrRenderable* PbrModelCache::SetModel(const String& name, PbrRenderable* pbr_renderable)
{
	pbr_renderable->ModelName = name;

	int idx = model_map.Find(name);
	if (idx >= 0 && pbr_resources) {
		One<Pbr::Model> owned = CloneModel(*model_map[idx], *pbr_resources);
		pbr_renderable->OwnedModel = pick(owned);
		pbr_renderable->Model = pbr_renderable->OwnedModel.Get();
	}

	return pbr_renderable;
}

PbrRenderable* PbrModelCache::SetModel(const String& name, Entity& entity)
{
	auto pbr_renderable = entity.val.Find<PbrRenderable>();
	if (!pbr_renderable)
		return nullptr;
	return SetModel(name, pbr_renderable);
}

bool PbrModelCache::ModelExists(String name)
{
	return model_map.Find(name) >= 0;
}

void PbrModelCache::Update(double)
{
	auto& root = GetEngine().GetRootPool();
	auto renderables = root.FindAllDeep<PbrRenderable>();

	for (auto& pbr_renderable : renderables) {
		// Find any PbrRenderable component which is waiting for a model to be loaded.
		if (!pbr_renderable->Model && !pbr_renderable->ModelName.IsEmpty())
			(void)SetModel(pbr_renderable->ModelName, pbr_renderable);

		// Apply any material updates as needed.
		if (pbr_renderable->Model) {
			for (uint32_t i = 0; i < pbr_renderable->Model->GetPrimitiveCount(); ++i) {
				const auto& material = pbr_renderable->Model->GetPrimitive(i).GetMaterial();

				DirectX::XMFLOAT4 current_color = material->Parameters.Get().BaseColorFactor;
				if (pbr_renderable->Color && i == 0)
					current_color = DirectX::XMFLOAT4(*pbr_renderable->Color);

				current_color.w = pbr_renderable->AlphaMultiplier ? *pbr_renderable->AlphaMultiplier : 1.0f;

				const DirectX::XMFLOAT4& material_base = material->Parameters.Get().BaseColorFactor;
				if (current_color.x != material_base.x ||
				    current_color.y != material_base.y ||
				    current_color.z != material_base.z ||
				    current_color.w != material_base.w) {
					material->Parameters.Set([&](Pbr::Material::ConstantBufferData& data) {
						data.BaseColorFactor = current_color;
					});
				}
			}
		}
	}
}

void PbrModelCache::Uninitialize()
{
	model_map.Clear();
}

} // namespace DemoRoom
