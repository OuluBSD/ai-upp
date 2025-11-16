# 00c Audio Generator – Loop → Router Notes

## Legacy `.eon`
```eon
chain tester:
	loop output0:
		center.customer
		center.audio.src.test
		center.audio.side.src.center.user[][loop == "pipe"]
	
	loop output1:
		center.customer
		center.audio.src.test
		center.audio.side.src.center.user[][loop == "input"]
	
	loop pipe:
		center.customer
		center.audio.side.sink.center.user[loop == "output0"]
		center.audio.side.src.center.user[][loop == "input"]
	
	loop input:
		center.customer
		center.audio.side.sink2.center.user[loop == "pipe", loop == "output1"]
		center.audio.sink.test.poller
```

- Four loops exchange packets via tagged side-links (`loop == ...` filters).
- `side.sink2` consumes two sources (`pipe`, `output1`) while `pipe` loops an intermediate side sink/source pair.
- No primary-link wiring crosses loop boundaries; everything flows over side channels.

## Router Sketch
```yaml
router tester:
  net output0:
    atoms:
      customer_output0: center.customer
      generator0: center.audio.src.test
      side_src_pipe0: center.audio.side.src.center.user
    connections:
      - customer_output0:0 -> generator0:0
      - generator0:0 -> side_src_pipe0:0

  net output1:
    atoms:
      customer_output1: center.customer
      generator1: center.audio.src.test
      side_src_input0: center.audio.side.src.center.user
    connections:
      - customer_output1:0 -> generator1:0
      - generator1:0 -> side_src_input0:0

  net pipe:
    atoms:
      customer_pipe0: center.customer
      side_sink_output0: center.audio.side.sink.center.user
      side_src_pipe_input: center.audio.side.src.center.user
    connections:
      - customer_pipe0:0 -> side_sink_output0:0
      - side_sink_output0:0 -> side_src_pipe_input:0

  net input:
    atoms:
      customer_input0: center.customer
      side_sink2_pipe_input: center.audio.side.sink2.center.user
      poller_sink0: center.audio.sink.test.poller
    connections:
      - customer_input0:0 -> side_sink2_pipe_input:0
      - side_sink2_pipe_input:0 -> poller_sink0:0

  bridges:
    - output0.side_src_pipe0:0 -> pipe.side_sink_output0:0
    - pipe.side_src_pipe_input:0 -> input.side_sink2_pipe_input:0
    - output1.side_src_input0:0 -> input.side_sink2_pipe_input:1
```

### Key differences
- `loop == "..."` filters collapse into explicit `bridges` that reference the target net by name. Multiple filters simply become multiple bridge entries (see the two connectors feeding `side_sink2_pipe_input`).
- The router form makes the mid-loop hop through `pipe` obvious: `output0` only ever feeds the `pipe` net, and `pipe` forwards one of its ports into the `input` net.
- Nothing special happens with customer atoms; they are placeholders until router-managed credits replace them.
- This document serves as the canonical template for multi-net side-link braids (one net sourcing multiple downstream nets plus a shared intermediate net).
