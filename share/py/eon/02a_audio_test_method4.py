import router

net = router.RouterNetContext("player.audio.generator")

customer = net.AddAtom("customer0", "center.customer")
customer_src = net.AddPort("customer0", router.Direction_Source, "audio.out").index

generator = net.AddAtom("generator0", "center.audio.src.dbg_generator")
generator_sink = net.AddPort("generator0", router.Direction_Sink, "audio.in").index
generator_src = net.AddPort("generator0", router.Direction_Source, "audio.out").index

sink = net.AddAtom("sink0", "center.audio.sink.hw")
sink_in = net.AddPort("sink0", router.Direction_Sink, "audio.in").index

net.Connect("customer0", customer_src, "generator0", generator_sink)
net.Connect("generator0", generator_src, "sink0", sink_in)

net.BuildLegacyLoop()
