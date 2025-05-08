#ifndef _EonDraw_EventSystem_h_
#define _EonDraw_EventSystem_h_


namespace Ecs {



class EventSystem :
	public System<EventSystem>,
	public BinderIfaceEvents
{
	//Serial::EcsEventsBase* serial = 0;
	
protected:
    bool Initialize() override;
    void Start() override;
    void Update(double dt) override;
    void Stop() override;
    void Uninitialize() override;
    void Dispatch(const GeomEvent& state) override {TODO}
    
public:
	using Base = System<EventSystem>;
    ECS_SYS_CTOR(EventSystem)
	ECS_SYS_DEF_VISIT
	
	//void Attach(Serial::EcsEventsBase* b);
	
};

using EventSystemPtr = Ptr<EventSystem>;


}

#endif
