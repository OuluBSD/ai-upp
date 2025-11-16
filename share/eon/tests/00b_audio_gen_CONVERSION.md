# 00b Audio Generator – Loop → Router Notes

## Legacy `.eon`
```eon
chain tester:
	loop output:
		center.customer
		center.audio.src.test
		center.audio.side.src.center
		
	loop input:
		center.customer
		center.audio.side.sink.center
		center.audio.sink.test.realtime:
			dbg_limit = 100
```

- Two loops (`tester.output`, `tester.input`) exchange packets exclusively via the side-link pair (`center.audio.side.src.center` ↔ `center.audio.side.sink.center`).
- The realtime sink still relies on `dbg_limit = 100`, but the limiter sits downstream of the side-link bridge.

## Router Sketch
```yaml
router tester:
  net output:
    atoms:
      customer_out0: center.customer
      generator0: center.audio.src.test
      side_src0: center.audio.side.src.center
    connections:
      - customer_out0:0 -> generator0:0
      - generator0:0 -> side_src0:0

  net input:
    atoms:
      customer_in0: center.customer
      side_sink0: center.audio.side.sink.center
      sink0:
        type: center.audio.sink.test.realtime
        args:
          dbg_limit: 100
    connections:
      - customer_in0:0 -> side_sink0:0
      - side_sink0:0 -> sink0:0

  bridges:
    - output.side_src0:0 -> input.side_sink0:0
```

### Key differences
- The router sketch keeps `output` and `input` nets separate but expresses the side-link bridge explicitly in the `bridges:` list (`net.atom:port -> net.atom:port`).
- Once the PacketRouter exists, `bridges` simply become additional `connections` that reference other nets; no implicit `loop` filtering is required.
- Customers still bootstrap packet flow but stop being the only ingress points once router-managed credits arrive.
- The note above doubles as a template for any “two loop + one side bridge” conversions during the migration.
