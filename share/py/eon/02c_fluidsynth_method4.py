import router

# Note: Using RouterNetContext to mirror the side-link braid logic
# tester.output -> tester.input bridge

bridge_conn = 1

# Event Net
event = router.RouterNetContext("midi.event")
customer_event = event.AddAtom("customer_event0", "center.customer")
customer_event_src = event.AddPort("customer_event0", router.Direction_Source, "primary").index

midi_reader = event.AddAtom("midi_reader0", "midi.file.reader.pipe", {"filepath": "midi/saturday_show.mid", "close_machine": True})
midi_reader_sink = event.AddPort("midi_reader0", router.Direction_Sink, "in").index
midi_reader_src = event.AddPort("midi_reader0", router.Direction_Source, "out").index

side_src = event.AddAtom("side_src0", "center.audio.side.src.center") # midi events use audio-vfs for transport in this test? 
# Wait, original .eon used fluidsynth.pipe[loop == "event"], which implies a side-link connector.
# My router binding support side links via bridge_conn.

event.SetSideSourceLink("side_src0", 1, bridge_conn, 1)
event.Connect("customer_event0", customer_event_src, "midi_reader0", midi_reader_sink)
event.Connect("midi_reader0", midi_reader_src, "side_src0", 0) # simplified

# Input Net (Fluidsynth)
input_net = router.RouterNetContext("midi.input")
customer_input = input_net.AddAtom("customer_input0", "center.customer")
customer_input_src = input_net.AddPort("customer_input0", router.Direction_Source, "primary").index

side_sink = input_net.AddAtom("side_sink0", "center.audio.side.sink.center")
input_net.SetSideSinkLink("side_sink0", 1, bridge_conn, 1)

synth = input_net.AddAtom("synth0", "fluidsynth.pipe", {"verbose": False, "queue": 10})
synth_sink = input_net.AddPort("synth0", router.Direction_Sink, "in").index
synth_src = input_net.AddPort("synth0", router.Direction_Source, "out").index

sink = input_net.AddAtom("sink0", "center.audio.sink.hw")
sink_in = input_net.AddPort("sink0", router.Direction_Sink, "in").index

input_net.Connect("customer_input0", customer_input_src, "side_sink0", 0)
input_net.Connect("side_sink0", 0, "synth0", synth_sink)
input_net.Connect("synth0", synth_src, "sink0", sink_in)

event.BuildLegacyLoop()
input_net.BuildLegacyLoop()
