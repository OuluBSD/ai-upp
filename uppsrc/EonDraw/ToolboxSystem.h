#ifndef _Eon_ToolboxSystem_h_
#define _Eon_ToolboxSystem_h_


namespace Ecs {


class ToolSystemBase;
class PlayerHandComponent;

using PlayerHandComponentPtr = RefT_Entity<PlayerHandComponent>;



class CustomToolComponent :
	public Component<CustomToolComponent> {
	
public:
	COPY_PANIC(CustomToolComponent)
	COMP_DEF_VISIT
	
	virtual bool LoadModel(ModelComponent&) = 0;
	
	void Serialize(Stream& e) override {}
	
	
};

using CustomToolComponentPtr = Ptr<CustomToolComponent>;



class ToolComponent : public Component<ToolComponent> {
	
public:
	COPY_PANIC(ToolComponent)
	COMP_DEF_VISIT_(vis & active_tool & active_hand; vis && tools;)
	
	void Serialize(Stream& e) override;
	void Initialize() override;
	void Uninitialize() override;
	bool Arg(String key, Value value) override;
	
	void SwitchNext();
	void SwitchOff();
	void AddTool(CustomToolComponentPtr cb);
	void RemoveTool(CustomToolComponentPtr cb);
	void RefreshModel();
	
	String title;
	String description;
	TypeId tool_type { AsVoidTypeId() };
	
	CustomToolComponentPtr active_tool;
	Array<CustomToolComponentPtr> tools;
	PlayerHandComponentPtr active_hand;
	
};

using ToolComponentPtr = Ptr<ToolComponent>;


class ToolboxSystemBase :
	public System<ToolboxSystemBase>,
	public InteractionListener
{
	
public:
	ECS_SYS_DEF_VISIT_(vis && tools && selectors && selector_objects)
	
	ECS_SYS_CTOR(ToolboxSystemBase);
	
	using Parent = Engine;
	
	
	void AddToolSystem(ToolSystemBasePtr system);
	void RemoveToolSystem(ToolSystemBasePtr system);
	
	void Attach(ToolComponentPtr tool);
	void Detach(ToolComponentPtr tool);
	
	const Array<ToolComponentPtr>& GetTools() const {return tools;}
	
protected:
	// System
	void Start() override;
	void Update(double dt) override;
	void Stop() override;
	bool Initialize() override;
	void Uninitialize() override;
	bool Arg(String key, Value value) override;
	
private:
	Array<ToolComponentPtr> tools;
	TypeMap<ToolSystemBasePtr> selectors;
	TypeMap<EntityPtr> selector_objects;
	
	bool test_tool_changer{ false };
	bool show_toolbox{ false };
	int active_tool_idx = -1;
	
	
protected:
	void SwitchToolType(EntityPtr entity, const TypeId& new_type);
	void OnControllerPressed(const CtrlEvent& e) override;
	
};

using ToolboxSystemBasePtr = Ptr<ToolboxSystemBase>;


}

#endif
