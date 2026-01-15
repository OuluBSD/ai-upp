import router

print("Loading 03e_x11_video_sw3d_method4.py")

# Machine x11.app with driver context and net video
# This corresponds to share/eon/tests/03e_x11_video_sw3d.eon

# Router-based version of the loop video chain
net = router.RouterNetContext("video")

# state event.register
# TODO: Add support for state declarations in Python bindings

# center.customer
customer = net.AddAtom("customer0", "center.customer")
customer_sink = net.AddPort("customer0", router.Direction_Sink, "video.in").index
customer_src = net.AddPort("customer0", router.Direction_Source, "video.out").index

# x11.sw.fbo.pipe with shaders
# Note: Argument assignment not fully supported in Python bindings yet
fbo_pipe = net.AddAtom("fbo_pipe0", "x11.sw.fbo.pipe")
# TODO: fbo_pipe.args - shader.vtx.name, shader.frag.name
fbo_pipe_sink = net.AddPort("fbo_pipe0", router.Direction_Sink, "video.in").index
fbo_pipe_src = net.AddPort("fbo_pipe0", router.Direction_Source, "video.out").index

# x11.sw.fbo.sink with arguments
# Note: Argument assignment not fully supported in Python bindings yet
fbo_sink = net.AddAtom("fbo_sink0", "x11.sw.fbo.sink")
# TODO: fbo_sink.args - close_machine, sizeable, env, recv.data, shader.vtx.name, shader.frag.name, avg_color_log, avg_color_interval
fbo_sink_sink = net.AddPort("fbo_sink0", router.Direction_Sink, "video.in").index
fbo_sink_src = net.AddPort("fbo_sink0", router.Direction_Source, "video.out").index

# Connections: center.customer -> x11.sw.fbo.pipe
net.Connect("customer0", customer_src, "fbo_pipe0", fbo_pipe_sink)

# Connections: x11.sw.fbo.pipe -> x11.sw.fbo.sink
net.Connect("fbo_pipe0", fbo_pipe_src, "fbo_sink0", fbo_sink_sink)

# Connections: x11.sw.fbo.sink -> center.customer
net.Connect("fbo_sink0", fbo_sink_src, "customer0", customer_sink)

# Build the network
if not net.BuildLegacyLoop():
    print("Failed to build legacy loop")
print("Successfully built video network")
