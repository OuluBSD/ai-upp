AGENTS

Scope
- Applies to `upptst/Eon03`.

Purpose
- Console harness for exercising the Eon03 sample loops (`bin/Eon03 <test> <method>`).

Notes
- `03c_audio_file` and `03d_audio_file_2` stream to the PortAudio hardware sink; heavy realtime logging from `flagDEBUG_RT` can starve the callback and produce audible buffer underruns.
	- Build the runner without realtime logging (`script/build_upptst_eon03.sh --no-debug-rt`) when verifying audio output so PortAudio keeps its buffers filled.
- Test assets live under `share/eon/tests`; update those scripts instead of hardcoding atom chains in C++.
