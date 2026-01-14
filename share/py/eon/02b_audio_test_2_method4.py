import router

# Output loop
output = router.RouterNetContext("player.audio.generator.output")
customer_out = output.AddAtom("customer_out0", "center.customer")
customer_out_src = output.AddPort("customer_out0", router.Direction_Source, "main").index

src = output.AddAtom("src0", "center.audio.src.dbg_generator")
src_sink = output.AddPort("src0", router.Direction_Sink, "in").index
src_src = output.AddPort("src0", router.Direction_Source, "out").index

side_src = output.AddAtom("side_src0", "center.audio.side.src.center")
side_src_sink = output.AddPort("side_src0", router.Direction_Sink, "loop-in").index

output.Connect("customer_out0", customer_out_src, "src0", src_sink)
output.Connect("src0", src_src, "side_src0", side_src_sink)

# Input loop
input_net = router.RouterNetContext("player.audio.generator.input")
customer_in = input_net.AddAtom("customer_in0", "center.customer")
customer_in_src = input_net.AddPort("customer_in0", router.Direction_Source, "main").index

side_sink = input_net.AddAtom("side_sink0", "center.audio.side.sink.center")
side_sink_sink = input_net.AddPort("side_sink0", router.Direction_Sink, "loop-in").index
side_sink_src = input_net.AddPort("side_sink0", router.Direction_Source, "loop-out").index

sink = input_net.AddAtom("sink0", "center.audio.sink.hw")
sink_in = input_net.AddPort("sink0", router.Direction_Sink, "in").index

input_net.Connect("customer_in0", customer_in_src, "side_sink0", side_sink_sink)
input_net.Connect("side_sink0", side_sink_src, "sink0", sink_in)

router.BuildRouterChain([output, input_net], "02b linked", dbg_limit=0)