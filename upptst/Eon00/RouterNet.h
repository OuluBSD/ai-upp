#ifndef _Eon00_RouterNet_h_
#define _Eon00_RouterNet_h_

#include "Eon00.h"

NAMESPACE_UPP

struct RouterPortDesc : Moveable<RouterPortDesc> {
	enum class Direction { Sink, Source };

	Direction direction = Direction::Sink;
	String    name;
	int       index = -1;
	ValueMap  metadata;
};

struct RouterAtomSpec : Moveable<RouterAtomSpec> {
	String id;
	String action;
	ArrayMap<String, Value> args;
	Vector<RouterPortDesc> ports;
	int next_sink_index = 0;
	int next_source_index = 0;

	RouterPortDesc& AddPort(RouterPortDesc::Direction dir, const String& name) {
		RouterPortDesc& port = ports.Add();
		port.direction = dir;
		port.name = name;
		if (dir == RouterPortDesc::Direction::Source)
			port.index = next_source_index++;
		else
			port.index = next_sink_index++;
		return port;
	}

	const RouterPortDesc* FindPort(RouterPortDesc::Direction dir, int port_index) const {
		for (const RouterPortDesc& port : ports)
			if (port.direction == dir && port.index == port_index)
				return &port;
		return nullptr;
	}
};

struct RouterConnectionDesc : Moveable<RouterConnectionDesc> {
	String src_atom;
	int    src_port = -1;
	String dst_atom;
	int    dst_port = -1;
};

class RouterNetContext {
public:
	explicit RouterNetContext(String loop_path) : loop_path(std::move(loop_path)) {}
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

	void Connect(const String& src_atom, int src_port, const String& dst_atom, int dst_port) {
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
		conn.src_atom = src_atom;
		conn.src_port = src_port;
		conn.dst_atom = dst_atom;
		conn.dst_port = dst_port;
	}

	Eon::LoopContext* AppendToChain(Engine& eng, Eon::ChainContext& cc, bool make_primary_links) const {
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
				LOG(Format("    %s #%d %s", dir, port.index, label));
			}
		}
		for (const RouterConnectionDesc& conn : connections)
			LOG(Format("  connect %s:%d -> %s:%d", conn.src_atom, conn.src_port, conn.dst_atom, conn.dst_port));
	}

private:
	void BuildAtomSpecs(Vector<Eon::ChainContext::AtomSpec>& atom_specs) const {
		using namespace Eon;

		for (int i = 0; i < atoms.GetCount(); i++) {
			const RouterAtomSpec& net_atom = atoms[i];
			ChainContext::AtomSpec& spec = atom_specs.Add();
			spec.idx = i;
			spec.action = net_atom.action;
			spec.args = net_atom.args;

			AtomTypeCls atom;
			LinkTypeCls link;
			if (!ChainContext::ResolveAction(spec.action, atom, link))
				throw Exc(Format("RouterNetContext: unknown atom action '%s'", spec.action));
			spec.iface.Realize(atom);
			spec.link = link;
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
	Vector<RouterAtomSpec> atoms;
	Index<String> atom_index;
	Vector<RouterConnectionDesc> connections;
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
