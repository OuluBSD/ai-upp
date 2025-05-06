#ifndef _Eon_EcsCommonComponents_h_
#define _Eon_EcsCommonComponents_h_


namespace Ecs {


class RigidBody : public Component<RigidBody> {
	
public:
	ECS_COMPONENT_CTOR(RigidBody)
	vec3 velocity;
	vec3 acceleration;
	vec3 angular_velocity;
	vec3 angular_acceleration;
	
	float damping_factor;
	
	
	void Visit(Vis& v) override;
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
	ECS_COMPONENT_CTOR(TextRenderable)
    String				text = "";
    double				font_size = 60.0;
    
	void Visit(Vis& v) override {
		_VIS_(text)
		 VIS_(font_size);
		VISIT_COMPONENT
	}
    
    
};




}

#endif
