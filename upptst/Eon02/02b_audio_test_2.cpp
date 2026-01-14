#include "Eon02.h"
#include <EonRouterSupport/EonRouterSupport.h>

/*
chain player.audio.generator:

	loop output:
		center.customer
		center.audio.src.dbg_generator
		center.audio.side.src.center

	loop input:
		center.customer
		center.audio.side.sink.center
		center.audio.sink.hw

*/

NAMESPACE_UPP

void Run02bAudioTest2(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 3: {
		RouterNetContext output("player.audio.generator.output");
		auto& customer_out = output.AddAtom("customer_out0", "center.customer");
		int customer_out_src = output.AddPort(customer_out.id, RouterPortDesc::Direction::Source, "main").index;

		auto& src = output.AddAtom("src0", "center.audio.src.dbg_generator");
		int src_sink = output.AddPort(src.id, RouterPortDesc::Direction::Sink, "in").index;
		int src_src = output.AddPort(src.id, RouterPortDesc::Direction::Source, "out").index;

		auto& side_src = output.AddAtom("side_src0", "center.audio.side.src.center");
		int side_src_sink = output.AddPort(side_src.id, RouterPortDesc::Direction::Sink, "loop-in").index;
		int side_src_src = output.AddPort(side_src.id, RouterPortDesc::Direction::Source, "side-out").index;

		output.Connect(customer_out.id, customer_out_src, src.id, src_sink);
		output.Connect(src.id, src_src, side_src.id, side_src_sink);

		RouterNetContext input("player.audio.generator.input");
		auto& customer_in = input.AddAtom("customer_in0", "center.customer");
		int customer_in_src = input.AddPort(customer_in.id, RouterPortDesc::Direction::Source, "main").index;

		auto& side_sink = input.AddAtom("side_sink0", "center.audio.side.sink.center");
		int side_sink_sink = input.AddPort(side_sink.id, RouterPortDesc::Direction::Sink, "loop-in").index;
		int side_sink_src = input.AddPort(side_sink.id, RouterPortDesc::Direction::Source, "loop-out").index;

		auto& sink = input.AddAtom("sink0", "center.audio.sink.hw");
		int sink_in = input.AddPort(sink.id, RouterPortDesc::Direction::Sink, "in").index;

		input.Connect(customer_in.id, customer_in_src, side_sink.id, side_sink_sink);
		input.Connect(side_sink.id, side_sink_src, sink.id, sink_in);

		Vector<RouterNetContext*> nets;
		nets.Add(&output);
		nets.Add(&input);
		if (!BuildRouterChain(eng, nets, "RouterNetContext: player.audio.generator output/input loops share router nets via side-link parity."))
			Exit(1);
		break;
	}
	case 1:
	case 2:
		LOG(Format("warning: Run02bAudioTest2: method %d not implemented yet", method));
	case 4:
		sys->PostLoadPythonFile(ShareDirFile("py/eon/02b_audio_test_2_method4.py"));
		return;
	case 0:
	default:
		throw Exc(Format("Run02bAudioTest2: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
