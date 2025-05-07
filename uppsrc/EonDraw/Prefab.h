#ifndef _EonDraw_Prefab_h_
#define _EonDraw_Prefab_h_

namespace Ecs {


namespace KnownModelNames {

extern const char* UnitSphere;
extern const char* UnitCube;
extern const char* UnitQuad;
extern const char* Baseball;
extern const char* PaintBrush;
extern const char* Gun;

String GetPath(String name);

}


struct StaticSkybox : EntityPrefab<Transform, Renderable, ModelComponent>
{
    static Components Make(Entity& e)
    {
        auto components = EntityPrefab::Make(e);
        
        float huge_distance = 10e5;
        ModelBuilder b;
        b.AddBox(vec3(0,0,0), vec3(huge_distance, huge_distance, huge_distance), true);
        
        Model* m = b.Detach();
        m->ReverseFaces();
        components.Get<Ref<ModelComponent>>()->Attach(m);
        
        return components;
    }
};

struct StaticSphere : EntityPrefab<Transform, ModelComponent>
{
    static Components Make(Entity& e)
    {
        auto components = EntityPrefab::Make(e);

        components.Get<ModelComponentPtr>()->SetPrefabModel(KnownModelNames::UnitSphere);
        components.Get<ModelComponentPtr>()->color = vec4(0.5, 0.5, 0.5, 1.0);

        return components;
    }
};

struct StaticCube : EntityPrefab<Transform, ModelComponent>
{
    static Components Make(Entity& e)
    {
        auto components = EntityPrefab::Make(e);

        components.Get<ModelComponentPtr>()->SetPrefabModel(KnownModelNames::UnitCube);
        components.Get<ModelComponentPtr>()->color = vec4(0.5, 0.5, 0.5, 1.0);
		
        return components;
    }
};


}

#endif
