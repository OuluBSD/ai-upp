import router

net = router.RouterNetContext("tester.fork")

src = net.AddAtom("src0", "center.audio.src.test")
customer = net.AddAtom("customer0", "center.customer")
sink = net.AddAtom("sink0", "center.audio.sink.test.realtime")

src_src = net.AddPort("src0", router.Direction_Source, "audio.out").index
customer_sink = net.AddPort("customer0", router.Direction_Sink, "audio.in").index
customer_src = net.AddPort("customer0", router.Direction_Source, "audio.out").index
sink_sink = net.AddPort("sink0", router.Direction_Sink, "audio").index

# Fan-out: src -> customer AND src -> sink
net.Connect("src0", src_src, "customer0", customer_sink)
net.Connect("src0", src_src, "sink0", sink_sink)

# Also customer -> sink
net.Connect("customer0", customer_src, "sink0", sink_sink)

net.BuildLegacyLoop()
