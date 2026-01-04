#ifndef _Eon_Core_PacketRouter_h_
#define _Eon_Core_PacketRouter_h_

#include <Core/Core.h>
#include <Vfs/Ecs/Ecs.h>


NAMESPACE_UPP


// Forward declarations
class AtomBase;
class PacketValue;
using Packet = SharedRecycler<PacketValue>;


class PacketRouter {
public:
	struct PortHandle {
		AtomBase* atom = nullptr;
		int port_index = -1;
		RouterPortDesc::Direction direction;
		int router_index = -1;  // Internal routing table index

		bool IsValid() const { return atom && port_index >= 0 && router_index >= 0; }
	};


	PacketRouter();
	~PacketRouter();


	// Port registration (called during Atom::Initialize)
	PortHandle RegisterPort(AtomBase* atom,
	                       RouterPortDesc::Direction dir,
	                       int port_index,
	                       const ValDevTuple& vd,
	                       const ValueMap& metadata = ValueMap());

	// Connection management
	void Connect(PortHandle src, PortHandle dst, const ValueMap& policy = ValueMap());
	void Disconnect(PortHandle src, PortHandle dst);

	// Packet routing (called by Atom::EmitPacket)
	bool RoutePacket(PortHandle src_port, const Packet& packet);

	// Flow control (credit-based)
	int  RequestCredits(PortHandle port, int requested_count);
	void AckCredits(PortHandle port, int ack_count);
	int  AvailableCredits(PortHandle port) const;

	// Diagnostics
	String DumpTopology() const;
	String DumpPortStatus() const;
	String DumpConnectionTable() const;

	// Query
	int GetPortCount() const { return ports.GetCount(); }
	int GetConnectionCount() const { return connection_table.GetCount(); }
	int GetTotalPacketsRouted() const;
	int GetPacketsRouted(int connection_idx) const;
	int GetTotalDeliveryFailures() const;
	int GetDeliveryFailures(int connection_idx) const;
	
	struct ConnectionInfo : Moveable<ConnectionInfo> {
		PortHandle	src_handle;
		PortHandle	dst_handle;
		int			packets_routed = 0;
		int			delivery_failures = 0;
		bool		active = false;
		ValueMap	policy;
		ValueMap	src_metadata;
		ValueMap	dst_metadata;
	};
	bool GetConnectionInfo(int connection_idx, ConnectionInfo& info) const;

	// Connection management helpers
	void DisconnectAtom(AtomBase* atom);  // Marks all connections involving atom as inactive


private:
	struct Port : Moveable<Port> {
		PortHandle handle;
		ValDevTuple vd;
		ValueMap metadata;
		Vector<int> outgoing_connections;  // Indices into connection_table (for src ports)
		Vector<int> incoming_connections;  // Indices into connection_table (for sink ports)
		int credits_available = 1;  // Default: legacy-loop policy
		int credits_requested = 0;
		int credits_acked = 0;
	};

	struct Connection : Moveable<Connection> {
		int src_port_idx = -1;
		int dst_port_idx = -1;
		ValueMap policy;
		bool active = true;
		int packets_routed = 0;    // Diagnostics counter
		int delivery_failures = 0; // Delivery failure counter

		bool IsValid() const { return src_port_idx >= 0 && dst_port_idx >= 0; }
	};


	Vector<Port> ports;
	Vector<Connection> connection_table;
	Index<AtomBase*> atom_index;


	// Helpers
	Port* FindPort(PortHandle handle);
	const Port* FindPort(PortHandle handle) const;
	int FindPortIndex(PortHandle handle) const;

	bool ValidateConnection(const PortHandle& src, const PortHandle& dst, String* err_msg = nullptr) const;
};


END_UPP_NAMESPACE


#endif
