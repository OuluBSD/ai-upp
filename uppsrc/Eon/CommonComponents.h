#ifndef _Eon_CommonComponents_h_
#define _Eon_CommonComponents_h_

namespace Ecs {


extern const vec3 EarthGravity;


class Transform : public Component<Transform> {
	
public:
	COMP_DEF_VISIT
	
	TransformMatrix data;
	vec3 size;
	vec3 relative_position;
	vec3 anchor_position;
	quat anchor_orientation;
	bool verbose = false;
	
	void operator=(const Transform& t);
	mat4 GetMatrix() const;
	vec3 GetForwardDirection() const;
	
	void Serialize(Stream& e) override;
	void Initialize() override;
	void Uninitialize() override;
	bool Arg(String key, Value value) override;
	String ToString() const override;
	
};

typedef Ptr<Transform> TransformPtr;


void CopyTransformPos(EntityPtr from, EntityPtr to);

class Transform2D : public Component<Transform2D> {
	
public:
	COMP_DEF_VISIT
	
	
	vec2 position = zero<vec2>();
	vec2 size = one<vec2>();
	
	void operator=(const Transform2D& t);
	
	void Serialize(Stream& e) override;
	
};


}

#endif
