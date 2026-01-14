import router

def build_output(name, idx):
    net = router.RouterNetContext(name)
    customer = net.AddAtom("customer_output" + idx, "center.customer")
    customer_src = net.AddPort("customer_output" + idx, router.Direction_Source, "primary").index
    
    src = net.AddAtom("src" + idx, "center.audio.src.test")
    src_sink = net.AddPort("src" + idx, router.Direction_Sink, "in").index
    src_src = net.AddPort("src" + idx, router.Direction_Source, "out").index
    
    side_src = net.AddAtom("side_src_pipe" + idx, "center.audio.side.src.center.user")
    side_src_sink = net.AddPort("side_src_pipe" + idx, router.Direction_Sink, "loop-in").index
    
    net.Connect("customer_output" + idx, customer_src, "src" + idx, src_sink)
    net.Connect("src" + idx, src_src, "side_src_pipe" + idx, side_src_sink)
    net.BuildLegacyLoop()

build_output("tester.output0", "0")
build_output("tester.output1", "1")

pipe = router.RouterNetContext("tester.pipe")
customer_pipe = pipe.AddAtom("customer_pipe0", "center.customer")
customer_pipe_src = pipe.AddPort("customer_pipe0", router.Direction_Source, "primary").index

side_sink_output0 = pipe.AddAtom("side_sink_output0", "center.audio.side.sink.center.user")
side_sink_output0_sink = pipe.AddPort("side_sink_output0", router.Direction_Sink, "loop-in").index
side_sink_output0_src = pipe.AddPort("side_sink_output0", router.Direction_Source, "loop-out").index

pipe_src = pipe.AddAtom("side_src_pipe_input", "center.audio.side.src.center.user")
pipe_src_sink = pipe.AddPort("side_src_pipe_input", router.Direction_Sink, "loop-in").index

pipe.Connect("customer_pipe0", customer_pipe_src, "side_sink_output0", side_sink_output0_sink)
pipe.Connect("side_sink_output0", side_sink_output0_src, "side_src_pipe_input", pipe_src_sink)
pipe.BuildLegacyLoop()

input_net = router.RouterNetContext("tester.input")
customer_input = input_net.AddAtom("customer_input0", "center.customer")
customer_input_src = input_net.AddPort("customer_input0", router.Direction_Source, "primary").index

sink2 = input_net.AddAtom("side_sink2_pipe_input", "center.audio.side.sink2.center.user")
sink2_sink = input_net.AddPort("side_sink2_pipe_input", router.Direction_Sink, "loop-in").index
sink2_src = input_net.AddPort("side_sink2_pipe_input", router.Direction_Source, "loop-out").index

poller_sink = input_net.AddAtom("sink_poller0", "center.audio.sink.test.poller")
poller_sink_in = input_net.AddPort("sink_poller0", router.Direction_Sink, "in").index

input_net.Connect("customer_input0", customer_input_src, "side_sink2_pipe_input", sink2_sink)
input_net.Connect("side_sink2_pipe_input", sink2_src, "sink_poller0", poller_sink_in)
input_net.BuildLegacyLoop()