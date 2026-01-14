import eon

eon.machine("audio.gen")
eon.net("tester.side_bridge")
eon.atom("center.customer")
eon.atom("center.audio.src.test")
eon.atom("center.audio.side.src.center")
eon.atom("center.audio.side.sink.center")
eon.atom("center.audio.sink.test.realtime", {"dbg_limit": 100})

eon.connect("center.customer.0", "center.audio.src.test.0")
eon.connect("center.audio.src.test.0", "center.audio.side.src.center.0")
eon.connect("center.audio.side.src.center.1", "center.audio.side.sink.center.1")
eon.connect("center.audio.side.sink.center.0", "center.audio.sink.test.realtime.0")
eon.connect("center.customer.0", "center.audio.side.sink.center.0")
