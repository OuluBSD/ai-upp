#include "Core.h"

NAMESPACE_UPP
namespace Eon {

static String Indent(int n) { String s; s.Cat('\t', n); return s; }

static String PathOf(VfsValue& v) {
    Vector<String> parts;
    VfsValue* it = &v;
    while (it) {
        if (it->id.GetCount())
            parts.Add(it->id);
        VfsValue* owner = it->FindOwnerNull();
        if (!owner) break;
        it = owner;
    }
    String out;
    for (int i = parts.GetCount() - 1; i >= 0; i--) {
        if (!out.IsEmpty()) out << ".";
        out << parts[i];
    }
    return out;
}

static bool MakePrimaryLink(VfsValue& space, AtomBasePtr src_atom, AtomBasePtr dst_atom) {
    InterfaceSourcePtr src = src_atom->GetSource();
    ExchangeSourceProviderPtr src_ep = CastPtr<ExchangeSourceProvider>(&*src);
    if (!src_ep)
        return false;

    auto sink = dst_atom->GetSink();
    Ptr<ExchangeSinkProvider> sinkT = dst_atom->GetSinkT<ExchangeSinkProvider>();
    if (!src || !sink || !sinkT)
        return false;

    int src_ch = 0;
    int sink_ch = 0;

    ValueFormat src_fmt = src->GetSourceValue(src_ch).GetFormat();
    ValueFormat sink_fmt = sink->GetValue(sink_ch).GetFormat();
    if (src_fmt.vd != sink_fmt.vd)
        return false;

    ASSERT(src_atom != dst_atom);
    ASSERT(src_atom->GetLink() != dst_atom->GetLink());

    CookiePtr src_cookie, sink_cookie;
    if (!src->Accept(sinkT, src_cookie, sink_cookie))
        return false;

    // create exchange point in owning space
    auto& sdmap = VfsValueExtFactory::IfaceLinkDataMap();
    int i = sdmap.Find(src_fmt.vd);
    if (i < 0)
        return false;
    const auto& src_d = sdmap[i];
    if (src_d.vd != src_fmt.vd)
        return false;

    String name = src_d.name;
    hash_t type_hash = src_d.type_hash;
    auto owner_space = space.FindOwnerNull();
    if (!type_hash || !owner_space)
        return false;

    VfsValue& link_val = owner_space->Add(name, type_hash);
    ExchangePointPtr ep = link_val.FindExt<ExchangePoint>();
    if (!ep)
        return false;

    src->Link(ep, sinkT, src_cookie, sink_cookie);
    ep->Init(owner_space);
    ep->Set(src_ep, sinkT, src_cookie, sink_cookie);
    src_atom->GetLink()->SetPrimarySink(dst_atom->GetLink());
    dst_atom->GetLink()->SetPrimarySource(src_atom->GetLink());
    return true;
}

VfsValue* ResolveLoopPath(Engine& eng, const Vector<String>& parts) {
    VfsValue* l1 = &eng.GetRootLoop();
    VfsValue* result = nullptr;
    for (int i = 0; i < parts.GetCount(); i++) {
        const String& part = parts[i];
        bool last = i == parts.GetCount() - 1;
        if (last)
            result = &l1->GetAdd(part, 0);
        else
            l1 = &l1->GetAdd(part, 0);
    }
    return result ? result : l1;
}

VfsValue* ResolveLoopPath(Engine& eng, const String& dotted) {
    Vector<String> parts = Split(dotted, ".");
    return ResolveLoopPath(eng, parts);
}

PoolContext::PoolContext(VfsValue& v, ErrorSource* err, const FileLocation* loc)
    : v(v), err(err) {
    if (loc)
        this->loc = *loc;
}

PoolContext PoolContext::AddPool(const String& name, const ArrayMap<String, Value>* args, const FileLocation* loc) {
    VfsValue& pool0 = v.GetAdd(name, 0);
    (void)args; // args reserved for future pool-level properties
    return PoolContext(pool0, err, loc ? loc : &this->loc);
}

EntityContext PoolContext::AddEntity(const String& name, const ArrayMap<String, Value>* args, const FileLocation* /*loc*/) {
    Entity& ent = v.GetAdd<Entity>(name);
    (void)args; // entity-level args optional; components carry most args
    return EntityContext(ent, err);
}

String PoolContext::GetTreeString(int indent) const {
    String s;
    s << Indent(indent) << "PoolContext path: " << PathOf(const_cast<VfsValue&>(v)) << '\n';
    return s;
}

EntityContext::EntityContext(Entity& e, ErrorSource* err)
    : ent(e), err(err) {}

ComponentContext EntityContext::AddComponent(const String& comp_id, const ArrayMap<String, Value>* args, const FileLocation* /*loc*/) {
    ComponentPtr cb = ent.CreateEon(comp_id);
    if (!cb)
        return ComponentContext();
    if (args) {
        for (int i = 0; i < args->GetCount(); i++) {
            String key = args->GetKey(i);
            const Value& value = args->operator[](i);
            if (!value.IsVoid())
                cb->Arg(key, value);
        }
    }
    return ComponentContext(*cb, err);
}

String EntityContext::GetTreeString(int indent) const {
    String s;
    s << Indent(indent) << "EntityContext: " << ent.val.id << '\n';
    return s;
}

String ComponentContext::GetTreeString(int indent) const {
    String s;
    s << Indent(indent) << "ComponentContext: ";
    if (comp)
        s << comp->GetTypeName();
    else
        s << "<null>";
    s << '\n';
    return s;
}

LoopContext::LoopContext(VfsValue& space)
    : space(space) {}

AtomBasePtr LoopContext::AddAtom(AtomTypeCls atom, LinkTypeCls link, const IfaceConnTuple& iface, const ArrayMap<String, Value>* args, int idx) {
    // create atom
    VfsValue& sub_atom = space.Add();
    int ai = VfsValueExtFactory::AtomDataMap().Find(atom);
    if (ai < 0) return nullptr;
    const auto& af = VfsValueExtFactory::AtomDataMap()[ai];
    VfsValueExt* aext = af.new_fn(sub_atom);
    AtomBasePtr ab = CastPtr<AtomBase>(aext);
    if (!ab) return nullptr;
    sub_atom.id = ToVarName(ClassPathTop(af.name));
    sub_atom.ext = ab;
    sub_atom.type_hash = ab->GetTypeHash();

    LinkBasePtr lb;
    if (link.IsValid()) {
        VfsValue& sub_link = space.Add();
        int li = VfsValueExtFactory::LinkDataMap().Find(link);
        if (li >= 0) {
            const auto& lf = VfsValueExtFactory::LinkDataMap()[li];
            VfsValueExt* lext = lf.new_fn(sub_link);
            lb = CastPtr<LinkBase>(lext);
            sub_link.id = ToVarName(ClassPathTop(lf.name));
            sub_link.ext = lb;
            sub_link.type_hash = lb->GetTypeHash();
        }
        if (!lb) return nullptr;
        lb->atom = CastPtr<Atom>(&*ab);
        ab->link = &*lb;
        if (idx >= 0) lb->SetId(idx);
    }

    if (idx >= 0) ab->SetIdx(idx);

    // set interface and initialize
    ab->SetInterface(iface);
    WorldState ws;
    if (args) {
        for (int i = 0; i < args->GetCount(); i++) {
            String key = args->GetKey(i);
            const Value& obj = args->operator[](i);
            ws.values.GetAdd("." + key) = obj;
        }
    }
    if (!ab->InitializeAtom(ws) || !ab->Initialize(ws))
        return nullptr;
    ab->SetInitialized();
    if (lb) {
        if (!lb->Initialize(ws))
            return nullptr;
        lb->SetInitialized();
    }

    AddedAtom& rec = added.Add();
    rec.a = ab;
    rec.l = lb;
    rec.iface = iface;
    return ab;
}

bool LoopContext::MakePrimaryLinks() {
    if (added.IsEmpty() || added.GetCount() == 1)
        return true;
    for (int i = 0; i < added.GetCount(); i++) {
        AddedAtom& src = added[i];
        AddedAtom& dst = added[(i + 1) % added.GetCount()];
        if (!MakePrimaryLink(space, src.a, dst.a))
            return false;
    }
    return true;
}

bool LoopContext::ValidateSideLinks(String* err) const {
    const String loop_path = PathOf(const_cast<VfsValue&>(space));
    for (const auto& info : added) {
        const IfaceConnTuple& iface = info.iface;
        const String atom_name = info.a ? info.a->GetType().ToString() : String("<null>");
        for (int sink_ch = 1; sink_ch < iface.type.iface.sink.GetCount(); sink_ch++) {
            const IfaceConnLink& link = iface.sink[sink_ch];
            if (link.conn >= 0 && !iface.type.IsSinkChannelOptional(sink_ch)) {
                if (err)
                    *err = Format("Loop '%s': atom '%s' sink side channel %d not connected",
                                  loop_path, atom_name, sink_ch);
                return false;
            }
        }
        for (int src_ch = 1; src_ch < iface.type.iface.src.GetCount(); src_ch++) {
            const IfaceConnLink& link = iface.src[src_ch];
            if (link.conn >= 0 && !iface.type.IsSourceChannelOptional(src_ch)) {
                if (err)
                    *err = Format("Loop '%s': atom '%s' source side channel %d not connected",
                                  loop_path, atom_name, src_ch);
                return false;
            }
        }
    }
    return true;
}

bool LoopContext::ConnectSides(const LoopContext& loop0, const LoopContext& loop1) {
    for (const auto& sink_info : loop0.added) {
        LinkBasePtr sink_link = sink_info.a->GetLink();
        IfaceConnTuple& sink_iface = const_cast<IfaceConnTuple&>(sink_info.iface);
        for (int sink_ch = 1; sink_ch < sink_iface.type.iface.sink.GetCount(); sink_ch++) {
            IfaceConnLink& sink_conn = sink_iface.sink[sink_ch];
            if (sink_conn.conn < 0)
                continue;
            bool linked = false;
            for (const auto& src_info : loop1.added) {
                LinkBasePtr src_link = src_info.a->GetLink();
                IfaceConnTuple& src_iface = const_cast<IfaceConnTuple&>(src_info.iface);
                for (int src_ch = 1; src_ch < src_iface.type.iface.src.GetCount(); src_ch++) {
                    IfaceConnLink& src_conn = src_iface.src[src_ch];
                    if (src_conn.conn < 0)
                        continue;
                    if (sink_conn.conn == src_conn.conn) {
                        int src_ch_i = src_conn.local;
                        int sink_ch_i = sink_conn.local;
                        if (!src_link->LinkSideSink(sink_link, src_ch_i, sink_ch_i))
                            return false;
                        // Mark as linked; remaining passes will skip this pair.
                        sink_conn.conn = -1;
                        src_conn.conn = -1;
                        linked = true;
                        break;
                    }
                }
                if (linked)
                    break;
            }
        }
    }
    return true;
}

bool LoopContext::PostInitializeAll() {
    int c = added.GetCount() - 1;
    for (int i = c; i >= 0; i--) {
        const AddedAtom& a = added[i];
        if (!a.a->PostInitialize()) {
            RTLOG("LoopContext::PostInitializeAll: atom " << a.a->GetType().ToString() << " PostInitialize failed");
            return false;
        }
        if (a.l && !a.l->PostInitialize()) {
            RTLOG("LoopContext::PostInitializeAll: link " << a.l->GetTypeName() << " PostInitialize failed");
            return false;
        }
    }
    return true;
}

bool LoopContext::StartAll() {
    int c = added.GetCount() - 1;
    for (int i = c; i >= 0; i--) {
        const AddedAtom& a = added[i];
        if (!a.a->Start())
            return false;
        a.a->SetRunning();
        if (a.l && !a.l->Start())
            return false;
    }
    return true;
}

void LoopContext::UndoAll() {
    for (int i = added.GetCount() - 1; i >= 0; i--)
        added[i].a->Stop();
    for (int i = added.GetCount() - 1; i >= 0; i--)
        added[i].a->UninitializeDeep();
}

String LoopContext::GetTreeString(int indent) const {
    String s;
    s << Indent(indent) << "LoopContext space: " << PathOf(const_cast<VfsValue&>(space)) << '\n';
    int i = 0;
    for (const auto& a : added) {
        s << Indent(indent+1) << i++ << ": Atom " << a.iface.type.ToString();
        if (a.l)
            s << ", link: " << a.l->GetTypeName();
        s << '\n';
    }
    return s;
}

bool LoopContext::RegisterRouterPorts() {
    if (!router)
        router.Create();

    RTLOG("LoopContext::RegisterRouterPorts: registering " << added.GetCount() << " atoms");

    for (auto& info : added) {
        if (info.a) {
            info.a->RegisterPorts(*router);
        }
    }

    RTLOG("LoopContext::RegisterRouterPorts: router has " << router->GetPortCount() << " ports");
    return true;
}

bool LoopContext::MakeRouterConnections() {
    if (!router || added.GetCount() < 2)
        return true;

    RTLOG("LoopContext::MakeRouterConnections: connecting " << added.GetCount() << " atoms");

    // For each atom pair (i, i+1), connect source port 0 to sink port 0
    // This mirrors the primary link topology
    for (int i = 0; i < added.GetCount(); i++) {
        AddedAtom& src_info = added[i];
        AddedAtom& dst_info = added[(i + 1) % added.GetCount()];

        if (!src_info.a || !dst_info.a)
            continue;

        // Get router indices for primary ports (port 0)
        // These are stored in AtomBase::router_source_ports and router_sink_ports
        const Vector<int>& src_ports = src_info.a->router_source_ports;
        const Vector<int>& dst_ports = dst_info.a->router_sink_ports;

        if (src_ports.IsEmpty() || dst_ports.IsEmpty()) {
            // Atom didn't register ports - skip (backward compatible)
            continue;
        }

        int src_router_idx = src_ports[0];
        int dst_router_idx = dst_ports[0];

        if (src_router_idx < 0 || dst_router_idx < 0)
            continue;

        // Build PortHandles
        PacketRouter::PortHandle src_handle;
        src_handle.atom = &*src_info.a;
        src_handle.port_index = 0;
        src_handle.direction = RouterPortDesc::Direction::Source;
        src_handle.router_index = src_router_idx;

        PacketRouter::PortHandle dst_handle;
        dst_handle.atom = &*dst_info.a;
        dst_handle.port_index = 0;
        dst_handle.direction = RouterPortDesc::Direction::Sink;
        dst_handle.router_index = dst_router_idx;

        router->Connect(src_handle, dst_handle);

        RTLOG("LoopContext::MakeRouterConnections: connected atom " << i
            << " port " << src_router_idx << " -> atom " << ((i + 1) % added.GetCount())
            << " port " << dst_router_idx);
    }

    RTLOG("LoopContext::MakeRouterConnections: router has " << router->GetConnectionCount() << " connections");
    return true;
}

String LoopContext::DumpRouterTopology() const {
    if (!router)
        return "No router";
    return router->DumpTopology();
}

static bool FindAtomByAction(const String& action, AtomTypeCls& out_atom, LinkTypeCls& out_link) {
    const auto& map = VfsValueExtFactory::AtomDataMap();
    for (const auto& data : map.GetValues()) {
        bool match = false;
        for (const String& a : data.actions) {
            if (a == action) { match = true; break; }
        }
        if (!match) continue;
        out_atom = data.cls;
        out_link = data.link_type;
        return out_atom.IsValid();
    }
    return false;
}

bool ChainContext::ResolveAction(const String& action, AtomTypeCls& out_atom, LinkTypeCls& out_link) {
    return FindAtomByAction(action, out_atom, out_link);
}

LoopContext& ChainContext::AddLoop(VfsValue& loop_space, const Vector<AtomSpec>& atoms, bool make_primary_links) {
    LoopContext& lc = loops.Add(new LoopContext(loop_space));
    RTLOG("ChainContext::AddLoop: space=" << PathOf(loop_space)
        << " atoms=" << atoms.GetCount()
        << " make_primary_links=" << make_primary_links);
    int idx = 0;
    for (const auto& spec : atoms) {
        AtomTypeCls atom;
        LinkTypeCls link;
        if (!spec.action.IsEmpty()) {
            if (!FindAtomByAction(spec.action, atom, link)) {
                // unresolved action; fail fast by skipping add
                return lc;
            }
        }
        else {
            atom = spec.iface.type; // allow caller to pre-set
        }
        if (spec.link.IsValid())
            link = spec.link;
        int use_idx = spec.idx >= 0 ? spec.idx : idx;
        RTLOG("  atom #" << idx << " action=" << spec.action
            << " type=" << atom.ToString()
            << " link=" << (link.IsValid() ? link.ToString() : String("<null>")));
        if (!lc.AddAtom(atom, link, spec.iface, &spec.args, use_idx)) {
            RTLOG("ChainContext::AddLoop: atom #" << idx << " failed to initialize");
            lc.failed = true;
            return lc;
        }
        idx++;
    }
    auto has_audio_channel = [](const ValDevTuple& tuple) {
        for (int i = 0; i < tuple.GetCount(); i++)
            if (tuple[i].vd.val == ValCls::AUDIO)
                return true;
        return false;
    };
    bool loop_has_audio = false;
    for (const auto& info : lc.added) {
        if (!info.a)
            continue;
        const AtomTypeCls type = info.a->GetType();
        if (has_audio_channel(type.iface.sink) || has_audio_channel(type.iface.src)) {
            loop_has_audio = true;
            break;
        }
    }
    if (loop_has_audio) {
        for (auto& info : lc.added) {
            if (!info.a)
                continue;
            if (CustomerBase* customer = dynamic_cast<CustomerBase*>(&*info.a))
                customer->EnsureAudioDefaultQueue();
        }
    }
    if (make_primary_links)
        lc.MakePrimaryLinks();

    // Router integration (Phase 2)
    // Register ports and generate router connections
    lc.RegisterRouterPorts();
    lc.MakeRouterConnections();

    return lc;
}

bool ChainContext::PostInitializeAll() {
    for (auto& lc : loops)
        if (!lc.PostInitializeAll())
            return false;
    return true;
}

bool ChainContext::StartAll() {
    for (auto& lc : loops)
        if (!lc.StartAll())
            return false;
    return true;
}

void ChainContext::UndoAll() {
    for (int i = loops.GetCount() - 1; i >= 0; i--)
        loops[i].UndoAll();
}

bool ChainContext::ValidateSideLinks(String* err) const {
    for (const auto& lc : loops)
        if (!lc.ValidateSideLinks(err))
            return false;
    return true;
}

String ChainContext::GetTreeString(int indent) const {
    String s;
    s << Indent(indent) << "ChainContext\n";
    for (const auto& lc : loops)
        s << lc.GetTreeString(indent+1);
    return s;
}

// NetContext implementation (Phase 3)

NetContext::NetContext(VfsValue& space)
    : net_space(space) {
    router.Create();
}

AtomBasePtr NetContext::AddAtom(const String& name, const String& action, const IfaceConnTuple& iface, const ArrayMap<String, Value>* args) {
    // Resolve action to atom type and link type
    AtomTypeCls atom_type;
    LinkTypeCls link_type;
    if (!FindAtomByAction(action, atom_type, link_type)) {
        RTLOG("NetContext::AddAtom: could not resolve action '" << action << "'");
        failed = true;
        return nullptr;
    }

    // Create atom using VfsValueExtFactory
    VfsValue& sub_atom = net_space.Add();
    int ai = VfsValueExtFactory::AtomDataMap().Find(atom_type);
    if (ai < 0) {
        RTLOG("NetContext::AddAtom: atom type not found in factory");
        failed = true;
        return nullptr;
    }

    const auto& af = VfsValueExtFactory::AtomDataMap()[ai];
    VfsValueExt* aext = af.new_fn(sub_atom);
    AtomBasePtr ab = CastPtr<AtomBase>(aext);
    if (!ab) {
        RTLOG("NetContext::AddAtom: failed to cast to AtomBase");
        failed = true;
        return nullptr;
    }

    sub_atom.id = name; // Use atom name from definition
    sub_atom.ext = ab;
    sub_atom.type_hash = ab->GetTypeHash();

    // Set interface
    ab->SetInterface(iface);

    // Initialize with args
    WorldState ws;
    if (args) {
        for (int i = 0; i < args->GetCount(); i++) {
            String key = args->GetKey(i);
            const Value& obj = args->operator[](i);
            ws.values.GetAdd("." + key) = obj;
        }
    }

    if (!ab->InitializeAtom(ws) || !ab->Initialize(ws)) {
        RTLOG("NetContext::AddAtom: initialization failed for '" << name << "'");
        failed = true;
        return nullptr;
    }

    ab->SetInitialized();

    // Store atom instance
    AtomInstance& inst = atoms.Add();
    inst.name = name;
    inst.atom = ab;
    inst.iface = iface;

    RTLOG("NetContext::AddAtom: created atom '" << name << "' (" << action << ")");
    return ab;
}

void NetContext::AddConnection(int from_atom_idx, int from_port, int to_atom_idx, int to_port) {
    Connection& conn = connections.Add();
    conn.from_atom_idx = from_atom_idx;
    conn.from_port = from_port;
    conn.to_atom_idx = to_atom_idx;
    conn.to_port = to_port;
}

bool NetContext::RegisterPorts() {
    if (!router) {
        RTLOG("NetContext::RegisterPorts: router not initialized");
        return false;
    }

    RTLOG("NetContext::RegisterPorts: registering " << atoms.GetCount() << " atoms");

    for (auto& inst : atoms) {
        if (inst.atom) {
            inst.atom->RegisterPorts(*router);
        }
    }

    RTLOG("NetContext::RegisterPorts: router has " << router->GetPortCount() << " ports");
    return true;
}

bool NetContext::MakeConnections() {
    if (!router) {
        RTLOG("NetContext::MakeConnections: router not initialized");
        return false;
    }

    RTLOG("NetContext::MakeConnections: wiring " << connections.GetCount() << " connections");

    for (const Connection& conn : connections) {
        if (conn.from_atom_idx < 0 || conn.from_atom_idx >= atoms.GetCount()) {
            RTLOG("NetContext::MakeConnections: invalid from_atom_idx " << conn.from_atom_idx);
            return false;
        }
        if (conn.to_atom_idx < 0 || conn.to_atom_idx >= atoms.GetCount()) {
            RTLOG("NetContext::MakeConnections: invalid to_atom_idx " << conn.to_atom_idx);
            return false;
        }

        AtomInstance& from_inst = atoms[conn.from_atom_idx];
        AtomInstance& to_inst = atoms[conn.to_atom_idx];

        if (!from_inst.atom || !to_inst.atom) {
            RTLOG("NetContext::MakeConnections: null atom pointer");
            return false;
        }

        // Get router port indices
        const Vector<int>& src_ports = from_inst.atom->router_source_ports;
        const Vector<int>& dst_ports = to_inst.atom->router_sink_ports;

        if (conn.from_port < 0 || conn.from_port >= src_ports.GetCount()) {
            RTLOG("NetContext::MakeConnections: invalid from_port " << conn.from_port
                << " (atom has " << src_ports.GetCount() << " source ports)");
            return false;
        }
        if (conn.to_port < 0 || conn.to_port >= dst_ports.GetCount()) {
            RTLOG("NetContext::MakeConnections: invalid to_port " << conn.to_port
                << " (atom has " << dst_ports.GetCount() << " sink ports)");
            return false;
        }

        int src_router_idx = src_ports[conn.from_port];
        int dst_router_idx = dst_ports[conn.to_port];

        // Build PortHandles
        PacketRouter::PortHandle src_handle;
        src_handle.atom = &*from_inst.atom;
        src_handle.port_index = conn.from_port;
        src_handle.direction = RouterPortDesc::Direction::Source;
        src_handle.router_index = src_router_idx;

        PacketRouter::PortHandle dst_handle;
        dst_handle.atom = &*to_inst.atom;
        dst_handle.port_index = conn.to_port;
        dst_handle.direction = RouterPortDesc::Direction::Sink;
        dst_handle.router_index = dst_router_idx;

        router->Connect(src_handle, dst_handle);

        RTLOG("NetContext::MakeConnections: connected " << from_inst.name << ":" << conn.from_port
            << " -> " << to_inst.name << ":" << conn.to_port);
    }

    RTLOG("NetContext::MakeConnections: router has " << router->GetConnectionCount() << " connections");
    return true;
}

bool NetContext::PostInitializeAll() {
    RTLOG("NetContext::PostInitializeAll: " << atoms.GetCount() << " atoms");
    for (int i = atoms.GetCount() - 1; i >= 0; i--) {
        if (atoms[i].atom && !atoms[i].atom->PostInitialize()) {
            RTLOG("NetContext::PostInitializeAll: atom " << atoms[i].name << " failed");
            return false;
        }
    }
    return true;
}

bool NetContext::StartAll() {
    RTLOG("NetContext::StartAll: " << atoms.GetCount() << " atoms");
    for (int i = atoms.GetCount() - 1; i >= 0; i--) {
        if (atoms[i].atom) {
            if (!atoms[i].atom->Start()) {
                RTLOG("NetContext::StartAll: atom " << atoms[i].name << " failed to start");
                return false;
            }
            atoms[i].atom->SetRunning();
        }
    }
    return true;
}

void NetContext::UndoAll() {
    for (int i = atoms.GetCount() - 1; i >= 0; i--) {
        if (atoms[i].atom) {
            atoms[i].atom->Stop();
        }
    }
    for (int i = atoms.GetCount() - 1; i >= 0; i--) {
        if (atoms[i].atom) {
            atoms[i].atom->UninitializeDeep();
        }
    }
}

String NetContext::GetTreeString(int indent) const {
    String s;
    s << Indent(indent) << "NetContext space: " << PathOf(const_cast<VfsValue&>(net_space)) << '\n';
    s << Indent(indent+1) << "Atoms: " << atoms.GetCount() << '\n';
    for (int i = 0; i < atoms.GetCount(); i++) {
        s << Indent(indent+2) << i << ": " << atoms[i].name;
        if (atoms[i].atom)
            s << " (" << atoms[i].iface.type.ToString() << ")";
        s << '\n';
    }
    s << Indent(indent+1) << "Connections: " << connections.GetCount() << '\n';
    for (const Connection& conn : connections) {
        s << Indent(indent+2) << atoms[conn.from_atom_idx].name << ":" << conn.from_port
            << " -> " << atoms[conn.to_atom_idx].name << ":" << conn.to_port << '\n';
    }
    return s;
}

// Phase 5: Net execution driver
void NetContext::Update(double dt) {
    // Update all atoms
    for (auto& inst : atoms) {
        if (inst.atom) {
            inst.atom->Update(dt);
        }
    }

    // Drive packet flow
    accumulated_time += dt;
    ProcessFrame();
}

int NetContext::ProcessFrame(int max_iterations) {
    if (!router) {
        RTLOG("NetContext::ProcessFrame: no router");
        return 0;
    }

    int packets_routed = 0;
    int iteration = 0;

    // Process packets up to max_iterations
    while (iteration < max_iterations) {
        bool any_sent = false;

        // Poll source atoms for packets
        for (auto& inst : atoms) {
            if (!inst.atom) continue;

            // Check if atom has source ports
            const Vector<int>& src_ports = inst.atom->router_source_ports;
            if (src_ports.IsEmpty()) continue;

            // For each source port, try to get a packet
            for (int src_ch = 0; src_ch < src_ports.GetCount(); src_ch++) {
                // Create packet value for Send() to fill
                off32 packet_offset = packet_offset_gen.Create();
                PacketValue pv(packet_offset);

                // Get source format
                InterfaceSourcePtr src = inst.atom->GetSource();
                if (!src || src_ch >= src->GetSourceCount())
                    continue;

                ValueFormat fmt = src->GetSourceValue(src_ch).GetFormat();
                pv.SetFormat(fmt);

                // Create config for Send() call
                RealtimeSourceConfig cfg(packet_offset_gen);
                cfg.time_total = accumulated_time;
                cfg.cur_offset = packet_offset;
                cfg.prev_offset = packet_offset;

                // Call atom's Send() method
                if (inst.atom->Send(cfg, pv, src_ch)) {
                    // Atom produced a packet - it will be routed via EmitViaRouter()
                    // which is called inside atom's Send() method
                    any_sent = true;
                }
            }
        }

        iteration++;
        iteration_count++;

        // If no atoms sent packets, we're done for this frame
        if (!any_sent)
            break;
    }

    // Query router for actual packets routed
    if (router) {
        packets_routed = router->GetTotalPacketsRouted();
    }

    RTLOG("NetContext::ProcessFrame: completed " << iteration << " iterations, "
        << packets_routed << " total packets routed");

    return packets_routed;
}

}
END_UPP_NAMESPACE
