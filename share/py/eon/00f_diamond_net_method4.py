import router

net = router.RouterNetContext("tester.diamond")

src = net.AddAtom("src0", "center.audio.src.test")
customer1 = net.AddAtom("customer1", "center.customer")
customer2 = net.AddAtom("customer2", "center.customer")
sink = net.AddAtom("sink0", "center.audio.sink.test.realtime")

src_src = net.AddPort("src0", router.Direction_Source, "audio.out").index
c1_sink = net.AddPort("customer1", router.Direction_Sink, "audio.in").index
c1_src = net.AddPort("customer1", router.Direction_Source, "audio.out").index
c2_sink = net.AddPort("customer2", router.Direction_Sink, "audio.in").index
c2_src = net.AddPort("customer2", router.Direction_Source, "audio.out").index
sink_sink = net.AddPort("sink0", router.Direction_Sink, "audio").index

# Path 1: src -> customer1 -> sink
net.Connect("src0", src_src, "customer1", c1_sink)
net.Connect("customer1", c1_src, "sink0", sink_sink)

# Path 2: src -> customer2 -> sink
net.Connect("src0", src_src, "customer2", c2_sink)
net.Connect("customer2", c2_src, "sink0", sink_sink)

net.BuildLegacyLoop()
