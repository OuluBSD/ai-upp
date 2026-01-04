#include "Eon00.h"
#include <EonRouterSupport/EonRouterSupport.h>

NAMESPACE_UPP

void Run00jAudioGenBridge(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 3: {
		int bridge_conn = 1;

		RouterNetContext output("tester.output");
		auto& customer_out = output.AddAtom("customer_out0", "center.customer");
		int customer_out_src = output.AddPort(customer_out.id, RouterPortDesc::Direction::Source, "main").index;
		auto& src = output.AddAtom("src0", "center.audio.src.test");
		int src_sink = output.AddPort(src.id, RouterPortDesc::Direction::Sink, "in").index;
		int src_src = output.AddPort(src.id, RouterPortDesc::Direction::Source, "out").index;
		auto& side_src = output.AddAtom("side_src0", "center.audio.side.src.center");
		int side_src_sink = output.AddPort(side_src.id, RouterPortDesc::Direction::Sink, "loop-in").index;
		int side_src_src = output.AddPort(side_src.id, RouterPortDesc::Direction::Source, "side-out").index;
		output.SetSideSourceLink(side_src.id, 1, bridge_conn, 1);
		output.Connect(customer_out.id, customer_out_src, src.id, src_sink);
		output.Connect(src.id, src_src, side_src.id, side_src_sink);

		RouterNetContext input("tester.input");
		auto& customer_in = input.AddAtom("customer_in0", "center.customer");
		int customer_in_src = input.AddPort(customer_in.id, RouterPortDesc::Direction::Source, "main").index;
		auto& side_sink = input.AddAtom("side_sink0", "center.audio.side.sink.center");
		int side_sink_sink = input.AddPort(side_sink.id, RouterPortDesc::Direction::Sink, "loop-in").index;
		int side_sink_src = input.AddPort(side_sink.id, RouterPortDesc::Direction::Source, "loop-out").index;
		input.SetSideSinkLink(side_sink.id, 1, bridge_conn, 1);
		auto& sink = input.AddAtom("sink0", "center.audio.sink.test.realtime");
		sink.args.GetAdd("dbg_limit") = 100;
		int sink_sink = input.AddPort(sink.id, RouterPortDesc::Direction::Sink, "in").index;
		input.Connect(customer_in.id, customer_in_src, side_sink.id, side_sink_sink);
		input.Connect(side_sink.id, side_sink_src, sink.id, sink_sink);

		Vector<RouterNetContext*> nets;
		nets.Add(&output);
		nets.Add(&input);
		if (!BuildRouterChain(eng, nets,
				"RouterNetContext: tester.output/tester.input nets linked through side bridge."))
			Exit(1);
		break;
	}
	case 1:
	case 2:
		LOG(Format("warning: Run00jAudioGenBridge: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/00j_audio_gen_bridge.eon"));
		break;
	default:
		throw Exc(Format("Run00jAudioGenBridge: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
