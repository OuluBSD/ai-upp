#include "Eon00.h"
#include <EonRouterSupport/EonRouterSupport.h>

/*
PacketRouter DSL Test - Audio generator using net syntax

net tester.generator:
	center.customer
	center.audio.src.test
	center.audio.sink.test.realtime:
		dbg_limit = 100
	center.customer.0 -> center.audio.src.test.0
	center.audio.src.test.0 -> center.audio.sink.test.realtime.0
*/

NAMESPACE_UPP

void Run00dAudioGenNet(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 0:
		// Load from .eon file using PacketRouter net syntax
		sys->PostLoadFile(ShareDirFile("eon/tests/00d_audio_gen_net.eon"));
		break;
	case 2:
	case 3: {
		// C++ version that builds the same net topology via RouterNetContext
		RouterNetContext net("tester.generator");

		auto& customer = net.AddAtom("customer0", "center.customer");
		int customer_src = net.AddPort(customer.id, RouterPortDesc::Direction::Source, "audio").index;

		auto& generator = net.AddAtom("generator0", "center.audio.src.test");
		int generator_sink = net.AddPort(generator.id, RouterPortDesc::Direction::Sink, "audio.in").index;
		int generator_src = net.AddPort(generator.id, RouterPortDesc::Direction::Source, "audio.out").index;

		auto& sink = net.AddAtom("sink0", "center.audio.sink.test.realtime");
		sink.args.GetAdd("dbg_limit") = 100;
		int sink_in = net.AddPort(sink.id, RouterPortDesc::Direction::Sink, "audio").index;

		net.Connect("customer0", customer_src, "generator0", generator_sink);
		net.Connect("generator0", generator_src, "sink0", sink_in);

		if (!net.BuildLegacyLoop(eng))
			Exit(1);
		break;
	}
	default:
		throw Exc(Format("Run00dAudioGenNet: method %d not implemented yet", method));
	}
}

END_UPP_NAMESPACE
