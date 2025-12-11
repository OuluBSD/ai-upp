#ifndef SOFTAUDIO_INSTRUMENTRUNTIME_H
#define SOFTAUDIO_INSTRUMENTRUNTIME_H

#include "SoftAudio.h"
#include "InstrumentGraph.h"
#include "SoftAudio/Graph/GraphCore.h"

NAMESPACE_AUDIO_BEGIN

class InstrumentRuntime {
public:
    static bool RenderInstrument(
        const InstrumentGraph& instrument,
        Vector<float>& out_left,
        Vector<float>& out_right
    );
};

NAMESPACE_AUDIO_END

#endif