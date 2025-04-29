#include "Eon.h"


NAMESPACE_UPP


Space::Space(MetaNode& n) : MetaNodeExt(n) {
	DBG_CONSTRUCT
}

Space::~Space() {
	DBG_DESTRUCT
}

SpaceId Space::GetNextId() {
	static Atomic next_id;
	return ++next_id;
}

Loop* Space::GetLoop() const {
	return loop;
}

/*Space* Space::GetParent() const {
	return static_cast<Space*>(RefScopeParent<SpaceParent>::GetParentUnsafe().b);
}*/

/*Machine& Space::GetMachine() const {
	if (machine)
		return *machine;
	const Space* l = this;
	int levels = 0;
	while (l && levels++ < 1000) {
		const SpaceParent& par = l->RefScopeParent<SpaceParent>::GetParent();
		if (par.a) {
			machine = &static_cast<SpaceStore*>(par.a)->GetMachine();
			ASSERT(machine);
			return *machine;
		}
		ASSERT(l != par.b);
		l = static_cast<Space*>(par.b);
	}
	throw Exc("Machine ptr not found");
}*/

AtomBasePtr Space::GetTypeCls(AtomTypeCls atom_type) {
	for (AtomBase& comp : atoms) {
		AtomTypeCls type = comp.GetType();
		ASSERT(type.IsValid());
		if (type == atom_type)
			return &comp;
	}
	return 0;
}

AtomBasePtr Space::AddTypeCls(AtomTypeCls cls) {
	return AddPtr(GetMachine().Get<AtomStore>()->CreateAtomTypeCls(cls));
}

AtomBasePtr Space::GetAddTypeCls(AtomTypeCls cls) {
	AtomBasePtr cb = FindTypeCls(cls);
	return cb ? cb : AddPtr(GetMachine().Get<AtomStore>()->CreateAtomTypeCls(cls));
}

AtomBasePtr Space::FindTypeCls(AtomTypeCls atom_type) {
	for (AtomBaseRef& comp : atoms) {
		AtomTypeCls type = comp->GetType();
		if (type == atom_type)
			return comp;
	}
	return AtomBaseRef();
}

AtomBasePtr Space::FindAtom(TypeCls atom_type) {
	for (AtomBaseRef& comp : atoms) {
		#if IS_TS_CORE
		// This version checks inherited types too
		void* base = comp->GetBasePtr(atom_type);
		if (base)
			return comp;
		#else
		TypeCls type = comp->GetTypeId();
		if (type == atom_type)
			return comp;
		#endif
	}
	return AtomBaseRef();
}

AtomBasePtr Space::AddPtr(AtomBase* comp) {
	comp->SetParent(this);
	TypeCls type = comp->GetTypeId();
	atoms.AddBase(comp);
	InitializeAtom(*comp);
	return AtomBaseRef(this, comp);
}

void Space::InitializeAtoms() {
	for(auto& comp : atoms.GetValues())
		InitializeAtom(*comp);
}

void Space::InitializeAtom(AtomBase& comp) {
	comp.SetParent(this);
}


void Space::ClearAtoms() {
	AtomStorePtr sys = GetMachine().Get<AtomStore>();
	for (auto iter = atoms.rbegin(); iter; --iter)
		sys->ReturnAtom(atoms.Detach(iter));
	ASSERT(atoms.IsEmpty());
}

void Space::CopyTo(Space& l) const {
	l.AppendCopy(*this);
}

void Space::AppendCopy(const Space& l) {
	TODO
}

void Space::Visit(Vis& vis) {
	vis || atoms;
	vis || spaces;
	vis || states;
}

void Space::VisitSinks(Vis& vis) {
	for(AtomBaseRef& c : atoms)
		c->VisitSink(vis);
}

void Space::VisitSources(Vis& vis){
	for(AtomBaseRef& c : atoms)
		c->VisitSource(vis);
}

int Space::GetSpaceDepth() const {
	int d = 0;
	const Space* p = this;
	while (1) {
		p = p->GetParent();
		if (!p) break;
		++d;
	}
	return d;
}

bool Space::HasSpaceParent(SpacePtr pool) const {
	const Space* p = this;
	while (p) {
		if (p == &*pool)
			return true;
		p = p->GetParent();
	}
	return false;
}

void Space::Initialize(Space& l, String prefab) {
	uint64 ticks = GetMachine().GetTicks();
	l.SetPrefab(prefab);
	l.SetCreated(ticks);
	l.SetChanged(ticks);
	
}

SpacePtr Space::CreateEmpty() {
	Space& l = spaces.Add();
	l.SetParent(this);
	l.SetId(GetNextId());
	Initialize(l);
	return l;
}

void Space::Clear() {
	// useless ClearInterfacesDeep();
	UnrefDeep();
	UninitializeAtomsDeep();
	UnlinkDeep();
	ClearAtomsDeep();
	ClearStatesDeep();
	ClearDeep();
}

void Space::UnrefDeep() {
	RefClearVisitor vis;
	vis.Visit(*this);
}

void Space::UninitializeAtomsDeep() {
	for (auto p = spaces.rpbegin(), end = spaces.rpend(); p != end; --p)
		p->UninitializeAtomsDeep();
	
	for (auto it = atoms.rpbegin(); it != atoms.rpend(); --it) {
		it().UninitializeDeep();
	}
	
}

void Space::StopDeep() {
	for (auto it = spaces.rpbegin(); it != spaces.rpend(); --it) {
		it().StopDeep();
	}
	
	Stop();
}

void Space::Stop() {
	for (auto it = atoms.rpbegin(); it != atoms.rpend(); --it) {
		if (it->IsRunning()) {
			it->Stop();
			it->SetRunning(false);
		}
	}
}

void Space::UnlinkDeep() {
	for (auto it = spaces.rpbegin(); it != spaces.rpend(); --it) {
		it().UnlinkDeep();
	}
	
	UnlinkExchangePoints();
	
}

void Space::ClearStatesDeep() {
	for (auto p = spaces.pbegin(), end = spaces.pend(); p != end; ++p)
		p->ClearStatesDeep();
	
	states.Clear();
}

void Space::ClearAtomsDeep() {
	for (auto p = spaces.pbegin(), end = spaces.pend(); p != end; ++p)
		p->ClearAtomsDeep();
	
	AtomStore* sys = GetMachine().GetPtr<AtomStore>();
	for (auto it = atoms.rpbegin(); it != atoms.rpend(); --it) {
		sys->ReturnAtom(atoms.Detach(it));
	}
	
}

void Space::ClearDeep() {
	for (auto p = spaces.pbegin(), end = spaces.pend(); p != end; ++p)
		p->ClearDeep();
	spaces.Clear();
	
	atoms.Clear();
	states.Clear();
}

SpacePtr Space::GetAddEmpty(String name) {
	SpacePtr l = FindSpaceByName(name);
	if (l)
		return l;
	l = CreateEmpty();
	l->SetName(name);
	return l;
}

SpacePtr Space::FindSpaceByName(String name) {
	for (SpacePtr object : spaces)
		if (object->GetName() == name)
			return object;
	return SpacePtr();
}

AtomBasePtr Space::FindDeepCls(TypeCls type) {
	AtomBasePtr b = FindAtom(type);
	if (b)
		return b;
	for (SpacePtr object : spaces) {
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
	
	s << ".." << (name.IsEmpty() ? (String)"unnamed" : "\"" + name + "\"") << "[" << (int)id << "]\n";
	
	for (AtomBaseRef& a : atoms)
		s << a->ToString();
	
	for (SpacePtr& l : spaces)
		s << l->GetTreeString(indent+1);
	
	return s;
}

EnvStatePtr Space::FindNearestState(String name) {
	Space* l = this;
	while (l) {
		EnvStatePtr e = l->FindState(name);
		if (e)
			return e;
		l = l->GetParent();
	}
	return EnvStateRef();
}

EnvStatePtr Space::FindStateDeep(String name) {
	EnvStatePtr e = FindState(name);
	if (e)
		return e;
	
	for (SpacePtr& p : spaces) {
		EnvStatePtr e = p->FindStateDeep(name);
		if (e)
			return e;
	}
	
	return EnvStateRef();
}

String Space::GetDeepName() const {
	String s = name;
	Space* l = GetParent();
	while (l) {
		s = l->name + "." + s;
		l = l->GetParent();
	}
	return s;
}

void Space::UnlinkExchangePoints() {
	for (ExchangePointRef& pt : pts) {
		pt->Source()	->ClearLink();
		pt->Sink()		->ClearLink();
		pt->Clear();
	}
	pts.Clear();
}



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




END_UPP_NAMESPACE
