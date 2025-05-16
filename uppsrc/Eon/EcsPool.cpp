#include "Eon.h"


NAMESPACE_UPP namespace Ecs {


//PoolPtr GetConnectorBasePool(ConnectorBase* conn) {return conn->GetPool();}

Engine& GetPoolEngine(PoolPtr pool) {return pool->GetEngine();}



Pool::Pool(MetaNode& n) : MetaNodeExt(n) {
	DBG_CONSTRUCT
}

Pool::~Pool() {
	DBG_DESTRUCT
}

void Pool::Visit(Vis& v) {
	_VIS_(freeze_bits)
	 VIS_(name)
	 VIS_(id);
}

PoolId Pool::GetNextId() {
	static Atomic next_id;
	return ++next_id;
}

Pool* Pool::GetParent() const {
	return node.GetOwnerExt<Pool>();
}

Engine& Pool::GetEngine() {
	Engine* e = node.FindOwner<Engine>();
	ASSERT(e);
	if (!e) throw Exc("No engine found");
	return *e;
}

void Pool::Initialize(Entity& e, String prefab) {
	uint64 ticks = GetEngine().GetTicks();
	e.SetPrefab(prefab);
	e.SetCreated(ticks);
	e.SetChanged(ticks);
	
}

EntityPtr Pool::CreateEmpty() {
	Entity& e = node.Add<Ecs::Entity>();
	e.SetId(GetNextId());
	Initialize(e);
	return &e;
}

EntityPtr Pool::GetAddEmpty(String name) {
	EntityPtr e = FindEntityByName(name);
	if (e)
		return e;
	e = CreateEmpty();
	e->SetName(name);
	return e;
}

EntityPtr Pool::Clone(const Entity& c) {
	Ecs::Entity& e = node.Add<Ecs::Entity>();
	VisitCopy(c, e);
	return &e;
}

void Pool::UnlinkDeep() {
	auto pools = node.FindAll<Pool>();
	for (int i = pools.GetCount()-1; i >= 0; i--) {
		pools[i]->UnlinkDeep();
	}
}

void Pool::UnrefDeep() {
	RefClearVisitor vis;
	TODO // implemented?
	vis.Visit("Pool",*this);
}

void Pool::UninitializeComponentsDeep() {
	auto pools = node.FindAll<Pool>();
	for (PoolPtr& p : pools)
		p->UninitializeComponentsDeep();
	
	auto ents = node.FindAll<Ecs::Entity>();
	for (int i = ents.GetCount()-1; i >= 0; i--)
		ents[i]->UninitializeComponents();
	
}

void Pool::ClearComponentsDeep() {
	auto pools = node.FindAll<Pool>();
	for (PoolPtr& p : pools)
		p->ClearComponentsDeep();
	
	auto ents = node.FindAll<Ecs::Entity>();
	for (int i = ents.GetCount()-1; i >= 0; i--)
		ents[i]->ClearComponents();
	
}

void Pool::ClearDeep() {
	auto pools = node.FindAll<Pool>();
	for (PoolPtr& p : pools)
		if (p)
			p->ClearDeep();
	
	node.RemoveAllDeep<Pool>();
	node.RemoveAllDeep<Ecs::Entity>();
}

#if 0
void Pool::ReverseEntities() {
	objects.Reverse();
}
#endif

void Pool::Clear() {
	//UnrefDeep();
	UnlinkDeep();
	UninitializeComponentsDeep();
	ClearComponentsDeep();
	ClearDeep();
}

void Pool::PruneFromContainer() {
	auto pools = node.FindAll<Pool>();
	for (auto& pool : pools)
		pool->PruneFromContainer();
	
	Vector<int> rmlist;
	for(int i = 0; i < node.sub.GetCount(); i++) {
		auto& s = node.sub[i];
		Entity* e = s.ext ? CastPtr<Ecs::Entity>(&*s.ext) : 0;
		if (e && e->destroyed)
			rmlist << i;
	}
	node.sub.Remove(rmlist);
}

void Pool::Dump() {
	LOG(GetTreeString());
}

String Pool::GetTreeString(int indent) {
	String s;
	
	String pre;
	pre.Cat('\t', indent);
	
	s << ".." << name << "[" << id << "]\n";
	
	auto objects = node.FindAll<Ecs::Entity>();
	for (EntityPtr& e : objects)
		s << e->GetTreeString(indent+1);
	
	auto pools = node.FindAll<Pool>();
	for (PoolPtr& p : pools)
		s << p->GetTreeString(indent+1);
	
	return s;
}

PoolPtr Pool::FindPool(String name) {
	auto pools = node.FindAll<Pool>();
	for (PoolPtr& p : pools) {
		if (p->GetName() == name)
			return p;
	}
	return PoolPtr();
}

EntityPtr Pool::FindEntityByName(String name) {
	auto objects = node.FindAll<Ecs::Entity>();
	for (EntityPtr object : objects)
		if (object->GetName() == name)
			return object;
	return EntityPtr();
}

PoolPtr Pool::AddPool(String name) {
	Pool& p = node.Add<Pool>();
	p.SetName(name);
	p.SetId(GetNextId());
	return &p;
}

PoolPtr Pool::GetAddPool(String name) {
	auto pools = node.FindAll<Pool>();
	for (PoolPtr& pool : pools)
		if (pool->GetName() == name)
			return pool;
	return AddPool(name);
}

void Pool::RemoveEntity(Entity* e) {
	auto objects = node.FindAll<Ecs::Entity>();
	int i = 0;
	auto it = objects.begin();
	auto end = objects.end();
	while (it != end) {
		if (e == &**it) {
			objects.Remove(i);
			break;
		}
		++i;
		++it;
	}
}

ComponentBasePtr Pool::RealizeComponentPath(const Vector<String>& path) {
	int c = path.GetCount();
	ASSERT(c >= 2);
	if (c < 2) return ComponentBasePtr();
	
	String ent_name = path[c-2];
	String comp_name = path[c-1];
	ASSERT(!ent_name.IsEmpty());
	ASSERT(!comp_name.IsEmpty());
	
	Pool* pool = this;
	for(int i = 0; i < c-2; i++) {
		String pool_name = path[i];
		ASSERT(!pool_name.IsEmpty());
		pool = &*pool->GetAddPool(pool_name);
		ASSERT(pool);
	}
	
	EntityPtr ent = pool->GetAddEmpty(ent_name);
	ComponentBasePtr comp = ent->GetAdd(comp_name);
	ASSERT(comp);
	
	return comp;
}

EntityPtr Pool::RealizeEntityPath(const Vector<String>& path) {
	int c = path.GetCount();
	ASSERT(c >= 1);
	if (c < 1) return EntityPtr();
	
	String ent_name = path[c-1];
	ASSERT(!ent_name.IsEmpty());
	
	Pool* pool = this;
	for(int i = 0; i < c-1; i++) {
		String pool_name = path[i];
		ASSERT(!pool_name.IsEmpty());
		pool = &*pool->GetAddPool(pool_name);
		ASSERT(pool);
	}
	
	EntityPtr ent = pool->GetAddEmpty(ent_name);
	
	return ent;
}



#if 0
bool PoolHashVisitor::OnEntry(const RTTI& type, TypeCls derived, const char* derived_name, void* mem, LockedScopeRefCounter* ref) {
	if (derived == AsTypeCls<Pool>()) {
		Pool& p = *(Pool*)mem;
		ch.Put(p.GetId());
	}
	return true;
}
#endif





} END_UPP_NAMESPACE
