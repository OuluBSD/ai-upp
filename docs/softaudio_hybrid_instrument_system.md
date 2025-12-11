# Hybrid DSP+Analog Composition Layer in SoftAudio

## Overview

The Hybrid DSP+Analog Composition Layer provides a framework for creating and rendering complex instruments that combine both digital signal processing (DSP) and analog circuit models. This system allows for the creation of instruments that can use analog blocks as sound sources, processed by digital filters, LFOs, and other DSP effects, all orchestrated through a graph-based audio engine.

## Core Concepts

### InstrumentGraph IR

The system introduces an Instrument IR (Intermediate Representation) built on top of the existing `SAGraph::Graph` system:

- `InstrumentGraph`: Main structure representing a complete instrument with multiple voices
- `NoteDesc`: Describes the note to be played (frequency, velocity, duration)
- `VoiceConfig`: Configuration for individual voices (detuning, analog vs digital source)
- `InstrumentVoiceTemplate`: Template describing the signal chain of each voice

### Voice Architecture

Each instrument voice can contain:
- Analog oscillator blocks (from circuit models)
- Digital oscillators (Sine, Saw, Square, etc.)
- LFOs for modulation (panning, filters, etc.)
- Filters and panners
- Simple gain/mix nodes

## API Components

### InstrumentBuilder
Used to construct instrument configurations with appropriate detuning for multi-voice setups.

### InstrumentToGraph
Converts high-level instrument descriptions into audio graph structures that can be processed by the graph engine.

### InstrumentRuntime
Executes the instrument graph to produce rendered audio output.

## CLI Commands

### instrument-build-hybrid

Builds a hybrid instrument configuration:

```
instrument-build-hybrid \
  --instrument_id "my_hybrid_osc" \
  --sample_rate 48000 \
  --voice_count 4 \
  --base_freq_hz 440.0 \
  --duration_sec 3.0 \
  --detune_spread_cents 10.0 \
  --pan_lfo_hz 0.25 \
  --use_analog_primary true
```

### instrument-render-hybrid

Renders the hybrid instrument to produce audio:

```
instrument-render-hybrid \
  --instrument_id "my_hybrid_osc" \
  --sample_rate 48000 \
  --voice_count 4 \
  --base_freq_hz 440.0 \
  --duration_sec 3.0 \
  --detune_spread_cents 10.0 \
  --pan_lfo_hz 0.25 \
  --use_analog_primary true
```

## Example Usage

```cpp
#include "SoftAudio.h"
using namespace Upp;
using namespace Audio;

// Define a simple hybrid instrument
InstrumentVoiceTemplate voice_template;
voice_template.has_pan_lfo = true;
voice_template.pan_lfo_hz = 0.5;
voice_template.use_analog_source = true;  // Use analog oscillator

NoteDesc note;
note.base_freq_hz = 440.0;  // A4
note.duration_sec = 3.0;
note.velocity = 0.8;

// Build the instrument with 4 voices
InstrumentGraph instrument;
if (InstrumentBuilder::BuildHybridInstrument(
        "MyHybridInstrument",
        voice_template,
        48000.0,  // Sample rate
        4,        // Voice count
        note,
        10.0,     // Detune spread
        instrument)) {
    
    // Render the instrument to audio buffers
    Vector<float> left_channel, right_channel;
    if (InstrumentRuntime::RenderInstrument(instrument, left_channel, right_channel)) {
        // Successfully rendered: left_channel and right_channel contain stereo audio
        LOG("Rendered " + AsString(left_channel.GetCount()) + " samples");
    }
}
```

## Implementation Notes

The system leverages the existing SoftAudio/Graph infrastructure while adding:
- Custom source nodes for analog/digital oscillators
- LFO and panning nodes
- Multi-voice mixing capabilities
- Proper detuning algorithms for unison effect
- Integration with the CLI command system

The implementation is designed to be real-time safe during the audio processing phase while allowing more complex operations during the setup and compilation phases.