#ifndef SOFTAUDIO_INSTRUMENTTOGRAPH_H
#define SOFTAUDIO_INSTRUMENTTOGRAPH_H

#include "SoftAudio.h"
#include "InstrumentGraph.h"
#include "SoftAudio/Graph/GraphCore.h"

NAMESPACE_AUDIO_BEGIN

class InstrumentToGraph {
public:
    static bool BuildAudioGraphForInstrument(
        const InstrumentGraph& instrument,
        SAGraph::Graph& graph
    );
};

NAMESPACE_AUDIO_END

#endif