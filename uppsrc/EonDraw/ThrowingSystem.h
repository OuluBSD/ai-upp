#ifndef _EonDraw_ThrowingSystem_h_
#define _EonDraw_ThrowingSystem_h_

namespace Ecs {


class ThrowingComponent :
	public CustomToolComponent {
	
public:
	void Visit(Vis& v) override;
	void Initialize() override;
	void Uninitialize() override;
	void SetEnabled(bool enable) override;
	void Destroy() override;
	bool LoadModel(ModelComponent&) override;
	
	EntityPtr ball_object;
	
	float distance_from_pointer = 0.05f;
	float scale = 0.25f;
};

using ThrowingComponentPtr = Ptr<ThrowingComponent>;


// ThrowingInteractionSystem
// This ToolSystem manages the Throwing tool which allows you to throw baseballs in 3D scene
class ThrowingInteractionSystemBase :
	public ToolSystemBaseT<ThrowingInteractionSystemBase, ThrowingComponent>
{
	Array<ThrowingComponentPtr> comps;
	
public:
	using ToolSys = ToolSystemBaseT<ThrowingInteractionSystemBase, ThrowingComponent>;
	ECS_SYS_CTOR(ThrowingInteractionSystemBase);
	void Visit(Vis& vis) override;
	
	using Parent = Engine;
	float ball_holding_distance;
	static constexpr const char* POOL_NAME = "throwing";
	
	void Attach(ThrowingComponentPtr c);
	void Detach(ThrowingComponentPtr c);
	PoolPtr GetPool() const;
	
	
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
