#include <SoftAudio/SoftAudio.h>
#include <SoftAudio/Graph/Graph.h>
#include <SoftAudio/GraphNodes.h>

using namespace Upp;
using namespace Upp::SAGraph;
using namespace Upp::Audio;

CONSOLE_APP_MAIN
{
    // Configure graph
    Graph g;
    g.SetSampleRate(44100);
    g.SetBlockSize(RT_BUFFER_SIZE);

    // Nodes
    One<Node> sine1; sine1 = MakeOne<SineNode>();
    ((SineNode*)~sine1)->SetFrequency(440.0f);
    int n_sine1 = g.AddNode(pick(sine1));

    One<Node> sine2; sine2 = MakeOne<SineNode>();
    ((SineNode*)~sine2)->SetFrequency(660.0f);
    int n_sine2 = g.AddNode(pick(sine2));

    One<Node> gain; gain = MakeOne<GainNode>();
    ((GainNode*)~gain)->SetGain(0.2f);
    int n_gain = g.AddNode(pick(gain));

    One<Node> mix; mix = MakeOne<MixerNode>();
    ((MixerNode*)~mix)->SetOutputChannels(2);
    ((MixerNode*)~mix)->SetInputCount(2);
    ((MixerNode*)~mix)->SetInputGain(0, 0.2f);
    ((MixerNode*)~mix)->SetInputPan(0, 0.25f);
    ((MixerNode*)~mix)->SetInputGain(1, 0.2f);
    ((MixerNode*)~mix)->SetInputPan(1, 0.75f);
    int n_mix = g.AddNode(pick(mix));

    One<Node> verb; verb = MakeOne<FreeVerbNode>();
    ((FreeVerbNode*)~verb)->SetMix(0.3f);
    ((FreeVerbNode*)~verb)->SetRoomSize(0.6f);
    int n_verb = g.AddNode(pick(verb));

    One<Node> out; out = MakeOne<FileOutNode>();
    ((FileOutNode*)~out)->Open(GetExeDirFile("softaudiograph_demo.wav"), 2);
    int n_out = g.AddNode(pick(out));

    // Connections: sine1 + sine2 -> mixer -> gain -> verb -> out
    g.Connect(n_sine1, n_mix);
    g.Connect(n_sine2, n_mix);
    g.Connect(n_mix, n_gain);
    g.Connect(n_gain, n_verb);
    g.Connect(n_verb, n_out);

    String err;
    if(!g.Compile(err)) {
        Cout() << "Compile failed: " << err << '\n';
        return;
    }
    ProcessContext ctx; ctx.sample_rate = 44100; ctx.block_size = RT_BUFFER_SIZE;
    g.Prepare(ctx);

    // Render 3 seconds offline
    int total_frames = 3 * ctx.sample_rate;
    int blocks = (total_frames + ctx.block_size - 1) / ctx.block_size;
    for(int i = 0; i < blocks; ++i)
        g.ProcessBlock();

    Cout() << "Wrote: " << GetExeDirFile("softaudiograph_demo.wav") << '\n';
}
