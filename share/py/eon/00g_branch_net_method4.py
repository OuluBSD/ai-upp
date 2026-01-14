import router

net = router.RouterNetContext("tester.branch")

src = net.AddAtom("src0", "center.audio.src.test")
sink1 = net.AddAtom("sink1", "center.audio.sink.test.realtime")
sink2 = net.AddAtom("sink2", "center.audio.sink.test.realtime")

src_src = net.AddPort("src0", router.Direction_Source, "audio.out").index
s1_sink = net.AddPort("sink1", router.Direction_Sink, "audio").index
s2_sink = net.AddPort("sink2", router.Direction_Sink, "audio").index

net.Connect("src0", src_src, "sink1", s1_sink)
net.Connect("src0", src_src, "sink2", s2_sink)

net.BuildLegacyLoop()
