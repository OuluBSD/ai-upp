#include <Core/Core.h>
#include <EonRouterSupport/EonRouterSupport.h>
#include <Vfs/Ecs/Formats.h>
#include <Vfs/Storage/VfsStorage.h>

using namespace Upp;

static RouterSchema MakeFanoutSchema() {
	RouterNetContext net("router.tests.fanout");
	auto& splitter = net.AddAtom("splitter0", "center.audio.side.src.center.user");
	auto& sink_left = net.AddAtom("sink_left", "center.audio.side.sink.center.user");
	auto& sink_right = net.AddAtom("sink_right", "center.audio.side.sink.center.user");
	int split_left = net.AddPort(splitter.id, RouterPortDesc::Direction::Source, "fanout.left").index;
	int split_right = net.AddPort(splitter.id, RouterPortDesc::Direction::Source, "fanout.right").index;
	int sink_left_in = net.AddPort(sink_left.id, RouterPortDesc::Direction::Sink, "audio.in").index;
	int sink_right_in = net.AddPort(sink_right.id, RouterPortDesc::Direction::Sink, "audio.in").index;
	RouterConnectionDesc& left_conn = net.Connect(splitter.id, split_left, sink_left.id, sink_left_in);
	left_conn.metadata.Set("fanout_group", "L");
	left_conn.metadata.Set("burst_packets", 2);
	RouterConnectionDesc& right_conn = net.Connect(splitter.id, split_right, sink_right.id, sink_right_in);
	right_conn.metadata.Set("fanout_group", "R");
	right_conn.metadata.Set("burst_packets", 2);
	net.FlowControlMetadata().Set("burst_group", "fanout");
	net.FlowControlMetadata().Set("max_packets", 2);
	Value router_value = net.GetRouterMetadata();
	RouterSchema schema;
	ASSERT(LoadRouterSchema(router_value, schema));
	return schema;
}

static void TestFanoutPorts() {
	RouterSchema schema = MakeFanoutSchema();
	int splitter_ports = 0;
	for (const RouterPortEntry& entry : schema.ports) {
		if (entry.atom_id == "splitter0" && entry.desc.direction == RouterPortDesc::Direction::Source)
			splitter_ports++;
	}
	ASSERT(splitter_ports >= 2);
}

static void TestFanoutConnections() {
	RouterSchema schema = MakeFanoutSchema();
	ASSERT(schema.connections.GetCount() == 2);
	for (const RouterConnectionDesc& conn : schema.connections) {
		ASSERT(conn.from_atom == "splitter0");
		Value burst = RouterLookupValue(conn.metadata, "burst_packets");
		ASSERT(burst.Is<int>() && burst.Get<int>() == 2);
	}
}

static void TestFanoutFlowControl() {
	RouterSchema schema = MakeFanoutSchema();
	ValueMap flow = schema.flow_control;
	Value group = RouterLookupValue(flow, "burst_group");
	ASSERT(group.Is<String>() && group.Get<String>() == "fanout");
	Value max_packets = RouterLookupValue(flow, "max_packets");
	ASSERT(max_packets.Is<int>() && max_packets.Get<int>() == 2);
}

CONSOLE_APP_MAIN {
	StdLogSetup(LOG_COUT|LOG_FILE);
	TestFanoutPorts();
	TestFanoutConnections();
	TestFanoutFlowControl();
	LOG("Router fan-out tests completed");
}
