#include "Eon.h"


NAMESPACE_UPP


Space::Space(VfsValue& n) : MetaSpaceBase(n) {
	
}

Space::~Space() {
	
}

SpaceId Space::GetNextId() {
	static Atomic next_id;
	return ++next_id;
}

Loop* Space::GetLoop() const {
	return loop;
}

Space* Space::GetParent() const {
	TODO return 0 ;//return static_cast<Space*>(RefScopeParent<SpaceParent>::GetParentUnsafe().b);
}

Machine& Space::GetMachine() const {
	if (machine)
		return *machine;
	VfsValue* n = &val;
	int levels = 0;
	while (n && levels++ < 1000) {
		machine = n->Find<Machine>();
		if (machine)
			return *machine;
		n = n->owner;
	}
	throw Exc("Machine ptr not found");
}

AtomBasePtr Space::AsTypeCls(AtomTypeCls atom_type) {
	auto atoms = val.FindAll<AtomBase>();
	for (auto& it : atoms) {
		auto& comp = *it;
		AtomTypeCls type = comp.GetType();
		ASSERT(type.IsValid());
		if (type == atom_type)
			return &comp;
	}
	return 0;
}

AtomBasePtr Space::AddTypeCls(AtomTypeCls cls) {
	VfsValue& sub = val.Add();
	
	int i = Factory::AtomDataMap().Find(cls);
	ASSERT_(i >= 0, "Invalid to create non-existant atom");
	if (i < 0) return 0;
	const auto& f = Factory::AtomDataMap()[i];
	AtomBase* obj = f.new_fn(sub);
	
	sub.id = ToVarName(ClassPathTop(f.name));
	sub.ext = obj;
	sub.type_hash = obj->GetTypeHash();
	InitializeAtom(*obj);
	return AtomBasePtr(obj);
}

AtomBasePtr Space::GetAddTypeCls(AtomTypeCls cls) {
	for (auto& n : val.sub) {
		if (n.ext) {
			AtomBase* atom = CastPtr<AtomBase>(&*n.ext);
			if (atom && atom->GetType() == cls) {
				return atom;
			}
		}
	}
	return AddTypeCls(cls);
}

AtomBasePtr Space::FindTypeCls(AtomTypeCls atom_type) {
	auto atoms = val.FindAll<AtomBase>();
	for (auto& it : atoms) {
		auto& comp = *it;
		AtomTypeCls type = comp.GetType();
		if (type == atom_type)
			return &comp;
	}
	return AtomBasePtr();
}

AtomBasePtr Space::FindAtom(AtomTypeCls atom_type) {
	auto atoms = val.FindAll<AtomBase>();
	for (auto& it : atoms) {
		auto& comp = *it;
		AtomTypeCls type = comp.GetType();
		if (type == atom_type)
			return &comp;
	}
	return AtomBasePtr();
}

AtomBasePtr Space::AddPtr(AtomBase* comp) {
	VfsValue& sub = val.Add();
	sub.ext = comp;
	sub.type_hash = comp->GetTypeHash();
	InitializeAtom(*comp);
	return AtomBasePtr(comp);
}

void Space::InitializeAtoms() {
	auto atoms = val.FindAll<AtomBase>();
	for(auto& it : atoms)
		InitializeAtom(*it);
}

void Space::InitializeAtom(AtomBase& comp) {
	ASSERT(comp.val.FindOwner<Space>() == this);
}


void Space::ClearAtoms() {
	Vector<int> rmlist;
	int i = 0;
	for (auto& n : val.sub) {
		if (n.ext && CastPtr<AtomBase>(&*n.ext))
			rmlist << i;
		i++;
	}
	if (!rmlist.IsEmpty())
		val.sub.Remove(rmlist);
	/*AtomStorePtr sys = GetMachine().Get<AtomStore>();
	for (auto iter = atoms.rbegin(); iter; --iter)
		sys->ReturnAtom(atoms.Detach(iter));
	ASSERT(atoms.IsEmpty());*/
}

void Space::CopyTo(Space& l) const {
	l.AppendCopy(*this);
}

void Space::AppendCopy(const Space& l) {
	TODO
}

void Space::Visit(Vis& v) {
	_VIS_(prefab)
	 VIS_((int&)id)
	 VIS_(created)
	 VIS_(changed)
	 VISV(states);
	v & machine & loop;
	v.VisitT<MetaSpaceBase>("MetaSpaceBase",*this);
}

void Space::VisitSinks(Vis& vis) {
	auto atoms = val.FindAll<AtomBase>();
	for(auto& it : atoms)
		it->VisitSink(vis);
}

void Space::VisitSources(Vis& vis){
	auto atoms = val.FindAll<AtomBase>();
	for(auto& it : atoms)
		it->VisitSource(vis);
}

int Space::GetSpaceDepth() const {
	int d = 0;
	const Space* p = this;
	TODO
	/*while (1) {
		p = p->GetParent();
		if (!p) break;
		++d;
	}*/
	return d;
}

bool Space::HasSpaceParent(SpacePtr pool) const {
	const Space* p = this;
	TODO
	/*while (p) {
		if (p == &*pool)
			return true;
		p = p->GetParent();
	}*/
	return false;
}

void Space::Initialize(Space& l, String prefab) {
	uint64 ticks = GetMachine().GetTicks();
	l.SetPrefab(prefab);
	l.SetCreated(ticks);
	l.SetChanged(ticks);
}

SpacePtr Space::CreateEmpty(String id) {
	Space& l = val.Add<Space>();
	l.val.id = id;
	l.SetId(GetNextId());
	Initialize(l);
	return &l;
}

void Space::Clear() {
	// useless ClearInterfacesDeep();
	//UnrefDeep();
	UninitializeAtomsDeep();
	UnlinkDeep();
	ClearAtomsDeep();
	ClearStatesDeep();
	ClearDeep();
}

void Space::UnrefDeep() {
	RefClearVisitor vis;
	Visit(vis);
}

void Space::UninitializeAtomsDeep() {
	auto spaces = val.FindAll<Space>();
	for (int i = spaces.GetCount()-1; i >= 0; i--)
		spaces[i]->UninitializeAtomsDeep();
	
	auto atoms = val.FindAll<AtomBase>();
	for (int i = atoms.GetCount()-1; i >= 0; i--)
		atoms[i]->UninitializeDeep();
	
}

void Space::StopDeep() {
	auto spaces = val.FindAll<Space>();
	for (int i = spaces.GetCount()-1; i >= 0; i--)
		spaces[i]->StopDeep();
	
	Stop();
}

void Space::Stop() {
	auto atoms = val.FindAll<AtomBase>();
	for (int i = atoms.GetCount()-1; i >= 0; i--) {
		auto& it = *atoms[i];
		if (it.IsRunning()) {
			it.Stop();
			it.SetRunning(false);
		}
	}
}

void Space::UnlinkDeep() {
	auto atoms = val.FindAll<AtomBase>();
	for (int i = atoms.GetCount()-1; i >= 0; i--) {
		auto& it = *atoms[i];
		TODO //it.UnlinkDeep();
	}
	
	UnlinkExchangePoints();
	
}

void Space::ClearStatesDeep() {
	auto spaces = val.FindAll<Space>();
	for (auto& p : spaces)
		p->ClearStatesDeep();
	
	states.Clear();
}

void Space::ClearAtomsDeep() {
	auto spaces = val.FindAll<Space>();
	for (auto& p : spaces)
		p->ClearAtomsDeep();
	
	/*AtomStore* sys = GetMachine().GetPtr<AtomStore>();
	for (int i = atoms.GetCount()-1; i >= 0; i--) {
		auto& it = atoms[i];
		sys->ReturnAtom(atoms.Detach(it));
	}*/
}

void Space::ClearDeep() {
	auto spaces = val.FindAll<Space>();
	for (auto& p : spaces)
		if (p)
			p->ClearDeep();
	spaces.Clear();
	
	val.RemoveAllShallow<AtomBase>();
	states.Clear();
}

SpacePtr Space::GetAddEmpty(String name) {
	SpacePtr l = FindSpaceByName(name);
	if (l)
		return l;
	l = CreateEmpty(name);
	return l;
}

SpacePtr Space::FindSpaceByName(String name) {
	auto spaces = val.FindAll<Space>();
	for (Space* object : spaces)
		if (object->val.id == name)
			return object;
	return SpacePtr();
}

AtomBasePtr Space::FindDeepCls(AtomTypeCls type) {
	AtomBasePtr b = FindAtom(type);
	if (b)
		return b;
	auto spaces = val.FindAll<Space>();
	for (Space* object : spaces) {
		b = object->FindDeepCls(type);
		if (b)
			return b;
	}
	return b;
}

void Space::Dump() {
	LOG(GetTreeString());
}

String Space::GetTreeString(int indent) {
	String s;
	
	String pre;
	pre.Cat('\t', indent);
	
	s << ".." << (val.id.IsEmpty() ? (String)"unnamed" : "\"" + val.id + "\"") << "[" << (int)id << "]\n";
	
	auto atoms = val.FindAll<AtomBase>();
	for (auto& it : atoms)
		s << it->ToString();
	
	auto spaces = val.FindAll<Space>();
	for (Space* l : spaces)
		s << l->GetTreeString(indent+1);
	
	return s;
}

EnvStatePtr Space::FindNearestState(String name) {
	Space* l = this;
	while (l) {
		EnvStatePtr e = l->FindState(name);
		if (e)
			return e;
		l = l->val.FindOwner<Space>();
	}
	return EnvStatePtr();
}

EnvStatePtr Space::FindStateDeep(String name) {
	EnvStatePtr e = FindState(name);
	if (e)
		return e;
	
	auto spaces = val.FindAll<Space>();
	for (Space* p : spaces) {
		EnvStatePtr e = p->FindStateDeep(name);
		if (e)
			return e;
	}
	
	return EnvStatePtr();
}

String Space::GetDeepName() const {
	String s = val.id;
	Space* l = GetParent();
	while (l) {
		s = l->val.id + "." + s;
		l = l->GetParent();
	}
	return s;
}

void Space::UnlinkExchangePoints() {
	for (ExchangePointPtr& pt : pts) {
		pt->Source()	->ClearLink();
		pt->Sink()		->ClearLink();
		pt->Clear();
	}
	pts.Clear();
}

SpacePtr Space::AddSpace(String name) {
	ASSERT(name.GetCount());
	Space& p = val.Add<Space>();
	p.val.id = name;
	p.SetId(GetNextId());
	return &p;
}

SpacePtr Space::GetAddSpace(String name) {
	ASSERT(name.GetCount());
	auto spaces = val.FindAll<Space>();
	for (auto& pool : spaces)
		if (pool->val.id == name)
			return pool;
	return AddSpace(name);
}

EnvStatePtr Space::AddState(String name) {
	EnvState& p = states.Add();
	//p.SetParent(this);
	p.SetName(name);
	return &p;
}

EnvStatePtr Space::GetAddEnv(String name) {
	if (EnvStatePtr e = FindState(name))
		return e;
	return AddState(name);
}

EnvStatePtr Space::FindState(String name) {
	for (EnvState& s : states)
		if (s.GetName() == name)
			return &s;
	return EnvStatePtr();
}


#if 0
bool SpaceHashVisitor::OnEntry(const RTTI& type, TypeCls derived, const char* derived_name, void* mem, LockedScopeRefCounter* ref) {
	if (derived == AsTypeCls<Space>()) {
		Space& e = *(Space*)mem;
		ch.Put(1);
		ch.Put(e.GetId());
	}
	else if (derived == AsTypeCls<Space>()) {
		Space& p = *(Space*)mem;
		ch.Put(2);
		ch.Put(p.GetId());
	}
	return true;
}
#endif




END_UPP_NAMESPACE
