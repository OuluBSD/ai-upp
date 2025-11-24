#include <Core/Core.h>
#include <Eon/Core/Core.h>
#include <Eon/Script/Script.h>
#include <EonRouterSupport/EonRouterSupport.h>
#include <Shell/Shell.h>
#include <Vfs/Ecs/Engine.h>
#include <Vfs/Ecs/Engine2.h>
#include <Vfs/Ecs/Formats.h>
#include <Vfs/Storage/VfsStorage.h>
#include <Vfs/Core/VfsValue.h>
#include <Vfs/Overlay/VfsOverlay.h>

using namespace Upp;

struct EngineGuard {
	Engine* eng = nullptr;
	~EngineGuard() {
		if (eng)
			Engine::Uninstall(true, eng);
	}
};

static void ConfigureRouterEngine(Engine& eng) {
	eng.ClearCallbacks();
	eng.WhenInitialize << callback(MachineEcsInit);
	eng.WhenPreFirstUpdate << callback(DefaultStartup);
	eng.WhenBoot << callback(DefaultSerialInitializerInternalEon);
}

static bool CompareVfsNodes(const VfsValue& a, const VfsValue& b) {
	if (a.id != b.id)
		return false;
	if (a.type_hash != b.type_hash)
		return false;
	if (a.pkg_hash != b.pkg_hash || a.file_hash != b.file_hash)
		return false;
	if (a.value != b.value)
		return false;
	if (a.sub.GetCount() != b.sub.GetCount())
		return false;
	for (int i = 0; i < a.sub.GetCount(); i++) {
		if (!CompareVfsNodes(a.sub[i], b.sub[i]))
			return false;
	}
	return true;
}

static bool CompareOverlayIndexes(const VfsOverlayIndex& a, const VfsOverlayIndex& b) {
	if (a.nodes.GetCount() != b.nodes.GetCount())
		return false;
	for (int i = 0; i < a.nodes.GetCount(); i++) {
		const OverlayNodeRecord& lhs = a.nodes[i];
		const OverlayNodeRecord& rhs = b.nodes[i];
		if (lhs.path != rhs.path) {
			fprintf(stderr, "Overlay path mismatch: expected='%s' loaded='%s'\n", lhs.path.Begin(), rhs.path.Begin());
			return false;
		}
		if (lhs.sources.GetCount() != rhs.sources.GetCount()) {
			fprintf(stderr, "Overlay source count mismatch at '%s'\n", lhs.path.Begin());
			return false;
		}
		for (int j = 0; j < lhs.sources.GetCount(); j++) {
			const SourceRef& ls = lhs.sources[j];
			const SourceRef& rs = rhs.sources[j];
			if (ls.pkg_hash != rs.pkg_hash || ls.file_hash != rs.file_hash)
				return false;
			if (ls.local_path != rs.local_path || ls.priority != rs.priority || ls.flags != rs.flags) {
				fprintf(stderr, "Overlay source metadata mismatch at '%s'\n", lhs.path.Begin());
				return false;
			}
		}
		if (Value(lhs.metadata) != Value(rhs.metadata)) {
			String expected = AsJSON(Value(lhs.metadata), true);
			String loaded = AsJSON(Value(rhs.metadata), true);
			fprintf(stderr, "Overlay metadata mismatch\nexpected:\n%s\nloaded:\n%s\n", expected.Begin(), loaded.Begin());
			return false;
		}
	}
	return true;
}

static void TestRouterPortMetadata() {
	RouterNetContext net("tester.generator");
	auto& sink_atom = net.AddAtom("sink0", "center.audio.sink.test.realtime");
	RouterPortDesc& sink_desc = net.AddPort(sink_atom.id, RouterPortDesc::Direction::Sink, "audio.in");
	auto& generator = net.AddAtom("generator0", "center.audio.src.test");
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

static void TestRouterFragmentRoundTrip() {
	RouterNetContext net("tester.router.fragment");
	auto& generator = net.AddAtom("generator0", "center.audio.src.test");
	auto& sink = net.AddAtom("sink0", "center.audio.sink.test.realtime");
	int gen_out = net.AddPort(generator.id, RouterPortDesc::Direction::Source, "audio.out").index;
	int sink_in = net.AddPort(sink.id, RouterPortDesc::Direction::Sink, "audio.in").index;
	net.Connect(generator.id, gen_out, sink.id, sink_in);

	VfsValue fragment;
	fragment.id = "loop_fragment";
	fragment.pkg_hash = 0x12345678;
	fragment.file_hash = 0x87654321;
	ValueMap node_value;
	node_value.Set("router", net.GetRouterMetadata());
	fragment.value = Value(node_value);

	String temp_path = GetTempFileName("router_fragment");
	ASSERT(VfsSaveFragment(temp_path, fragment));

	VfsValue loaded;
	ASSERT(VfsLoadFragment(temp_path, loaded));
	ASSERT(loaded.pkg_hash == fragment.pkg_hash);
	ASSERT(loaded.file_hash == fragment.file_hash);
	ASSERT(loaded.value.Is<ValueMap>());
	ValueMap loaded_value = loaded.value;
	Value router_value = RouterLookupValue(loaded_value, "router");
	ASSERT(router_value.Is<ValueMap>());

	RouterSchema schema;
	ASSERT(LoadRouterSchema(router_value, schema));
	ASSERT(schema.ports.GetCount() >= 2);
	ASSERT(schema.connections.GetCount() == 1);
	const RouterConnectionDesc& conn = schema.connections[0];
	ASSERT(conn.from_atom == generator.id && conn.to_atom == sink.id);

	FileDelete(temp_path);
}

static void TestRouterFragmentBinaryRoundTrip() {
	RouterNetContext net("tester.router.fragment.bin");
	auto& generator = net.AddAtom("generator0", "center.audio.src.test");
	auto& sink = net.AddAtom("sink0", "center.audio.sink.test.realtime");
	int gen_out = net.AddPort(generator.id, RouterPortDesc::Direction::Source, "audio.out").index;
	int sink_in = net.AddPort(sink.id, RouterPortDesc::Direction::Sink, "audio.in").index;
	net.Connect(generator.id, gen_out, sink.id, sink_in);

	VfsValue fragment;
	fragment.id = "loop_fragment_bin";
	fragment.pkg_hash = 0xCAFEBABE;
	fragment.file_hash = 0xABADBABE;
	ValueMap node_value;
	node_value.Set("router", net.GetRouterMetadata());
	fragment.value = Value(node_value);

	String temp_path = GetTempFileName("router_fragment_bin");
	ASSERT(VfsSaveFragmentBinary(temp_path, fragment));

	VfsValue loaded;
	ASSERT(VfsLoadFragmentBinary(temp_path, loaded));
	ASSERT(loaded.pkg_hash == fragment.pkg_hash);
	ASSERT(loaded.file_hash == fragment.file_hash);
	ASSERT(loaded.value.Is<ValueMap>());
	ValueMap loaded_value = loaded.value;
	Value router_value = RouterLookupValue(loaded_value, "router");
	ASSERT(router_value.Is<ValueMap>());

	RouterSchema schema;
	ASSERT(LoadRouterSchema(router_value, schema));
	ASSERT(schema.connections.GetCount() == 1);
	const RouterConnectionDesc& conn = schema.connections[0];
	ASSERT(conn.from_atom == generator.id && conn.to_atom == sink.id);

	FileDelete(temp_path);
}

static void TestRouterOverlayIndexRoundTrip() {
	RouterNetContext net("tester.router.overlay");
	auto& generator = net.AddAtom("generator0", "center.audio.src.test");
	auto& sink = net.AddAtom("sink0", "center.audio.sink.test.realtime");
	int gen_out = net.AddPort(generator.id, RouterPortDesc::Direction::Source, "audio.out").index;
	int sink_in = net.AddPort(sink.id, RouterPortDesc::Direction::Sink, "audio.in").index;
	net.Connect(generator.id, gen_out, sink.id, sink_in);

	VfsOverlayIndex index;
	OverlayNodeRecord& node = index.nodes.Add();
	node.path = "tester.router.overlay";
	SourceRef ref;
	ref.pkg_hash = 0x11111111;
	ref.file_hash = 0x22222222;
	ref.local_path = "router_fragment.json";
	ref.priority = 5;
	node.sources.Add(ref);
	node.metadata.Set("router", net.GetRouterMetadata());

	String temp_path = GetTempFileName("router_overlay");
	ASSERT(VfsSaveOverlayIndex(temp_path, index));

	VfsOverlayIndex loaded;
	ASSERT(VfsLoadOverlayIndex(temp_path, loaded));
	ASSERT(loaded.nodes.GetCount() == 1);
	const OverlayNodeRecord& loaded_node = loaded.nodes[0];
	ASSERT(loaded_node.path == node.path);
	ASSERT(loaded_node.sources.GetCount() == 1);
	const SourceRef& loaded_ref = loaded_node.sources[0];
	ASSERT(loaded_ref.pkg_hash == ref.pkg_hash);
	ASSERT(loaded_ref.file_hash == ref.file_hash);
	ASSERT(loaded_ref.priority == ref.priority);

	Value router_value = RouterLookupValue(loaded_node.metadata, "router");
	ASSERT(router_value.Is<ValueMap>());
	RouterSchema schema;
	ASSERT(LoadRouterSchema(router_value, schema));
	ASSERT(schema.connections.GetCount() == 1);

	FileDelete(temp_path);
}

static void TestRouterOverlayIndexBinaryRoundTrip() {
	RouterNetContext net("tester.router.overlay.bin");
	auto& generator = net.AddAtom("generator0", "center.audio.src.test");
	auto& sink = net.AddAtom("sink0", "center.audio.sink.test.realtime");
	int gen_out = net.AddPort(generator.id, RouterPortDesc::Direction::Source, "audio.out").index;
	int sink_in = net.AddPort(sink.id, RouterPortDesc::Direction::Sink, "audio.in").index;
	net.Connect(generator.id, gen_out, sink.id, sink_in);

	VfsOverlayIndex index;
	OverlayNodeRecord& node = index.nodes.Add();
	node.path = "tester.router.overlay.bin";
	SourceRef ref;
	ref.pkg_hash = 0x31415926;
	ref.file_hash = 0x27182818;
	ref.local_path = "router_fragment_bin.vfs";
	ref.priority = 3;
	node.sources.Add(ref);
	node.metadata.Set("router", net.GetRouterMetadata());

	String temp_path = GetTempFileName("router_overlay_bin");
	ASSERT(VfsSaveOverlayIndexBinary(temp_path, index));

	VfsOverlayIndex loaded;
	ASSERT(VfsLoadOverlayIndexBinary(temp_path, loaded));
	ASSERT(loaded.nodes.GetCount() == 1);
	const OverlayNodeRecord& loaded_node = loaded.nodes[0];
	ASSERT(loaded_node.path == node.path);
	ASSERT(loaded_node.sources.GetCount() == 1);
	const SourceRef& loaded_ref = loaded_node.sources[0];
	ASSERT(loaded_ref.pkg_hash == ref.pkg_hash);
	ASSERT(loaded_ref.file_hash == ref.file_hash);
	ASSERT(loaded_ref.priority == ref.priority);

	Value router_value = RouterLookupValue(loaded_node.metadata, "router");
	ASSERT(router_value.Is<ValueMap>());
	RouterSchema schema;
	ASSERT(LoadRouterSchema(router_value, schema));
	ASSERT(schema.connections.GetCount() == 1);

	FileDelete(temp_path);
}

static void TestRouterOverlayChunkRoundTrip() {
	RouterNetContext net("tester.router.overlay.chunk");
	auto& generator = net.AddAtom("generator0", "center.audio.src.test");
	auto& sink = net.AddAtom("sink0", "center.audio.sink.test.realtime");
	int gen_out = net.AddPort(generator.id, RouterPortDesc::Direction::Source, "audio.out").index;
	int sink_in = net.AddPort(sink.id, RouterPortDesc::Direction::Sink, "audio.in").index;
	net.Connect(generator.id, gen_out, sink.id, sink_in);

	VfsValue fragment;
	fragment.id = "router_chunk_node";
	fragment.pkg_hash = 0x42424242;
	fragment.file_hash = 0x31313131;
	ValueMap node_value;
	node_value.Set("router", net.GetRouterMetadata());
	fragment.value = Value(node_value);

	VfsOverlayIndex expected_index;
	BuildRouterOverlayIndex(fragment, expected_index);

	String base = GetTempFileName("router_overlay_chunk");
	String chunk_path = base + ".overlay.vfsch";

	ASSERT(VfsSaveOverlayIndexChunked(chunk_path, fragment));

	VfsOverlayIndex loaded_index;
	ASSERT(VfsLoadOverlayIndexChunked(chunk_path, loaded_index));
	ASSERT(CompareOverlayIndexes(expected_index, loaded_index));

	FileDelete(chunk_path);
	FileDelete(base);
}

static void TestRouterBuilderArtifactParity() {
	RouterNetContext net("tester.router.builder");
	auto& generator = net.AddAtom("generator0", "center.audio.src.test");
	auto& sink = net.AddAtom("sink0", "center.audio.sink.test.realtime");
	int gen_out = net.AddPort(generator.id, RouterPortDesc::Direction::Source, "audio.out").index;
	int sink_in = net.AddPort(sink.id, RouterPortDesc::Direction::Sink, "audio.in").index;
	net.Connect(generator.id, gen_out, sink.id, sink_in);

	VfsValue fragment;
	fragment.id = "loop_fragment_builder";
	fragment.pkg_hash = 0xABCD1234;
	fragment.file_hash = 0x5678EF01;
	ValueMap node_value;
	node_value.Set("router", net.GetRouterMetadata());
	fragment.value = Value(node_value);

	String base = GetTempFileName("router_builder");
	String fragment_json = base + ".fragment.json";
	String fragment_bin = base + ".fragment.vfsbin";
	String overlay_json = base + ".overlay.json";
	String overlay_bin = base + ".overlay.vfsbin";

	ASSERT(VfsSaveFragment(fragment_json, fragment));
	ASSERT(VfsSaveFragmentBinary(fragment_bin, fragment));

	VfsOverlayIndex built_index;
	BuildRouterOverlayIndex(fragment, built_index);
	ASSERT(VfsSaveOverlayIndex(overlay_json, built_index));
	ASSERT(VfsSaveOverlayIndexBinary(overlay_bin, built_index));

	VfsValue json_fragment;
	VfsValue bin_fragment;
	ASSERT(VfsLoadFragment(fragment_json, json_fragment));
	ASSERT(VfsLoadFragmentBinary(fragment_bin, bin_fragment));
	ASSERT(CompareVfsNodes(json_fragment, bin_fragment));

	VfsOverlayIndex index_json;
	VfsOverlayIndex index_bin;
	ASSERT(VfsLoadOverlayIndex(overlay_json, index_json));
	ASSERT(VfsLoadOverlayIndexBinary(overlay_bin, index_bin));
	ASSERT(CompareOverlayIndexes(index_json, index_bin));
	ASSERT(CompareOverlayIndexes(index_json, built_index));

	FileDelete(fragment_json);
	FileDelete(fragment_bin);
	FileDelete(overlay_json);
	FileDelete(overlay_bin);
	FileDelete(base);
}

static void TestRouterRuntimeFlowCounters() {
	EngineGuard guard;
	guard.eng = &ShellMainEngine();
	Engine& eng = *guard.eng;

	ConfigureRouterEngine(eng);

	ValueMap args;
	args.Add("MACHINE_TIME_LIMIT", 2);

	String script_path = ShareDirFile("eon/tests/00h_router_flow.eon");
	ASSERT(eng.StartLoad("RouterRuntimeFlow", script_path, args));
	eng.MainLoop();

	Ptr<Eon::ScriptLoader> script = eng.Find<Eon::ScriptLoader>();
	ASSERT(script && script->GetNetCount() > 0);

	PacketRouter* router = script->GetNetRouter(0);
	ASSERT(router);

	int total_routed = router->GetTotalPacketsRouted();
	int total_failures = router->GetTotalDeliveryFailures();
	LOG(Format("Router runtime flow: routed=%d, failures=%d", total_routed, total_failures));

	ASSERT(total_routed > 0);
	ASSERT(total_failures == 0);

	int connection_count = router->GetConnectionCount();
	ASSERT(connection_count >= 5);

	Eon::NetContext* net_ctx = script->GetNetContext(0);
	ASSERT(net_ctx);
	ASSERT(net_ctx->connections.GetCount() >= 5);

	Vector<String> expected_atoms = {
		"center.audio.src.dbg_generator",
		"center.video.src.dbg_generator",
		"sdl.audio",
		"corefx.pipe",
		"sdl.ogl.center.fbo.audio",
		"sdl.video.pipe",
		"center.customer",
		"sdl.event.pipe",
		"state.event.pipe",
		"midi.src.side.portmidi",
		"center.audio.sink.test.realtime",
		"midi.null.sink"
	};

	for (const String& expected : expected_atoms) {
		bool found = false;
		for (const Eon::NetContext::AtomInstance& inst : net_ctx->atoms) {
			if (inst.name == expected) {
				found = true;
				break;
			}
		}
		ASSERT(found);
	}

	Vector<String> essential_sources = {
		"center.audio.src.dbg_generator",
		"center.video.src.dbg_generator",
		"sdl.audio",
		"corefx.pipe",
		"sdl.ogl.center.fbo.audio",
		"sdl.video.pipe",
		"sdl.event.pipe",
		"midi.src.side.portmidi"
	};
	Index<String> essential_index;
	for (int i = 0; i < essential_sources.GetCount(); i++)
		essential_index.Add(essential_sources[i]);
	Vector<bool> essential_routed(essential_sources.GetCount());

	Vector<PacketRouter::ConnectionInfo> router_connections;
	router_connections.SetCount(connection_count);

	int per_connection_total = 0;
	bool routed_connection_found = false;
	for (int i = 0; i < connection_count; i++) {
		PacketRouter::ConnectionInfo info;
		ASSERT(router->GetConnectionInfo(i, info));
		router_connections[i] = info;

		ASSERT(info.delivery_failures == 0);
		per_connection_total += info.packets_routed;
		if (info.packets_routed > 0)
			routed_connection_found = true;
	}

	for (const Eon::NetContext::Connection& conn : net_ctx->connections) {
		bool matched = false;
		AtomBase* src_atom = net_ctx->atoms[conn.from_atom_idx].atom;
		AtomBase* dst_atom = net_ctx->atoms[conn.to_atom_idx].atom;
		ASSERT(src_atom && dst_atom);
		String src_name = net_ctx->atoms[conn.from_atom_idx].name;

		for (const PacketRouter::ConnectionInfo& info : router_connections) {
			if (info.src_handle.atom == src_atom &&
				info.src_handle.port_index == conn.from_port &&
				info.dst_handle.atom == dst_atom &&
				info.dst_handle.port_index == conn.to_port)
			{
				matched = true;
				int essential_idx = essential_index.Find(src_name);
				if (essential_idx >= 0 && info.packets_routed > 0)
					essential_routed[essential_idx] = true;
				break;
			}
		}
		ASSERT(matched);
	}

	for (int i = 0; i < essential_routed.GetCount(); i++) {
		ASSERT(essential_routed[i]);
	}

	LOG(Format("Router runtime flow: conn_count=%d, sum_routed=%d", connection_count, per_connection_total));
	ASSERT(per_connection_total == total_routed);
	ASSERT(routed_connection_found);
	ASSERT(script->GetTotalPacketsRouted() == total_routed);
}

CONSOLE_APP_MAIN {
	StdLogSetup(LOG_COUT|LOG_FILE);
	TestRouterPortMetadata();
	TestConnectionMetadata();
	TestDescriptorRoundTrip();
	TestRouterSchemaRoundTrip();
	TestRouterFragmentRoundTrip();
	TestRouterFragmentBinaryRoundTrip();
	TestRouterOverlayIndexRoundTrip();
	TestRouterOverlayIndexBinaryRoundTrip();
	TestRouterOverlayChunkRoundTrip();
	TestRouterBuilderArtifactParity();
	TestRouterRuntimeFlowCounters();
	LOG("Router descriptor tests completed");
}
