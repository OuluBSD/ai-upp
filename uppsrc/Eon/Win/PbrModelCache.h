////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

namespace Pbr {
	struct Model;
	struct Resources;
}

namespace DemoRoom {

	struct PbrRenderable;

	////////////////////////////////////////////////////////////////////////////////
	// PbrModelCache
	// Stores all of the PbrModels in the system to avoid duplication. As well as
	// it allows for lazy-assignment of Model files to PbrRenderable components
	// This allows you to set the ModelName on a PbrRenderable component, and the 
	// PbrModelCache will automatically assign the Model field of the PbrRenderable once the model has been loaded
	class PbrModelCache : public System
	{
	public:
		CLASSTYPE(PbrModelCache)
		PbrModelCache(VfsValue& v, Pbr::Resources& pbr_resources);

		void RegisterModel(String name, One<Pbr::Model> model);
		bool ModelExists(String name);
		PbrRenderable* SetModel(const String& name, PbrRenderable* pbr_renderable);
		PbrRenderable* SetModel(const String& name, Entity& entity);

	protected:
		void Update(double) override;
		void Uninitialize() override;

	private:
		Pbr::Resources* pbr_resources = nullptr;
		ArrayMap<String, One<Pbr::Model>> model_map;
	};
}
