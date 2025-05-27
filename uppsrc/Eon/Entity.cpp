#include "Eon.h"


NAMESPACE_UPP


Entity::Entity(VfsValue& n) : VfsValueExt(n) {
	DBG_CONSTRUCT
}

Entity::~Entity() {
	//UnrefDeep();
	UninitializeComponents();
	ClearComponents();
	DBG_DESTRUCT
}

void Entity::Visit(Vis& v) {
	_VIS_((int&)id)
	 VIS_(created)
	 VIS_(changed)
	 VIS_(prefab)
	 VIS_(name)
	 VIS_((int&)m_id);
}

void Entity::UnrefDeep() {
	RefClearVisitor vis;
	val.Visit(vis);
}

EntityId Entity::GetNextId() {
	static Atomic next_id;
	return ++next_id;
}

String Entity::GetTreeString(int indent) {
	String s;
	
	s.Cat('\t', indent);
	
	s << (name.IsEmpty() ? (String)"unnamed" : "\"" + name + "\"") << ": " << prefab << "\n";
	
	auto comps = val.FindAll<ComponentBase>();
	for (auto& c : comps) {
		s.Cat('\t', indent+1);
		s << c->ToString();
		s.Cat('\n');
	}
	
	return s;
}

void Entity::OnChange() {
	changed = GetEngine().GetTicks();
}

ComponentBasePtr Entity::GetTypeCls(TypeCls comp_type) {
	auto comps = val.FindAll<ComponentBase>();
	for (auto& c : comps) {
		TypeCls type = c->GetTypeCls();
		if (type == comp_type)
			return c;
	}
	return ComponentBasePtr();
}

ComponentBasePtr Entity::GetAddTypeCls(TypeCls cls) {
	ComponentBasePtr cb = FindTypeCls(cls);
	if (cb)
		return cb;
	int i = Ecs::ComponentFactory::CompDataMap().Find(cls);
	if (i < 0)
		return 0;
	VfsValue& n = val.Add();
	n.id = ToVarName(ClassPathTop(cls.GetName()));
	auto* p = Ecs::ComponentFactory::CompDataMap()[i].new_fn(n);
	n.ext = p;
	n.type_hash = p->GetTypeHash();
	ASSERT(n.type_hash);
	return p;
}

ComponentBasePtr Entity::FindTypeCls(TypeCls comp_type) {
	auto comps = val.FindAll<ComponentBase>();
	for (auto& c : comps) {
		TypeCls type = c->GetTypeCls();
		if (type == comp_type)
			return c;
	}
	return ComponentBasePtr();
}

ComponentBasePtr Entity::AddPtr(ComponentBase* comp) {
	TODO
	#if 0
	comps.AddBase(comp);
	InitializeComponent(*comp);
	return comp;
	#endif
	return 0;
}

void Entity::InitializeComponents() {
	auto comps = val.FindAll<ComponentBase>();
	for(auto& comp : comps)
		InitializeComponent(*comp);
}

void Entity::InitializeComponent(ComponentBase& comp) {
	comp.Initialize();
}

void Entity::UninitializeComponents() {
	auto comps = val.FindAll<ComponentBase>();
	int dbg_i = 0;
	for (auto it = comps.End()-1; it != comps.Begin()-1; --it) {
		(*it)->Uninitialize();
		dbg_i++;
	}
}

void Entity::ClearComponents() {
	val.RemoveAllDeep<ComponentBase>();
}

Entity* Entity::Clone() const {
	EntityPtr ent = GetPool().Clone(*this);
	ent->InitializeComponents();
	return ent;
}

ComponentBasePtr Entity::CreateEon(String id) {
	int i = ComponentFactory::CompEonIds().Find(id);
	if (i < 0)
		return ComponentBasePtr();
	
	const auto& d = ComponentFactory::CompDataMap()[i];
	return GetAddTypeCls(d.rtti_cls);
}

ComponentBasePtr Entity::CreateComponent(TypeCls type) {
	return ComponentFactory::CreateComponent(val, type);
}

void Entity::Destroy() {
	Destroyable::Destroy();
	
	auto comps = val.FindAll<ComponentBase>();
	for (auto& component : comps)
		component->Destroy();
	
	//if (auto es = GetEngine().TryGet<EntityStore>())
	//	es->AddToDestroyList(this);
	
}

Engine& Entity::GetEngine() {
	return GetPool().GetEngine();
}

const Engine& Entity::GetEngine() const {
	return GetPool().GetEngine();
}

Pool& Entity::GetPool() const {
	Pool* p = val.GetOwnerExt<Pool>();
	ASSERT(p);
	return *p;
}

Pool& Entity::GetRoot() {
	Pool* p = val.FindOwnerRoot<Pool>();
	ASSERT(p);
	if (!p) throw Exc("no pool");
	return *p;
}

void Entity::GetEntityPath(Vector<String>& path) {
	Pool* p = val.FindOwner<Pool>();
	while (p) {
		Pool* par = p->val.FindOwner<Pool>();
		if (!par)
			break;
		path.Add(p->GetName());
		p = par;
	}
	Reverse(path);
}

int Entity::GetPoolDepth() const {
	int d = 0;
	Pool* p = &GetPool();
	while (1) {
		p = p->GetParent();
		if (!p) break;
		++d;
	}
	return d;
}

bool Entity::HasPoolParent(Pool* pool) const {
	return val.IsOwnerDeep(*pool);
}

ComponentBasePtr Entity::GetAdd(String comp_name) {
	TypeCls type = ComponentFactory::GetComponentType(comp_name);
	if (type == TypeCls())
		return ComponentBasePtr();
	
	ComponentBasePtr c = FindTypeCls(type);
	if (c)
		return c;
	
	c = this->CreateComponent(type);
	ASSERT(c);
	return c;
}






#if 0
bool EntityHashVisitor::OnEntry(const RTTI& type, TypeCls derived, const char* derived_name, void* mem, LockedScopeRefCounter* ref) {
	if (derived == AsTypeCls<Ecs::Entity>()) {
		Entity& e = *(Entity*)mem;
		ch.Put(1);
		ch.Put(e.GetId());
	}
	else if (derived == AsTypeCls<Pool>()) {
		Pool& p = *(Pool*)mem;
		ch.Put(2);
		ch.Put(p.GetId());
	}
	return true;
}
#endif


END_UPP_NAMESPACE
