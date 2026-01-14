#include "Eon02.h"
#include <EonRouterSupport/EonRouterSupport.h>

/*
machine midi.app:
	chain program:
		loop event:
			center.customer
			midi.file.reader[][loop == "input"]:
				filepath = "midi/short_piano.mid"
				close_machine = true

		loop input:
			center.customer
			coresynth.pipe[loop == "event"]:
				instrument = "rhodey"
				verbose = false
			center.audio.sink.hw

*/

NAMESPACE_UPP

void Run02fCoreaudioInstru(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 3: {
		RouterNetContext event("midi.app.program.event");
		auto& event_customer = event.AddAtom("customer_event0", "center.customer");
		int event_customer_src = event.AddPort(event_customer.id, RouterPortDesc::Direction::Source, "main").index;

		auto& reader = event.AddAtom("reader0", "midi.file.reader");
		reader.args.GetAdd("filepath") = ShareDirFile("midi/short_piano.mid");
		reader.args.GetAdd("close_machine") = true;
		reader.args.GetAdd("loop") = String("input");
		int reader_sink = event.AddPort(reader.id, RouterPortDesc::Direction::Sink, "events.in").index;
		int reader_src = event.AddPort(reader.id, RouterPortDesc::Direction::Source, "events.out").index;

		event.Connect(event_customer.id, event_customer_src, reader.id, reader_sink);

		RouterNetContext input("midi.app.program.input");
		auto& input_customer = input.AddAtom("customer_input0", "center.customer");
		int input_customer_src = input.AddPort(input_customer.id, RouterPortDesc::Direction::Source, "main").index;

		auto& pipe = input.AddAtom("pipe0", "coresynth.pipe");
		pipe.args.GetAdd("loop") = String("event");
		pipe.args.GetAdd("instrument") = String("rhodey");
		pipe.args.GetAdd("verbose") = false;
		int pipe_sink = input.AddPort(pipe.id, RouterPortDesc::Direction::Sink, "events.in").index;
		int pipe_src = input.AddPort(pipe.id, RouterPortDesc::Direction::Source, "audio.out").index;

		auto& sink = input.AddAtom("sink0", "center.audio.sink.hw");
		int sink_sink = input.AddPort(sink.id, RouterPortDesc::Direction::Sink, "audio.in").index;

		input.Connect(input_customer.id, input_customer_src, pipe.id, pipe_sink);
		input.Connect(pipe.id, pipe_src, sink.id, sink_sink);

		Vector<RouterNetContext*> nets;
		nets.Add(&event);
		nets.Add(&input);
		if (!BuildRouterChain(eng, nets, "RouterNetContext: midi.app.program coresynth nets link event/input loops."))
			Exit(1);
		break;
	}
	case 1:
	case 2:
		LOG(Format("warning: Run02fCoreaudioInstru: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/02f_coreaudio_instru.eon"));
		break;
	case 4:
		sys->PostLoadPythonFile(ShareDirFile("py/eon/02f_coreaudio_instru_method4.py"));
		break;
	default:
		throw Exc(Format("Run02fCoreaudioInstru: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
