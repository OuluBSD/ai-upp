import router

net = router.RouterNetContext("event")

customer = net.AddAtom("customer0", "center.customer")
customer_src = net.AddPort("customer0", router.Direction_Source, "audio").index

midi_reader = net.AddAtom("midi_reader0", "midi.file.reader.pipe", {"filepath": "midi/saturday_show.mid", "close_machine": True})
midi_reader_sink = net.AddPort("midi_reader0", router.Direction_Sink, "in").index
midi_reader_src = net.AddPort("midi_reader0", router.Direction_Source, "out").index

midi_sink = net.AddAtom("midi_sink0", "midi.null.sink", {"verbose": False})
midi_sink_sink = net.AddPort("midi_sink0", router.Direction_Sink, "in").index

net.Connect("customer0", customer_src, "midi_reader0", midi_reader_sink)
net.Connect("midi_reader0", midi_reader_src, "midi_sink0", midi_sink_sink)

net.BuildLegacyLoop()