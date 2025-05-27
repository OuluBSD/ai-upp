#include "Eon.h"


NAMESPACE_UPP 
	

void RigidBody::Visit(Vis& v) {
	v
	 VISN(velocity)
	 VISN(acceleration)
	 VISN(angular_velocity)
	 VISN(angular_acceleration)
	 VIS_(damping_factor);
	
	VISIT_COMPONENT
}

void RigidBody::Initialize() {
	velocity = zero<vec3>();
	acceleration = zero<vec3>();
	angular_velocity = zero<vec3>();
	angular_acceleration = zero<vec3>();
	
	damping_factor = 0.999f;
	
}


END_UPP_NAMESPACE
