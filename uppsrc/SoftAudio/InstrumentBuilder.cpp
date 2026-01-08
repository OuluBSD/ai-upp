#include "SoftAudio.h" 
#include "InstrumentBuilder.h"

NAMESPACE_AUDIO_BEGIN

// Helper function to convert cents to frequency multiplier
static double CentsToMultiplier(double cents) {
    return exp((cents * log(2.0)) / 1200.0);
}

bool InstrumentBuilder::BuildHybridInstrument(
    const String& instrument_id,
    const InstrumentVoiceTemplate& voice_template,
    double sample_rate_hz,
    int voice_count,
    const NoteDesc& note,
    double detune_spread_cents,
    InstrumentGraph& out_instrument
) {
    out_instrument.instrument_id = instrument_id;
    out_instrument.sample_rate_hz = sample_rate_hz;
    out_instrument.voice_count = voice_count;
    out_instrument.voice_template = voice_template;
    out_instrument.note = note;
    out_instrument.use_analog_primary = voice_template.analog_block_id.GetCount() > 0;

    // Generate voice configurations
    out_instrument.voices.Clear();
    out_instrument.voices.SetCount(voice_count);

    // Distribute detune linearly across voices
    if (voice_count > 1) {
        double step = detune_spread_cents / (voice_count - 1);
        double start = -detune_spread_cents / 2.0;

        for (int i = 0; i < voice_count; i++) {
            out_instrument.voices[i].id = "voice" + AsString(i);
            out_instrument.voices[i].detune_cents = start + i * step;
            out_instrument.voices[i].use_analog_source = out_instrument.use_analog_primary;
        }
    } else {
        // Single voice: no detuning
        out_instrument.voices[0].id = "voice0";
        out_instrument.voices[0].detune_cents = 0.0;
        out_instrument.voices[0].use_analog_source = out_instrument.use_analog_primary;
    }

    return true;
}

NAMESPACE_AUDIO_END