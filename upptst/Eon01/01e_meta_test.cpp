#include "Eon01.h"

/*
machine daw.core:
	meta int channel.count = 3

	meta loopstmt channel.effects():
		corefx.pipe:
			filter="chorus"
		corefx.pipe:
			filter="jcrev"

	meta params channel.links():
		meta for int i = 0, i < channel.count, i++:
			loop channel.$i

	chain program:
		meta for int i = 0, i < channel.count, i++:
			loop channel.$i:
				center.customer
				softinstru.pipe [,]:
					filepath = "TimGM6mb.sf2"
					verbose = false
				$channel.effects()
				center.audio.side.src.center [][,]

		loop event:
			center.customer
			midi.file.reader [][$channel.links()]:
				filepath = "midi/short_piano.mid"
				close_machine = true
				separate.channels = true

		loop master.out:
			center.customer
			audio.mixer[$channel.links()]
			center.audio.sink.hw


*/

NAMESPACE_UPP

void Run01eMetaTest(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run01eMetaTest: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("01e_meta_test.eon"));
		break;
	default:
		throw Exc(Format("Run01eMetaTest: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
