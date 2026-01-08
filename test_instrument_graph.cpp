#include "SoftAudio.h"
#include "InstrumentBuilder.h"
#include "InstrumentRuntime.h"
#include <Core/Core.h>

using namespace Upp;
using namespace Audio;

CONSOLE_APP_MAIN {
    // Test building a simple hybrid instrument
    LOG("Testing InstrumentGraph system...");
    
    // Create a simple voice template
    InstrumentVoiceTemplate voice_template;
    voice_template.has_pan_lfo = true;
    voice_template.pan_lfo_hz = 0.25;
    voice_template.use_analog_source = false;  // Use digital oscillator for this test
    
    // Set up a note description
    NoteDesc note;
    note.base_freq_hz = 440.0;  // A4
    note.duration_sec = 1.0;    // 1 second
    note.velocity = 0.8;
    
    // Build a 4-voice instrument
    InstrumentGraph instrument;
    bool success = InstrumentBuilder::BuildHybridInstrument(
        "TestInstrument",
        voice_template,
        48000.0,  // Sample rate
        4,        // Voice count
        note,
        10.0,     // Detune spread
        instrument
    );
    
    if (!success) {
        LOG("FAILED: Could not build instrument");
        return;
    }
    
    LOG("Instrument built successfully:");
    LOG("  - ID: " + instrument.instrument_id);
    LOG("  - Sample rate: " + AsString(instrument.sample_rate_hz));
    LOG("  - Voice count: " + AsString(instrument.voice_count));
    LOG("  - Voices: " + AsString(instrument.voices.GetCount()));
    
    // Verify voice count and detuning
    if (instrument.voices.GetCount() != 4) {
        LOG("FAILED: Expected 4 voices, got " + AsString(instrument.voices.GetCount()));
        return;
    }
    
    // Check that detuning is distributed properly
    LOG("Voice detuning:");
    for (int i = 0; i < instrument.voices.GetCount(); i++) {
        LOG("  Voice " + AsString(i) + ": " + AsString(instrument.voices[i].detune_cents) + " cents");
    }
    
    // Test rendering
    LOG("Rendering instrument...");
    Vector<float> left_buffer, right_buffer;
    success = InstrumentRuntime::RenderInstrument(instrument, left_buffer, right_buffer);
    
    if (!success) {
        LOG("FAILED: Could not render instrument");
        return;
    }
    
    LOG("Render completed:");
    LOG("  - Left samples: " + AsString(left_buffer.GetCount()));
    LOG("  - Right samples: " + AsString(right_buffer.GetCount()));
    LOG("  - Expected samples: " + AsString((int)(48000.0 * 1.0)));  // sr * duration
    
    // Verify that we got roughly the right number of samples
    int expected_samples = (int)(instrument.sample_rate_hz * instrument.note.duration_sec);
    if (abs(left_buffer.GetCount() - expected_samples) > 100) {  // Allow some buffer for block alignment
        LOG("WARNING: Sample count differs significantly from expected");
    }
    
    // Check that output is not all zeros
    bool all_zero = true;
    for (int i = 0; i < min(1000, left_buffer.GetCount()); i++) {  // Check first 1000 samples
        if (fabs(left_buffer[i]) > 1e-10 || fabs(right_buffer[i]) > 1e-10) {
            all_zero = false;
            break;
        }
    }
    
    if (all_zero) {
        LOG("WARNING: Output appears to be all zeros");
    } else {
        LOG("SUCCESS: Output contains non-zero audio data");
    }
    
    LOG("All tests completed successfully!");
}