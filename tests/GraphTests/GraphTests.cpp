#include "GraphTests.h"

/*
This test follows the original first eon test: 00a_audio_gen.eon
	
loop tester.generator:
	center.customer
	center.audio.src.test
	center.audio.sink.test.realtime:
		dbg_limit = 100

It's a nice minimal working example.
I started re-writing the realtime engine to ai-upp starting
from this test 24.4.2025

*/

CONSOLE_APP_MAIN {
	Nod root;
	Nod& worker = root.AddPath(METAKIND_RT_WORKER, "main");
	Nod& world  = root.AddPath(METAKIND_RT_WORLDSTATE, "world");
	Nod& loop   = root.AddPath(METAKIND_RT_LOOP, "tester/generator");
	Nod& atom0  = loop.AddPath(METAKIND_RT_CENTER_CUSTOMER, "center/customer");
	Nod& atom1  = loop.AddPath(METAKIND_RT_CENTER_AUDIO_SRC_TEST, "center/audio/src/test");
	Nod& atom2  = loop.AddPath(METAKIND_RT_CENTER_AUDIO_SINK_TEST, "center/audio/sink/test/realtime");
	
	// Initialize atoms with the world state
	//world...
	
	// Link atoms
	//loop...
	
	// Post initialize atoms
	//loop...
	
	// Run worker
	// worker... run main
	
}
