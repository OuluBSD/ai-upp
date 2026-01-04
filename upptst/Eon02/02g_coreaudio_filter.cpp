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
			softinstru.pipe[loop == "event"]:
				filepath = "TimGM6mb.sf2"
				verbose = false
			corefx.pipe:
				filter = "echo"
			center.audio.sink.hw

*/

NAMESPACE_UPP

void Run02gCoreaudioFilter(Engine& eng, int method) {
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

		auto& soft = input.AddAtom("soft0", "softinstru.pipe");
		soft.args.GetAdd("loop") = String("event");
		soft.args.GetAdd("filepath") = String("TimGM6mb.sf2");
		soft.args.GetAdd("verbose") = false;
		int soft_sink = input.AddPort(soft.id, RouterPortDesc::Direction::Sink, "events.in").index;
		int soft_src = input.AddPort(soft.id, RouterPortDesc::Direction::Source, "audio.out").index;

		auto& fx = input.AddAtom("fx0", "corefx.pipe");
		fx.args.GetAdd("filter") = String("echo");
		int fx_sink = input.AddPort(fx.id, RouterPortDesc::Direction::Sink, "audio.in").index;
		int fx_src = input.AddPort(fx.id, RouterPortDesc::Direction::Source, "audio.out").index;

		auto& sink = input.AddAtom("sink0", "center.audio.sink.hw");
		int sink_sink = input.AddPort(sink.id, RouterPortDesc::Direction::Sink, "audio.in").index;

		input.Connect(input_customer.id, input_customer_src, soft.id, soft_sink);
		input.Connect(soft.id, soft_src, fx.id, fx_sink);
		input.Connect(fx.id, fx_src, sink.id, sink_sink);

		Vector<RouterNetContext*> nets;
		nets.Add(&event);
		nets.Add(&input);
		if (!BuildRouterChain(eng, nets, "RouterNetContext: midi.app.program softinstru+corefx nets link event/input loops."))
			Exit(1);
		break;
	}
	case 1:
	case 2:
		LOG(Format("warning: Run02gCoreaudioFilter: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/02g_coreaudio_filter.eon"));
		break;
	default:
		throw Exc(Format("Run02gCoreaudioFilter: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
