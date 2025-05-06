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
	#if 0
	Nod& worker = root.Add(METAKIND_RT_WORKER, "main");
	Nod& world  = root.Add(METAKIND_RT_WORLDSTATE, "world");
	Nod& loop   = root.Add(METAKIND_RT_LOOP, "loop0");
	Nod& atom0  = loop.Add(METAKIND_RT_CENTER_CUSTOMER, "customer");
	Nod& atom1  = loop.Add(METAKIND_RT_CENTER_AUDIO_SRC_TEST, "source");
	Nod& atom2  = loop.Add(METAKIND_RT_CENTER_AUDIO_SINK_TEST, "sink");
	#endif
	
	// Initialize atoms with the world state
	//world...
	
	// Link atoms
	//loop...
	
	// Post initialize atoms
	//loop...
	
	// Run worker
	// worker... run main
	
}
