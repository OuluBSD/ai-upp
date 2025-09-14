#ifndef _SoftAudio_GraphNodes_h_
#define _SoftAudio_GraphNodes_h_

#include "SoftAudio.h"
#include <SoftAudio/Graph/Graph.h>
#include <string.h>

NAMESPACE_AUDIO_BEGIN

// Wraps SoftAudio::SineWave as a graph source node (mono)
class SineNode : public SAGraph::Node {
public:
    void SetFrequency(float hz) { freq_ = hz; sine_.SetFrequency(hz); }

    void Prepare(const SAGraph::ProcessContext& ctx) override {
        SAGraph::Node::Prepare(ctx);
        temp_.SetCount(ctx.block_size, 1);
        sine_.SetFrequency(freq_);
    }

    int GetInputCount() const override { return 0; }
    SAGraph::PortSpec GetOutputSpec(int) const override { SAGraph::PortSpec p; p.channels = 1; return p; }

    void Process(const SAGraph::ProcessContext& ctx, const Vector<SAGraph::Bus*>&, SAGraph::Bus& output) override {
        temp_.SetCount(ctx.block_size, 1);
        temp_.Zero();
        sine_.Tick(temp_, 0);
        output.SetSize(ctx.block_size, 1);
        memcpy(output.data.Begin(), &temp_[0], ctx.block_size * sizeof(float));
    }

private:
    SineWave sine_;
    float freq_ = 440.0f;
    AudioFrames temp_;
};

// Wraps SoftAudio::FreeVerb as a graph effect node (stereo)
class FreeVerbNode : public SAGraph::Node {
public:
    void SetMix(float m) { verb_.SetEffectMix(m); }
    void SetRoomSize(float r) { verb_.SetRoomSize(r); }
    void SetDamping(float d) { verb_.SetDamping(d); }
    void SetWidth(float w) { verb_.SetWidth(w); }

    void Prepare(const SAGraph::ProcessContext& ctx) override {
        SAGraph::Node::Prepare(ctx);
        in_.SetCount(ctx.block_size, 2);
        out_.SetCount(ctx.block_size, 2);
    }

    SAGraph::PortSpec GetInputSpec(int) const override { SAGraph::PortSpec p; p.channels = 2; return p; }
    SAGraph::PortSpec GetOutputSpec(int) const override { SAGraph::PortSpec p; p.channels = 2; return p; }

    void Process(const SAGraph::ProcessContext& ctx, const Vector<SAGraph::Bus*>& inputs, SAGraph::Bus& output) override {
        const SAGraph::Bus* in = inputs.IsEmpty() ? nullptr : inputs[0];
        output.SetSize(ctx.block_size, 2);
        if(!in) { output.Zero(); return; }
        in_.SetCount(ctx.block_size, 2);
        // copy/expand input to stereo
        if(in->channels == 1) {
            for(int i = 0; i < ctx.block_size; ++i) {
                float s = in->At(i, 0);
                in_(i,0) = s; in_(i,1) = s;
            }
        } else {
            for(int i = 0; i < ctx.block_size; ++i) {
                in_(i,0) = in->At(i,0);
                in_(i,1) = in->At(i,1);
            }
        }
        out_.SetCount(ctx.block_size, 2);
        verb_.Tick(in_, out_, 0, 0);
        // copy to graph bus
        for(int i = 0; i < ctx.block_size; ++i) {
            output.At(i,0) = out_(i,0);
            output.At(i,1) = out_(i,1);
        }
    }

private:
    FreeVerb verb_;
    AudioFrames in_;
    AudioFrames out_;
};

// Sink to a file via SoftAudio::FileWaveOut
class FileOutNode : public SAGraph::Node {
public:
    void Open(String path, int channels = 2, FileWrite::FILE_TYPE type = FileWrite::FILE_WAV, Audio::AudioFormat fmt = Audio::AUDIO_SINT16) {
        if(open_) Close();
        out_.OpenFile(path, channels, type, fmt);
        open_ = true;
        channels_ = channels;
    }
    void Close() { if(open_) { out_.CloseFile(); open_ = false; } }
    ~FileOutNode() { Close(); }

    SAGraph::PortSpec GetInputSpec(int) const override { SAGraph::PortSpec p; p.channels = channels_ ? channels_ : 2; return p; }
    int GetOutputCount() const override { return 0; }

    void Process(const SAGraph::ProcessContext& ctx, const Vector<SAGraph::Bus*>& inputs, SAGraph::Bus&) override {
        if(inputs.IsEmpty()) return;
        const SAGraph::Bus* in = inputs[0];
        frames_.SetCount(ctx.block_size, in->channels > 1 ? in->channels : 1);
        // copy interleaved
        for(int i = 0; i < ctx.block_size; ++i)
            for(int c = 0; c < frames_.GetChannelCount(); ++c)
                frames_(i, c) = in->At(i, min(c, in->channels - 1));
        out_.Tick(frames_);
    }

private:
    FileWaveOut out_;
    bool open_ = false;
    int channels_ = 2;
    AudioFrames frames_;
};

NAMESPACE_AUDIO_END

#endif
