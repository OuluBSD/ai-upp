#ifndef _Eon_ThrowingSystem_h_
#define _Eon_ThrowingSystem_h_

namespace Ecs {


class ThrowingComponent :
	public CustomToolComponent {
	
public:
	RTTI_COMP1(ThrowingComponent, CustomToolComponent)
	void Visit(Vis& vis) override {vis.VisitT<CustomToolComponent>(this); vis & ball_object;}
	
	
	void Etherize(Ether& e) override;
	void Initialize() override;
	void Uninitialize() override;
	void SetEnabled(bool enable) override;
	void Destroy() override;
	bool LoadModel(ModelComponent&) override;
	
	EntityPtr ball_object;
	
	float distance_from_pointer = 0.05f;
	float scale = 0.25f;
};

using ThrowingComponentPtr = Ref<ThrowingComponent>;


// ThrowingInteractionSystem
// This ToolSystem manages the Throwing tool which allows you to throw baseballs in 3D scene
class ThrowingInteractionSystemBase :
	public ToolSystemBaseT<ThrowingInteractionSystemBase, ThrowingComponent>
{
	Array<ThrowingComponentRef> comps;
	
public:
	using ToolSys = ToolSystemBaseT<ThrowingInteractionSystemBase, ThrowingComponent>;
	ECS_SYS_CTOR(ThrowingInteractionSystemBase);
	void Visit(Vis& vis) override {vis.VisitT<ToolSys>(this);}
	
	using Parent = Engine;
	float ball_holding_distance;
	static constexpr const char* POOL_NAME = "throwing";
	
	void Attach(ThrowingComponentPtr c);
	void Detach(ThrowingComponentPtr c);
	PoolPtr GetPool() const {return GetEngine().Get<EntityStore>()->GetRoot()->GetAddPool(POOL_NAME);}
	
	
protected:
	// System
	bool Initialize() override;
	void Uninitialize() override;
	void Update(double dt) override;
	void OnControllerPressed(const CtrlEvent& e) override;
	void OnControllerUpdated(const CtrlEvent& e) override;
	void OnControllerReleased(const CtrlEvent& e) override;
	
	// IInteractionModeSystem
	String GetInstructions() const override;
	String GetDisplayName() const override;
	EntityPtr CreateToolSelector() const override;
	void Register() override;
	void Unregister() override;
	void Activate(EntityPtr entity) override;
	void Deactivate(EntityPtr entity) override;
	
};


}

#endif
