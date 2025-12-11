#include "SoftAudio.h"
#include "InstrumentGraph.h"
#include <plugin/json/json.h>

NAMESPACE_AUDIO_BEGIN

void Jsonize(JsonIO& json, NoteDesc& x) {
    json("base_freq_hz", x.base_freq_hz)
        ("velocity", x.velocity)
        ("duration_sec", x.duration_sec);
}

void Jsonize(JsonIO& json, VoiceConfig& x) {
    json("id", x.id)
        ("detune_cents", x.detune_cents)
        ("use_analog_source", x.use_analog_source);
}

void Jsonize(JsonIO& json, InstrumentVoiceTemplate& x) {
    json("id", x.id)
        ("analog_block_id", x.analog_block_id)
        ("digital_block_id", x.digital_block_id)
        ("has_pan_lfo", x.has_pan_lfo)
        ("pan_lfo_hz", x.pan_lfo_hz)
        ("has_filter", x.has_filter);
}

void Jsonize(JsonIO& json, InstrumentGraph& x) {
    json("instrument_id", x.instrument_id)
        ("sample_rate_hz", x.sample_rate_hz)
        ("voice_count", x.voice_count)
        ("voice_template", x.voice_template)
        ("voices", x.voices)
        ("note", x.note)
        ("use_analog_primary", x.use_analog_primary);
}

NAMESPACE_AUDIO_END