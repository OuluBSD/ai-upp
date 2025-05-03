#include "Eon.h"

NAMESPACE_UPP namespace Ecs {

void WorldLogicSystem::Attach(Transform* t) {
	list.Add(t);
}

void WorldLogicSystem::Detach(Transform* t) {
	VectorRemoveKey(list, t);
}

void WorldLogicSystem::Update(double dt)
{
	if (0)
		UpdateByVisit(dt);
	else
		UpdateByList(dt); // high performance
}

void WorldLogicSystem::UpdateByVisit(double dt) {
	EntityComponentVisitor<Transform> visitor(GetEngine());
	visitor.Skip(Pool::BIT_TRANSFORM);
    for (;visitor; visitor++)
    {
        EntityPtr entity = *visitor;
        TransformPtr transform = visitor.Get<Transform>();
        UpdateTransform(*transform, dt);
    }
}

void WorldLogicSystem::UpdateByList(double dt) {
	for (Transform* t : list)
		UpdateTransform(*t, dt);
}

void WorldLogicSystem::UpdateTransform(Transform& t, double dt) {
    if (t.data.position[1] < -1000.0f) {
        t.GetEntity()->Destroy();
    }
}


} END_UPP_NAMESPACE
