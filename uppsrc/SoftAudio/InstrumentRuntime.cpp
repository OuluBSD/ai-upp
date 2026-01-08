#include "SoftAudio.h"
#include "InstrumentRuntime.h"
#include "InstrumentToGraph.h"
#include "SoftAudio/Graph/GraphPlayer.h"

NAMESPACE_AUDIO_BEGIN

// A sink node that captures audio output for rendering
class AudioCaptureNode : public SAGraph::Node {
public:
    AudioCaptureNode(Vector<float>& left_buffer, Vector<float>& right_buffer)
        : left_buffer_(left_buffer), right_buffer_(right_buffer) {}

    void Prepare(const ProcessContext& ctx) override {
        Node::Prepare(ctx);
        // Clear buffers at start of processing
        if (ctx.is_start_of_render) {
            left_buffer_.Clear();
            right_buffer_.Clear();
        }
    }

    void Process(const ProcessContext& ctx, const Vector<SAGraph::Bus*>& inputs, SAGraph::Bus& output) override {
        if (inputs.IsEmpty() || !inputs[0]) {
            output.SetSize(ctx.block_size, 2);
            output.Zero();
            return;
        }

        const SAGraph::Bus* input = inputs[0];
        int frames = min(ctx.block_size, input->frames);
        int channels = input->channels;

        // Set up output with same characteristics as input
        output.SetSize(ctx.block_size, 2); // Always stereo output
        output.Zero();

        // Copy input to output and capture to buffers
        for (int f = 0; f < frames; f++) {
            // Capture to our stereo buffers
            if (channels >= 2) {
                left_buffer_.Add(input->At(f, 0));
                right_buffer_.Add(input->At(f, 1));
                output.At(f, 0) = input->At(f, 0);
                output.At(f, 1) = input->At(f, 1);
            } else {
                // Mono input: duplicate to stereo
                left_buffer_.Add(input->At(f, 0));
                right_buffer_.Add(input->At(f, 0));
                output.At(f, 0) = input->At(f, 0);
                output.At(f, 1) = input->At(f, 0);
            }
        }

        // Add zeros if input was shorter than block size
        for (int f = frames; f < ctx.block_size; f++) {
            left_buffer_.Add(0.0f);
            right_buffer_.Add(0.0f);
            output.At(f, 0) = 0.0f;
            output.At(f, 1) = 0.0f;
        }
    }

private:
    Vector<float>& left_buffer_;
    Vector<float>& right_buffer_;
};

bool InstrumentRuntime::RenderInstrument(
    const InstrumentGraph& instrument,
    Vector<float>& out_left,
    Vector<float>& out_right
) {
    // Create the audio graph from the instrument description
    SAGraph::Graph graph;
    if (!InstrumentToGraph::BuildAudioGraphForInstrument(instrument, graph)) {
        return false;
    }

    // Add an output capture node to the graph
    One<AudioCaptureNode> capture_node = new AudioCaptureNode(out_left, out_right);
    int capture_node_idx = graph.AddNodeWithName("output_capture", pick(capture_node));

    // Connect the mixer to our capture node (assuming mixer is last node in the graph)
    // The mixer was the first node added in BuildAudioGraphForInstrument, so it has index 0
    // Actually, let's find the mixer which was named "output_mixer"
    int mixer_idx = graph.FindNode("output_mixer");
    if (mixer_idx >= 0) {
        graph.Connect(mixer_idx, capture_node_idx, 0, 0, 1.0f);
    }

    // Compile the graph
    String error;
    if (!graph.Compile(error)) {
        // Log error or handle appropriately
        return false;
    }

    // Prepare the graph for processing
    ProcessContext ctx;
    ctx.sample_rate = (int)instrument.sample_rate_hz;
    ctx.block_size = 512;  // Default block size
    ctx.is_start_of_render = true;  // Indicate this is the start of a render
    graph.Prepare(ctx);

    // Calculate total samples needed
    int total_samples = (int)(instrument.note.duration_sec * instrument.sample_rate_hz);
    int block_size = ctx.block_size;
    int num_blocks = (total_samples + block_size - 1) / block_size;  // Round up

    // Process in blocks
    for (int block = 0; block < num_blocks; block++) {
        // Update the render state for subsequent blocks
        if (block > 0) {
            ctx.is_start_of_render = false;
            // Update frame cursor to reflect how many frames have been processed
            ctx.frame_cursor = block * block_size;
        }

        graph.ProcessBlock();

        // Check if we've reached the required duration
        if (out_left.GetCount() >= total_samples) {
            break;
        }
    }

    // Trim buffers to exact size needed
    if (out_left.GetCount() > total_samples) {
        out_left.SetCount(total_samples);
        out_right.SetCount(total_samples);
    } else if (out_left.GetCount() < total_samples) {
        // Pad with zeros if needed
        int needed = total_samples - out_left.GetCount();
        for (int i = 0; i < needed; i++) {
            out_left.Add(0.0f);
            out_right.Add(0.0f);
        }
    }

    return true;
}

NAMESPACE_AUDIO_END