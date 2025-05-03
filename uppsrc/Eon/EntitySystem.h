#ifndef _Eon_EntitySystem_h_
#define _Eon_EntitySystem_h_

namespace Ecs {
class Engine;
}

Ecs::Engine* CreateEcsEngine();

class EntitySystem : public System<EntitySystem> {
	Ecs::Engine* engine = 0;
	ValueMap reg;
	
	
protected:
    bool Initialize() override;
    void Start() override;
    void Update(double dt) override;
    void Stop() override;
    void Uninitialize() override;
    
    void ClearEngine();
    
public:
	SYS_CTOR(EntitySystem)
	void Visit(Vis& vis) override;
	~EntitySystem() {ClearEngine();}
	
	static ParallelTypeCls::Type GetSerialType() {return ParallelTypeCls::ENTITY_SYSTEM;}
	
	Ecs::Engine& GetEngine() {return *engine;}
	
};


#endif
