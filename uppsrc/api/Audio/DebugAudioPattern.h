#ifndef _Audio_DebugAudioPattern_h_
#define _Audio_DebugAudioPattern_h_

#include <Core/Core.h>

NAMESPACE_UPP

inline constexpr int DebugAudioPatternPeriod() { return 64; }

inline float DebugAudioPatternValue(uint64 frame_index, int channel, int seed) {
	uint64 offset = frame_index + (uint64)seed + (uint64)channel * (DebugAudioPatternPeriod() / 4);
	int mod = int(offset % DebugAudioPatternPeriod());
	float normalized = (float)mod / (float)(DebugAudioPatternPeriod() - 1);
	return -1.0f + 2.0f * normalized;
}

inline void DebugAudioPatternFill(float* out, int frames, int channels, uint64 start_frame, int seed) {
	for (int frame = 0; frame < frames; ++frame) {
		for (int ch = 0; ch < channels; ++ch) {
			*out++ = DebugAudioPatternValue(start_frame + frame, ch, seed);
		}
	}
}

END_UPP_NAMESPACE

#endif
