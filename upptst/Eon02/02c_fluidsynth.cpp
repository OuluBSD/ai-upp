#include "Eon02.h"
#include <EonRouterSupport/EonRouterSupport.h>

/*
machine midi.app:

	chain program:

		loop event:
			center.customer
			midi.file.reader[][loop == "input"]:
				filepath = "midi/saturday_show.mid"
				close_machine = true

		loop input:
			center.customer
			fluidsynth.pipe[loop == "event"]:
				verbose = false
			center.audio.sink.hw

*/

NAMESPACE_UPP

void Run02cFluidsynth(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 3: {
		RouterNetContext event("midi.app.program.event");
		auto& event_customer = event.AddAtom("customer_event0", "center.customer");
		int event_customer_src = event.AddPort(event_customer.id, RouterPortDesc::Direction::Source, "main").index;

		auto& reader = event.AddAtom("reader0", "midi.file.reader");
		reader.args.GetAdd("filepath") = ShareDirFile("midi/saturday_show.mid");
		reader.args.GetAdd("close_machine") = true;
		reader.args.GetAdd("loop") = String("input");
		int reader_sink = event.AddPort(reader.id, RouterPortDesc::Direction::Sink, "events.in").index;
		int reader_src = event.AddPort(reader.id, RouterPortDesc::Direction::Source, "events.out").index;

		event.Connect(event_customer.id, event_customer_src, reader.id, reader_sink);

		RouterNetContext input("midi.app.program.input");
		auto& input_customer = input.AddAtom("customer_input0", "center.customer");
		int input_customer_src = input.AddPort(input_customer.id, RouterPortDesc::Direction::Source, "main").index;

		auto& pipe = input.AddAtom("pipe0", "fluidsynth.pipe");
		pipe.args.GetAdd("loop") = String("event");
		pipe.args.GetAdd("verbose") = false;
		pipe.args.GetAdd("queue") = 10;
		pipe.args.GetAdd("debug_sound_output") = String();
		pipe.args.GetAdd("debug_sound_seed") = 0;
		int pipe_sink = input.AddPort(pipe.id, RouterPortDesc::Direction::Sink, "events.in").index;
		int pipe_src = input.AddPort(pipe.id, RouterPortDesc::Direction::Source, "audio.out").index;

		auto& sink = input.AddAtom("sink0", "center.audio.sink.hw");
		sink.args.GetAdd("debug_sound_output") = String();
		sink.args.GetAdd("debug_sound_seed") = 0;
		int sink_sink = input.AddPort(sink.id, RouterPortDesc::Direction::Sink, "audio.in").index;

		input.Connect(input_customer.id, input_customer_src, pipe.id, pipe_sink);
		input.Connect(pipe.id, pipe_src, sink.id, sink_sink);

		Vector<RouterNetContext*> nets;
		nets.Add(&event);
		nets.Add(&input);
		if (!BuildRouterChain(eng, nets, "RouterNetContext: midi.app.program router nets link event/input loops via fluidsynth pipe."))
			Exit(1);
		break;
	}
	case 9:
		sys->PostLoadFile(ShareDirFile("eon/tests/02c_fluidsynth_debug.eon"));
		break;
	case 1:
	case 2:
		LOG(Format("warning: Run02cFluidsynth: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/02c_fluidsynth.eon"));
		break;
	default:
		throw Exc(Format("Run02cFluidsynth: unknown method %d", method));
	}
}

void Run02cFluidsynthDebug(Engine& eng, int method) {
	if (method <= 0)
		Run02cFluidsynth(eng, 9);
	else
		Run02cFluidsynth(eng, method);
}

END_UPP_NAMESPACE
