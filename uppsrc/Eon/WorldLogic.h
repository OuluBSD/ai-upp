#ifndef _Eon_WorldLogic_h_
#define _Eon_WorldLogic_h_

namespace Ecs {
	
class WorldLogicSystem : public System<WorldLogicSystem>
{
	
	
public:
    ECS_SYS_CTOR(WorldLogicSystem)
	ECS_SYS_DEF_VISIT
	
	void Attach(Transform* t);
	void Detach(Transform* t);
	
protected:
	void Update(double dt) override;
	void UpdateByVisit(double dt);
	void UpdateByList(double dt);
	void UpdateTransform(Transform& t, double dt);
	
	Vector<Transform*> list;
	
};

}

#endif
