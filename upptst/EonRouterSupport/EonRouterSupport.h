#ifndef _EonRouterSupport_EonRouterSupport_h_
#define _EonRouterSupport_EonRouterSupport_h_

#include <Shell/Shell.h>
#include <Eon/Core/Context.h>
#include <Vfs/Ecs/Interface.h>
#include <Vfs/Storage/VfsStorage.h>

NAMESPACE_UPP

struct RouterAtomSpec : Moveable<RouterAtomSpec> {
	String id;
	String action;
	ArrayMap<String, Value> args;
	Array<RouterPortDesc> ports;
	int next_sink_index = 0;
	int next_source_index = 0;
	mutable AtomTypeCls atom_type;
	mutable LinkTypeCls link_type;
	mutable bool type_resolved = false;

	RouterPortDesc& AddPort(RouterPortDesc::Direction dir, const String& name) {
		RouterPortDesc& port = ports.Add();
		port.direction = dir;
		port.name = name;
		if (dir == RouterPortDesc::Direction::Source)
			port.index = next_source_index++;
		else
			port.index = next_sink_index++;
		if (!EnsureTypeResolved())
			throw Exc(Format("RouterAtomSpec: unknown action '%s' while describing port '%s'", action, name));
		ApplyPortTraits(port);
		return port;
	}

	const RouterPortDesc* FindPort(RouterPortDesc::Direction dir, int port_index) const {
		for (const RouterPortDesc& port : ports)
			if (port.direction == dir && port.index == port_index)
				return &port;
		return nullptr;
	}

	bool EnsureTypeResolved() const {
		if (type_resolved)
			return true;
		AtomTypeCls resolved_atom;
		LinkTypeCls resolved_link;
		if (!Eon::ChainContext::ResolveAction(action, resolved_atom, resolved_link))
			return false;
		atom_type = resolved_atom;
		link_type = resolved_link;
		type_resolved = true;
		return true;
	}

private:
	void ApplyPortTraits(RouterPortDesc& port) const {
		const ValDevTuple::Channel* ch = ResolveChannel(port.direction, port.index);
		if (!ch)
			return;
		port.vd.Clear();
		port.vd.Add(ch->vd, ch->is_opt);
		port.metadata.Set("optional", ch->is_opt);
		port.metadata.Set("vd", ch->vd.ToString());
		port.metadata.Set("vd_name", ch->vd.GetName());
	}

	const ValDevTuple::Channel* ResolveChannel(RouterPortDesc::Direction dir, int port_index) const {
		if (!type_resolved)
			return nullptr;
		const ValDevTuple& tuple = dir == RouterPortDesc::Direction::Source ? atom_type.iface.src : atom_type.iface.sink;
		if (port_index < 0 || port_index >= tuple.GetCount())
			return nullptr;
		return &tuple[port_index];
	}
};

class RouterNetContext {
public:
	explicit RouterNetContext(String loop_path) : loop_path(std::move(loop_path)) {
		ResetFlowControlDefaults();
	}
	RouterAtomSpec& AddAtom(const String& id, const String& action) {
		if (atom_index.Find(id) >= 0)
			throw Exc(Format("RouterNetContext: duplicate atom id '%s'", id));
		RouterAtomSpec& atom = atoms.Add();
		atom.id = id;
		atom.action = action;
		atom_index.Add(id);
		return atom;
	}

	RouterPortDesc& AddPort(const String& atom_id, RouterPortDesc::Direction dir, const String& name = String()) {
		RouterAtomSpec* atom = FindAtom(atom_id);
		if (!atom)
			throw Exc(Format("RouterNetContext: unknown atom '%s' while adding port", atom_id));
		return atom->AddPort(dir, name);
	}

	const RouterAtomSpec* GetAtom(const String& id) const {
		return FindAtom(id);
	}

	const Vector<RouterConnectionDesc>& GetConnections() const {
		return connections;
	}

	ValueMap& FlowControlMetadata() {
		return flow_control;
	}

	const ValueMap& FlowControlMetadata() const {
		return flow_control;
	}

	void SetFlowControlPolicy(const String& policy, int credits_per_port = 1) {
		flow_control.Set("policy", policy);
		flow_control.Set("credits_per_port", credits_per_port);
	}

	ValueMap GetRouterMetadata() const {
		return StoreRouterSchema(BuildRouterSchema());
	}

	RouterConnectionDesc& Connect(const String& src_atom, int src_port, const String& dst_atom, int dst_port) {
		const RouterAtomSpec* src = FindAtom(src_atom);
		const RouterAtomSpec* dst = FindAtom(dst_atom);
		if (!src)
			throw Exc(Format("RouterNetContext: missing source atom '%s'", src_atom));
		if (!dst)
			throw Exc(Format("RouterNetContext: missing sink atom '%s'", dst_atom));
		if (!src->FindPort(RouterPortDesc::Direction::Source, src_port))
			throw Exc(Format("RouterNetContext: atom '%s' has no source port #%d", src_atom, src_port));
		if (!dst->FindPort(RouterPortDesc::Direction::Sink, dst_port))
			throw Exc(Format("RouterNetContext: atom '%s' has no sink port #%d", dst_atom, dst_port));
		RouterConnectionDesc& conn = connections.Add();
		conn.from_atom = src_atom;
		conn.from_port = src_port;
		conn.to_atom = dst_atom;
		conn.to_port = dst_port;
		conn.metadata.Set("policy", String("legacy-loop"));
		conn.metadata.Set("credits", 1);
		return conn;
	}

	Eon::LoopContext* AppendToChain(Engine& eng, Eon::ChainContext& cc, bool make_primary_links = true) const {
		using namespace Eon;

		VfsValue* loop_space = EnsureLoopPath(eng);
		if (!loop_space) {
			LOG("RouterNetContext: failed to resolve loop path '" << loop_path << "'");
			return nullptr;
		}

		Vector<ChainContext::AtomSpec> atom_specs;
		BuildAtomSpecs(atom_specs);

		LoopContext& loop = cc.AddLoop(*loop_space, atom_specs, make_primary_links);
		if (loop.failed) {
			LOG("RouterNetContext: AddLoop failed while appending '" << loop_path << "'");
			return nullptr;
		}
		StoreRouterMetadata(*loop_space);
		return &loop;
	}

	bool BuildLegacyLoop(Engine& eng) const {
		using namespace Eon;

		ChainContext cc;
		LoopContext* loop = AppendToChain(eng, cc);
		if (!loop)
			return false;
		LOG(cc.GetTreeString(0));
		LOG(loop->GetTreeString(0));
		DumpTopology();

		if (!cc.PostInitializeAll()) {
			LOG("RouterNetContext: PostInitialize failed");
			return false;
		}
		if (!cc.StartAll()) {
			LOG("RouterNetContext: Start failed");
			cc.UndoAll();
			return false;
		}

		return true;
	}

	void DumpTopology() const {
		LOG(Format("RouterNetContext[%s]: %d atoms, %d connections", loop_path, atoms.GetCount(), connections.GetCount()));
		for (const RouterAtomSpec& atom : atoms) {
			LOG(Format("  atom %s -> %s", atom.id, atom.action));
			for (const RouterPortDesc& port : atom.ports) {
				const char* dir = port.direction == RouterPortDesc::Direction::Source ? "src" : "sink";
				String label = port.name.IsEmpty() ? String("(unnamed)") : port.name;
				String vd = port.vd.IsValid() ? port.vd.ToString() : String("vd: ?");
				LOG(Format("    %s #%d %s [%s]", dir, port.index, label, vd));
				if (!port.metadata.IsEmpty())
					LOG("      meta: " << AsJSON(Value(port.metadata)));
			}
		}
		for (const RouterConnectionDesc& conn : connections) {
			LOG(Format("  connect %s:%d -> %s:%d", conn.from_atom, conn.from_port, conn.to_atom, conn.to_port));
			if (!conn.metadata.IsEmpty())
				LOG("      meta: " << AsJSON(Value(conn.metadata)));
		}
	}

	void SetSideSourceLink(const String& atom_id, int src_ch, int conn_id, int sink_ch) {
		AddSideLink(atom_id, true, src_ch, conn_id, sink_ch);
	}

	void SetSideSinkLink(const String& atom_id, int sink_ch, int conn_id, int src_ch) {
		AddSideLink(atom_id, false, sink_ch, conn_id, src_ch);
	}

private:
	struct SideLinkSpec : Moveable<SideLinkSpec> {
		String atom_id;
		bool is_source = false;
		int local_ch = -1;
		int other_ch = -1;
		int conn_id = -1;
	};

	void BuildAtomSpecs(Vector<Eon::ChainContext::AtomSpec>& atom_specs) const {
		using namespace Eon;

		for (int i = 0; i < atoms.GetCount(); i++) {
			const RouterAtomSpec& net_atom = atoms[i];
			ChainContext::AtomSpec& spec = atom_specs.Add();
			spec.idx = i;
			spec.action = net_atom.action;
			for (int j = 0; j < net_atom.args.GetCount(); j++)
				spec.args.GetAdd(net_atom.args.GetKey(j)) = net_atom.args[j];

			if (!net_atom.EnsureTypeResolved())
				throw Exc(Format("RouterNetContext: unknown atom action '%s'", spec.action));
			spec.iface.Realize(net_atom.atom_type);
			ApplySideLinks(spec.iface, net_atom.id);
			spec.link = net_atom.link_type;
		}
	}

	RouterAtomSpec* FindAtom(const String& id) {
		int idx = atom_index.Find(id);
		return idx >= 0 ? &atoms[idx] : nullptr;
	}

	const RouterAtomSpec* FindAtom(const String& id) const {
		int idx = atom_index.Find(id);
		return idx >= 0 ? &atoms[idx] : nullptr;
	}

	VfsValue* EnsureLoopPath(Engine& eng) const {
		VfsValue* loop = &eng.GetRootLoop();
		VfsValue* space = &eng.GetRootSpace();
		for (const String& part : Split(loop_path, ".")) {
			loop = &loop->GetAdd(part, 0);
			space = &space->GetAdd(part, 0);
		}
		(void)space;
		if (VfsValue* resolved = Eon::ResolveLoopPath(eng, loop_path))
			return resolved;
		return loop;
	}

	String loop_path;
	Array<RouterAtomSpec> atoms;
	Index<String> atom_index;
	Vector<RouterConnectionDesc> connections;
	Vector<SideLinkSpec> side_links;
	ValueMap flow_control;

	RouterSchema BuildRouterSchema() const {
		RouterSchema schema;
		for (const RouterAtomSpec& atom : atoms) {
			for (const RouterPortDesc& port : atom.ports) {
				RouterPortEntry& entry = schema.ports.Add();
				entry.atom_id = atom.id;
				entry.desc = port;
			}
		}
		for (const RouterConnectionDesc& conn : connections)
			schema.connections.Add(conn);
		if (!flow_control.IsEmpty())
			schema.flow_control = flow_control;
		if (schema.flow_control.IsEmpty()) {
			schema.flow_control.Set("policy", String("legacy-loop"));
			schema.flow_control.Set("credits_per_port", 1);
		}
		return schema;
	}

	void StoreRouterMetadata(VfsValue& loop_space) const {
		ValueMap router_value = StoreRouterSchema(BuildRouterSchema());
		if (router_value.IsEmpty())
			return;
		ValueMap loop_value;
		if (loop_space.value.Is<ValueMap>())
			loop_value = ValueMap(loop_space.value);
		loop_value.Set("router", router_value);
		loop_space.value = loop_value;
	}
private:
	void AddSideLink(const String& atom_id, bool is_source, int local_ch, int conn_id, int other_ch) {
		SideLinkSpec& link = side_links.Add();
		link.atom_id = atom_id;
		link.is_source = is_source;
		link.local_ch = local_ch;
		link.other_ch = other_ch;
		link.conn_id = conn_id;
	}

	void ApplySideLinks(Eon::IfaceConnTuple& iface, const String& atom_id) const {
		for (const SideLinkSpec& link : side_links) {
			if (link.atom_id != atom_id)
				continue;
			if (link.is_source) {
				iface.SetSource(link.conn_id, link.local_ch, link.other_ch);
			}
			else if (link.local_ch >= 0 && link.local_ch < iface.sink.GetCount()) {
				iface.sink[link.local_ch].Set(link.conn_id, link.local_ch, link.other_ch);
			}
		}
	}

	void ResetFlowControlDefaults() {
		flow_control.Clear();
		flow_control.Set("policy", String("legacy-loop"));
		flow_control.Set("credits_per_port", 1);
	}
};

inline bool BuildRouterChain(Engine& eng, const Vector<RouterNetContext*>& nets, const String& log_label = String()) {
	using namespace Eon;

	if (nets.IsEmpty()) {
		LOG("RouterNetContext: BuildRouterChain called without nets");
		return false;
	}

	ChainContext cc;
	for (RouterNetContext* net : nets) {
		if (!net)
			continue;
		net->DumpTopology();
		if (!net->AppendToChain(eng, cc))
			return false;
	}

	for (int i = 0; i < cc.loops.GetCount(); i++) {
		for (int j = 0; j < cc.loops.GetCount(); j++) {
			if (i == j)
				continue;
			if (!LoopContext::ConnectSides(cc.loops[i], cc.loops[j])) {
				LOG(Format("RouterNetContext: ConnectSides failed between loop %d and %d", i, j));
				return false;
			}
		}
	}

	String err_msg;
	if (!cc.ValidateSideLinks(&err_msg)) {
		LOG("RouterNetContext: side-link validation failed: " << err_msg);
		return false;
	}

	if (!cc.PostInitializeAll()) {
		LOG("RouterNetContext: PostInitialize failed for router chain");
		return false;
	}
	if (!cc.StartAll()) {
		LOG("RouterNetContext: Start failed for router chain");
		cc.UndoAll();
		return false;
	}

	LOG(cc.GetTreeString(0));
	if (!log_label.IsEmpty())
		LOG(log_label);
	return true;
}

END_UPP_NAMESPACE

#endif
