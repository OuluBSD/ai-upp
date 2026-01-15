import router

print("Loading 03e_x11_video_sw3d_method4.py")

# Machine x11.app with driver context and net video
# This corresponds to share/eon/tests/03e_x11_video_sw3d.eon

# Router-based version of the loop video chain
# Loop must be under x11.app.program to find X11 context
net = router.RouterNetContext("x11.app.program.video")

# state event.register
# TODO: Add support for state declarations in Python bindings

# center.customer
customer = net.AddAtom("customer0", "center.customer")
customer_sink = net.AddPort("customer0", router.Direction_Sink, "video.in").index
customer_src = net.AddPort("customer0", router.Direction_Source, "video.out").index

# x11.sw.fbo.pipe with shaders
fbo_pipe = net.AddAtom("fbo_pipe0", "x11.sw.fbo.pipe", {
    "shader.vtx.name": "pass",
    "shader.frag.name": "color_test"
})
fbo_pipe_sink = net.AddPort("fbo_pipe0", router.Direction_Sink, "video.in").index
fbo_pipe_src = net.AddPort("fbo_pipe0", router.Direction_Source, "video.out").index

# x11.sw.fbo.sink with arguments
fbo_sink = net.AddAtom("fbo_sink0", "x11.sw.fbo.sink", {
    "close_machine": True,
    "sizeable": True,
    "env": "/event/register",
    "recv.data": False,
    "shader.vtx.name": "pass",
    "shader.frag.name": "proxy_input0",
    "avg_color_log": True,
    "avg_color_interval": 16
})
fbo_sink_sink = net.AddPort("fbo_sink0", router.Direction_Sink, "video.in").index
fbo_sink_src = net.AddPort("fbo_sink0", router.Direction_Source, "video.out").index

# Connections: center.customer -> x11.sw.fbo.pipe
net.Connect("customer0", customer_src, "fbo_pipe0", fbo_pipe_sink)

# Connections: x11.sw.fbo.pipe -> x11.sw.fbo.sink
net.Connect("fbo_pipe0", fbo_pipe_src, "fbo_sink0", fbo_sink_sink)

# Connections: x11.sw.fbo.sink -> center.customer
net.Connect("fbo_sink0", fbo_sink_src, "customer0", customer_sink)

# Build the video loop - X11 context is initialized at this point
print("Building video network...")
if not net.BuildLegacyLoop():
    print("Failed to build legacy loop")
else:
    print("Successfully built video network")
