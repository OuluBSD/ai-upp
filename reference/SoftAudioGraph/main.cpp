#include <CtrlLib/CtrlLib.h>
#include <SoftAudio/SoftAudio.h>
#include <SoftAudio/Graph/Graph.h>
#include <SoftAudio/GraphNodes.h>

using namespace Upp;
using namespace Upp::SAGraph;
using namespace Upp::Audio;

GUI_APP_MAIN
{
    // Configure graph
    Graph g;
    g.SetSampleRate(44100);
    g.SetBlockSize(RT_BUFFER_SIZE);
	
	// Nodes
	One<Node> sine1; sine1 = MakeOne<SineNode>();
	((SineNode*)~sine1)->SetFrequency(440.0f);
	int n_sine1 = g.AddNodeWithName("sine1", pick(sine1));
	
	One<Node> sine2; sine2 = MakeOne<SineNode>();
	((SineNode*)~sine2)->SetFrequency(660.0f);
	int n_sine2 = g.AddNodeWithName("sine2", pick(sine2));

    One<Node> gain; gain = MakeOne<GainNode>();
    ((GainNode*)~gain)->SetGain(0.2f);
    int n_gain = g.AddNode(pick(gain));
	
	One<Node> mix; mix = MakeOne<MixerNode>();
	((MixerNode*)~mix)->SetInputCount(2);
	int n_mix = g.AddNodeWithName("mix1", pick(mix));

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
    g.ConnectWithName("mix_to_gain", n_mix, n_gain);
    g.Connect(n_gain, n_verb);
    g.Connect(n_verb, n_out);

    String err;
    if(!g.Compile(err)) {
        LOG("Compile failed: " << err);
        return;
    }
    ProcessContext ctx; ctx.sample_rate = 44100; ctx.block_size = RT_BUFFER_SIZE;
    g.Prepare(ctx);
    
	// Demonstrate Graph::SetParam for node parameters
    	g.SetParams("mix1", {
    	    {"out_channels", 2},
    	    {"in0_gain", 0.2},
    	    {"in0_pan", 0.25},
    	    {"in1_gain", 0.2},
    	    {"in1_pan", 0.75}
    	});

    // Optionally, find and inspect an edge by name
    int eidx = g.FindEdge("mix_to_gain");
    const Edge* ep = g.GetEdge(eidx);
    if(ep)
        Cout() << "Edge 'mix_to_gain': from=" << ep->from << " to=" << ep->to << " gain=" << ep->gain << '\n';

    // Render 3 seconds offline
    int total_frames = 3 * ctx.sample_rate;
    int blocks = (total_frames + ctx.block_size - 1) / ctx.block_size;
    for(int i = 0; i < blocks; ++i)
        g.ProcessBlock();

    LOG("Wrote: " << GetExeDirFile("softaudiograph_demo.wav"));

    // Second example: mix sources through a Compressor to file
    Graph g2;
    g2.SetSampleRate(44100);
    g2.SetBlockSize(RT_BUFFER_SIZE);

    One<Node> s1; s1 = MakeOne<SineNode>(); ((SineNode*)~s1)->SetFrequency(220.0f); int s1i = g2.AddNode(pick(s1));
    One<Node> s2; s2 = MakeOne<SineNode>(); ((SineNode*)~s2)->SetFrequency(330.0f); int s2i = g2.AddNode(pick(s2));

    One<Node> mx; mx = MakeOne<MixerNode>();
    ((MixerNode*)~mx)->SetOutputChannels(2);
    ((MixerNode*)~mx)->SetInputCount(2);
    ((MixerNode*)~mx)->SetInputGain(0, 0.8f);
    ((MixerNode*)~mx)->SetInputPan(0, 0.2f);
    ((MixerNode*)~mx)->SetInputGain(1, 0.8f);
    ((MixerNode*)~mx)->SetInputPan(1, 0.8f);
    int n_mx = g2.AddNode(pick(mx));

    One<Node> comp; comp = MakeOne<CompressorNode>(); int n_comp = g2.AddNode(pick(comp));

    One<Node> byp; byp = MakeOne<BypassNode>();
    ((BypassNode*)~byp)->SetBypass(false); // use wet (compressed) path
    int n_byp = g2.AddNode(pick(byp));

    One<Node> out2; out2 = MakeOne<FileOutNode>();
    ((FileOutNode*)~out2)->Open(GetExeDirFile("softaudiograph_mix_compressor.wav"), 2);
    int n_out2 = g2.AddNode(pick(out2));

    // Connect dry to bypass first, then wet to bypass second
    g2.Connect(s1i, n_mx);
    g2.Connect(s2i, n_mx);
    g2.Connect(n_mx, n_comp);
    g2.Connect(n_mx, n_byp);   // dry first
    g2.Connect(n_comp, n_byp); // wet second
    g2.Connect(n_byp, n_out2);

    String err2;
    if(!g2.Compile(err2)) {
        LOG("Compile failed (g2): " << err2);
        return;
    }
    ProcessContext ctx2; ctx2.sample_rate = 44100; ctx2.block_size = RT_BUFFER_SIZE;
    g2.Prepare(ctx2);
    int total_frames2 = 3 * ctx2.sample_rate;
    int blocks2 = (total_frames2 + ctx2.block_size - 1) / ctx2.block_size;
    for(int i = 0; i < blocks2; ++i)
        g2.ProcessBlock();
    LOG("Wrote: " << GetExeDirFile("softaudiograph_mix_compressor.wav"));

    // Third example: Voicer-driven instrument path (with MidiInputNode scheduling)
    Graph g3;
    g3.SetSampleRate(44100);
    g3.SetBlockSize(RT_BUFFER_SIZE);
    One<Node> v; v = MakeOne<VoicerNode>(); VoicerNode* vp = (VoicerNode*)~v; int n_v = g3.AddNode(pick(v));
    One<Node> midi; midi = MakeOne<MidiInputNode>();
	MidiInputNode* mp = (MidiInputNode*)~midi;
    
    ((MidiInputNode*)~midi)->SetTarget(vp);
    int n_midi = g3.AddNode(pick(midi));

    One<Node> r; r = MakeOne<RouterNode>();
    ((RouterNode*)~r)->SetTargetChannels(2);
    int n_r = g3.AddNode(pick(r));

    One<Node> verb3; verb3 = MakeOne<FreeVerbNode>(); ((FreeVerbNode*)~verb3)->SetMix(0.25f); int n_verb3 = g3.AddNode(pick(verb3));
    One<Node> out3; out3 = MakeOne<FileOutNode>(); ((FileOutNode*)~out3)->Open(GetExeDirFile("softaudiograph_voicer.wav"), 2); int n_out3 = g3.AddNode(pick(out3));

    // Connect midi -> voicer (dummy audio edge for ordering), then voicer -> router
    g3.Connect(n_midi, n_v);
    g3.Connect(n_v, n_r);
    g3.Connect(n_r, n_verb3);
    g3.Connect(n_verb3, n_out3);

    String err3; if(!g3.Compile(err3)) {
		LOG("Compile failed (g3): " << err3);
		return;
	}
    ProcessContext ctx3; ctx3.sample_rate = 44100; ctx3.block_size = RT_BUFFER_SIZE; g3.Prepare(ctx3);

    int total_frames3 = 3 * ctx3.sample_rate;
    int blocks3 = (total_frames3 + ctx3.block_size - 1) / ctx3.block_size;
    // Schedule MIDI via MidiInputNode in absolute frame time
    ASSERT(mp);
    auto at = [&](double sec){ return (unsigned long long)(sec * ctx3.sample_rate); };
    mp->EnqueueNoteOn(60.0f, 0.9f, at(0.00)); // C4
    mp->EnqueueNoteOff(60.0f, 0.7f, at(1.40));
    mp->EnqueueNoteOn(67.0f, 0.9f, at(1.50)); // G4
    mp->EnqueueNoteOff(67.0f, 0.7f, at(2.90));

    for(int i = 0; i < blocks3; ++i) g3.ProcessBlock();
    LOG("Wrote: " << GetExeDirFile("softaudiograph_voicer.wav"));

    // Live example (requires PortAudio): play mixed sines for ~2 seconds
    Graph live;
    live.SetSampleRate(44100);
    live.SetBlockSize(RT_BUFFER_SIZE);
    One<Node> ls1; ls1 = MakeOne<SineNode>(); ((SineNode*)~ls1)->SetFrequency(440.0f); int ls1i = live.AddNode(pick(ls1));
    One<Node> ls2; ls2 = MakeOne<SineNode>(); ((SineNode*)~ls2)->SetFrequency(550.0f); int ls2i = live.AddNode(pick(ls2));
    One<Node> lmix; lmix = MakeOne<MixerNode>(); ((MixerNode*)~lmix)->SetInputCount(2); int lmixi = live.AddNode(pick(lmix));
    One<Node> lout; lout = MakeOne<LiveOutNode>(); int louti = live.AddNode(pick(lout));
    live.Connect(ls1i, lmixi);
    live.Connect(ls2i, lmixi);
    live.Connect(lmixi, louti);
    String el; if(!live.Compile(el)) { LOG("Compile failed (live): " << el); return; }
    ProcessContext lctx; lctx.sample_rate = 44100; lctx.block_size = RT_BUFFER_SIZE; live.Prepare(lctx);
    for(int i = 0; i < 2 * 44100 / RT_BUFFER_SIZE; ++i) live.ProcessBlock();
}
