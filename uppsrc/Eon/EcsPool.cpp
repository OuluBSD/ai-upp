#include "Eon.h"

#if 0

NAMESPACE_UPP


//PoolPtr GetConnectorBasePool(ConnectorBase* conn) {return conn->GetPool();}

Engine& GetPoolEngine(PoolPtr pool) {return pool->GetEngine();}



Pool::Pool(VfsValue& n) : VfsValueExt(n) {
	DBG_CONSTRUCT
}

Pool::~Pool() {
	DBG_DESTRUCT
}

void Pool::Visit(Vis& v) {
	_VIS_(freeze_bits)
	 VIS_(id);
}

PoolId Pool::GetNextId() {
	static Atomic next_id;
	return ++next_id;
}

Pool* Pool::GetParent() const {
	return val.GetOwnerExt<Pool>();
}

Engine& Pool::GetEngine() {
	Engine* e = val.FindOwner<Engine>();
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

EntityPtr Pool::CreateEmpty(String id) {
	Entity& e = val.Add<Entity>();
	e.val.id = id;
	e.SetId(GetNextId());
	Initialize(e);
	return &e;
}

EntityPtr Pool::GetAddEmpty(String name) {
	EntityPtr e = FindEntityByName(name);
	if (e)
		return e;
	e = CreateEmpty(name);
	return e;
}

EntityPtr Pool::Clone(const Entity& c) {
	Entity& e = val.Add<Entity>();
	VisitCopy(c, e);
	return &e;
}

void Pool::UnlinkDeep() {
	auto pools = val.FindAll<Pool>();
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
	auto pools = val.FindAll<Pool>();
	for (PoolPtr& p : pools)
		p->UninitializeComponentsDeep();
	
	auto ents = val.FindAll<Entity>();
	for (int i = ents.GetCount()-1; i >= 0; i--)
		ents[i]->UninitializeComponents();
	
}

void Pool::ClearComponentsDeep() {
	auto pools = val.FindAll<Pool>();
	for (PoolPtr& p : pools)
		p->ClearComponentsDeep();
	
	auto ents = val.FindAll<Entity>();
	for (int i = ents.GetCount()-1; i >= 0; i--)
		ents[i]->ClearComponents();
	
}

void Pool::ClearDeep() {
	auto pools = val.FindAll<Pool>();
	for (PoolPtr& p : pools)
		if (p)
			p->ClearDeep();
	
	val.RemoveAllDeep<Pool>();
	val.RemoveAllDeep<Entity>();
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
	auto pools = val.FindAll<Pool>();
	for (auto& pool : pools)
		pool->PruneFromContainer();
	
	Vector<int> rmlist;
	for(int i = 0; i < val.sub.GetCount(); i++) {
		auto& s = val.sub[i];
		Entity* e = s.ext ? CastPtr<Entity>(&*s.ext) : 0;
		if (e && e->destroyed)
			rmlist << i;
	}
	val.sub.Remove(rmlist);
}

void Pool::Dump() {
	LOG(GetTreeString());
}

String Pool::GetTreeString(int indent) {
	String s;
	
	String pre;
	pre.Cat('\t', indent);
	
	s << ".." << val.id << "[" << id << "]\n";
	
	auto objects = val.FindAll<Entity>();
	for (EntityPtr& e : objects)
		s << e->GetTreeString(indent+1);
	
	auto pools = val.FindAll<Pool>();
	for (PoolPtr& p : pools)
		s << p->GetTreeString(indent+1);
	
	return s;
}

PoolPtr Pool::FindPool(String name) {
	auto pools = val.FindAll<Pool>();
	for (PoolPtr& p : pools) {
		if (p->GetName() == name)
			return p;
	}
	return PoolPtr();
}

EntityPtr Pool::FindEntityByName(String name) {
	auto objects = val.FindAll<Entity>();
	for (EntityPtr object : objects)
		if (object->GetName() == name)
			return object;
	return EntityPtr();
}

PoolPtr Pool::AddPool(String name) {
	Pool& p = val.Add<Pool>();
	p.val.id = name;
	p.SetId(GetNextId());
	return &p;
}

PoolPtr Pool::GetAddPool(String name) {
	auto pools = val.FindAll<Pool>();
	for (PoolPtr& pool : pools)
		if (pool->GetName() == name)
			return pool;
	return AddPool(name);
}

void Pool::RemoveEntity(Entity* e) {
	auto objects = val.FindAll<Entity>();
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

ComponentPtr Pool::RealizeComponentPath(const Vector<String>& path) {
	int c = path.GetCount();
	ASSERT(c >= 2);
	if (c < 2) return ComponentPtr();
	
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
	ComponentPtr comp = ent->GetAdd(comp_name);
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


END_UPP_NAMESPACE

#endif
