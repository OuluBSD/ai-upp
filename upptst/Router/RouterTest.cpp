#include <Core/Core.h>
#include <EonRouterSupport/EonRouterSupport.h>
#include <Vfs/Ecs/Formats.h>
#include <Vfs/Storage/VfsStorage.h>

using namespace Upp;

static void TestRouterPortMetadata() {
	RouterNetContext net("tester.generator");
	auto& generator = net.AddAtom("generator0", "center.audio.src.test");
	RouterPortDesc& sink_desc = net.AddPort(generator.id, RouterPortDesc::Direction::Sink, "audio.in");
	RouterPortDesc& src_desc = net.AddPort(generator.id, RouterPortDesc::Direction::Source, "audio.out");
	ASSERT(sink_desc.vd.GetCount() == 1);
	ASSERT(src_desc.vd.GetCount() == 1);
	ASSERT(sink_desc.vd[0].vd.val == ValCls::AUDIO);
	ASSERT(src_desc.vd[0].vd.val == ValCls::AUDIO);

	Value vd_name = RouterLookupValue(sink_desc.metadata, "vd_name");
	Value optional = RouterLookupValue(sink_desc.metadata, "optional");
	ASSERT(vd_name.Is<String>());
	ASSERT(optional.Is<bool>());
}

static void TestConnectionMetadata() {
	RouterNetContext net("tester.generator");
	auto& customer = net.AddAtom("customer0", "center.customer");
	auto& sink = net.AddAtom("sink0", "center.audio.sink.test.realtime");
	int customer_src = net.AddPort(customer.id, RouterPortDesc::Direction::Source, "audio").index;
	int sink_in = net.AddPort(sink.id, RouterPortDesc::Direction::Sink, "audio").index;
	net.Connect(customer.id, customer_src, sink.id, sink_in);

	const Vector<RouterConnectionDesc>& conns = net.GetConnections();
	ASSERT(conns.GetCount() == 1);
	const RouterConnectionDesc& conn = conns[0];
	Value policy = RouterLookupValue(conn.metadata, "policy");
	Value credits = RouterLookupValue(conn.metadata, "credits");
	ASSERT(policy.Is<String>() && policy.Get<String>() == "legacy-loop");
	ASSERT(credits.Is<int>() && credits.Get<int>() == 1);
}

static void TestDescriptorRoundTrip() {
	RouterNetContext net("tester.generator");
	auto& sink = net.AddAtom("sink0", "center.audio.sink.test.realtime");
	RouterPortDesc& sink_in = net.AddPort(sink.id, RouterPortDesc::Direction::Sink, "audio");

	ValueMap stored_port = StoreRouterPortDesc(sink_in);
	RouterPortDesc loaded_port;
	ASSERT(LoadRouterPortDesc(stored_port, loaded_port));
	ASSERT(loaded_port.direction == sink_in.direction);
	ASSERT(loaded_port.index == sink_in.index);

	RouterConnectionDesc conn;
	conn.from_atom = "a";
	conn.to_atom = "b";
	conn.from_port = 1;
	conn.to_port = 2;
	conn.metadata.Set("policy", "legacy-loop");
	ValueMap stored_conn = StoreRouterConnectionDesc(conn);
	RouterConnectionDesc loaded_conn;
	ASSERT(LoadRouterConnectionDesc(stored_conn, loaded_conn));
	ASSERT(loaded_conn.from_atom == conn.from_atom);
	ASSERT(loaded_conn.to_atom == conn.to_atom);
	ASSERT(loaded_conn.metadata.GetCount() == conn.metadata.GetCount());
}

static void TestRouterSchemaRoundTrip() {
	RouterNetContext net("tester.router.schema");
	auto& generator = net.AddAtom("generator0", "center.audio.src.test");
	auto& sink = net.AddAtom("sink0", "center.audio.sink.test.realtime");
	int gen_out = net.AddPort(generator.id, RouterPortDesc::Direction::Source, "audio.out").index;
	int sink_in = net.AddPort(sink.id, RouterPortDesc::Direction::Sink, "audio.in").index;
	net.Connect(generator.id, gen_out, sink.id, sink_in);

	ValueMap router_meta = net.GetRouterMetadata();
	ASSERT(router_meta.GetCount());

	RouterSchema schema;
	Value router_value = router_meta;
	ASSERT(LoadRouterSchema(router_value, schema));
	bool found_source = false;
	bool found_sink = false;
	for (const RouterPortEntry& entry : schema.ports) {
		if (entry.atom_id == generator.id && entry.desc.direction == RouterPortDesc::Direction::Source)
			found_source = true;
		if (entry.atom_id == sink.id && entry.desc.direction == RouterPortDesc::Direction::Sink)
			found_sink = true;
	}
	ASSERT(found_source && found_sink);
	ASSERT(schema.connections.GetCount() == 1);
	const RouterConnectionDesc& conn = schema.connections[0];
	ASSERT(conn.from_atom == generator.id && conn.to_atom == sink.id);
	ValueMap flow = schema.flow_control;
	Value policy = RouterLookupValue(flow, "policy");
	ASSERT(policy.Is<String>() && policy.Get<String>().StartsWith("legacy"));
}

CONSOLE_APP_MAIN {
	StdLogSetup(LOG_COUT|LOG_FILE);
	TestRouterPortMetadata();
	TestConnectionMetadata();
	TestDescriptorRoundTrip();
	TestRouterSchemaRoundTrip();
	LOG("Router descriptor tests completed");
}
