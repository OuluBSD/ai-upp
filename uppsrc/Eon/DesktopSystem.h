#ifndef _Eon_DesktopSystem_h_
#define _Eon_DesktopSystem_h_

namespace Ecs {


class DesktopSuiteSystem : public System<DesktopSuiteSystem> {
	Array<TopWindow> apps;
	
	
protected:
    bool Initialize() override;
    bool Arg(String key, Value value) override;
    void Start() override;
    void Update(double dt) override;
    void Stop() override;
    void Uninitialize() override;
    
    
public:
	SYS_RTTI(DesktopSuiteSystem)
    ECS_SYS_CTOR(DesktopSuiteSystem)
	void Visit(Vis& vis) override;
	~DesktopSuiteSystem() {ASSERT(apps.IsEmpty());}
	
	
};


}

#endif
