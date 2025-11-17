#include "Eon00.h"
#include <EonRouterSupport/EonRouterSupport.h>

NAMESPACE_UPP

void Run00cAudioGen(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);
	
	switch(method) {
	case 3: {
		RouterNetContext output0("tester.output0");
		auto& customer_out0 = output0.AddAtom("customer_output0", "center.customer");
		int customer_out0_src = output0.AddPort(customer_out0.id, RouterPortDesc::Direction::Source, "primary").index;
		auto& src0 = output0.AddAtom("src0", "center.audio.src.test");
		int src0_sink = output0.AddPort(src0.id, RouterPortDesc::Direction::Sink, "in").index;
		int src0_src = output0.AddPort(src0.id, RouterPortDesc::Direction::Source, "out").index;
		auto& side_src_pipe = output0.AddAtom("side_src_pipe0", "center.audio.side.src.center.user");
		int side_src_pipe_sink = output0.AddPort(side_src_pipe.id, RouterPortDesc::Direction::Sink, "loop-in").index;
		int side_src_pipe_src = output0.AddPort(side_src_pipe.id, RouterPortDesc::Direction::Source, "side-out").index;
		output0.Connect(customer_out0.id, customer_out0_src, src0.id, src0_sink);
		output0.Connect(src0.id, src0_src, side_src_pipe.id, side_src_pipe_sink);

		RouterNetContext output1("tester.output1");
		auto& customer_out1 = output1.AddAtom("customer_output1", "center.customer");
		int customer_out1_src = output1.AddPort(customer_out1.id, RouterPortDesc::Direction::Source, "primary").index;
		auto& src1 = output1.AddAtom("src1", "center.audio.src.test");
		int src1_sink = output1.AddPort(src1.id, RouterPortDesc::Direction::Sink, "in").index;
		int src1_src = output1.AddPort(src1.id, RouterPortDesc::Direction::Source, "out").index;
		auto& side_src_input = output1.AddAtom("side_src_input0", "center.audio.side.src.center.user");
		int side_src_input_sink = output1.AddPort(side_src_input.id, RouterPortDesc::Direction::Sink, "loop-in").index;
		int side_src_input_src = output1.AddPort(side_src_input.id, RouterPortDesc::Direction::Source, "side-out").index;
		output1.Connect(customer_out1.id, customer_out1_src, src1.id, src1_sink);
		output1.Connect(src1.id, src1_src, side_src_input.id, side_src_input_sink);

		RouterNetContext pipe("tester.pipe");
		auto& customer_pipe = pipe.AddAtom("customer_pipe0", "center.customer");
		int customer_pipe_src = pipe.AddPort(customer_pipe.id, RouterPortDesc::Direction::Source, "primary").index;
		auto& sink_from_output0 = pipe.AddAtom("side_sink_output0", "center.audio.side.sink.center.user");
		int sink_output0_sink = pipe.AddPort(sink_from_output0.id, RouterPortDesc::Direction::Sink, "loop-in").index;
		int sink_output0_src = pipe.AddPort(sink_from_output0.id, RouterPortDesc::Direction::Source, "loop-out").index;
		auto& pipe_src = pipe.AddAtom("side_src_pipe_input", "center.audio.side.src.center.user");
		int pipe_src_sink = pipe.AddPort(pipe_src.id, RouterPortDesc::Direction::Sink, "loop-in").index;
		int pipe_src_src = pipe.AddPort(pipe_src.id, RouterPortDesc::Direction::Source, "side-out").index;
		pipe.Connect(customer_pipe.id, customer_pipe_src, sink_from_output0.id, sink_output0_sink);
		pipe.Connect(sink_from_output0.id, sink_output0_src, pipe_src.id, pipe_src_sink);

		RouterNetContext input("tester.input");
		auto& customer_input = input.AddAtom("customer_input0", "center.customer");
		int customer_input_src = input.AddPort(customer_input.id, RouterPortDesc::Direction::Source, "primary").index;
		auto& sink2 = input.AddAtom("side_sink2_pipe_input", "center.audio.side.sink2.center.user");
		int sink2_sink = input.AddPort(sink2.id, RouterPortDesc::Direction::Sink, "loop-in").index;
		int sink2_src = input.AddPort(sink2.id, RouterPortDesc::Direction::Source, "loop-out").index;
		auto& poller_sink = input.AddAtom("sink_poller0", "center.audio.sink.test.poller");
		int poller_sink_in = input.AddPort(poller_sink.id, RouterPortDesc::Direction::Sink, "in").index;
		input.Connect(customer_input.id, customer_input_src, sink2.id, sink2_sink);
		input.Connect(sink2.id, sink2_src, poller_sink.id, poller_sink_in);

		Vector<RouterNetContext*> nets;
		nets.Add(&output0);
		nets.Add(&output1);
		nets.Add(&pipe);
		nets.Add(&input);
		if (!BuildRouterChain(eng, nets, "RouterNetContext: tester output/input/pipe nets linked through router-style side bridges."))
			Exit(1);
		break;
	}
	case 1:
	case 2:
		LOG(Format("warning: Run00cAudioGen: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/00c_audio_gen.eon"));
		break;
	default:
		throw Exc(Format("Run00cAudioGen: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
