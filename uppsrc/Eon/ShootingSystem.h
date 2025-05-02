#ifndef _Eon_ShootingSystem_h_
#define _Eon_ShootingSystem_h_

namespace Ecs {


class ShootingComponent :
	public CustomToolComponent {
	
public:
	RTTI_COMP1(ShootingComponent, CustomToolComponent)
	void Visit(Vis& vis) override {vis.VisitThis<CustomToolComponent>(this); /*vis & gun;*/}
	
	
	void Etherize(Ether& e) override;
	void Initialize() override;
	void Uninitialize() override;
	bool LoadModel(ModelComponent&) override;
	
	float bullet_speed = 10.0f;
	mat4 barrel_to_ctrl;
};

using ShootingComponentPtr = Ref<ShootingComponent>;


// ShootingInteractionSystem
// This ToolSystem manages the Gun tool which allows you to shoot balls in the 3D scene

class ShootingInteractionSystemBase :
	public ToolSystemBaseT<ShootingInteractionSystemBase, ShootingComponent>
{
	Array<ShootingComponentRef> comps;
	
public:
	using ToolSys = ToolSystemBaseT<ShootingInteractionSystemBase, ShootingComponent>;
	ECS_SYS_CTOR(ShootingInteractionSystemBase);
	void Visit(Vis& vis) override {vis.VisitThis<ToolSys>(this);}
	
	using Parent = Engine;
	
	static constexpr const char* POOL_NAME = "shooting";
	
	void Attach(ShootingComponentPtr c);
	void Detach(ShootingComponentPtr c);
	PoolPtr GetPool() const {return GetEngine().Get<EntityStore>()->GetRoot()->GetAddPool(POOL_NAME);}
	
protected:
	// ToolSystemBase
	bool Initialize() override;
	void Uninitialize() override;
	String GetInstructions() const override;
	String GetDisplayName() const override;
	EntityPtr CreateToolSelector() const override;
	void OnControllerPressed(const CtrlEvent& e) override;
	void OnControllerUpdated(const CtrlEvent& e) override;
	void OnControllerReleased(const CtrlEvent& e) override;
	
	void Register() override;
	void Unregister() override;
	void Activate(EntityPtr entity) override;
	void Deactivate(EntityPtr entity) override;
	
};


}

#endif
