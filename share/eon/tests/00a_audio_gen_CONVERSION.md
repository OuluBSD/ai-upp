# 00a Audio Generator – Loop → Router Notes

## Legacy `.eon`
```eon
loop tester.generator:
    center.customer
    center.audio.src.test
    center.audio.sink.test.realtime:
        dbg_limit = 100
```

- Implicit loop wiring forces the atoms into a ring and relies on the customer to keep a single packet in flight.
- `.out`/`.in` directionality is implicit (primary channel 0 only); there are no explicit port names.

## Router Sketch
```yaml
net tester.generator:
  atoms:
    customer0: center.customer
    generator0: center.audio.src.test
    sink0:
      type: center.audio.sink.test.realtime
      args:
        dbg_limit: 100

  connections:
    - customer0:0 -> generator0:0
    - generator0:0 -> sink0:0
```

Key differences:
- Ports are explicit (`atom:port_id`) and every connection is declared, so primary/secondary channel semantics disappear.
- Flow control metadata (queue depth, sync) moves from loop-level constructs (`CustomerBase`) into per-port descriptors.
- Router nets do not require an artificial “loop” owner; the `net` name becomes the routing namespace while the PacketRouter enforces scheduling/credits.
