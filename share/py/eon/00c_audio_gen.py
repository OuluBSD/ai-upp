import eon

eon.machine("audio.gen")
eon.net("tester.side_braid")
eon.atom("center.customer")
eon.atom("center.audio.src.test")
eon.atom("center.audio.side.src.center.user")
eon.atom("center.audio.side.sink.center.user")
eon.atom("center.audio.side.sink2.center.user")
eon.atom("center.audio.sink.test.poller")

eon.connect("center.customer.0", "center.audio.src.test.0")
eon.connect("center.audio.src.test.0", "center.audio.side.src.center.user.0")
eon.connect("center.customer.0", "center.audio.side.sink.center.user.0")
eon.connect("center.audio.side.src.center.user.1", "center.audio.side.sink.center.user.1")
eon.connect("center.customer.0", "center.audio.side.sink2.center.user.0")
eon.connect("center.audio.side.sink.center.user.0", "center.audio.side.sink2.center.user.1")
eon.connect("center.audio.side.sink2.center.user.0", "center.audio.sink.test.poller.0")
