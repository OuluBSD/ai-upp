#ifndef _Eon_EcsCommonComponents_h_
#define _Eon_EcsCommonComponents_h_


namespace Ecs {


class RigidBody : public Component<RigidBody> {
	
public:
	RTTI_COMP0(RigidBody)
	COMP_DEF_VISIT
	
	
	vec3 velocity;
	vec3 acceleration;
	vec3 angular_velocity;
	vec3 angular_acceleration;
	
	float damping_factor;
	
	
	void Etherize(Ether& e) override;
	void Initialize() override;
	
    void operator=(const RigidBody& r) {
        velocity = r.velocity;
        acceleration = r.acceleration;
        angular_velocity = r.angular_velocity;
        angular_acceleration = r.angular_acceleration;
        damping_factor = r.damping_factor;
    }
    
	
};

using RigidBodyPtr = Ref<RigidBody>;







class TextRenderable :
	public Component<TextRenderable>
{
	
public:
	RTTI_COMP0(TextRenderable)
	COPY_PANIC(TextRenderable);
	COMP_DEF_VISIT
	
	
    String				text = "";
    double				font_size = 60.0;
    
    
	void Etherize(Ether& e) override {e % text % font_size;}
    
    
};




}

#endif
