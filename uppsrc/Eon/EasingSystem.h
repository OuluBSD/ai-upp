#ifndef _Eon_EasingSystem_h_
#define _Eon_EasingSystem_h_

namespace Ecs {

	
class Easing :
	public Component<Easing>
{
	
public:
	RTTI_COMP0(Easing)
	COMP_DEF_VISIT
	
	
    vec3 target_position = { 0,0,0 };
    quat target_orientation = Identity<quat>();
    float position_easing_factor = 0;
    float orientation_easing_factor = 0;
    
    void operator=(const Easing& e) {
        target_position = e.target_position;
        target_orientation = e.target_orientation;
        position_easing_factor = e.position_easing_factor;
        orientation_easing_factor = e.orientation_easing_factor;
    }
    
	void Serialize(Stream& e) override;
    void Initialize() override;
    void Uninitialize() override;
    
};

using EasingPtr = Ref<Easing>;



class EasingSystem : public System<EasingSystem>
{
	Vector<Easing*> comps;
public:
	ECS_SYS_CTOR(EasingSystem)
	ECS_SYS_DEF_VISIT
	
	void Attach(Easing* e);
	void Detach(Easing* e);
	
protected:
    void Update(double) override;
};


}

#endif
