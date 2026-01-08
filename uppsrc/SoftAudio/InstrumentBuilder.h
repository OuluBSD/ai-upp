#ifndef SOFTAUDIO_INSTRUMENTBUILDER_H
#define SOFTAUDIO_INSTRUMENTBUILDER_H

#include "SoftAudio.h"
#include "InstrumentGraph.h"

NAMESPACE_AUDIO_BEGIN

class InstrumentBuilder {
public:
    // Build a simple hybrid instrument configuration.
    static bool BuildHybridInstrument(
        const String& instrument_id,
        const InstrumentVoiceTemplate& voice_template,
        double sample_rate_hz,
        int voice_count,
        const NoteDesc& note,
        double detune_spread_cents, // e.g. +/- total spread for voices
        InstrumentGraph& out_instrument
    );
};

NAMESPACE_AUDIO_END

#endif