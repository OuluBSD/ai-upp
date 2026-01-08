#include "SoftAudio.h"
#include "InstrumentToGraph.h"
#include "SoftAudio/Graph/GraphCore.h"
#include "SoftAudio/Graph/GainNode.h"
#include "SoftAudio/Graph/MixerNode.h"
#include "SoftAudio/Graph/GraphPlayer.h"

NAMESPACE_AUDIO_BEGIN

// A simple node to generate a sine wave with a specific frequency
class SineSourceNode : public SAGraph::Node {
public:
    SineSourceNode(double frequency, double sample_rate)
        : frequency_(frequency), sample_rate_(sample_rate), phase_(0.0) {}

    void Prepare(const ProcessContext& ctx) override {
        Node::Prepare(ctx);
        sample_rate_ = ctx.sample_rate;
        // Reset phase when preparing
        phase_ = 0.0;
    }

    void Process(const ProcessContext& ctx, const Vector<SAGraph::Bus*>& inputs, SAGraph::Bus& output) override {
        int block_size = ctx.block_size;
        output.SetSize(block_size, 1); // Mono output

        double phase_increment = 2.0 * M_PI * frequency_ / sample_rate_;

        for (int i = 0; i < block_size; i++) {
            double sample = sin(phase_);
            output.At(i, 0) = (float)sample;
            phase_ += phase_increment;
            if (phase_ > 2.0 * M_PI) phase_ -= 2.0 * M_PI;
        }
    }

private:
    double frequency_;
    double sample_rate_;
    double phase_;
};

// A simple LFO node for panning
class PanLfoNode : public SAGraph::Node {
public:
    PanLfoNode(double rate_hz, double sample_rate)
        : rate_hz_(rate_hz), sample_rate_(sample_rate), phase_(0.0) {}

    void Prepare(const ProcessContext& ctx) override {
        Node::Prepare(ctx);
        sample_rate_ = ctx.sample_rate;
        // Reset phase when preparing
        phase_ = 0.0;
    }

    void Process(const ProcessContext& ctx, const Vector<SAGraph::Bus*>& inputs, SAGraph::Bus& output) override {
        int block_size = ctx.block_size;
        output.SetSize(block_size, 1); // Mono output (pan value)

        double phase_increment = 2.0 * M_PI * rate_hz_ / sample_rate_;

        for (int i = 0; i < block_size; i++) {
            // Generate a value from 0 to 1 (sine wave shifted and scaled)
            double value = (sin(phase_) + 1.0) / 2.0;
            output.At(i, 0) = (float)value;
            phase_ += phase_increment;
            if (phase_ > 2.0 * M_PI) phase_ -= 2.0 * M_PI;
        }
    }

private:
    double rate_hz_;
    double sample_rate_;
    double phase_;
};

bool InstrumentToGraph::BuildAudioGraphForInstrument(
    const InstrumentGraph& instrument,
    SAGraph::Graph& graph
) {
    // Set graph parameters based on instrument
    graph.SetSampleRate((int)instrument.sample_rate_hz);
    graph.SetBlockSize(512); // Default block size

    // Create a mixer node for combining all voices
    One<SAGraph::MixerNode> mixer = new SAGraph::MixerNode();
    mixer->SetOutputChannels(2); // Stereo output
    int mixer_node_idx = graph.AddNodeWithName("output_mixer", pick(mixer));

    // Create voice nodes and connect them to the mixer
    for (int i = 0; i < instrument.voices.GetCount(); i++) {
        const VoiceConfig& voice = instrument.voices[i];

        // Calculate detuned frequency
        double detune_factor = exp((voice.detune_cents * log(2.0)) / 1200.0);
        double frequency = instrument.note.base_freq_hz * detune_factor;

        // Add a source node depending on whether analog or digital
        int source_node_idx;
        if (voice.use_analog_source) {
            // For now, use a SineSourceNode as placeholder - in a real implementation,
            // this would be a more complex analog circuit simulation node
            One<SineSourceNode> sine_node = new SineSourceNode(frequency, instrument.sample_rate_hz);
            source_node_idx = graph.AddNodeWithName("voice_" + voice.id + "_source", pick(sine_node));
        } else {
            // Digital oscillator - using SineSourceNode
            One<SineSourceNode> sine_node = new SineSourceNode(frequency, instrument.sample_rate_hz);
            source_node_idx = graph.AddNodeWithName("voice_" + voice.id + "_source", pick(sine_node));
        }

        int pan_lfo_node_idx = -1;
        if (instrument.voice_template.has_pan_lfo) {
            // Add a pan LFO for this voice
            One<PanLfoNode> pan_lfo_node = new PanLfoNode(instrument.voice_template.pan_lfo_hz, instrument.sample_rate_hz);
            pan_lfo_node_idx = graph.AddNodeWithName("voice_" + voice.id + "_pan_lfo", pick(pan_lfo_node));
        }

        // Create a simple stereo panner using a gain node for each channel
        // In a more complete implementation, this would be a dedicated panner node
        One<SAGraph::GainNode> left_gain = new SAGraph::GainNode();
        One<SAGraph::GainNode> right_gain = new SAGraph::GainNode();
        int left_gain_idx = graph.AddNodeWithName("voice_" + voice.id + "_left_gain", pick(left_gain));
        int right_gain_idx = graph.AddNodeWithName("voice_" + voice.id + "_right_gain", pick(right_gain));

        // Connect to mixer with appropriate gains based on pan
        if (pan_lfo_node_idx != -1) {
            // We'd need a more sophisticated panner in practice
            graph.Connect(source_node_idx, left_gain_idx, 0, 0, 0.7f);  // Left gain as example
            graph.Connect(source_node_idx, right_gain_idx, 0, 0, 0.7f); // Right gain as example

            // In a full implementation, we'd have a panner node that takes both audio and pan LFO input
        } else {
            // Connect directly with default center pan
            graph.Connect(source_node_idx, left_gain_idx, 0, 0, 0.7f);
            graph.Connect(source_node_idx, right_gain_idx, 0, 0, 0.7f);
        }

        // Connect gain nodes to mixer
        mixer->SetInputCount(i * 2 + 2); // Two inputs per voice (L/R)
        mixer->SetInputGain(i * 2, (float)instrument.note.velocity * 0.5f);     // Left
        mixer->SetInputGain(i * 2 + 1, (float)instrument.note.velocity * 0.5f); // Right
        mixer->SetInputPan(i * 2, 0.0f);   // Left channel to left
        mixer->SetInputPan(i * 2 + 1, 1.0f); // Right channel to right

        graph.Connect(left_gain_idx, mixer_node_idx, 0, 0, 1.0f);  // Connect to left input of mixer
        graph.Connect(right_gain_idx, mixer_node_idx, 0, 1, 1.0f); // Connect to right input of mixer
    }

    return true;
}

NAMESPACE_AUDIO_END