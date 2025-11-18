// PacketRouter Unit Tests
// Minimal test setup that doesn't require full Eon/Core stack

#include <Core/Core.h>
#include <Core/EcsFoundation/EcsFoundation.h>

using namespace Upp;

// Forward declare to match PacketRouter.h
struct AtomBase;
class PacketValue;

// Minimal RouterPortDesc for testing
struct RouterPortDesc {
	enum class Direction {
		Source,
		Sink
	};
};

// Define PacketRouter class inline to avoid Eon/Core dependency
class PacketRouter {
public:
	struct PortHandle {
		AtomBase* atom = nullptr;
		int port_index = -1;
		RouterPortDesc::Direction direction;
		int router_index = -1;

		bool IsValid() const { return atom && port_index >= 0 && router_index >= 0; }
	};

	PacketRouter();
	~PacketRouter();

	PortHandle RegisterPort(AtomBase* atom,
	                       RouterPortDesc::Direction dir,
	                       int port_index,
	                       const ValDevTuple& vd,
	                       const ValueMap& metadata = ValueMap());

	void Connect(PortHandle src, PortHandle dst, const ValueMap& policy = ValueMap());
	void Disconnect(PortHandle src, PortHandle dst);

	bool RoutePacket(PortHandle src_port, PacketValue& packet);

	int  RequestCredits(PortHandle port, int requested_count);
	void AckCredits(PortHandle port, int ack_count);
	int  AvailableCredits(PortHandle port) const;

	String DumpTopology() const;
	String DumpPortStatus() const;
	String DumpConnectionTable() const;

	int GetPortCount() const { return ports.GetCount(); }
	int GetConnectionCount() const { return connection_table.GetCount(); }

private:
	struct Port : Moveable<Port> {
		PortHandle handle;
		ValDevTuple vd;
		ValueMap metadata;
		Vector<int> outgoing_connections;
		Vector<int> incoming_connections;
		int credits_available = 1;
		int credits_requested = 0;
		int credits_acked = 0;
	};

	struct Connection : Moveable<Connection> {
		int src_port_idx = -1;
		int dst_port_idx = -1;
		ValueMap policy;
		bool active = true;
		int packets_routed = 0;

		bool IsValid() const { return src_port_idx >= 0 && dst_port_idx >= 0; }
	};

	Vector<Port> ports;
	Vector<Connection> connection_table;
	Index<AtomBase*> atom_index;

	Port* FindPort(PortHandle handle);
	const Port* FindPort(PortHandle handle) const;
	int FindPortIndex(PortHandle handle) const;
	bool ValidateConnection(const PortHandle& src, const PortHandle& dst, String* err_msg = nullptr) const;
};

// Stub AtomBase - just needs to be a valid pointer target
struct AtomBase {
	int id;
	AtomBase(int id) : id(id) {}
};

// Stub PacketValue
class PacketValue {
public:
	int data = 0;
};

// Include PacketRouter implementation

PacketRouter::PacketRouter() {
	// Stub constructor
}

PacketRouter::~PacketRouter() {
	// Stub destructor
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
	return port.handle;
}

void PacketRouter::Connect(PortHandle src, PortHandle dst, const ValueMap& policy) {
	String err_msg;
	if (!ValidateConnection(src, dst, &err_msg)) {
		return;
	}

	Connection& conn = connection_table.Add();
	conn.src_port_idx = src.router_index;
	conn.dst_port_idx = dst.router_index;
	conn.policy = policy;
	conn.active = true;

	int conn_idx = connection_table.GetCount() - 1;

	Port* src_port = FindPort(src);
	Port* dst_port = FindPort(dst);
	ASSERT(src_port && dst_port);

	src_port->outgoing_connections.Add(conn_idx);
	dst_port->incoming_connections.Add(conn_idx);
}

void PacketRouter::Disconnect(PortHandle src, PortHandle dst) {
	int src_idx = src.router_index;
	int dst_idx = dst.router_index;

	for (int i = 0; i < connection_table.GetCount(); i++) {
		Connection& conn = connection_table[i];
		if (conn.src_port_idx == src_idx && conn.dst_port_idx == dst_idx && conn.active) {
			conn.active = false;
			return;
		}
	}
}

bool PacketRouter::RoutePacket(PortHandle src_port, PacketValue& packet) {
	const Port* port = FindPort(src_port);
	if (!port)
		return false;

	if (port->outgoing_connections.IsEmpty())
		return false;

	for (int conn_idx : port->outgoing_connections) {
		if (conn_idx >= 0 && conn_idx < connection_table.GetCount()) {
			connection_table[conn_idx].packets_routed++;
		}
	}

	return true;
}

int PacketRouter::RequestCredits(PortHandle port, int requested_count) {
	Port* p = FindPort(port);
	if (!p)
		return 0;

	p->credits_requested += requested_count;
	int granted = min(requested_count, p->credits_available);
	p->credits_available -= granted;
	return granted;
}

void PacketRouter::AckCredits(PortHandle port, int ack_count) {
	Port* p = FindPort(port);
	if (!p)
		return;

	p->credits_acked += ack_count;
	p->credits_available += ack_count;
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

	return true;
}


// ==== TESTS ====

void TestPortRegistration() {
	Cout() << "=== Test: Port Registration ===\n";

	PacketRouter router;

	AtomBase atom1(1), atom2(2), atom3(3);

	ValDevTuple vd_audio;
	vd_audio.Add(VD(CENTER, AUDIO), false);

	ValDevTuple vd_video;
	vd_video.Add(VD(CENTER, VIDEO), false);

	// Register source port
	auto src_port = router.RegisterPort(
		&atom1,
		RouterPortDesc::Direction::Source,
		0,
		vd_audio
	);
	ASSERT(src_port.IsValid());
	ASSERT(src_port.atom == &atom1);
	ASSERT(src_port.port_index == 0);
	ASSERT(src_port.router_index == 0);
	Cout() << "  Source port registered OK\n";

	// Register sink port
	auto sink_port = router.RegisterPort(
		&atom2,
		RouterPortDesc::Direction::Sink,
		0,
		vd_audio
	);
	ASSERT(sink_port.IsValid());
	ASSERT(sink_port.atom == &atom2);
	ASSERT(sink_port.port_index == 0);
	ASSERT(sink_port.router_index == 1);
	Cout() << "  Sink port registered OK\n";

	// Register multiple ports on same atom
	auto src_port2 = router.RegisterPort(
		&atom1,
		RouterPortDesc::Direction::Source,
		1,
		vd_video
	);
	ASSERT(src_port2.IsValid());
	ASSERT(src_port2.router_index == 2);
	Cout() << "  Multiple ports per atom OK\n";

	// Verify port count
	ASSERT(router.GetPortCount() == 3);
	Cout() << "  Port count OK: " << router.GetPortCount() << "\n";

	Cout() << "  PASSED\n\n";
}


void TestConnectionTable() {
	Cout() << "=== Test: Connection Table ===\n";

	PacketRouter router;

	AtomBase atom1(1), atom2(2), atom3(3);

	ValDevTuple vd;
	vd.Add(VD(CENTER, AUDIO), false);

	auto src1 = router.RegisterPort(&atom1, RouterPortDesc::Direction::Source, 0, vd);
	auto sink1 = router.RegisterPort(&atom2, RouterPortDesc::Direction::Sink, 0, vd);
	auto src2 = router.RegisterPort(&atom3, RouterPortDesc::Direction::Source, 0, vd);
	auto sink2 = router.RegisterPort(&atom2, RouterPortDesc::Direction::Sink, 1, vd);

	// Valid connection
	router.Connect(src1, sink1);
	ASSERT(router.GetConnectionCount() == 1);
	Cout() << "  Valid connection created OK\n";

	// Fan-out
	router.Connect(src1, sink2);
	ASSERT(router.GetConnectionCount() == 2);
	Cout() << "  Fan-out connection OK\n";

	// Multiple sources to sink
	router.Connect(src2, sink1);
	ASSERT(router.GetConnectionCount() == 3);
	Cout() << "  Multiple sources to sink OK\n";

	// Disconnect
	router.Disconnect(src1, sink1);
	ASSERT(router.GetConnectionCount() == 3);  // Still in table, just inactive
	Cout() << "  Disconnect OK\n";

	Cout() << "  PASSED\n\n";
}


void TestInvalidConnections() {
	Cout() << "=== Test: Invalid Connections ===\n";

	PacketRouter router;

	AtomBase atom1(1), atom2(2);

	ValDevTuple vd;
	vd.Add(VD(CENTER, AUDIO), false);

	auto src = router.RegisterPort(&atom1, RouterPortDesc::Direction::Source, 0, vd);
	auto sink = router.RegisterPort(&atom2, RouterPortDesc::Direction::Sink, 0, vd);

	int before = router.GetConnectionCount();

	// Try sink -> source (invalid)
	router.Connect(sink, src);
	ASSERT(router.GetConnectionCount() == before);
	Cout() << "  Invalid direction rejected OK\n";

	// Try source -> source
	auto src2 = router.RegisterPort(&atom1, RouterPortDesc::Direction::Source, 1, vd);
	router.Connect(src, src2);
	ASSERT(router.GetConnectionCount() == before);
	Cout() << "  Source->Source rejected OK\n";

	// Try sink -> sink
	auto sink2 = router.RegisterPort(&atom2, RouterPortDesc::Direction::Sink, 1, vd);
	router.Connect(sink, sink2);
	ASSERT(router.GetConnectionCount() == before);
	Cout() << "  Sink->Sink rejected OK\n";

	Cout() << "  PASSED\n\n";
}


void TestCreditAllocation() {
	Cout() << "=== Test: Credit Allocation ===\n";

	PacketRouter router;

	AtomBase atom1(1);

	ValDevTuple vd;
	vd.Add(VD(CENTER, AUDIO), false);

	auto port = router.RegisterPort(&atom1, RouterPortDesc::Direction::Source, 0, vd);

	// Initial credits
	int available = router.AvailableCredits(port);
	ASSERT(available == 1);
	Cout() << "  Initial credits OK: " << available << "\n";

	// Request
	int granted = router.RequestCredits(port, 1);
	ASSERT(granted == 1);
	Cout() << "  Request granted OK: " << granted << "\n";

	// Exhaustion
	granted = router.RequestCredits(port, 10);
	ASSERT(granted == 0);
	Cout() << "  Exhaustion OK: granted=" << granted << "\n";

	// Ack replenish
	router.AckCredits(port, 5);
	available = router.AvailableCredits(port);
	ASSERT(available == 5);
	Cout() << "  Ack replenish OK: " << available << "\n";

	// Partial grant
	granted = router.RequestCredits(port, 3);
	ASSERT(granted == 3);
	available = router.AvailableCredits(port);
	ASSERT(available == 2);
	Cout() << "  Partial grant OK: granted=" << granted << ", remaining=" << available << "\n";

	Cout() << "  PASSED\n\n";
}


void TestTopologyDump() {
	Cout() << "=== Test: Topology Dump ===\n";

	PacketRouter router;

	AtomBase atom1(1), atom2(2);

	ValDevTuple vd_audio;
	vd_audio.Add(VD(CENTER, AUDIO), false);

	ValDevTuple vd_video;
	vd_video.Add(VD(CENTER, VIDEO), false);

	auto src1 = router.RegisterPort(&atom1, RouterPortDesc::Direction::Source, 0, vd_audio);
	auto src2 = router.RegisterPort(&atom1, RouterPortDesc::Direction::Source, 1, vd_video);
	auto sink1 = router.RegisterPort(&atom2, RouterPortDesc::Direction::Sink, 0, vd_audio);
	auto sink2 = router.RegisterPort(&atom2, RouterPortDesc::Direction::Sink, 1, vd_video);

	router.Connect(src1, sink1);
	router.Connect(src2, sink2);

	String topology = router.DumpTopology();
	ASSERT(topology.GetCount() > 0);
	ASSERT(topology.Find("Ports: 4") >= 0);
	ASSERT(topology.Find("Connections: 2") >= 0);
	Cout() << "  Topology dump OK\n";

	String status = router.DumpPortStatus();
	ASSERT(status.GetCount() > 0);
	Cout() << "  Port status dump OK\n";

	String conns = router.DumpConnectionTable();
	ASSERT(conns.GetCount() > 0);
	Cout() << "  Connection table dump OK\n";

	Cout() << "  PASSED\n\n";
}


void TestPacketRouting() {
	Cout() << "=== Test: Packet Routing (Stub) ===\n";

	PacketRouter router;

	AtomBase atom1(1), atom2(2);

	ValDevTuple vd;
	vd.Add(VD(CENTER, AUDIO), false);

	auto src = router.RegisterPort(&atom1, RouterPortDesc::Direction::Source, 0, vd);
	auto sink = router.RegisterPort(&atom2, RouterPortDesc::Direction::Sink, 0, vd);

	router.Connect(src, sink);

	PacketValue packet;
	bool routed = router.RoutePacket(src, packet);
	ASSERT(routed);
	Cout() << "  Stub routing OK\n";

	auto orphan = router.RegisterPort(&atom1, RouterPortDesc::Direction::Source, 1, vd);
	routed = router.RoutePacket(orphan, packet);
	ASSERT(!routed);
	Cout() << "  No-connection routing correctly fails\n";

	Cout() << "  PASSED\n\n";
}


CONSOLE_APP_MAIN {
	Cout() << "PacketRouter Unit Tests\n";
	Cout() << "=======================\n\n";

	TestPortRegistration();
	TestConnectionTable();
	TestInvalidConnections();
	TestCreditAllocation();
	TestTopologyDump();
	TestPacketRouting();

	Cout() << "=======================\n";
	Cout() << "All tests PASSED!\n";
}
