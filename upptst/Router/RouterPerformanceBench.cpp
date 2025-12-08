#include <Core/Core.h>
#include <Eon/Core/Core.h>
#include <Eon/Script/Script.h>
#include <Eon/Core/Context.h>
#include <EonRouterSupport/EonRouterSupport.h>
#include <Shell/Shell.h>
#include <Vfs/Ecs/Engine.h>
#include <Vfs/Ecs/Engine2.h>
#include <Vfs/Core/VfsValue.h>

using namespace Upp;

struct EngineGuard {
	Engine* eng = nullptr;
	~EngineGuard() {
		if (eng)
			Engine::Uninstall(true, eng);
	}
};

static void ConfigureEngine(Engine& eng) {
	eng.ClearCallbacks();
	eng.WhenInitialize << callback(MachineEcsInit);
	eng.WhenPreFirstUpdate << callback(DefaultStartup);
	eng.WhenBoot << callback(DefaultSerialInitializerInternalEon);
}

// Benchmark legacy ChainContext/LoopContext performance
static int BenchmarkLegacyLoop(int iterations) {
	EngineGuard guard;
	guard.eng = &ShellMainEngine();
	Engine& eng = *guard.eng;
	ConfigureEngine(eng);

	VfsValue* loop_space = Eon::ResolveLoopPath(eng, String("benchmark.legacy.test"));
	ASSERT(loop_space);

	Eon::ChainContext cc;
	Vector<Eon::ChainContext::AtomSpec> atoms;

	// Add customer (source)
	{
		Eon::ChainContext::AtomSpec& a = atoms.Add();
		a.action = "center.customer";
		Eon::AtomTypeCls atom; Eon::LinkTypeCls link;
		if (Eon::ChainContext::ResolveAction(a.action, atom, link))
			a.iface.Realize(atom);
	}

	// Add debug generator
	{
		Eon::ChainContext::AtomSpec& a = atoms.Add();
		a.action = "center.audio.src.dbg_generator";
		a.args.GetAdd("rate") = 44100;
		a.args.GetAdd("freq") = 440;
		Eon::AtomTypeCls atom; Eon::LinkTypeCls link;
		if (Eon::ChainContext::ResolveAction(a.action, atom, link))
			a.iface.Realize(atom);
	}

	// Add sink
	{
		Eon::ChainContext::AtomSpec& a = atoms.Add();
		a.action = "center.audio.sink.test.realtime";
		a.args.GetAdd("dbg_limit") = iterations / 10; // Limit output
		Eon::AtomTypeCls atom; Eon::LinkTypeCls link;
		if (Eon::ChainContext::ResolveAction(a.action, atom, link))
			a.iface.Realize(atom);
	}

	TimeStop ts;
	Eon::LoopContext& loop = cc.AddLoop(*loop_space, atoms, true);
	
	if (!cc.PostInitializeAll()) {
		LOG("Legacy benchmark: PostInitialize failed");
		return -1;
	}
	
	if (!cc.StartAll()) {
		LOG("Legacy benchmark: Start failed");
		cc.UndoAll();
		return -1;
	}

	// Let it run for the specified iterations
	Sleep(50); // Run for 50ms
	
	cc.UndoAll();
	
	return (int)ts.Elapsed();
}

// Benchmark new NetContext/Router performance
static int BenchmarkNetRouter(int iterations) {
	EngineGuard guard;
	guard.eng = &ShellMainEngine();
	Engine& eng = *guard.eng;
	ConfigureEngine(eng);

	VfsValue* net_space = Eon::ResolveLoopPath(eng, String("benchmark.net.test"));
	ASSERT(net_space);

	Eon::NetContext net(*net_space);

	// Add customer (source)
	Eon::IfaceConnTuple customer_iface;
	Eon::AtomTypeCls customer_type;
	Eon::LinkTypeCls customer_link;
	if (Eon::ChainContext::ResolveAction("center.customer", customer_type, customer_link)) {
		customer_iface.Realize(customer_type);
		net.AddAtom("customer0", "center.customer", customer_iface);
	}

	// Add debug generator
	Eon::IfaceConnTuple gen_iface;
	Eon::AtomTypeCls gen_type;
	Eon::LinkTypeCls gen_link;
	if (Eon::ChainContext::ResolveAction("center.audio.src.dbg_generator", gen_type, gen_link)) {
		gen_iface.Realize(gen_type);
		ArrayMap<String, Value> gen_args;
		gen_args.GetAdd("rate") = 44100;
		gen_args.GetAdd("freq") = 440;
		net.AddAtom("gen0", "center.audio.src.dbg_generator", gen_iface, &gen_args);
	}

	// Add sink
	Eon::IfaceConnTuple sink_iface;
	Eon::AtomTypeCls sink_type;
	Eon::LinkTypeCls sink_link;
	if (Eon::ChainContext::ResolveAction("center.audio.sink.test.realtime", sink_type, sink_link)) {
		sink_iface.Realize(sink_type);
		ArrayMap<String, Value> sink_args;
		sink_args.GetAdd("dbg_limit") = iterations / 10;
		net.AddAtom("sink0", "center.audio.sink.test.realtime", sink_iface, &sink_args);
	}

	// Add explicit connections
	net.AddConnection(0, 0, 1, 0); // customer -> generator
	net.AddConnection(1, 0, 2, 0); // generator -> sink

	TimeStop ts;
	
	// Register ports and make connections
	if (!net.RegisterPorts()) {
		LOG("Net router benchmark: Port registration failed");
		return -1;
	}
	
	if (!net.MakeConnections()) {
		LOG("Net router benchmark: Connection setup failed");
		return -1;
	}
	
	if (!net.PostInitializeAll()) {
		LOG("Net router benchmark: PostInitialize failed");
		return -1;
	}
	
	if (!net.StartAll()) {
		LOG("Net router benchmark: Start failed");
		net.UndoAll();
		return -1;
	}

	// Run for the specified iterations
	for (int i = 0; i < iterations; i++) {
		net.ProcessFrame(10); // Small number of iterations per frame
		if (i % (iterations/10) == 0) { // Log progress every 10%
			LOG(Format("Net router benchmark progress: %d%%", (i * 100) / iterations));
		}
	}
	
	net.UndoAll();
	
	return (int)ts.Elapsed();
}

// Compare packet routing performance
static void ComparePacketRoutingPerformance() {
	LOG("=== Packet Router Performance Comparison ===");
	
	const int iterations = 1000;
	
	LOG("Running legacy ChainContext/LoopContext benchmark...");
	int legacy_time = BenchmarkLegacyLoop(iterations);
	LOG(Format("Legacy approach took: %d ms", legacy_time));
	
	LOG("Running new NetContext/Router benchmark...");
	int router_time = BenchmarkNetRouter(iterations);
	LOG(Format("Router approach took: %d ms", router_time));
	
	if (legacy_time > 0 && router_time > 0) {
		double ratio = (double)router_time / legacy_time;
		LOG(Format("Performance ratio: %.2fx (router is %.2fx %s than legacy)", 
			ratio, 
			abs(ratio - 1.0), 
			ratio > 1.0 ? "slower" : "faster"));
		
		if (ratio < 1.05 && ratio > 0.95) {
			LOG("Performance is equivalent (within 5%)");
		} else if (ratio < 1.0) {
			LOG("Router is faster than legacy - Excellent!");
		} else {
			LOG("Router is slower than legacy - May need optimization");
		}
	} else {
		LOG("Could not complete performance comparison due to errors");
	}
}

// Benchmark router under different load conditions
static void BenchmarkRouterUnderLoad() {
	LOG("=== Router Load Testing ===");
	
	// Test with different numbers of connections
	Vector<int> connection_counts = {1, 5, 10, 20, 50};
	
	for (int conn_count : connection_counts) {
		LOG(Format("Testing with %d connections...", conn_count));
		
		EngineGuard guard;
		guard.eng = &ShellMainEngine();
		Engine& eng = *guard.eng;
		ConfigureEngine(eng);

		VfsValue* net_space = Eon::ResolveLoopPath(eng, String("benchmark.load.test"));
		ASSERT(net_space);

		Eon::NetContext net(*net_space);
		
		// Add a single source
		Eon::IfaceConnTuple src_iface;
		Eon::AtomTypeCls src_type;
		Eon::LinkTypeCls src_link;
		if (Eon::ChainContext::ResolveAction("center.customer", src_type, src_link)) {
			src_iface.Realize(src_type);
			net.AddAtom("source", "center.customer", src_iface);
		}
		
		// Add multiple sinks
		for (int i = 0; i < conn_count; i++) {
			Eon::IfaceConnTuple sink_iface;
			Eon::AtomTypeCls sink_type;
			Eon::LinkTypeCls sink_link;
			if (Eon::ChainContext::ResolveAction("center.audio.sink.test.realtime", sink_type, sink_link)) {
				sink_iface.Realize(sink_type);
				ArrayMap<String, Value> args;
				args.GetAdd("dbg_limit") = 10;
				net.AddAtom(Format("sink_%d", i), "center.audio.sink.test.realtime", sink_iface, &args);
			}
		}
		
		// Connect source to all sinks (fan-out)
		for (int i = 0; i < conn_count; i++) {
			net.AddConnection(0, 0, i+1, 0);  // Connect source to each sink
		}
		
		TimeStop ts;
		
		if (net.RegisterPorts() && net.MakeConnections() && 
			net.PostInitializeAll() && net.StartAll()) {
			
			// Run for a brief period
			net.ProcessFrame(50);
			
			int elapsed_ms = (int)ts.Elapsed();
			LOG(Format("  %d connections: %d ms", conn_count, elapsed_ms));
			
			net.UndoAll();
		} else {
			LOG(Format("  %d connections: Setup failed", conn_count));
		}
	}
}

// Benchmark router memory usage patterns
static void BenchmarkRouterMemoryUsage() {
	LOG("=== Router Memory Usage Patterns ===");
	
	// Create a moderately complex network
	EngineGuard guard;
	guard.eng = &ShellMainEngine();
	Engine& eng = *guard.eng;
	ConfigureEngine(eng);

	VfsValue* net_space = Eon::ResolveLoopPath(eng, String("benchmark.memory.test"));
	ASSERT(net_space);

	Eon::NetContext net(*net_space);
	
	// Create a chain: source -> filter -> processor -> sink
	Vector<String> atom_actions = {
		"center.customer",
		"center.audio.sink.test.realtime", 
		"center.audio.src.dbg_generator",
		"center.audio.sink.test.realtime"
	};
	
	Vector<String> atom_names = {"src", "filter", "proc", "sink"};
	
	Eon::AtomTypeCls type; 
	Eon::LinkTypeCls link;
	
	// Add atoms
	for (int i = 0; i < atom_names.GetCount(); i++) {
		Eon::IfaceConnTuple iface;
		if (Eon::ChainContext::ResolveAction(atom_actions[i], type, link)) {
			iface.Realize(type);
			if (i == 1 || i == 3) { // sinks
				ArrayMap<String, Value> args;
				args.GetAdd("dbg_limit") = 25;
				net.AddAtom(atom_names[i], atom_actions[i], iface, &args);
			} else {
				net.AddAtom(atom_names[i], atom_actions[i], iface);
			}
		}
	}
	
	// Add connections: src -> filter -> proc -> sink
	net.AddConnection(0, 0, 1, 0);  // src -> filter
	net.AddConnection(1, 0, 2, 0);  // filter -> proc  
	net.AddConnection(2, 0, 3, 0);  // proc -> sink
	
	LOG("Network created with 4 atoms and 3 connections");
	LOG(Format("Router has %d ports and %d connections before setup", 
		net.router->GetPortCount(), net.router->GetConnectionCount()));
	
	TimeStop ts_setup;
	net.RegisterPorts();
	net.MakeConnections();
	int setup_time = (int)ts_setup.Elapsed();
	
	LOG(Format("Setup completed in %d ms", setup_time));
	LOG(Format("Router now has %d ports and %d connections", 
		net.router->GetPortCount(), net.router->GetConnectionCount()));
	
	// Run for a bit and measure routing performance
	TimeStop ts_routing;
	for (int i = 0; i < 100; i++) {
		net.ProcessFrame(10);
	}
	int routing_time = (int)ts_routing.Elapsed();
	
	LOG(Format("Routing 100 frames took %d ms", routing_time));
	LOG(Format("Avg per frame: %.2f ms", (double)routing_time / 100.0));
	
	net.UndoAll();
}

CONSOLE_APP_MAIN {
	StdLogSetup(LOG_COUT);
	
	LOG("Starting Packet Router Performance Benchmarks...\n");
	
	ComparePacketRoutingPerformance();
	LOG("");
	
	BenchmarkRouterUnderLoad();
	LOG("");
	
	BenchmarkRouterMemoryUsage();
	LOG("");
	
	LOG("Performance benchmarks completed!");
}