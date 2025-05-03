#ifndef _Eon_RegistrySystem_h_
#define _Eon_RegistrySystem_h_


class RegistrySystem : public System<RegistrySystem> {
	ValueMap reg;
	
	
protected:
    bool Initialize() override;
    void Start() override;
    void Update(double dt) override;
    void Stop() override;
    void Uninitialize() override;
    
public:
	SYS_RTTI(RegistrySystem)
    SYS_CTOR(RegistrySystem)
	SYS_DEF_VISIT
	
	void Set(String key, Value value) {reg.GetAdd(key) = value;}
	Value Get(String key) {int i = reg.Find(key); return i >= 0 ? reg[i] : Value();}
	
	
	void SetAppName(String name) {Set("app.name", name);}
	
	String GetAppName() {return Get("app.name");}
	
	
	static ParallelTypeCls::Type GetSerialType() {return ParallelTypeCls::REGISTRY_SYSTEM;}
	
};

using RegistrySystemPtr = Ptr<RegistrySystem>;


#endif
