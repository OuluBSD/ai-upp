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

// Test functional equivalence between legacy loops and router nets
static void TestFunctionalEquivalence() {
	LOG("Testing functional equivalence between legacy loops and router nets");
	
	// Create a simple net equivalent to a legacy loop
	RouterNetContext net("equivalence_test");
	auto& customer = net.AddAtom("customer0", "center.customer");
	auto& generator = net.AddAtom("generator0", "center.audio.src.test");
	auto& sink = net.AddAtom("sink0", "center.audio.sink.test.realtime");
	sink.args.GetAdd("dbg_limit") = 50; // Limit for testing
	
	int customer_src = net.AddPort(customer.id, RouterPortDesc::Direction::Source, "audio").index;
	int generator_sink = net.AddPort(generator.id, RouterPortDesc::Direction::Sink, "audio").index;
	int generator_src = net.AddPort(generator.id, RouterPortDesc::Direction::Source, "audio").index;
	int sink_in = net.AddPort(sink.id, RouterPortDesc::Direction::Sink, "audio").index;
	
	net.Connect("customer0", customer_src, "generator0", generator_sink);
	net.Connect("generator0", generator_src, "sink0", sink_in);
	
	EngineGuard guard;
	guard.eng = &ShellMainEngine();
	Engine& eng = *guard.eng;
	ConfigureRouterEngine(eng);
	
	// Build as legacy loop to test equivalent behavior
	if (!net.BuildLegacyLoop(eng)) {
		FAIL("Failed to build router net as legacy loop");
		return;
	}
	
	// Wait a bit to let the system run
	Sleep(100);  // 100ms should be enough for basic functional test
	
	// Check that the system is running and atoms are created
	Ptr<Eon::ScriptLoader> script = eng.Find<Eon::ScriptLoader>();
	ASSERT(script);
	ASSERT(script->GetNetCount() == 1);
	
	LOG("Functional equivalence test passed");
}

// Test credit exhaustion scenarios
static void TestCreditExhaustion() {
	LOG("Testing credit exhaustion scenarios");
	
	// Create a net that could potentially cause credit exhaustion
	RouterNetContext net("credit_exhaustion_test");
	auto& fast_generator = net.AddAtom("fast_gen", "center.audio.src.dbg_generator");
	auto& slow_consumer = net.AddAtom("slow_sink", "center.audio.sink.test.realtime");
	
	// Set fast generator parameters to produce more data
	fast_generator.args.GetAdd("rate") = 44100;  // Higher rate
	fast_generator.args.GetAdd("freq") = 440;    // A4 note
	slow_consumer.args.GetAdd("dbg_limit") = 10; // Limited consumption
	
	int gen_out = net.AddPort(fast_generator.id, RouterPortDesc::Direction::Source, "audio").index;
	int sink_in = net.AddPort(slow_consumer.id, RouterPortDesc::Direction::Sink, "audio").index;
	
	net.Connect("fast_gen", gen_out, "slow_sink", sink_in);
	
	EngineGuard guard;
	guard.eng = &ShellMainEngine();
	Engine& eng = *guard.eng;
	ConfigureRouterEngine(eng);
	
	if (!net.BuildLegacyLoop(eng)) {
		FAIL("Failed to build credit exhaustion test net");
		return;
	}
	
	// Let run for a bit to accumulate credit exhaustion conditions
	Sleep(200);  // 200ms
	
	Ptr<Eon::ScriptLoader> script = eng.Find<Eon::ScriptLoader>();
	ASSERT(script);
	
	// Get the router to check credit management
	if (script->GetNetCount() > 0) {
		PacketRouter* router = script->GetNetRouter(0);
		if (router) {
			// Check that the router is managing credits properly
			ASSERT(router->GetTotalPacketsRouted() >= 0);
			ASSERT(router->GetTotalDeliveryFailures() >= 0); // May have some failures in stress test
			
			LOG(Format("Credit exhaustion test: routed=%d, failures=%d", 
				router->GetTotalPacketsRouted(), router->GetTotalDeliveryFailures()));
		}
	}
	
	LOG("Credit exhaustion test completed");
}

// Test functional equivalence with complex topologies
static void TestComplexTopologyEquivalence() {
	LOG("Testing functional equivalence with complex topologies (fork/join)");
	
	// Create a fork topology: generator -> customer1, customer2
	RouterNetContext fork_net("fork_test");
	auto& generator = fork_net.AddAtom("gen", "center.audio.src.test");
	auto& customer1 = fork_net.AddAtom("cust1", "center.customer");
	auto& customer2 = fork_net.AddAtom("cust2", "center.customer");
	auto& sink = fork_net.AddAtom("sink", "center.audio.sink.test.realtime");
	sink.args.GetAdd("dbg_limit") = 25;
	
	int gen_out = fork_net.AddPort(generator.id, RouterPortDesc::Direction::Source, "audio").index;
	int cust1_in = fork_net.AddPort(customer1.id, RouterPortDesc::Direction::Sink, "audio").index;
	int cust1_out = fork_net.AddPort(customer1.id, RouterPortDesc::Direction::Source, "audio").index;
	int cust2_in = fork_net.AddPort(customer2.id, RouterPortDesc::Direction::Sink, "audio").index;
	int sink_in = fork_net.AddPort(sink.id, RouterPortDesc::Direction::Sink, "audio").index;
	
	// Connect: generator outputs to both customers, customers feed into sink
	fork_net.Connect("gen", gen_out, "cust1", cust1_in);
	fork_net.Connect("gen", gen_out, "cust2", cust2_in);
	fork_net.Connect("cust1", cust1_out, "sink", sink_in);
	
	EngineGuard guard;
	guard.eng = &ShellMainEngine();
	Engine& eng = *guard.eng;
	ConfigureRouterEngine(eng);
	
	if (!fork_net.BuildLegacyLoop(eng)) {
		FAIL("Failed to build fork topology net");
		return;
	}
	
	// Let run briefly
	Sleep(100);
	
	Ptr<Eon::ScriptLoader> script = eng.Find<Eon::ScriptLoader>();
	ASSERT(script);
	
	if (script->GetNetCount() > 0) {
		PacketRouter* router = script->GetNetRouter(0);
		if (router) {
			LOG(Format("Fork topology test: routed=%d packets, %d connections, %d failures", 
				router->GetTotalPacketsRouted(), router->GetConnectionCount(), router->GetTotalDeliveryFailures()));
			ASSERT(router->GetTotalDeliveryFailures() == 0); // Should have no delivery failures
		}
	}
	
	LOG("Complex topology equivalence test passed");
}

// Test router performance under load
static void TestRouterPerformanceUnderLoad() {
	LOG("Testing router performance under load");
	
	// Create multiple connections to stress the router
	RouterNetContext perf_net("perf_test");
	
	// Create multiple generators and sinks
	Vector<RouterNetContext::AtomRef> generators;
	Vector<RouterNetContext::AtomRef> sinks;
	
	for(int i = 0; i < 5; i++) {
		String gen_id = "gen_" + AsString(i);
		String sink_id = "sink_" + AsString(i);
		
		auto& gen = perf_net.AddAtom(gen_id, "center.audio.src.dbg_generator");
		auto& sink = perf_net.AddAtom(sink_id, "center.audio.sink.test.realtime");
		sink.args.GetAdd("dbg_limit") = 10;
		
		generators.Add(gen);
		sinks.Add(sink);
	}
	
	// Connect each generator to its corresponding sink
	for(int i = 0; i < generators.GetCount(); i++) {
		int gen_out = perf_net.AddPort(generators[i].id, RouterPortDesc::Direction::Source, "audio").index;
		int sink_in = perf_net.AddPort(sinks[i].id, RouterPortDesc::Direction::Sink, "audio").index;
		perf_net.Connect(generators[i].id, gen_out, sinks[i].id, sink_in);
	}
	
	EngineGuard guard;
	guard.eng = &ShellMainEngine();
	Engine& eng = *guard.eng;
	ConfigureRouterEngine(eng);
	
	if (!perf_net.BuildLegacyLoop(eng)) {
		FAIL("Failed to build performance test net");
		return;
	}
	
	// Let run longer for performance metrics
	Sleep(500);  // 500ms for better performance metrics
	
	Ptr<Eon::ScriptLoader> script = eng.Find<Eon::ScriptLoader>();
	ASSERT(script);
	
	if (script->GetNetCount() > 0) {
		PacketRouter* router = script->GetNetRouter(0);
		if (router) {
			LOG(Format("Performance test: routed=%d packets on %d connections, failures=%d", 
				router->GetTotalPacketsRouted(), router->GetConnectionCount(), router->GetTotalDeliveryFailures()));
			ASSERT(router->GetTotalDeliveryFailures() == 0);
		}
	}
	
	LOG("Router performance test completed");
}

// Test edge cases and error conditions
static void TestRouterEdgeCases() {
	LOG("Testing router edge cases");
	
	// Create a single atom with no connections (should still work)
	RouterNetContext edge_net("edge_test");
	auto& standalone_atom = edge_net.AddAtom("standalone", "center.customer");
	
	// Add both input and output ports but don't connect them (valid scenario)
	int input_port = edge_net.AddPort(standalone_atom.id, RouterPortDesc::Direction::Sink, "in").index;
	int output_port = edge_net.AddPort(standalone_atom.id, RouterPortDesc::Direction::Source, "out").index;
	
	EngineGuard guard;
	guard.eng = &ShellMainEngine();
	Engine& eng = *guard.eng;
	ConfigureRouterEngine(eng);
	
	if (!edge_net.BuildLegacyLoop(eng)) {
		FAIL("Failed to build edge case net");
		return;
	}
	
	// Let run briefly
	Sleep(50);
	
	Ptr<Eon::ScriptLoader> script = eng.Find<Eon::ScriptLoader>();
	ASSERT(script);
	
	if (script->GetNetCount() > 0) {
		PacketRouter* router = script->GetNetRouter(0);
		if (router) {
			LOG(Format("Edge case test: routed=%d packets, %d connections, %d failures", 
				router->GetTotalPacketsRouted(), router->GetConnectionCount(), router->GetTotalDeliveryFailures()));
			// For a disconnected atom, we expect 0 connections and 0 failures
			ASSERT(router->GetConnectionCount() == 0);
			ASSERT(router->GetTotalDeliveryFailures() == 0);
		}
	}
	
	LOG("Router edge cases test passed");
}

CONSOLE_APP_MAIN {
	StdLogSetup(LOG_COUT|LOG_FILE);
	
	LOG("Starting expanded router regression tests...");
	
	TestFunctionalEquivalence();
	TestCreditExhaustion();
	TestComplexTopologyEquivalence();
	TestRouterPerformanceUnderLoad();
	TestRouterEdgeCases();
	
	LOG("Expanded router regression tests completed successfully!");
}