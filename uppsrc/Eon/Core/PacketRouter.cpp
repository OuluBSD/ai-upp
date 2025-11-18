#include "Core.h"


NAMESPACE_UPP


// PacketRouter

PacketRouter::PacketRouter() {
	LOG("PacketRouter: Created");
}

PacketRouter::~PacketRouter() {
	LOG("PacketRouter: Destroyed (" << ports.GetCount() << " ports, "
	    << connection_table.GetCount() << " connections)");
}


PacketRouter::PortHandle PacketRouter::RegisterPort(AtomBase* atom,
                                                    RouterPortDesc::Direction dir,
                                                    int port_index,
                                                    const ValDevTuple& vd,
                                                    const ValueMap& metadata) {
	ASSERT(atom);
	ASSERT(port_index >= 0);
	ASSERT(vd.IsValid());

	Port& port = ports.Add();
	port.handle.atom = atom;
	port.handle.port_index = port_index;
	port.handle.direction = dir;
	port.handle.router_index = ports.GetCount() - 1;
	port.vd = vd;
	port.metadata = metadata;

	atom_index.FindAdd(atom);

	const char* dir_str = (dir == RouterPortDesc::Direction::Source) ? "source" : "sink";
	LOG("PacketRouter::RegisterPort: " << dir_str << " port " << port_index
	    << " for atom " << FormatPtr(atom) << " (vd=" << vd.ToString()
	    << ", router_idx=" << port.handle.router_index << ")");

	return port.handle;
}


void PacketRouter::Connect(PortHandle src, PortHandle dst, const ValueMap& policy) {
	String err_msg;
	if (!ValidateConnection(src, dst, &err_msg)) {
		LOG("PacketRouter::Connect: FAILED validation - " << err_msg);
		return;
	}

	Connection& conn = connection_table.Add();
	conn.src_port_idx = src.router_index;
	conn.dst_port_idx = dst.router_index;
	conn.policy = policy;
	conn.active = true;

	int conn_idx = connection_table.GetCount() - 1;

	// Update port connection lists
	Port* src_port = FindPort(src);
	Port* dst_port = FindPort(dst);
	ASSERT(src_port && dst_port);

	src_port->outgoing_connections.Add(conn_idx);
	dst_port->incoming_connections.Add(conn_idx);

	LOG("PacketRouter::Connect: src_port " << src.router_index
	    << " -> dst_port " << dst.router_index
	    << " (conn_idx=" << conn_idx << ")");
}


void PacketRouter::Disconnect(PortHandle src, PortHandle dst) {
	int src_idx = src.router_index;
	int dst_idx = dst.router_index;

	for (int i = 0; i < connection_table.GetCount(); i++) {
		Connection& conn = connection_table[i];
		if (conn.src_port_idx == src_idx && conn.dst_port_idx == dst_idx && conn.active) {
			conn.active = false;
			LOG("PacketRouter::Disconnect: deactivated connection " << i
			    << " (src=" << src_idx << ", dst=" << dst_idx << ")");
			return;
		}
	}

	LOG("PacketRouter::Disconnect: WARNING - no active connection found "
	    << "(src=" << src_idx << ", dst=" << dst_idx << ")");
}


bool PacketRouter::RoutePacket(PortHandle src_port, PacketValue& packet) {
	const Port* port = FindPort(src_port);
	if (!port) {
		LOG("PacketRouter::RoutePacket: ERROR - invalid source port");
		return false;
	}

	if (port->outgoing_connections.IsEmpty()) {
		LOG("PacketRouter::RoutePacket: WARNING - source port " << src_port.router_index
		    << " has no outgoing connections");
		return false;
	}

	// STUB: Actual packet routing not yet implemented
	// Will need to enqueue packet to destination port's buffer
	LOG("PacketRouter::RoutePacket: STUB - would route packet from port "
	    << src_port.router_index << " to " << port->outgoing_connections.GetCount()
	    << " destination(s)");

	// Increment diagnostic counters
	for (int conn_idx : port->outgoing_connections) {
		if (conn_idx >= 0 && conn_idx < connection_table.GetCount()) {
			connection_table[conn_idx].packets_routed++;
		}
	}

	return true;
}


int PacketRouter::RequestCredits(PortHandle port, int requested_count) {
	Port* p = FindPort(port);
	if (!p) {
		LOG("PacketRouter::RequestCredits: ERROR - invalid port");
		return 0;
	}

	p->credits_requested += requested_count;
	int granted = min(requested_count, p->credits_available);
	p->credits_available -= granted;

	LOG("PacketRouter::RequestCredits: port " << port.router_index
	    << " requested " << requested_count << ", granted " << granted
	    << " (available now: " << p->credits_available << ")");

	return granted;
}


void PacketRouter::AckCredits(PortHandle port, int ack_count) {
	Port* p = FindPort(port);
	if (!p) {
		LOG("PacketRouter::AckCredits: ERROR - invalid port");
		return;
	}

	p->credits_acked += ack_count;
	p->credits_available += ack_count;

	LOG("PacketRouter::AckCredits: port " << port.router_index
	    << " acked " << ack_count << " credits (available now: " << p->credits_available << ")");
}


int PacketRouter::AvailableCredits(PortHandle port) const {
	const Port* p = FindPort(port);
	return p ? p->credits_available : 0;
}


String PacketRouter::DumpTopology() const {
	String out;
	out << "PacketRouter Topology:\n";
	out << "  Atoms: " << atom_index.GetCount() << "\n";
	out << "  Ports: " << ports.GetCount() << "\n";
	out << "  Connections: " << connection_table.GetCount() << "\n\n";

	out << "Ports:\n";
	for (int i = 0; i < ports.GetCount(); i++) {
		const Port& port = ports[i];
		const char* dir = (port.handle.direction == RouterPortDesc::Direction::Source) ? "SRC" : "SNK";
		out << Format("  [%d] %s atom=%p port=%d vd=%s credits=%d out=%d in=%d\n",
		              i, dir,
		              port.handle.atom,
		              port.handle.port_index,
		              port.vd.ToString(),
		              port.credits_available,
		              port.outgoing_connections.GetCount(),
		              port.incoming_connections.GetCount());
	}

	out << "\nConnections:\n";
	for (int i = 0; i < connection_table.GetCount(); i++) {
		const Connection& conn = connection_table[i];
		if (!conn.active)
			continue;
		out << Format("  [%d] port %d -> port %d (routed: %d packets)\n",
		              i, conn.src_port_idx, conn.dst_port_idx, conn.packets_routed);
	}

	return out;
}


String PacketRouter::DumpPortStatus() const {
	String out;
	out << "PacketRouter Port Status:\n";
	for (int i = 0; i < ports.GetCount(); i++) {
		const Port& port = ports[i];
		out << Format("  Port %d: credits=%d requested=%d acked=%d\n",
		              i,
		              port.credits_available,
		              port.credits_requested,
		              port.credits_acked);
	}
	return out;
}


String PacketRouter::DumpConnectionTable() const {
	String out;
	out << "PacketRouter Connection Table:\n";
	for (int i = 0; i < connection_table.GetCount(); i++) {
		const Connection& conn = connection_table[i];
		const char* active_str = conn.active ? "active" : "inactive";
		out << Format("  [%d] %d -> %d (%s, routed=%d)\n",
		              i, conn.src_port_idx, conn.dst_port_idx,
		              active_str, conn.packets_routed);
	}
	return out;
}


PacketRouter::Port* PacketRouter::FindPort(PortHandle handle) {
	if (handle.router_index < 0 || handle.router_index >= ports.GetCount())
		return nullptr;
	Port& port = ports[handle.router_index];
	if (port.handle.atom != handle.atom || port.handle.port_index != handle.port_index)
		return nullptr;
	return &port;
}


const PacketRouter::Port* PacketRouter::FindPort(PortHandle handle) const {
	if (handle.router_index < 0 || handle.router_index >= ports.GetCount())
		return nullptr;
	const Port& port = ports[handle.router_index];
	if (port.handle.atom != handle.atom || port.handle.port_index != handle.port_index)
		return nullptr;
	return &port;
}


int PacketRouter::FindPortIndex(PortHandle handle) const {
	const Port* port = FindPort(handle);
	return port ? handle.router_index : -1;
}


bool PacketRouter::ValidateConnection(const PortHandle& src, const PortHandle& dst, String* err_msg) const {
	if (!src.IsValid()) {
		if (err_msg) *err_msg = "Invalid source port handle";
		return false;
	}

	if (!dst.IsValid()) {
		if (err_msg) *err_msg = "Invalid destination port handle";
		return false;
	}

	if (src.direction != RouterPortDesc::Direction::Source) {
		if (err_msg) *err_msg = "Source port must have Source direction";
		return false;
	}

	if (dst.direction != RouterPortDesc::Direction::Sink) {
		if (err_msg) *err_msg = "Destination port must have Sink direction";
		return false;
	}

	const Port* src_port = FindPort(src);
	const Port* dst_port = FindPort(dst);

	if (!src_port) {
		if (err_msg) *err_msg = "Source port not found in router";
		return false;
	}

	if (!dst_port) {
		if (err_msg) *err_msg = "Destination port not found in router";
		return false;
	}

	// TODO: Validate ValDevTuple compatibility between src and dst

	return true;
}


END_UPP_NAMESPACE
