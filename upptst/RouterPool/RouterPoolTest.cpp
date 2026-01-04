#include <Core/Core.h>
#include <EonRouterSupport/EonRouterSupport.h>
#include <Vfs/Ecs/Formats.h>
#include <Vfs/Storage/VfsStorage.h>

using namespace Upp;

static RouterSchema MakePoolSchema() {
	RouterNetContext net("router.tests.pool");
	auto& gen = net.AddAtom("generator0", "center.audio.src.test");
	auto& sink_a = net.AddAtom("sinkA", "center.audio.sink.test.realtime");
	auto& sink_b = net.AddAtom("sinkB", "center.audio.sink.test.realtime");
	int gen_port = net.AddPort(gen.id, RouterPortDesc::Direction::Source, "audio.main").index;
	int sink_a_port = net.AddPort(sink_a.id, RouterPortDesc::Direction::Sink, "audio.in").index;
	int sink_b_port = net.AddPort(sink_b.id, RouterPortDesc::Direction::Sink, "audio.in").index;
	net.SetFlowControlPolicy("legacy-loop", 1);
	net.FlowControlMetadata().Set("pool_size", 4);
	net.FlowControlMetadata().Set("idle_reclaim", true);
	net.FlowControlMetadata().Set("burst_limit", 2);
	RouterConnectionDesc& conn_a = net.Connect(gen.id, gen_port, sink_a.id, sink_a_port);
	conn_a.metadata.Set("pool_share", 2);
	conn_a.metadata.Set("allow_idle", true);
	RouterConnectionDesc& conn_b = net.Connect(gen.id, gen_port, sink_b.id, sink_b_port);
	conn_b.metadata.Set("pool_share", 2);
	conn_b.metadata.Set("allow_idle", false);
	Value router_value = net.GetRouterMetadata();
	RouterSchema schema;
	ASSERT(LoadRouterSchema(router_value, schema));
	return schema;
}

static void TestPoolFlowControl() {
	RouterSchema schema = MakePoolSchema();
	Value pool_size = RouterLookupValue(schema.flow_control, "pool_size");
	ASSERT(pool_size.Is<int>() && pool_size.Get<int>() == 4);
	Value idle_reclaim = RouterLookupValue(schema.flow_control, "idle_reclaim");
	ASSERT(idle_reclaim.Is<bool>() && idle_reclaim.Get<bool>());
	Value burst_limit = RouterLookupValue(schema.flow_control, "burst_limit");
	ASSERT(burst_limit.Is<int>() && burst_limit.Get<int>() == 2);
}

static void TestPoolConnectionMetadata() {
	RouterSchema schema = MakePoolSchema();
	ASSERT(schema.connections.GetCount() == 2);
	for (const RouterConnectionDesc& conn : schema.connections) {
		Value share = RouterLookupValue(conn.metadata, "pool_share");
		ASSERT(share.Is<int>() && share.Get<int>() == 2);
		Value allow_idle = RouterLookupValue(conn.metadata, "allow_idle");
		ASSERT(allow_idle.Is<bool>());
	}
}

CONSOLE_APP_MAIN {
	StdLogSetup(LOG_COUT|LOG_FILE);
	TestPoolFlowControl();
	TestPoolConnectionMetadata();
	LOG("Router packet pool tests completed");
}
