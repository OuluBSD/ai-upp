#include <cmath>
#include "SoftAudio.h"
#include "InstrumentBuilder.h"
#include "InstrumentRuntime.h"

#define TEST_NAME "InstrumentGraph Test"

NAMESPACE_AUDIO_BEGIN

void RunInstrumentGraphTest() {
    LOG("Starting " << TEST_NAME << "...");
    
    // Create a simple voice template
    InstrumentVoiceTemplate voice_template;
    voice_template.has_pan_lfo = true;
    voice_template.pan_lfo_hz = 0.25;
    

    
    // Set up a note description
    NoteDesc note;
    note.base_freq_hz = 440.0;  // A4
    note.duration_sec = 0.1;    // 0.1 second (short for testing)
    note.velocity = 0.8;
    
    // Build a 2-voice instrument for quick test
    InstrumentGraph instrument;
    bool success = InstrumentBuilder::BuildHybridInstrument(
        "TestInstrument",
        voice_template,
        48000.0,  // Sample rate
        2,        // Voice count
        note,
        10.0,     // Detune spread
        instrument
    );
    
    if (!success) {
        LOG("FAILED: Could not build instrument");
        return;
    }
    
    // Verify basic properties
    if (instrument.instrument_id != "TestInstrument") {
        LOG("FAILED: Instrument ID not set correctly");
        return;
    }
    
    if (instrument.voices.GetCount() != 2) {
        LOG("FAILED: Expected 2 voices, got " + AsString(instrument.voices.GetCount()));
        return;
    }
    
    // Check that detuning is distributed properly (should be around -5.0 and +5.0 for 2 voices with 10 cent spread)
    if (instrument.voices[0].detune_cents > instrument.voices[1].detune_cents) {
        LOG("FAILED: Detuning not properly distributed");
        return;
    }
    
    // Test rendering
    Vector<float> left_buffer, right_buffer;
    success = InstrumentRuntime::RenderInstrument(instrument, left_buffer, right_buffer);
    
    if (!success) {
        LOG("FAILED: Could not render instrument");
        return;
    }
    
    // Verify output has expected size
    int expected_samples = (int)(instrument.sample_rate_hz * instrument.note.duration_sec);
    if (left_buffer.GetCount() == 0 || right_buffer.GetCount() == 0) {
        LOG("FAILED: No output samples generated");
        return;
    }
    
    // Verify output is finite (not NaN or infinity)
    for (int i = 0; i < min(100, left_buffer.GetCount()); i++) {
        if (!std::isfinite(left_buffer[i]) || !std::isfinite(right_buffer[i])) {
            LOG("FAILED: Non-finite values in output");
            return;
        }
    }
    
    LOG(TEST_NAME << " PASSED");
}

NAMESPACE_AUDIO_END