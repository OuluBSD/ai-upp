#ifndef SOFTAUDIO_INSTRUMENTGRAPH_H
#define SOFTAUDIO_INSTRUMENTGRAPH_H

#include "SoftAudio.h"

NAMESPACE_AUDIO_BEGIN

// Note description structure
struct NoteDesc {
    double base_freq_hz = 440.0;    // e.g. 440.0 (A4)
    double velocity = 1.0;          // 0..1 for now (simple amplitude scaling)
    double duration_sec = 3.0;      // e.g. 3.0
};

// Voice configuration
struct VoiceConfig : Moveable<VoiceConfig> {
    String id;                      // e.g. "voice0", "voice1"
    double detune_cents = 0.0;      // -50..+50 etc., optional
    bool use_analog_source = false; // if true, voice source is analog model; else digital oscillator
};

// Instrument voice template
struct InstrumentVoiceTemplate {
    String id = "main_voice";       // e.g. "main_voice"
    String analog_block_id;         // optional: underlying analog circuit block ID
    String digital_block_id;        // optional: digital oscillator block ID

    // Simple routing flags; we keep this phase minimal.
    bool has_pan_lfo = true;
    double pan_lfo_hz = 0.25;
    bool has_filter = false;        // reserved for future

    // Per-voice DSP parameters can be here or derived later.
};

// Main instrument graph structure
struct InstrumentGraph {
    String instrument_id;           // e.g. "HYBRID_OSC_1"

    double sample_rate_hz = 48000.0;// e.g. 48000.0
    int voice_count = 4;            // e.g. 4

    InstrumentVoiceTemplate voice_template;
    Vector<VoiceConfig> voices;

    NoteDesc note;

    // Optional: high-level mode
    bool use_analog_primary = true; // analog vs digital main source
};

NAMESPACE_AUDIO_END

#endif