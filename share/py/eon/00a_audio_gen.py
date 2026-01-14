import eon

eon.machine("audio.gen")
eon.net("tester.generator")
eon.atom("center.customer")
eon.atom("center.audio.src.test")
eon.atom("center.audio.sink.test.realtime", {"dbg_limit": 100})
eon.connect("center.customer.0", "center.audio.src.test.0")
eon.connect("center.audio.src.test.0", "center.audio.sink.test.realtime.0")
