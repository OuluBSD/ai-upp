import router

# Output Net
output = router.RouterNetContext("tester.output")
customer_out = output.AddAtom("customer_out0", "center.customer")
customer_out_src = output.AddPort("customer_out0", router.Direction_Source, "main").index

src = output.AddAtom("src0", "center.audio.src.test")
src_sink = output.AddPort("src0", router.Direction_Sink, "in").index
src_src = output.AddPort("src0", router.Direction_Source, "out").index

side_src = output.AddAtom("side_src0", "center.audio.side.src.center")
side_src_sink = output.AddPort("side_src0", router.Direction_Sink, "loop-in").index
side_src_src = output.AddPort("side_src0", router.Direction_Source, "side-out").index

output.Connect("customer_out0", customer_out_src, "src0", src_sink)
output.Connect("src0", src_src, "side_src0", side_src_sink)

# Input Net
input = router.RouterNetContext("tester.input")
customer_in = input.AddAtom("customer_in0", "center.customer")
customer_in_src = input.AddPort("customer_in0", router.Direction_Source, "main").index

side_sink = input.AddAtom("side_sink0", "center.audio.side.sink.center")
side_sink_sink = input.AddPort("side_sink0", router.Direction_Sink, "loop-in").index
side_sink_src = input.AddPort("side_sink0", router.Direction_Source, "loop-out").index

sink = input.AddAtom("sink0", "center.audio.sink.test.realtime")
sink_sink = input.AddPort("sink0", router.Direction_Sink, "in").index

input.Connect("customer_in0", customer_in_src, "side_sink0", side_sink_sink)
input.Connect("side_sink0", side_sink_src, "sink0", sink_sink)

# Build both
output.BuildLegacyLoop()
input.BuildLegacyLoop()
