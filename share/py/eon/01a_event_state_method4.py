import router

# Main event net
net = router.RouterNetContext("event")

customer = net.AddAtom("customer0", "center.customer")
customer_src = net.AddPort("customer0", router.Direction_Source, "out").index

event_src = net.AddAtom("event_src0", "event.src.test.pipe")
event_src_sink = net.AddPort("event_src0", router.Direction_Sink, "in").index
event_src_src = net.AddPort("event_src0", router.Direction_Source, "out").index

state_pipe = net.AddAtom("state_pipe0", "state.event.pipe", {"target": "event/register", "dbg_limit": 100})
state_pipe_sink = net.AddPort("state_pipe0", router.Direction_Sink, "in").index

net.Connect("customer0", customer_src, "event_src0", event_src_sink)
net.Connect("event_src0", event_src_src, "state_pipe0", state_pipe_sink)

# State register net
state_net = router.RouterNetContext("event.register")
state_register = state_net.AddAtom("state_reg0", "state.event.register")
state_net.AddPort("state_reg0", router.Direction_Sink, "in")
state_net.AddPort("state_reg0", router.Direction_Source, "out")

net.BuildLegacyLoop()
state_net.BuildLegacyLoop()