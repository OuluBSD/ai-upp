#include "Core.h"

NAMESPACE_UPP

void Component::AddToUpdateList() {GetEngine().AddUpdated(this);}
void Component::RemoveFromUpdateList() {GetEngine().RemoveUpdated(this);}

Engine& Component::GetEngine() {
	Engine* e = val.FindOwner<Engine>();
	if (!e)
		e = val.FindOwnerWith<Engine>();
	ASSERT(e);
	if (!e) throw Exc("Can't find Engine");
	return *e;
}

Entity* Component::GetEntity() {
	if (!val.owner)
		throw Exc("Can't find Entity: no owner");
	Entity* e = val.owner->FindExt<Entity>();
	ASSERT(e);
	if (!e) throw Exc("Can't cast owner Entity");
	return e;
}

String Component::ToString() const {
	return "Component(" + GetTypeCls().GetName() + ")";
}

void Component::Visit(Vis& vis) {Panic("Unimplemented (remove caller)");}


END_UPP_NAMESPACE
