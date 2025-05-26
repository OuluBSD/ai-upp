#ifndef _EonDraw_ShootingSystem_h_
#define _EonDraw_ShootingSystem_h_

namespace Ecs {


class ShootingComponent :
	public CustomToolComponent {
	
public:
	CLASSTYPE(ShootingComponent)
	ShootingComponent(VfsValue& e) : CustomToolComponent(e) {}
	void Visit(Vis& v) override;
	void Initialize() override;
	void Uninitialize() override;
	bool LoadModel(ModelComponent&) override;
	
	float bullet_speed = 10.0f;
	mat4 barrel_to_ctrl;
};

using ShootingComponentPtr = Ptr<ShootingComponent>;


// ShootingInteractionSystem
// This ToolSystem manages the Gun tool which allows you to shoot balls in the 3D scene

class ShootingInteractionSystemBase :
	public ToolSystemBaseT<ShootingInteractionSystemBase, ShootingComponent>
{
	Array<ShootingComponentPtr> comps;
	
public:
	using ToolSys = ToolSystemBaseT<ShootingInteractionSystemBase, ShootingComponent>;
	CLASSTYPE(ShootingInteractionSystemBase);
	ShootingInteractionSystemBase(VfsValue& m) : ToolSys(m) {}
	void Visit(Vis& v) override {VIS_THIS(ToolSys);}
	
	using Parent = Engine;
	
	static constexpr const char* POOL_NAME = "shooting";
	
	void Attach(ShootingComponentPtr c);
	void Detach(ShootingComponentPtr c);
	PoolPtr GetPool() const;
	
protected:
	// ToolSystemBase
	Ecs::SystemBase* GetSystem() override {return this;}
	bool Initialize() override;
	void Uninitialize() override;
	String GetInstructions() const override;
	String GetDisplayName() const override;
	EntityPtr CreateToolSelector() const override;
	void OnControllerPressed(const GeomEvent& e) override;
	void OnControllerUpdated(const GeomEvent& e) override;
	void OnControllerReleased(const GeomEvent& e) override;
	
	void Register() override;
	void Unregister() override;
	void Activate(EntityPtr entity) override;
	void Deactivate(EntityPtr entity) override;
	
};


}

#endif
