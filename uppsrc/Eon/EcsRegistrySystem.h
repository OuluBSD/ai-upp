#ifndef _Eon_EcsRegistrySystem_h_
#define _Eon_EcsRegistrySystem_h_

namespace Ecs {


class RegistrySystem : public Ecs::System<RegistrySystem> {
	ValueMap reg;
	
	
protected:
    bool Initialize() override;
    void Start() override;
    void Update(double dt) override;
    void Stop() override;
    void Uninitialize() override;
    
public:
	ECS_SYS_CTOR(RegistrySystem)
	ECS_SYS_DEF_VISIT
	
	void Set(String key, Value value) {reg.GetAdd(key) = value;}
	Value Get(String key) {int i = reg.Find(key); return i >= 0 ? reg[i] : Value();}
	
	
	void SetAppName(String name) {Set("app.name", name);}
	
	String GetAppName() {return Get("app.name");}
	
	
};


}

#endif
