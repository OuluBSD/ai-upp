import router

net = router.RouterNetContext("tester.generator")

customer = net.AddAtom("customer0", "center.customer")
customer_src = net.AddPort("customer0", router.Direction_Source, "audio").index

generator = net.AddAtom("generator0", "center.audio.src.test")
generator_sink = net.AddPort("generator0", router.Direction_Sink, "audio.in").index
generator_src = net.AddPort("generator0", router.Direction_Source, "audio.out").index

sink = net.AddAtom("sink0", "center.audio.sink.test.realtime")
# sink.args.GetAdd("dbg_limit") = 100 # TODO: support arg assignment in binding
sink_in = net.AddPort("sink0", router.Direction_Sink, "audio").index

net.Connect("customer0", customer_src, "generator0", generator_sink)
net.Connect("generator0", generator_src, "sink0", sink_in)

if not net.BuildLegacyLoop():
    print("Failed to build legacy loop")
