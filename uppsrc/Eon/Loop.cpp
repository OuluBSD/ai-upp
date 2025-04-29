#include "Eon.h"


NAMESPACE_UPP


Loop::Loop() {
	DBG_CONSTRUCT
}

Loop::~Loop() {
	DBG_DESTRUCT
}

Loop* Loop::GetParent() const {
	return static_cast<Loop*>(RefScopeParent<LoopParent>::GetParentUnsafe().b);
}

Space* Loop::GetSpace() const {
	return space;
}

void Loop::AppendCopy(const Loop& l) {
	TODO
}

int Loop::GetLoopDepth() const {
	int d = 0;
	const Loop* p = this;
	while (1) {
		p = p->GetParent();
		if (!p) break;
		++d;
	}
	return d;
}

bool Loop::HasLoopParent(LoopPtr pool) const {
	const Loop* p = this;
	while (p) {
		if (p == &*pool)
			return true;
		p = p->GetParent();
	}
	return false;
}

void Loop::Clear() {
	UnrefDeep();
	UninitializeLinksDeep();
	ClearDeep();
}

void Loop::UnrefDeep() {
	RefClearVisitor vis;
	vis.Visit(*this);
}

void Loop::UninitializeLinksDeep() {
	for (LoopPtr& l : loops)
		l->UninitializeLinksDeep();
	
	for (auto it = links.rbegin(); it != links.rend(); --it) {
		it().Uninitialize();
	}
	
}

void Loop::ClearDeep() {
	for (LoopPtr& p : loops)
		p->ClearDeep();
	loops.Clear();
	
	links.Clear();
}

LoopPtr Loop::GetAddEmpty(String name) {
	LoopPtr l = FindLoopByName(name);
	if (l)
		return l;
	l = CreateEmpty();
	l->SetName(name);
	return l;
}

LoopPtr Loop::CreateEmpty() {
	Loop& l = loops.Add();
	l.SetParent(this);
	l.SetId(GetNextId());
	Initialize(l);
	return l;
}

void Loop::Initialize(Loop& l, String prefab) {
	l.SetPrefab(prefab);
	
}

void Loop::Visit(Vis& vis) {
	vis || links;
	vis || loops;
}

LinkBasePtr Loop::AddTypeCls(LinkTypeCls cls) {
	return AddPtr(space->GetMachine().Get<LinkStore>()->CreateLinkTypeCls(cls));
}

LinkBasePtr Loop::GetAddTypeCls(LinkTypeCls cls) {
	LinkBasePtr cb = FindTypeCls(cls);
	return cb ? cb : AddPtr(space->GetMachine().Get<LinkStore>()->CreateLinkTypeCls(cls));
}

LinkBasePtr Loop::AddPtr(LinkBase* comp) {
	comp->SetParent(this);
	LinkTypeCls type = comp->GetLinkType();
	links.Add(type, comp);
	InitializeLink(*comp);
	return LinkBasePtr(this, comp);
}

LinkBasePtr Loop::FindTypeCls(LinkTypeCls atom_type) {
	for (LinkBasePtr& l : links) {
		LinkTypeCls type = l->GetLinkType();
		if (type == atom_type)
			return l;
	}
	ASSERT(!links.Find(atom_type));
	return LinkBasePtr();
}

LoopPtr Loop::FindLoopByName(String name) {
	for (LoopPtr object : loops)
		if (object->GetName() == name)
			return object;
	return LoopPtr();
}

void Loop::Dump() {
	LOG(GetTreeString());
}

void Loop::InitializeLinks() {
	for(auto& comp : links.GetValues())
		InitializeLink(*comp);
}

void Loop::InitializeLink(LinkBase& comp) {
	comp.SetParent(this);
}

String Loop::GetTreeString(int indent) {
	String s;
	
	String pre;
	pre.Cat('\t', indent);
	
	s << ".." << (name.IsEmpty() ? (String)"unnamed" : "\"" + name + "\"") << "[" << (int)id << "]\n";
	
	for (LinkBasePtr& l : links)
		s << l->ToString();
	
	for (LoopPtr& l : loops)
		s << l->GetTreeString(indent+1);
	
	return s;
}

bool Loop::MakeLink(AtomBasePtr src_atom, AtomBasePtr dst_atom) {
	// This is for primary link (src_ch==0 to sink_ch== 0) only...
	InterfaceSourcePtr src = src_atom->GetSource();
	InterfaceSinkPtr sink = dst_atom->GetSink();
	ASSERT(src && sink);
	if (!src || !sink)
		return false;
	
	int src_ch = 0;
	int sink_ch = 0;
	
	
	Format src_fmt = src->GetSourceValue(src_ch).GetFormat();
	Format sink_fmt = sink->GetValue(sink_ch).GetFormat();
	if (src_fmt.vd != sink_fmt.vd) {
		LOG("error: sink and source device-value-class mismatch: src(" + src_fmt.vd.ToString() + "), sink(" + sink_fmt.vd.ToString() + ")");
		return false;
	}
	
	ASSERT(src_atom != dst_atom);
	ASSERT(src_atom->GetLink() != dst_atom->GetLink()); // "stupid" but important
	ASSERT(src	->AsAtomBase()->GetSpace()->GetLoop()->HasLoopParent(AsRefT()));
	ASSERT(sink	->AsAtomBase()->GetSpace()->GetLoop()->HasLoopParent(AsRefT()));
	CookiePtr src_cookie, sink_cookie;
	
	if (src->Accept(sink, src_cookie, sink_cookie)) {
		
		// Create exchange-point object
		auto& sdmap = Parallel::Factory::IfaceLinkDataMap();
		int i = sdmap.Find(src_fmt.vd);
		if (i < 0) {
			LOG("error: no exchange-point class set for type " + src_fmt.vd.ToString());
			ASSERT(0);
			return false;
		}
		const auto& src_d = sdmap[i];
		if (src_d.vd != src_fmt.vd) {
			ASSERT(0);
			LOG("internal error: unexpected sink class type");
			return false;
		}
		
		TypeCls expt_type = src_d.cls;
		ASSERT(expt_type != GetTypeIdClass<void>());
		
		ExchangePointPtr ep = space->MetaSpaceBase::Add(expt_type);
		RTLOG("Loop::Link(...): created " << ep->GetDynamicName() << " at " << HexStr(&ep->GetRTTI()));
		RTLOG("                 src-atom: " << HexStr(&src_atom->GetRTTI()));
		RTLOG("                 src-link: " << HexStr(&src_atom->GetLink()->GetRTTI()));
		RTLOG("                 dst-atom: " << HexStr(&dst_atom->GetRTTI()));
		RTLOG("                 dst-link: " << HexStr(&dst_atom->GetLink()->GetRTTI()));
		src->Link(ep, sink, src_cookie, sink_cookie);
		ep->Init(this->GetSpace());
		ep->Set(src, sink, src_cookie, sink_cookie);
		src_atom->GetLink()->SetPrimarySink(dst_atom->GetLink()->AsRefT());
		dst_atom->GetLink()->SetPrimarySource(src_atom->GetLink()->AsRefT());
		return true;
	}
	return false;
}

String Loop::GetDeepName() const {
	String s = name;
	Loop* l = GetParent();
	while (l) {
		s = l->name + "." + s;
		l = l->GetParent();
	}
	return s;
}

LoopId Loop::GetNextId() {
	static Atomic next_id;
	return ++next_id;
}




bool LoopHashVisitor::OnEntry(const RTTI& type, TypeCls derived, const char* derived_name, void* mem, LockedScopeRefCounter* ref) {
	if (derived == AsTypeCls<Loop>()) {
		Loop& e = *(Loop*)mem;
		ch.Put(1);
		ch.Put(e.GetId());
	}
	else if (derived == AsTypeCls<Loop>()) {
		Loop& p = *(Loop*)mem;
		ch.Put(2);
		ch.Put(p.GetId());
	}
	return true;
}

END_UPP_NAMESPACE
