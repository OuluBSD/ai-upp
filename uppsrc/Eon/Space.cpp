#include "Eon.h"


NAMESPACE_UPP


Space::Space(MetaNode& n) : MetaSpaceBase(n) {
	
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
	MetaNode* n = &node;
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
	auto atoms = node.FindAll<AtomBase>();
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
	TODO return 0;//return AddPtr(GetMachine().Get<AtomStore>()->CreateAtomTypeCls(cls));
}

AtomBasePtr Space::GetAddTypeCls(AtomTypeCls cls) {
	AtomBasePtr cb = FindTypeCls(cls);
	TODO return 0;//return cb ? cb : AddPtr(GetMachine().Get<AtomStore>()->CreateAtomTypeCls(cls));
}

AtomBasePtr Space::FindTypeCls(AtomTypeCls atom_type) {
	auto atoms = node.FindAll<AtomBase>();
	for (auto& it : atoms) {
		auto& comp = *it;
		AtomTypeCls type = comp.GetType();
		if (type == atom_type)
			return &comp;
	}
	return AtomBasePtr();
}

AtomBasePtr Space::FindAtom(AtomTypeCls atom_type) {
	auto atoms = node.FindAll<AtomBase>();
	for (auto& it : atoms) {
		auto& comp = *it;
		AtomTypeCls type = comp.GetType();
		if (type == atom_type)
			return &comp;
	}
	return AtomBasePtr();
}

AtomBasePtr Space::AddPtr(AtomBase* comp) {
	MetaNode& sub = node.Add();
	sub.kind = 0; TODO // solve from comp
	sub.ext = comp;
	InitializeAtom(*comp);
	return AtomBasePtr(comp);
}

void Space::InitializeAtoms() {
	auto atoms = node.FindAll<AtomBase>();
	for(auto& it : atoms)
		InitializeAtom(*it);
}

void Space::InitializeAtom(AtomBase& comp) {
	TODO //comp.SetParent(this);
}


void Space::ClearAtoms() {
	TODO
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

void Space::Visit(Vis& vis) {
	vis.VisitT<MetaSpaceBase>("MetaSpaceBase",*this);
}

void Space::VisitSinks(Vis& vis) {
	auto atoms = node.FindAll<AtomBase>();
	for(auto& it : atoms)
		it->VisitSink(vis);
}

void Space::VisitSources(Vis& vis){
	auto atoms = node.FindAll<AtomBase>();
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
	TODO/*uint64 ticks = GetMachine().GetTicks();
	l.SetPrefab(prefab);
	l.SetCreated(ticks);
	l.SetChanged(ticks);*/
}

SpacePtr Space::CreateEmpty() {
	TODO
	#if 0
	Space& l = spaces.Add();
	//l.SetParent(this);
	l.SetId(GetNextId());
	Initialize(l);
	return &l;
	#endif
	return 0;
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
	Visit(vis);
}

void Space::UninitializeAtomsDeep() {
	auto spaces = node.FindAll<Space>();
	for (int i = spaces.GetCount()-1; i >= 0; i--)
		spaces[i]->UninitializeAtomsDeep();
	
	auto atoms = node.FindAll<AtomBase>();
	for (int i = atoms.GetCount()-1; i >= 0; i--)
		atoms[i]->UninitializeDeep();
	
}

void Space::StopDeep() {
	auto spaces = node.FindAll<Space>();
	for (int i = spaces.GetCount()-1; i >= 0; i--)
		spaces[i]->StopDeep();
	
	Stop();
}

void Space::Stop() {
	auto atoms = node.FindAll<AtomBase>();
	for (int i = atoms.GetCount()-1; i >= 0; i--) {
		auto& it = *atoms[i];
		if (it.IsRunning()) {
			it.Stop();
			it.SetRunning(false);
		}
	}
}

void Space::UnlinkDeep() {
	auto atoms = node.FindAll<AtomBase>();
	for (int i = atoms.GetCount()-1; i >= 0; i--) {
		auto& it = *atoms[i];
		TODO //it.UnlinkDeep();
	}
	
	UnlinkExchangePoints();
	
}

void Space::ClearStatesDeep() {
	auto spaces = node.FindAll<Space>();
	for (auto& p : spaces)
		p->ClearStatesDeep();
	
	states.Clear();
}

void Space::ClearAtomsDeep() {
	auto spaces = node.FindAll<Space>();
	for (auto& p : spaces)
		p->ClearAtomsDeep();
	
	TODO
	/*AtomStore* sys = GetMachine().GetPtr<AtomStore>();
	for (int i = atoms.GetCount()-1; i >= 0; i--) {
		auto& it = atoms[i];
		sys->ReturnAtom(atoms.Detach(it));
	}*/
}

void Space::ClearDeep() {
	auto spaces = node.FindAll<Space>();
	for (auto& p : spaces)
		p->ClearDeep();
	spaces.Clear();
	
	node.RemoveAllShallow<AtomBase>();
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
	auto spaces = node.FindAll<Space>();
	for (Space* object : spaces)
		if (object->GetName() == name)
			return object;
	return SpacePtr();
}

AtomBasePtr Space::FindDeepCls(AtomTypeCls type) {
	AtomBasePtr b = FindAtom(type);
	if (b)
		return b;
	auto spaces = node.FindAll<Space>();
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
	
	s << ".." << (name.IsEmpty() ? (String)"unnamed" : "\"" + name + "\"") << "[" << (int)id << "]\n";
	
	auto atoms = node.FindAll<AtomBase>();
	for (auto& it : atoms)
		s << it->ToString();
	
	auto spaces = node.FindAll<Space>();
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
		l = l->node.FindOwner<Space>();
	}
	return EnvStatePtr();
}

EnvStatePtr Space::FindStateDeep(String name) {
	EnvStatePtr e = FindState(name);
	if (e)
		return e;
	
	auto spaces = node.FindAll<Space>();
	for (Space* p : spaces) {
		EnvStatePtr e = p->FindStateDeep(name);
		if (e)
			return e;
	}
	
	return EnvStatePtr();
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
	for (ExchangePointPtr& pt : pts) {
		pt->Source()	->ClearLink();
		pt->Sink()		->ClearLink();
		pt->Clear();
	}
	pts.Clear();
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
