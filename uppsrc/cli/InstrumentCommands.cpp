#include "CommandExecutor.h"
#include "Command.h"
#include "SoftAudio/InstrumentBuilder.h"
#include "SoftAudio/InstrumentRuntime.h"

namespace Upp {

// Handler for building hybrid instruments
InvocationResult CommandExecutor::HandleInstrumentBuildHybrid(const VectorMap<String, String>& args) {
    // Extract arguments
    String instrument_id = args.Get("instrument_id", "HYBRID_OSC_DEFAULT");
    int sample_rate = StrInt(args.Get("sample_rate", "48000"));
    int voice_count = StrInt(args.Get("voice_count", "4"));
    double base_freq_hz = StrDouble(args.Get("base_freq_hz", "440.0"));
    double duration_sec = StrDouble(args.Get("duration_sec", "3.0"));
    double detune_spread_cents = StrDouble(args.Get("detune_spread_cents", "10.0"));
    double pan_lfo_hz = StrDouble(args.Get("pan_lfo_hz", "0.25"));
    bool use_analog_primary = args.Get("use_analog_primary", "true") == "true";
    String analog_block_id = args.Get("analog_block_id", "");
    String digital_block_id = args.Get("digital_block_id", "");

    // Create instrument voice template
    Audio::InstrumentVoiceTemplate voice_template;
    voice_template.analog_block_id = analog_block_id;
    voice_template.digital_block_id = digital_block_id;
    voice_template.pan_lfo_hz = pan_lfo_hz;
    voice_template.use_analog_source = use_analog_primary;

    // Create note description
    Audio::NoteDesc note_desc;
    note_desc.base_freq_hz = base_freq_hz;
    note_desc.duration_sec = duration_sec;

    // Build the instrument
    Audio::InstrumentGraph instrument;
    if (!Audio::InstrumentBuilder::BuildHybridInstrument(
            instrument_id,
            voice_template,
            sample_rate,
            voice_count,
            note_desc,
            detune_spread_cents,
            instrument)) {
        return InvocationResult(1, "Failed to build hybrid instrument");
    }

    // Create result payload
    Json js;
    js("instrument_id", instrument.instrument_id)
      ("sample_rate_hz", instrument.sample_rate_hz)
      ("voice_count", instrument.voice_count)
      ("use_analog_primary", instrument.use_analog_primary)
      ("note_base_freq_hz", instrument.note.base_freq_hz)
      ("note_duration_sec", instrument.note.duration_sec);

    JsonArray voice_array;
    for (const auto& voice : instrument.voices) {
        JsonObject voice_obj;
        voice_obj("id", voice.id)
                ("detune_cents", voice.detune_cents)
                ("use_analog_source", voice.use_analog_source);
        voice_array.Add(voice_obj);
    }
    js("voices", voice_array);

    return InvocationResult(0, "Successfully built hybrid instrument").SetPayload(js);
}

// Handler for rendering hybrid instruments
InvocationResult CommandExecutor::HandleInstrumentRenderHybrid(const VectorMap<String, String>& args) {
    // Extract arguments
    String instrument_id = args.Get("instrument_id", "HYBRID_OSC_DEFAULT");
    int sample_rate = StrInt(args.Get("sample_rate", "48000"));
    int voice_count = StrInt(args.Get("voice_count", "4"));
    double base_freq_hz = StrDouble(args.Get("base_freq_hz", "440.0"));
    double duration_sec = StrDouble(args.Get("duration_sec", "3.0"));
    double detune_spread_cents = StrDouble(args.Get("detune_spread_cents", "10.0"));
    double pan_lfo_hz = StrDouble(args.Get("pan_lfo_hz", "0.25"));
    bool use_analog_primary = args.Get("use_analog_primary", "true") == "true";
    String analog_block_id = args.Get("analog_block_id", "");
    String digital_block_id = args.Get("digital_block_id", "");

    // Create instrument voice template
    Audio::InstrumentVoiceTemplate voice_template;
    voice_template.analog_block_id = analog_block_id;
    voice_template.digital_block_id = digital_block_id;
    voice_template.pan_lfo_hz = pan_lfo_hz;
    voice_template.use_analog_source = use_analog_primary;

    // Create note description
    Audio::NoteDesc note_desc;
    note_desc.base_freq_hz = base_freq_hz;
    note_desc.duration_sec = duration_sec;

    // Build the instrument
    Audio::InstrumentGraph instrument;
    if (!Audio::InstrumentBuilder::BuildHybridInstrument(
            instrument_id,
            voice_template,
            sample_rate,
            voice_count,
            note_desc,
            detune_spread_cents,
            instrument)) {
        return InvocationResult(1, "Failed to build hybrid instrument for rendering");
    }

    // Render the instrument
    Vector<float> left_channel, right_channel;
    if (!Audio::InstrumentRuntime::RenderInstrument(instrument, left_channel, right_channel)) {
        return InvocationResult(1, "Failed to render hybrid instrument");
    }

    // Calculate RMS for the output
    double left_rms = 0.0, right_rms = 0.0;
    for (int i = 0; i < left_channel.GetCount(); i++) {
        left_rms += left_channel[i] * left_channel[i];
    }
    for (int i = 0; i < right_channel.GetCount(); i++) {
        right_rms += right_channel[i] * right_channel[i];
    }
    if (left_channel.GetCount() > 0) left_rms = sqrt(left_rms / left_channel.GetCount());
    if (right_channel.GetCount() > 0) right_rms = sqrt(right_rms / right_channel.GetCount());

    // Create result payload with preview data (first few samples)
    Json js;
    js("instrument_id", instrument.instrument_id)
      ("sample_rate_hz", instrument.sample_rate_hz)
      ("duration_sec", instrument.note.duration_sec)
      ("voice_count", instrument.voice_count)
      ("left_rms", left_rms)
      ("right_rms", right_rms);

    // Add preview of first 10 samples to avoid huge payload
    JsonArray left_preview, right_preview;
    int preview_count = min(10, left_channel.GetCount());
    for (int i = 0; i < preview_count; i++) {
        left_preview.Add(left_channel[i]);
        right_preview.Add(right_channel[i]);
    }
    js("left_preview", left_preview);
    js("right_preview", right_preview);

    return InvocationResult(0, "Successfully rendered hybrid instrument").SetPayload(js);
}

}