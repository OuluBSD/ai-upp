#ifndef _Eon_EcsCommonComponents_h_
#define _Eon_EcsCommonComponents_h_


namespace Ecs {


class RigidBody : public Component<RigidBody> {
	
public:
	COMP_DEF_VISIT
	
	
	vec3 velocity;
	vec3 acceleration;
	vec3 angular_velocity;
	vec3 angular_acceleration;
	
	float damping_factor;
	
	
	void Serialize(Stream& e) override;
	void Initialize() override;
	
    void operator=(const RigidBody& r) {
        velocity = r.velocity;
        acceleration = r.acceleration;
        angular_velocity = r.angular_velocity;
        angular_acceleration = r.angular_acceleration;
        damping_factor = r.damping_factor;
    }
    
	
};

using RigidBodyPtr = Ptr<RigidBody>;







class TextRenderable :
	public Component<TextRenderable>
{
	
public:
	COPY_PANIC(TextRenderable);
	COMP_DEF_VISIT
	
	
    String				text = "";
    double				font_size = 60.0;
    
    
	void Serialize(Stream& e) override {e % text % font_size;}
    
    
};




}

#endif
