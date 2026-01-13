// This file have been generated automatically.
// DO NOT MODIFY THIS FILE!

#ifndef _IHal_IHal_h_
#define _IHal_IHal_h_

#include <Eon/Eon.h>
#include <api/Graphics/Graphics.h>

NAMESPACE_UPP

// 1. Forward declarations
struct HalUpp;
struct HalSdl;
struct HalHolo;

// 2. Base Atom types
struct HalAudioSinkDevice : public Atom { using Atom::Atom; void Visit(Vis& v) override {VIS_THIS(Atom);} virtual ~HalAudioSinkDevice() {} };
struct HalCenterVideoSinkDevice : public Atom { using Atom::Atom; void Visit(Vis& v) override {VIS_THIS(Atom);} virtual ~HalCenterVideoSinkDevice() {} };
struct HalCenterFboSinkDevice : public Atom { using Atom::Atom; void Visit(Vis& v) override {VIS_THIS(Atom);} virtual ~HalCenterFboSinkDevice() {} };
struct HalOglVideoSinkDevice : public Atom { using Atom::Atom; void Visit(Vis& v) override {VIS_THIS(Atom);} virtual ~HalOglVideoSinkDevice() {} };
struct HalD12VideoSinkDevice : public Atom { using Atom::Atom; void Visit(Vis& v) override {VIS_THIS(Atom);} virtual ~HalD12VideoSinkDevice() {} };
struct HalContextBase : public Atom { using Atom::Atom; void Visit(Vis& v) override {VIS_THIS(Atom);} virtual ~HalContextBase() {} };
struct HalEventsBase : public Atom { using Atom::Atom; void Visit(Vis& v) override {VIS_THIS(Atom);} virtual ~HalEventsBase() {} };
struct HalGuiSinkBase : public Atom { using Atom::Atom; void Visit(Vis& v) override {VIS_THIS(Atom);} virtual ~HalGuiSinkBase() {} };
struct HalGuiFileSrc : public Atom { using Atom::Atom; void Visit(Vis& v) override {VIS_THIS(Atom);} virtual ~HalGuiFileSrc() {} };

// 3. Template T classes
template <class Hal> struct HalAudioSinkDeviceT : HalAudioSinkDevice {
    using CLASSNAME = HalAudioSinkDeviceT<Hal>; using HalAudioSinkDevice::HalAudioSinkDevice;
    void Visit(Vis& v) override { if (this->dev) Hal::AudioSinkDevice_Visit(*this->dev, *this, v); VIS_THIS(HalAudioSinkDevice); }
    typename Hal::NativeAudioSinkDevice* dev = 0;
    bool Initialize(const WorldState& ws) override { if (!Hal::AudioSinkDevice_Create(dev)) return false; if (!Hal::AudioSinkDevice_Initialize(*dev, *this, ws)) return false; return true; }
    bool PostInitialize() override { if (!Hal::AudioSinkDevice_PostInitialize(*dev, *this)) return false; return true; }
    bool Start() override { return Hal::AudioSinkDevice_Start(*dev, *this); }
    void Stop() override { Hal::AudioSinkDevice_Stop(*dev, *this); }
    void Uninitialize() override { Hal::AudioSinkDevice_Uninitialize(*dev, *this); Hal::AudioSinkDevice_Destroy(dev); }
    bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override { return Hal::AudioSinkDevice_Send(*dev, *this, cfg, out, src_ch); }
    bool Recv(int sink_ch, const Packet& in) override { return Hal::AudioSinkDevice_Recv(*dev, *this, sink_ch, in); }
    void Finalize(RealtimeSourceConfig& cfg) override { return Hal::AudioSinkDevice_Finalize(*dev, *this, cfg); }
    void Update(double dt) override { return Hal::AudioSinkDevice_Update(*dev, *this, dt); }
    bool IsReady(PacketIO& io) override { return Hal::AudioSinkDevice_IsReady(*dev, *this, io); }
    bool AttachContext(AtomBase& a) override { return Hal::AudioSinkDevice_AttachContext(*this->dev, *this, a); }
    void DetachContext(AtomBase& a) override { Hal::AudioSinkDevice_DetachContext(*this->dev, *this, a); }
};

template <class Hal> struct HalCenterVideoSinkDeviceT : HalCenterVideoSinkDevice {
    using CLASSNAME = HalCenterVideoSinkDeviceT<Hal>; using HalCenterVideoSinkDevice::HalCenterVideoSinkDevice;
    void Visit(Vis& v) override { if (this->dev) Hal::CenterVideoSinkDevice_Visit(*this->dev, *this, v); VIS_THIS(HalCenterVideoSinkDevice); }
    typename Hal::NativeCenterVideoSinkDevice* dev = 0;
    bool Initialize(const WorldState& ws) override { if (!Hal::CenterVideoSinkDevice_Create(dev)) return false; if (!Hal::CenterVideoSinkDevice_Initialize(*dev, *this, ws)) return false; return true; }
    bool PostInitialize() override { if (!Hal::CenterVideoSinkDevice_PostInitialize(*dev, *this)) return false; return true; }
    bool Start() override { return Hal::CenterVideoSinkDevice_Start(*dev, *this); }
    void Stop() override { Hal::CenterVideoSinkDevice_Stop(*dev, *this); }
    void Uninitialize() override { Hal::CenterVideoSinkDevice_Uninitialize(*dev, *this); Hal::CenterVideoSinkDevice_Destroy(dev); }
    bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override { return Hal::CenterVideoSinkDevice_Send(*dev, *this, cfg, out, src_ch); }
    bool Recv(int sink_ch, const Packet& in) override { return Hal::CenterVideoSinkDevice_Recv(*dev, *this, sink_ch, in); }
    void Finalize(RealtimeSourceConfig& cfg) override { return Hal::CenterVideoSinkDevice_Finalize(*dev, *this, cfg); }
    void Update(double dt) override { return Hal::CenterVideoSinkDevice_Update(*dev, *this, dt); }
    bool IsReady(PacketIO& io) override { return Hal::CenterVideoSinkDevice_IsReady(*dev, *this, io); }
    bool AttachContext(AtomBase& a) override { return Hal::CenterVideoSinkDevice_AttachContext(*this->dev, *this, a); }
    void DetachContext(AtomBase& a) override { Hal::CenterVideoSinkDevice_DetachContext(*this->dev, *this, a); }
};

template <class Hal> struct HalCenterFboSinkDeviceT : HalCenterFboSinkDevice {
    using CLASSNAME = HalCenterFboSinkDeviceT<Hal>; using HalCenterFboSinkDevice::HalCenterFboSinkDevice;
    void Visit(Vis& v) override { if (this->dev) Hal::CenterFboSinkDevice_Visit(*this->dev, *this, v); VIS_THIS(HalCenterFboSinkDevice); }
    typename Hal::NativeCenterFboSinkDevice* dev = 0;
    bool Initialize(const WorldState& ws) override { if (!Hal::CenterFboSinkDevice_Create(dev)) return false; if (!Hal::CenterFboSinkDevice_Initialize(*dev, *this, ws)) return false; return true; }
    bool PostInitialize() override { if (!Hal::CenterFboSinkDevice_PostInitialize(*dev, *this)) return false; return true; }
    bool Start() override { return Hal::CenterFboSinkDevice_Start(*dev, *this); }
    void Stop() override { Hal::CenterFboSinkDevice_Stop(*dev, *this); }
    void Uninitialize() override { Hal::CenterFboSinkDevice_Uninitialize(*dev, *this); Hal::CenterFboSinkDevice_Destroy(dev); }
    bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override { return Hal::CenterFboSinkDevice_Send(*dev, *this, cfg, out, src_ch); }
    bool Recv(int sink_ch, const Packet& in) override { return Hal::CenterFboSinkDevice_Recv(*dev, *this, sink_ch, in); }
    void Finalize(RealtimeSourceConfig& cfg) override { return Hal::CenterFboSinkDevice_Finalize(*dev, *this, cfg); }
    void Update(double dt) override { return Hal::CenterFboSinkDevice_Update(*dev, *this, dt); }
    bool IsReady(PacketIO& io) override { return Hal::CenterFboSinkDevice_IsReady(*dev, *this, io); }
    bool AttachContext(AtomBase& a) override { return Hal::CenterFboSinkDevice_AttachContext(*this->dev, *this, a); }
    void DetachContext(AtomBase& a) override { Hal::CenterFboSinkDevice_DetachContext(*this->dev, *this, a); }
};

template <class Hal> struct HalOglVideoSinkDeviceT : HalOglVideoSinkDevice {
    using CLASSNAME = HalOglVideoSinkDeviceT<Hal>; using HalOglVideoSinkDevice::HalOglVideoSinkDevice;
    void Visit(Vis& v) override { if (this->dev) Hal::OglVideoSinkDevice_Visit(*this->dev, *this, v); VIS_THIS(HalOglVideoSinkDevice); }
    typename Hal::NativeOglVideoSinkDevice* dev = 0;
    bool Initialize(const WorldState& ws) override { if (!Hal::OglVideoSinkDevice_Create(dev)) return false; if (!Hal::OglVideoSinkDevice_Initialize(*dev, *this, ws)) return false; return true; }
    bool PostInitialize() override { if (!Hal::OglVideoSinkDevice_PostInitialize(*dev, *this)) return false; return true; }
    bool Start() override { return Hal::OglVideoSinkDevice_Start(*dev, *this); }
    void Stop() override { Hal::OglVideoSinkDevice_Stop(*dev, *this); }
    void Uninitialize() override { Hal::OglVideoSinkDevice_Uninitialize(*dev, *this); Hal::OglVideoSinkDevice_Destroy(dev); }
    bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override { return Hal::OglVideoSinkDevice_Send(*dev, *this, cfg, out, src_ch); }
    bool Recv(int sink_ch, const Packet& in) override { return Hal::OglVideoSinkDevice_Recv(*dev, *this, sink_ch, in); }
    void Finalize(RealtimeSourceConfig& cfg) override { return Hal::OglVideoSinkDevice_Finalize(*dev, *this, cfg); }
    void Update(double dt) override { return Hal::OglVideoSinkDevice_Update(*dev, *this, dt); }
    bool IsReady(PacketIO& io) override { return Hal::OglVideoSinkDevice_IsReady(*dev, *this, io); }
    bool AttachContext(AtomBase& a) override { return Hal::OglVideoSinkDevice_AttachContext(*this->dev, *this, a); }
    void DetachContext(AtomBase& a) override { Hal::OglVideoSinkDevice_DetachContext(*this->dev, *this, a); }
};

template <class Hal> struct HalD12VideoSinkDeviceT : HalD12VideoSinkDevice {
    using CLASSNAME = HalD12VideoSinkDeviceT<Hal>; using HalD12VideoSinkDevice::HalD12VideoSinkDevice;
    void Visit(Vis& v) override { if (this->dev) Hal::D12VideoSinkDevice_Visit(*this->dev, *this, v); VIS_THIS(HalD12VideoSinkDevice); }
    typename Hal::NativeD12VideoSinkDevice* dev = 0;
    bool Initialize(const WorldState& ws) override { if (!Hal::D12VideoSinkDevice_Create(dev)) return false; if (!Hal::D12VideoSinkDevice_Initialize(*dev, *this, ws)) return false; return true; }
    bool PostInitialize() override { if (!Hal::D12VideoSinkDevice_PostInitialize(*dev, *this)) return false; return true; }
    bool Start() override { return Hal::D12VideoSinkDevice_Start(*dev, *this); }
    void Stop() override { Hal::D12VideoSinkDevice_Stop(*dev, *this); }
    void Uninitialize() override { Hal::D12VideoSinkDevice_Uninitialize(*dev, *this); Hal::D12VideoSinkDevice_Destroy(dev); }
    bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override { return Hal::D12VideoSinkDevice_Send(*dev, *this, cfg, out, src_ch); }
    bool Recv(int sink_ch, const Packet& in) override { return Hal::D12VideoSinkDevice_Recv(*dev, *this, sink_ch, in); }
    void Finalize(RealtimeSourceConfig& cfg) override { return Hal::D12VideoSinkDevice_Finalize(*dev, *this, cfg); }
    void Update(double dt) override { return Hal::D12VideoSinkDevice_Update(*dev, *this, dt); }
    bool IsReady(PacketIO& io) override { return Hal::D12VideoSinkDevice_IsReady(*dev, *this, io); }
    bool AttachContext(AtomBase& a) override { return Hal::D12VideoSinkDevice_AttachContext(*this->dev, *this, a); }
    void DetachContext(AtomBase& a) override { Hal::D12VideoSinkDevice_DetachContext(*this->dev, *this, a); }
};

template <class Hal> struct HalContextBaseT : HalContextBase {
    using CLASSNAME = HalContextBaseT<Hal>; using HalContextBase::HalContextBase;
    void Visit(Vis& v) override { if (this->dev) Hal::ContextBase_Visit(*this->dev, *this, v); VIS_THIS(HalContextBase); }
    typename Hal::NativeContextBase* dev = 0;
    bool Initialize(const WorldState& ws) override { if (!Hal::ContextBase_Create(dev)) return false; if (!Hal::ContextBase_Initialize(*dev, *this, ws)) return false; return true; }
    bool PostInitialize() override { if (!Hal::ContextBase_PostInitialize(*dev, *this)) return false; return true; }
    bool Start() override { return Hal::ContextBase_Start(*dev, *this); }
    void Stop() override { Hal::ContextBase_Stop(*dev, *this); }
    void Uninitialize() override { Hal::ContextBase_Uninitialize(*dev, *this); Hal::ContextBase_Destroy(dev); }
    bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override { return Hal::ContextBase_Send(*dev, *this, cfg, out, src_ch); }
    bool Recv(int sink_ch, const Packet& in) override { return Hal::ContextBase_Recv(*dev, *this, sink_ch, in); }
    void Finalize(RealtimeSourceConfig& cfg) override { return Hal::ContextBase_Finalize(*dev, *this, cfg); }
    void Update(double dt) override { return Hal::ContextBase_Update(*dev, *this, dt); }
    bool IsReady(PacketIO& io) override { return Hal::ContextBase_IsReady(*dev, *this, io); }
    bool AttachContext(AtomBase& a) override { return Hal::ContextBase_AttachContext(*this->dev, *this, a); }
    void DetachContext(AtomBase& a) override { Hal::ContextBase_DetachContext(*this->dev, *this, a); }
};

template <class Hal> struct HalEventsBaseT : HalEventsBase {
    using CLASSNAME = HalEventsBaseT<Hal>; using HalEventsBase::HalEventsBase;
    void Visit(Vis& v) override { if (this->dev) Hal::EventsBase_Visit(*this->dev, *this, v); VIS_THIS(HalEventsBase); }
    typename Hal::NativeEventsBase* dev = 0;
    bool Initialize(const WorldState& ws) override { if (!Hal::EventsBase_Create(dev)) return false; if (!Hal::EventsBase_Initialize(*dev, *this, ws)) return false; return true; }
    bool PostInitialize() override { if (!Hal::EventsBase_PostInitialize(*dev, *this)) return false; return true; }
    bool Start() override { return Hal::EventsBase_Start(*dev, *this); }
    void Stop() override { Hal::EventsBase_Stop(*dev, *this); }
    void Uninitialize() override { Hal::EventsBase_Uninitialize(*dev, *this); Hal::EventsBase_Destroy(dev); }
    bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override { return Hal::EventsBase_Send(*dev, *this, cfg, out, src_ch); }
    bool Recv(int sink_ch, const Packet& in) override { return Hal::EventsBase_Recv(*dev, *this, sink_ch, in); }
    void Finalize(RealtimeSourceConfig& cfg) override { return Hal::EventsBase_Finalize(*dev, *this, cfg); }
    void Update(double dt) override { return Hal::EventsBase_Update(*dev, *this, dt); }
    bool IsReady(PacketIO& io) override { return Hal::EventsBase_IsReady(*dev, *this, io); }
    bool AttachContext(AtomBase& a) override { return Hal::EventsBase_AttachContext(*this->dev, *this, a); }
    void DetachContext(AtomBase& a) override { Hal::EventsBase_DetachContext(*this->dev, *this, a); }
};

template <class Hal> struct HalGuiSinkBaseT : HalGuiSinkBase {
    using CLASSNAME = HalGuiSinkBaseT<Hal>; using HalGuiSinkBase::HalGuiSinkBase;
    void Visit(Vis& v) override { if (this->dev) Hal::GuiSinkBase_Visit(*this->dev, *this, v); VIS_THIS(HalGuiSinkBase); }
    typename Hal::NativeGuiSinkBase* dev = 0;
    bool Initialize(const WorldState& ws) override { if (!Hal::GuiSinkBase_Create(dev)) return false; if (!Hal::GuiSinkBase_Initialize(*dev, *this, ws)) return false; return true; }
    bool PostInitialize() override { if (!Hal::GuiSinkBase_PostInitialize(*dev, *this)) return false; return true; }
    bool Start() override { return Hal::GuiSinkBase_Start(*dev, *this); }
    void Stop() override { Hal::GuiSinkBase_Stop(*dev, *this); }
    void Uninitialize() override { Hal::GuiSinkBase_Uninitialize(*dev, *this); Hal::GuiSinkBase_Destroy(dev); }
    bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override { return Hal::GuiSinkBase_Send(*dev, *this, cfg, out, src_ch); }
    bool Recv(int sink_ch, const Packet& in) override { return Hal::GuiSinkBase_Recv(*dev, *this, sink_ch, in); }
    void Finalize(RealtimeSourceConfig& cfg) override { return Hal::GuiSinkBase_Finalize(*dev, *this, cfg); }
    void Update(double dt) override { return Hal::GuiSinkBase_Update(*dev, *this, dt); }
    bool IsReady(PacketIO& io) override { return Hal::GuiSinkBase_IsReady(*dev, *this, io); }
    bool AttachContext(AtomBase& a) override { return Hal::GuiSinkBase_AttachContext(*this->dev, *this, a); }
    void DetachContext(AtomBase& a) override { Hal::GuiSinkBase_DetachContext(*this->dev, *this, a); }
};

template <class Hal> struct HalGuiFileSrcT : HalGuiFileSrc {
    using CLASSNAME = HalGuiFileSrcT<Hal>; using HalGuiFileSrc::HalGuiFileSrc;
    void Visit(Vis& v) override { if (this->dev) Hal::GuiFileSrc_Visit(*this->dev, *this, v); VIS_THIS(HalGuiFileSrc); }
    typename Hal::NativeGuiFileSrc* dev = 0;
    bool Initialize(const WorldState& ws) override { if (!Hal::GuiFileSrc_Create(dev)) return false; if (!Hal::GuiFileSrc_Initialize(*dev, *this, ws)) return false; return true; }
    bool PostInitialize() override { if (!Hal::GuiFileSrc_PostInitialize(*dev, *this)) return false; return true; }
    bool Start() override { return Hal::GuiFileSrc_Start(*dev, *this); }
    void Stop() override { Hal::GuiFileSrc_Stop(*dev, *this); }
    void Uninitialize() override { Hal::GuiFileSrc_Uninitialize(*dev, *this); Hal::GuiFileSrc_Destroy(dev); }
    bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override { return Hal::GuiFileSrc_Send(*dev, *this, cfg, out, src_ch); }
    bool Recv(int sink_ch, const Packet& in) override { return Hal::GuiFileSrc_Recv(*dev, *this, sink_ch, in); }
    void Finalize(RealtimeSourceConfig& cfg) override { return Hal::GuiFileSrc_Finalize(*dev, *this, cfg); }
    void Update(double dt) override { return Hal::GuiFileSrc_Update(*dev, *this, dt); }
    bool IsReady(PacketIO& io) override { return Hal::GuiFileSrc_IsReady(*dev, *this, io); }
    bool AttachContext(AtomBase& a) override { return Hal::GuiFileSrc_AttachContext(*this->dev, *this, a); }
    void DetachContext(AtomBase& a) override { Hal::GuiFileSrc_DetachContext(*this->dev, *this, a); }
};

// 4. Vendor implementations
struct HalUpp {
    #if (defined flagHAL && defined flagAUDIO)
    struct NativeAudioSinkDevice;
    static bool AudioSinkDevice_Create(NativeAudioSinkDevice*& dev);
    static void AudioSinkDevice_Destroy(NativeAudioSinkDevice*& dev);
    static bool AudioSinkDevice_Initialize(NativeAudioSinkDevice&, AtomBase&, const WorldState&);
    static bool AudioSinkDevice_PostInitialize(NativeAudioSinkDevice&, AtomBase&);
    static bool AudioSinkDevice_Start(NativeAudioSinkDevice&, AtomBase&);
    static void AudioSinkDevice_Stop(NativeAudioSinkDevice&, AtomBase&);
    static void AudioSinkDevice_Uninitialize(NativeAudioSinkDevice&, AtomBase&);
    static bool AudioSinkDevice_Send(NativeAudioSinkDevice&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void AudioSinkDevice_Visit(NativeAudioSinkDevice&, AtomBase&, Visitor& vis);
    static bool AudioSinkDevice_Recv(NativeAudioSinkDevice&, AtomBase&, int, const Packet&);
    static void AudioSinkDevice_Finalize(NativeAudioSinkDevice&, AtomBase&, RealtimeSourceConfig&);
    static void AudioSinkDevice_Update(NativeAudioSinkDevice&, AtomBase&, double dt);
    static bool AudioSinkDevice_IsReady(NativeAudioSinkDevice&, AtomBase&, PacketIO& io);
    static bool AudioSinkDevice_AttachContext(NativeAudioSinkDevice&, AtomBase& a, AtomBase& other);
    static void AudioSinkDevice_DetachContext(NativeAudioSinkDevice&, AtomBase& a, AtomBase& other);
    #endif
    #if (defined flagHAL && defined flagVIDEO)
    struct NativeCenterVideoSinkDevice;
    static bool CenterVideoSinkDevice_Create(NativeCenterVideoSinkDevice*& dev);
    static void CenterVideoSinkDevice_Destroy(NativeCenterVideoSinkDevice*& dev);
    static bool CenterVideoSinkDevice_Initialize(NativeCenterVideoSinkDevice&, AtomBase&, const WorldState&);
    static bool CenterVideoSinkDevice_PostInitialize(NativeCenterVideoSinkDevice&, AtomBase&);
    static bool CenterVideoSinkDevice_Start(NativeCenterVideoSinkDevice&, AtomBase&);
    static void CenterVideoSinkDevice_Stop(NativeCenterVideoSinkDevice&, AtomBase&);
    static void CenterVideoSinkDevice_Uninitialize(NativeCenterVideoSinkDevice&, AtomBase&);
    static bool CenterVideoSinkDevice_Send(NativeCenterVideoSinkDevice&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void CenterVideoSinkDevice_Visit(NativeCenterVideoSinkDevice&, AtomBase&, Visitor& vis);
    static bool CenterVideoSinkDevice_Recv(NativeCenterVideoSinkDevice&, AtomBase&, int, const Packet&);
    static void CenterVideoSinkDevice_Finalize(NativeCenterVideoSinkDevice&, AtomBase&, RealtimeSourceConfig&);
    static void CenterVideoSinkDevice_Update(NativeCenterVideoSinkDevice&, AtomBase&, double dt);
    static bool CenterVideoSinkDevice_IsReady(NativeCenterVideoSinkDevice&, AtomBase&, PacketIO& io);
    static bool CenterVideoSinkDevice_AttachContext(NativeCenterVideoSinkDevice&, AtomBase& a, AtomBase& other);
    static void CenterVideoSinkDevice_DetachContext(NativeCenterVideoSinkDevice&, AtomBase& a, AtomBase& other);
    #endif
    #if (defined flagHAL && defined flagFBO)
    struct NativeCenterFboSinkDevice;
    static bool CenterFboSinkDevice_Create(NativeCenterFboSinkDevice*& dev);
    static void CenterFboSinkDevice_Destroy(NativeCenterFboSinkDevice*& dev);
    static bool CenterFboSinkDevice_Initialize(NativeCenterFboSinkDevice&, AtomBase&, const WorldState&);
    static bool CenterFboSinkDevice_PostInitialize(NativeCenterFboSinkDevice&, AtomBase&);
    static bool CenterFboSinkDevice_Start(NativeCenterFboSinkDevice&, AtomBase&);
    static void CenterFboSinkDevice_Stop(NativeCenterFboSinkDevice&, AtomBase&);
    static void CenterFboSinkDevice_Uninitialize(NativeCenterFboSinkDevice&, AtomBase&);
    static bool CenterFboSinkDevice_Send(NativeCenterFboSinkDevice&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void CenterFboSinkDevice_Visit(NativeCenterFboSinkDevice&, AtomBase&, Visitor& vis);
    static bool CenterFboSinkDevice_Recv(NativeCenterFboSinkDevice&, AtomBase&, int, const Packet&);
    static void CenterFboSinkDevice_Finalize(NativeCenterFboSinkDevice&, AtomBase&, RealtimeSourceConfig&);
    static void CenterFboSinkDevice_Update(NativeCenterFboSinkDevice&, AtomBase&, double dt);
    static bool CenterFboSinkDevice_IsReady(NativeCenterFboSinkDevice&, AtomBase&, PacketIO& io);
    static bool CenterFboSinkDevice_AttachContext(NativeCenterFboSinkDevice&, AtomBase& a, AtomBase& other);
    static void CenterFboSinkDevice_DetachContext(NativeCenterFboSinkDevice&, AtomBase& a, AtomBase& other);
    #endif
    #if (defined flagHAL && defined flagOGL)
    struct NativeOglVideoSinkDevice;
    static bool OglVideoSinkDevice_Create(NativeOglVideoSinkDevice*& dev);
    static void OglVideoSinkDevice_Destroy(NativeOglVideoSinkDevice*& dev);
    static bool OglVideoSinkDevice_Initialize(NativeOglVideoSinkDevice&, AtomBase&, const WorldState&);
    static bool OglVideoSinkDevice_PostInitialize(NativeOglVideoSinkDevice&, AtomBase&);
    static bool OglVideoSinkDevice_Start(NativeOglVideoSinkDevice&, AtomBase&);
    static void OglVideoSinkDevice_Stop(NativeOglVideoSinkDevice&, AtomBase&);
    static void OglVideoSinkDevice_Uninitialize(NativeOglVideoSinkDevice&, AtomBase&);
    static bool OglVideoSinkDevice_Send(NativeOglVideoSinkDevice&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void OglVideoSinkDevice_Visit(NativeOglVideoSinkDevice&, AtomBase&, Visitor& vis);
    static bool OglVideoSinkDevice_Recv(NativeOglVideoSinkDevice&, AtomBase&, int, const Packet&);
    static void OglVideoSinkDevice_Finalize(NativeOglVideoSinkDevice&, AtomBase&, RealtimeSourceConfig&);
    static void OglVideoSinkDevice_Update(NativeOglVideoSinkDevice&, AtomBase&, double dt);
    static bool OglVideoSinkDevice_IsReady(NativeOglVideoSinkDevice&, AtomBase&, PacketIO& io);
    static bool OglVideoSinkDevice_AttachContext(NativeOglVideoSinkDevice&, AtomBase& a, AtomBase& other);
    static void OglVideoSinkDevice_DetachContext(NativeOglVideoSinkDevice&, AtomBase& a, AtomBase& other);
    #endif
    #if (defined flagHAL && defined flagDX12)
    struct NativeD12VideoSinkDevice;
    static bool D12VideoSinkDevice_Create(NativeD12VideoSinkDevice*& dev);
    static void D12VideoSinkDevice_Destroy(NativeD12VideoSinkDevice*& dev);
    static bool D12VideoSinkDevice_Initialize(NativeD12VideoSinkDevice&, AtomBase&, const WorldState&);
    static bool D12VideoSinkDevice_PostInitialize(NativeD12VideoSinkDevice&, AtomBase&);
    static bool D12VideoSinkDevice_Start(NativeD12VideoSinkDevice&, AtomBase&);
    static void D12VideoSinkDevice_Stop(NativeD12VideoSinkDevice&, AtomBase&);
    static void D12VideoSinkDevice_Uninitialize(NativeD12VideoSinkDevice&, AtomBase&);
    static bool D12VideoSinkDevice_Send(NativeD12VideoSinkDevice&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void D12VideoSinkDevice_Visit(NativeD12VideoSinkDevice&, AtomBase&, Visitor& vis);
    static bool D12VideoSinkDevice_Recv(NativeD12VideoSinkDevice&, AtomBase&, int, const Packet&);
    static void D12VideoSinkDevice_Finalize(NativeD12VideoSinkDevice&, AtomBase&, RealtimeSourceConfig&);
    static void D12VideoSinkDevice_Update(NativeD12VideoSinkDevice&, AtomBase&, double dt);
    static bool D12VideoSinkDevice_IsReady(NativeD12VideoSinkDevice&, AtomBase&, PacketIO& io);
    static bool D12VideoSinkDevice_AttachContext(NativeD12VideoSinkDevice&, AtomBase& a, AtomBase& other);
    static void D12VideoSinkDevice_DetachContext(NativeD12VideoSinkDevice&, AtomBase& a, AtomBase& other);
    #endif
    #if defined flagHAL
    struct NativeContextBase;
    static bool ContextBase_Create(NativeContextBase*& dev);
    static void ContextBase_Destroy(NativeContextBase*& dev);
    static bool ContextBase_Initialize(NativeContextBase&, AtomBase&, const WorldState&);
    static bool ContextBase_PostInitialize(NativeContextBase&, AtomBase&);
    static bool ContextBase_Start(NativeContextBase&, AtomBase&);
    static void ContextBase_Stop(NativeContextBase&, AtomBase&);
    static void ContextBase_Uninitialize(NativeContextBase&, AtomBase&);
    static bool ContextBase_Send(NativeContextBase&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void ContextBase_Visit(NativeContextBase&, AtomBase&, Visitor& vis);
    static bool ContextBase_Recv(NativeContextBase&, AtomBase&, int, const Packet&);
    static void ContextBase_Finalize(NativeContextBase&, AtomBase&, RealtimeSourceConfig&);
    static void ContextBase_Update(NativeContextBase&, AtomBase&, double dt);
    static bool ContextBase_IsReady(NativeContextBase&, AtomBase&, PacketIO& io);
    static bool ContextBase_AttachContext(NativeContextBase&, AtomBase& a, AtomBase& other);
    static void ContextBase_DetachContext(NativeContextBase&, AtomBase& a, AtomBase& other);
    #endif
    #if defined flagHAL
    struct NativeEventsBase;
    static bool EventsBase_Create(NativeEventsBase*& dev);
    static void EventsBase_Destroy(NativeEventsBase*& dev);
    static bool EventsBase_Initialize(NativeEventsBase&, AtomBase&, const WorldState&);
    static bool EventsBase_PostInitialize(NativeEventsBase&, AtomBase&);
    static bool EventsBase_Start(NativeEventsBase&, AtomBase&);
    static void EventsBase_Stop(NativeEventsBase&, AtomBase&);
    static void EventsBase_Uninitialize(NativeEventsBase&, AtomBase&);
    static bool EventsBase_Send(NativeEventsBase&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void EventsBase_Visit(NativeEventsBase&, AtomBase&, Visitor& vis);
    static bool EventsBase_Recv(NativeEventsBase&, AtomBase&, int, const Packet&);
    static void EventsBase_Finalize(NativeEventsBase&, AtomBase&, RealtimeSourceConfig&);
    static void EventsBase_Update(NativeEventsBase&, AtomBase&, double dt);
    static bool EventsBase_IsReady(NativeEventsBase&, AtomBase&, PacketIO& io);
    static bool EventsBase_AttachContext(NativeEventsBase&, AtomBase& a, AtomBase& other);
    static void EventsBase_DetachContext(NativeEventsBase&, AtomBase& a, AtomBase& other);
    #endif
    #if defined flagHAL
    struct NativeGuiSinkBase;
    static bool GuiSinkBase_Create(NativeGuiSinkBase*& dev);
    static void GuiSinkBase_Destroy(NativeGuiSinkBase*& dev);
    static bool GuiSinkBase_Initialize(NativeGuiSinkBase&, AtomBase&, const WorldState&);
    static bool GuiSinkBase_PostInitialize(NativeGuiSinkBase&, AtomBase&);
    static bool GuiSinkBase_Start(NativeGuiSinkBase&, AtomBase&);
    static void GuiSinkBase_Stop(NativeGuiSinkBase&, AtomBase&);
    static void GuiSinkBase_Uninitialize(NativeGuiSinkBase&, AtomBase&);
    static bool GuiSinkBase_Send(NativeGuiSinkBase&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void GuiSinkBase_Visit(NativeGuiSinkBase&, AtomBase&, Visitor& vis);
    static bool GuiSinkBase_Recv(NativeGuiSinkBase&, AtomBase&, int, const Packet&);
    static void GuiSinkBase_Finalize(NativeGuiSinkBase&, AtomBase&, RealtimeSourceConfig&);
    static void GuiSinkBase_Update(NativeGuiSinkBase&, AtomBase&, double dt);
    static bool GuiSinkBase_IsReady(NativeGuiSinkBase&, AtomBase&, PacketIO& io);
    static bool GuiSinkBase_AttachContext(NativeGuiSinkBase&, AtomBase& a, AtomBase& other);
    static void GuiSinkBase_DetachContext(NativeGuiSinkBase&, AtomBase& a, AtomBase& other);
    #endif
    #if defined flagHAL
    struct NativeGuiFileSrc;
    static bool GuiFileSrc_Create(NativeGuiFileSrc*& dev);
    static void GuiFileSrc_Destroy(NativeGuiFileSrc*& dev);
    static bool GuiFileSrc_Initialize(NativeGuiFileSrc&, AtomBase&, const WorldState&);
    static bool GuiFileSrc_PostInitialize(NativeGuiFileSrc&, AtomBase&);
    static bool GuiFileSrc_Start(NativeGuiFileSrc&, AtomBase&);
    static void GuiFileSrc_Stop(NativeGuiFileSrc&, AtomBase&);
    static void GuiFileSrc_Uninitialize(NativeGuiFileSrc&, AtomBase&);
    static bool GuiFileSrc_Send(NativeGuiFileSrc&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void GuiFileSrc_Visit(NativeGuiFileSrc&, AtomBase&, Visitor& vis);
    static bool GuiFileSrc_Recv(NativeGuiFileSrc&, AtomBase&, int, const Packet&);
    static void GuiFileSrc_Finalize(NativeGuiFileSrc&, AtomBase&, RealtimeSourceConfig&);
    static void GuiFileSrc_Update(NativeGuiFileSrc&, AtomBase&, double dt);
    static bool GuiFileSrc_IsReady(NativeGuiFileSrc&, AtomBase&, PacketIO& io);
    static bool GuiFileSrc_AttachContext(NativeGuiFileSrc&, AtomBase& a, AtomBase& other);
    static void GuiFileSrc_DetachContext(NativeGuiFileSrc&, AtomBase& a, AtomBase& other);
    #endif

    struct NativeUppOglDevice;
    struct NativeUppEventsBase;

    static bool NativeUppOglDevice_Create(NativeUppOglDevice*& dev);
    static void NativeUppOglDevice_Destroy(NativeUppOglDevice*& dev);
    static bool NativeUppOglDevice_Initialize(NativeUppOglDevice& dev, AtomBase& a, const WorldState& ws);
    static bool NativeUppOglDevice_PostInitialize(NativeUppOglDevice& dev, AtomBase& a);
    static bool NativeUppOglDevice_Start(NativeUppOglDevice&, AtomBase&);
    static void NativeUppOglDevice_Stop(NativeUppOglDevice&, AtomBase& a);
    static void NativeUppOglDevice_Uninitialize(NativeUppOglDevice& dev, AtomBase& a);
    static bool NativeUppOglDevice_Send(NativeUppOglDevice& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static bool NativeUppOglDevice_Recv(NativeUppOglDevice&, AtomBase&, int sink_ch, const Packet& in);
    static void NativeUppOglDevice_Finalize(NativeUppOglDevice& dev, AtomBase& a, RealtimeSourceConfig& cfg);
    static void NativeUppOglDevice_Update(NativeUppOglDevice&, AtomBase&, double dt);
    static bool NativeUppOglDevice_IsReady(NativeUppOglDevice&, AtomBase&, PacketIO& io);
    static void NativeUppOglDevice_Visit(NativeUppOglDevice&, AtomBase&, Vis& v);
    static bool NativeUppOglDevice_AttachContext(NativeUppOglDevice&, AtomBase&, AtomBase& a);
    static void NativeUppOglDevice_DetachContext(NativeUppOglDevice&, AtomBase&, AtomBase& a);

    static bool UppEventsBase_Create(NativeUppEventsBase*& dev);
    static void UppEventsBase_Destroy(NativeUppEventsBase*& dev);
    static bool UppEventsBase_Initialize(NativeUppEventsBase& dev, AtomBase& a, const WorldState&);
    static bool UppEventsBase_PostInitialize(NativeUppEventsBase& dev, AtomBase& a);
    static bool UppEventsBase_Start(NativeUppEventsBase&, AtomBase&);
    static void UppEventsBase_Stop(NativeUppEventsBase&, AtomBase& a);
    static void UppEventsBase_Uninitialize(NativeUppEventsBase& dev, AtomBase& a);
    static bool UppEventsBase_Send(NativeUppEventsBase& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static bool UppEventsBase_Recv(NativeUppEventsBase&, AtomBase&, int sink_ch, const Packet& in);
    static void UppEventsBase_Finalize(NativeUppEventsBase&, AtomBase&, RealtimeSourceConfig&);
    static void UppEventsBase_Update(NativeUppEventsBase&, AtomBase&, double dt);
    static bool UppEventsBase_IsReady(NativeUppEventsBase&, AtomBase&, PacketIO& io);
    static void UppEventsBase_Visit(NativeUppEventsBase&, AtomBase&, Vis& v);
    static bool UppEventsBase_AttachContext(NativeUppEventsBase&, AtomBase&, AtomBase& a);
    static void UppEventsBase_DetachContext(NativeUppEventsBase&, AtomBase&, AtomBase& a);

    struct Thread { };
    static Thread& Local() {thread_local static Thread t; return t;}
};

struct HalSdl {
    #if (defined flagHAL && defined flagAUDIO)
    struct NativeAudioSinkDevice;
    static bool AudioSinkDevice_Create(NativeAudioSinkDevice*& dev);
    static void AudioSinkDevice_Destroy(NativeAudioSinkDevice*& dev);
    static bool AudioSinkDevice_Initialize(NativeAudioSinkDevice&, AtomBase&, const WorldState&);
    static bool AudioSinkDevice_PostInitialize(NativeAudioSinkDevice&, AtomBase&);
    static bool AudioSinkDevice_Start(NativeAudioSinkDevice&, AtomBase&);
    static void AudioSinkDevice_Stop(NativeAudioSinkDevice&, AtomBase&);
    static void AudioSinkDevice_Uninitialize(NativeAudioSinkDevice&, AtomBase&);
    static bool AudioSinkDevice_Send(NativeAudioSinkDevice&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void AudioSinkDevice_Visit(NativeAudioSinkDevice&, AtomBase&, Visitor& vis);
    static bool AudioSinkDevice_Recv(NativeAudioSinkDevice&, AtomBase&, int, const Packet&);
    static void AudioSinkDevice_Finalize(NativeAudioSinkDevice&, AtomBase&, RealtimeSourceConfig&);
    static void AudioSinkDevice_Update(NativeAudioSinkDevice&, AtomBase&, double dt);
    static bool AudioSinkDevice_IsReady(NativeAudioSinkDevice&, AtomBase&, PacketIO& io);
    static bool AudioSinkDevice_AttachContext(NativeAudioSinkDevice&, AtomBase& a, AtomBase& other);
    static void AudioSinkDevice_DetachContext(NativeAudioSinkDevice&, AtomBase& a, AtomBase& other);
    #endif
    #if (defined flagHAL && defined flagVIDEO)
    struct NativeCenterVideoSinkDevice;
    static bool CenterVideoSinkDevice_Create(NativeCenterVideoSinkDevice*& dev);
    static void CenterVideoSinkDevice_Destroy(NativeCenterVideoSinkDevice*& dev);
    static bool CenterVideoSinkDevice_Initialize(NativeCenterVideoSinkDevice&, AtomBase&, const WorldState&);
    static bool CenterVideoSinkDevice_PostInitialize(NativeCenterVideoSinkDevice&, AtomBase&);
    static bool CenterVideoSinkDevice_Start(NativeCenterVideoSinkDevice&, AtomBase&);
    static void CenterVideoSinkDevice_Stop(NativeCenterVideoSinkDevice&, AtomBase&);
    static void CenterVideoSinkDevice_Uninitialize(NativeCenterVideoSinkDevice&, AtomBase&);
    static bool CenterVideoSinkDevice_Send(NativeCenterVideoSinkDevice&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void CenterVideoSinkDevice_Visit(NativeCenterVideoSinkDevice&, AtomBase&, Visitor& vis);
    static bool CenterVideoSinkDevice_Recv(NativeCenterVideoSinkDevice&, AtomBase&, int, const Packet&);
    static void CenterVideoSinkDevice_Finalize(NativeCenterVideoSinkDevice&, AtomBase&, RealtimeSourceConfig&);
    static void CenterVideoSinkDevice_Update(NativeCenterVideoSinkDevice&, AtomBase&, double dt);
    static bool CenterVideoSinkDevice_IsReady(NativeCenterVideoSinkDevice&, AtomBase&, PacketIO& io);
    static bool CenterVideoSinkDevice_AttachContext(NativeCenterVideoSinkDevice&, AtomBase& a, AtomBase& other);
    static void CenterVideoSinkDevice_DetachContext(NativeCenterVideoSinkDevice&, AtomBase& a, AtomBase& other);
    #endif
    #if (defined flagHAL && defined flagFBO)
    struct NativeCenterFboSinkDevice;
    static bool CenterFboSinkDevice_Create(NativeCenterFboSinkDevice*& dev);
    static void CenterFboSinkDevice_Destroy(NativeCenterFboSinkDevice*& dev);
    static bool CenterFboSinkDevice_Initialize(NativeCenterFboSinkDevice&, AtomBase&, const WorldState&);
    static bool CenterFboSinkDevice_PostInitialize(NativeCenterFboSinkDevice&, AtomBase&);
    static bool CenterFboSinkDevice_Start(NativeCenterFboSinkDevice&, AtomBase&);
    static void CenterFboSinkDevice_Stop(NativeCenterFboSinkDevice&, AtomBase&);
    static void CenterFboSinkDevice_Uninitialize(NativeCenterFboSinkDevice&, AtomBase&);
    static bool CenterFboSinkDevice_Send(NativeCenterFboSinkDevice&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void CenterFboSinkDevice_Visit(NativeCenterFboSinkDevice&, AtomBase&, Visitor& vis);
    static bool CenterFboSinkDevice_Recv(NativeCenterFboSinkDevice&, AtomBase&, int, const Packet&);
    static void CenterFboSinkDevice_Finalize(NativeCenterFboSinkDevice&, AtomBase&, RealtimeSourceConfig&);
    static void CenterFboSinkDevice_Update(NativeCenterFboSinkDevice&, AtomBase&, double dt);
    static bool CenterFboSinkDevice_IsReady(NativeCenterFboSinkDevice&, AtomBase&, PacketIO& io);
    static bool CenterFboSinkDevice_AttachContext(NativeCenterFboSinkDevice&, AtomBase& a, AtomBase& other);
    static void CenterFboSinkDevice_DetachContext(NativeCenterFboSinkDevice&, AtomBase& a, AtomBase& other);
    #endif
    #if (defined flagHAL && defined flagOGL)
    struct NativeOglVideoSinkDevice;
    static bool OglVideoSinkDevice_Create(NativeOglVideoSinkDevice*& dev);
    static void OglVideoSinkDevice_Destroy(NativeOglVideoSinkDevice*& dev);
    static bool OglVideoSinkDevice_Initialize(NativeOglVideoSinkDevice&, AtomBase&, const WorldState&);
    static bool OglVideoSinkDevice_PostInitialize(NativeOglVideoSinkDevice&, AtomBase&);
    static bool OglVideoSinkDevice_Start(NativeOglVideoSinkDevice&, AtomBase&);
    static void OglVideoSinkDevice_Stop(NativeOglVideoSinkDevice&, AtomBase&);
    static void OglVideoSinkDevice_Uninitialize(NativeOglVideoSinkDevice&, AtomBase&);
    static bool OglVideoSinkDevice_Send(NativeOglVideoSinkDevice&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void OglVideoSinkDevice_Visit(NativeOglVideoSinkDevice&, AtomBase&, Visitor& vis);
    static bool OglVideoSinkDevice_Recv(NativeOglVideoSinkDevice&, AtomBase&, int, const Packet&);
    static void OglVideoSinkDevice_Finalize(NativeOglVideoSinkDevice&, AtomBase&, RealtimeSourceConfig&);
    static void OglVideoSinkDevice_Update(NativeOglVideoSinkDevice&, AtomBase&, double dt);
    static bool OglVideoSinkDevice_IsReady(NativeOglVideoSinkDevice&, AtomBase&, PacketIO& io);
    static bool OglVideoSinkDevice_AttachContext(NativeOglVideoSinkDevice&, AtomBase& a, AtomBase& other);
    static void OglVideoSinkDevice_DetachContext(NativeOglVideoSinkDevice&, AtomBase& a, AtomBase& other);
    #endif
    #if (defined flagHAL && defined flagDX12)
    struct NativeD12VideoSinkDevice;
    static bool D12VideoSinkDevice_Create(NativeD12VideoSinkDevice*& dev);
    static void D12VideoSinkDevice_Destroy(NativeD12VideoSinkDevice*& dev);
    static bool D12VideoSinkDevice_Initialize(NativeD12VideoSinkDevice&, AtomBase&, const WorldState&);
    static bool D12VideoSinkDevice_PostInitialize(NativeD12VideoSinkDevice&, AtomBase&);
    static bool D12VideoSinkDevice_Start(NativeD12VideoSinkDevice&, AtomBase&);
    static void D12VideoSinkDevice_Stop(NativeD12VideoSinkDevice&, AtomBase&);
    static void D12VideoSinkDevice_Uninitialize(NativeD12VideoSinkDevice&, AtomBase&);
    static bool D12VideoSinkDevice_Send(NativeD12VideoSinkDevice&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void D12VideoSinkDevice_Visit(NativeD12VideoSinkDevice&, AtomBase&, Visitor& vis);
    static bool D12VideoSinkDevice_Recv(NativeD12VideoSinkDevice&, AtomBase&, int, const Packet&);
    static void D12VideoSinkDevice_Finalize(NativeD12VideoSinkDevice&, AtomBase&, RealtimeSourceConfig&);
    static void D12VideoSinkDevice_Update(NativeD12VideoSinkDevice&, AtomBase&, double dt);
    static bool D12VideoSinkDevice_IsReady(NativeD12VideoSinkDevice&, AtomBase&, PacketIO& io);
    static bool D12VideoSinkDevice_AttachContext(NativeD12VideoSinkDevice&, AtomBase& a, AtomBase& other);
    static void D12VideoSinkDevice_DetachContext(NativeD12VideoSinkDevice&, AtomBase& a, AtomBase& other);
    #endif
    #if defined flagHAL
    struct NativeContextBase;
    static bool ContextBase_Create(NativeContextBase*& dev);
    static void ContextBase_Destroy(NativeContextBase*& dev);
    static bool ContextBase_Initialize(NativeContextBase&, AtomBase&, const WorldState&);
    static bool ContextBase_PostInitialize(NativeContextBase&, AtomBase&);
    static bool ContextBase_Start(NativeContextBase&, AtomBase&);
    static void ContextBase_Stop(NativeContextBase&, AtomBase&);
    static void ContextBase_Uninitialize(NativeContextBase&, AtomBase&);
    static bool ContextBase_Send(NativeContextBase&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void ContextBase_Visit(NativeContextBase&, AtomBase&, Visitor& vis);
    static bool ContextBase_Recv(NativeContextBase&, AtomBase&, int, const Packet&);
    static void ContextBase_Finalize(NativeContextBase&, AtomBase&, RealtimeSourceConfig&);
    static void ContextBase_Update(NativeContextBase&, AtomBase&, double dt);
    static bool ContextBase_IsReady(NativeContextBase&, AtomBase&, PacketIO& io);
    static bool ContextBase_AttachContext(NativeContextBase&, AtomBase& a, AtomBase& other);
    static void ContextBase_DetachContext(NativeContextBase&, AtomBase& a, AtomBase& other);
    #endif
    #if defined flagHAL
    struct NativeEventsBase;
    static bool EventsBase_Create(NativeEventsBase*& dev);
    static void EventsBase_Destroy(NativeEventsBase*& dev);
    static bool EventsBase_Initialize(NativeEventsBase&, AtomBase&, const WorldState&);
    static bool EventsBase_PostInitialize(NativeEventsBase&, AtomBase&);
    static bool EventsBase_Start(NativeEventsBase&, AtomBase&);
    static void EventsBase_Stop(NativeEventsBase&, AtomBase&);
    static void EventsBase_Uninitialize(NativeEventsBase&, AtomBase&);
    static bool EventsBase_Send(NativeEventsBase&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void EventsBase_Visit(NativeEventsBase&, AtomBase&, Visitor& vis);
    static bool EventsBase_Recv(NativeEventsBase&, AtomBase&, int, const Packet&);
    static void EventsBase_Finalize(NativeEventsBase&, AtomBase&, RealtimeSourceConfig&);
    static void EventsBase_Update(NativeEventsBase&, AtomBase&, double dt);
    static bool EventsBase_IsReady(NativeEventsBase&, AtomBase&, PacketIO& io);
    static bool EventsBase_AttachContext(NativeEventsBase&, AtomBase& a, AtomBase& other);
    static void EventsBase_DetachContext(NativeEventsBase&, AtomBase& a, AtomBase& other);
    #endif
    #if defined flagHAL
    struct NativeGuiSinkBase;
    static bool GuiSinkBase_Create(NativeGuiSinkBase*& dev);
    static void GuiSinkBase_Destroy(NativeGuiSinkBase*& dev);
    static bool GuiSinkBase_Initialize(NativeGuiSinkBase&, AtomBase&, const WorldState&);
    static bool GuiSinkBase_PostInitialize(NativeGuiSinkBase&, AtomBase&);
    static bool GuiSinkBase_Start(NativeGuiSinkBase&, AtomBase&);
    static void GuiSinkBase_Stop(NativeGuiSinkBase&, AtomBase&);
    static void GuiSinkBase_Uninitialize(NativeGuiSinkBase&, AtomBase&);
    static bool GuiSinkBase_Send(NativeGuiSinkBase&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void GuiSinkBase_Visit(NativeGuiSinkBase&, AtomBase&, Visitor& vis);
    static bool GuiSinkBase_Recv(NativeGuiSinkBase&, AtomBase&, int, const Packet&);
    static void GuiSinkBase_Finalize(NativeGuiSinkBase&, AtomBase&, RealtimeSourceConfig&);
    static void GuiSinkBase_Update(NativeGuiSinkBase&, AtomBase&, double dt);
    static bool GuiSinkBase_IsReady(NativeGuiSinkBase&, AtomBase&, PacketIO& io);
    static bool GuiSinkBase_AttachContext(NativeGuiSinkBase&, AtomBase& a, AtomBase& other);
    static void GuiSinkBase_DetachContext(NativeGuiSinkBase&, AtomBase& a, AtomBase& other);
    #endif
    #if defined flagHAL
    struct NativeGuiFileSrc;
    static bool GuiFileSrc_Create(NativeGuiFileSrc*& dev);
    static void GuiFileSrc_Destroy(NativeGuiFileSrc*& dev);
    static bool GuiFileSrc_Initialize(NativeGuiFileSrc&, AtomBase&, const WorldState&);
    static bool GuiFileSrc_PostInitialize(NativeGuiFileSrc&, AtomBase&);
    static bool GuiFileSrc_Start(NativeGuiFileSrc&, AtomBase&);
    static void GuiFileSrc_Stop(NativeGuiFileSrc&, AtomBase&);
    static void GuiFileSrc_Uninitialize(NativeGuiFileSrc&, AtomBase&);
    static bool GuiFileSrc_Send(NativeGuiFileSrc&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void GuiFileSrc_Visit(NativeGuiFileSrc&, AtomBase&, Visitor& vis);
    static bool GuiFileSrc_Recv(NativeGuiFileSrc&, AtomBase&, int, const Packet&);
    static void GuiFileSrc_Finalize(NativeGuiFileSrc&, AtomBase&, RealtimeSourceConfig&);
    static void GuiFileSrc_Update(NativeGuiFileSrc&, AtomBase&, double dt);
    static bool GuiFileSrc_IsReady(NativeGuiFileSrc&, AtomBase&, PacketIO& io);
    static bool GuiFileSrc_AttachContext(NativeGuiFileSrc&, AtomBase& a, AtomBase& other);
    static void GuiFileSrc_DetachContext(NativeGuiFileSrc&, AtomBase& a, AtomBase& other);
    #endif

    struct NativeUppOglDevice;
    struct NativeUppEventsBase;

    static bool NativeUppOglDevice_Create(NativeUppOglDevice*& dev);
    static void NativeUppOglDevice_Destroy(NativeUppOglDevice*& dev);
    static bool NativeUppOglDevice_Initialize(NativeUppOglDevice& dev, AtomBase& a, const WorldState& ws);
    static bool NativeUppOglDevice_PostInitialize(NativeUppOglDevice& dev, AtomBase& a);
    static bool NativeUppOglDevice_Start(NativeUppOglDevice&, AtomBase&);
    static void NativeUppOglDevice_Stop(NativeUppOglDevice&, AtomBase& a);
    static void NativeUppOglDevice_Uninitialize(NativeUppOglDevice& dev, AtomBase& a);
    static bool NativeUppOglDevice_Send(NativeUppOglDevice& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static bool NativeUppOglDevice_Recv(NativeUppOglDevice&, AtomBase&, int sink_ch, const Packet& in);
    static void NativeUppOglDevice_Finalize(NativeUppOglDevice& dev, AtomBase& a, RealtimeSourceConfig& cfg);
    static void NativeUppOglDevice_Update(NativeUppOglDevice&, AtomBase&, double dt);
    static bool NativeUppOglDevice_IsReady(NativeUppOglDevice&, AtomBase&, PacketIO& io);
    static void NativeUppOglDevice_Visit(NativeUppOglDevice&, AtomBase&, Vis& v);
    static bool NativeUppOglDevice_AttachContext(NativeUppOglDevice&, AtomBase&, AtomBase& a);
    static void NativeUppOglDevice_DetachContext(NativeUppOglDevice&, AtomBase&, AtomBase& a);

    static bool UppEventsBase_Create(NativeUppEventsBase*& dev);
    static void UppEventsBase_Destroy(NativeUppEventsBase*& dev);
    static bool UppEventsBase_Initialize(NativeUppEventsBase& dev, AtomBase& a, const WorldState&);
    static bool UppEventsBase_PostInitialize(NativeUppEventsBase& dev, AtomBase& a);
    static bool UppEventsBase_Start(NativeUppEventsBase&, AtomBase&);
    static void UppEventsBase_Stop(NativeUppEventsBase&, AtomBase& a);
    static void UppEventsBase_Uninitialize(NativeUppEventsBase& dev, AtomBase& a);
    static bool UppEventsBase_Send(NativeUppEventsBase& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static bool UppEventsBase_Recv(NativeUppEventsBase&, AtomBase&, int sink_ch, const Packet& in);
    static void UppEventsBase_Finalize(NativeUppEventsBase&, AtomBase&, RealtimeSourceConfig&);
    static void UppEventsBase_Update(NativeUppEventsBase&, AtomBase&, double dt);
    static bool UppEventsBase_IsReady(NativeUppEventsBase&, AtomBase&, PacketIO& io);
    static void UppEventsBase_Visit(NativeUppEventsBase&, AtomBase&, Vis& v);
    static bool UppEventsBase_AttachContext(NativeUppEventsBase&, AtomBase&, AtomBase& a);
    static void UppEventsBase_DetachContext(NativeUppEventsBase&, AtomBase&, AtomBase& a);

    struct Thread { };
    static Thread& Local() {thread_local static Thread t; return t;}
};

struct HalHolo {
    #if (defined flagHAL && defined flagAUDIO)
    struct NativeAudioSinkDevice;
    static bool AudioSinkDevice_Create(NativeAudioSinkDevice*& dev);
    static void AudioSinkDevice_Destroy(NativeAudioSinkDevice*& dev);
    static bool AudioSinkDevice_Initialize(NativeAudioSinkDevice&, AtomBase&, const WorldState&);
    static bool AudioSinkDevice_PostInitialize(NativeAudioSinkDevice&, AtomBase&);
    static bool AudioSinkDevice_Start(NativeAudioSinkDevice&, AtomBase&);
    static void AudioSinkDevice_Stop(NativeAudioSinkDevice&, AtomBase&);
    static void AudioSinkDevice_Uninitialize(NativeAudioSinkDevice&, AtomBase&);
    static bool AudioSinkDevice_Send(NativeAudioSinkDevice&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void AudioSinkDevice_Visit(NativeAudioSinkDevice&, AtomBase&, Visitor& vis);
    static bool AudioSinkDevice_Recv(NativeAudioSinkDevice&, AtomBase&, int, const Packet&);
    static void AudioSinkDevice_Finalize(NativeAudioSinkDevice&, AtomBase&, RealtimeSourceConfig&);
    static void AudioSinkDevice_Update(NativeAudioSinkDevice&, AtomBase&, double dt);
    static bool AudioSinkDevice_IsReady(NativeAudioSinkDevice&, AtomBase&, PacketIO& io);
    static bool AudioSinkDevice_AttachContext(NativeAudioSinkDevice&, AtomBase& a, AtomBase& other);
    static void AudioSinkDevice_DetachContext(NativeAudioSinkDevice&, AtomBase& a, AtomBase& other);
    #endif
    #if (defined flagHAL && defined flagVIDEO)
    struct NativeCenterVideoSinkDevice;
    static bool CenterVideoSinkDevice_Create(NativeCenterVideoSinkDevice*& dev);
    static void CenterVideoSinkDevice_Destroy(NativeCenterVideoSinkDevice*& dev);
    static bool CenterVideoSinkDevice_Initialize(NativeCenterVideoSinkDevice&, AtomBase&, const WorldState&);
    static bool CenterVideoSinkDevice_PostInitialize(NativeCenterVideoSinkDevice&, AtomBase&);
    static bool CenterVideoSinkDevice_Start(NativeCenterVideoSinkDevice&, AtomBase&);
    static void CenterVideoSinkDevice_Stop(NativeCenterVideoSinkDevice&, AtomBase&);
    static void CenterVideoSinkDevice_Uninitialize(NativeCenterVideoSinkDevice&, AtomBase&);
    static bool CenterVideoSinkDevice_Send(NativeCenterVideoSinkDevice&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void CenterVideoSinkDevice_Visit(NativeCenterVideoSinkDevice&, AtomBase&, Visitor& vis);
    static bool CenterVideoSinkDevice_Recv(NativeCenterVideoSinkDevice&, AtomBase&, int, const Packet&);
    static void CenterVideoSinkDevice_Finalize(NativeCenterVideoSinkDevice&, AtomBase&, RealtimeSourceConfig&);
    static void CenterVideoSinkDevice_Update(NativeCenterVideoSinkDevice&, AtomBase&, double dt);
    static bool CenterVideoSinkDevice_IsReady(NativeCenterVideoSinkDevice&, AtomBase&, PacketIO& io);
    static bool CenterVideoSinkDevice_AttachContext(NativeCenterVideoSinkDevice&, AtomBase& a, AtomBase& other);
    static void CenterVideoSinkDevice_DetachContext(NativeCenterVideoSinkDevice&, AtomBase& a, AtomBase& other);
    #endif
    #if (defined flagHAL && defined flagFBO)
    struct NativeCenterFboSinkDevice;
    static bool CenterFboSinkDevice_Create(NativeCenterFboSinkDevice*& dev);
    static void CenterFboSinkDevice_Destroy(NativeCenterFboSinkDevice*& dev);
    static bool CenterFboSinkDevice_Initialize(NativeCenterFboSinkDevice&, AtomBase&, const WorldState&);
    static bool CenterFboSinkDevice_PostInitialize(NativeCenterFboSinkDevice&, AtomBase&);
    static bool CenterFboSinkDevice_Start(NativeCenterFboSinkDevice&, AtomBase&);
    static void CenterFboSinkDevice_Stop(NativeCenterFboSinkDevice&, AtomBase&);
    static void CenterFboSinkDevice_Uninitialize(NativeCenterFboSinkDevice&, AtomBase&);
    static bool CenterFboSinkDevice_Send(NativeCenterFboSinkDevice&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void CenterFboSinkDevice_Visit(NativeCenterFboSinkDevice&, AtomBase&, Visitor& vis);
    static bool CenterFboSinkDevice_Recv(NativeCenterFboSinkDevice&, AtomBase&, int, const Packet&);
    static void CenterFboSinkDevice_Finalize(NativeCenterFboSinkDevice&, AtomBase&, RealtimeSourceConfig&);
    static void CenterFboSinkDevice_Update(NativeCenterFboSinkDevice&, AtomBase&, double dt);
    static bool CenterFboSinkDevice_IsReady(NativeCenterFboSinkDevice&, AtomBase&, PacketIO& io);
    static bool CenterFboSinkDevice_AttachContext(NativeCenterFboSinkDevice&, AtomBase& a, AtomBase& other);
    static void CenterFboSinkDevice_DetachContext(NativeCenterFboSinkDevice&, AtomBase& a, AtomBase& other);
    #endif
    #if (defined flagHAL && defined flagOGL)
    struct NativeOglVideoSinkDevice;
    static bool OglVideoSinkDevice_Create(NativeOglVideoSinkDevice*& dev);
    static void OglVideoSinkDevice_Destroy(NativeOglVideoSinkDevice*& dev);
    static bool OglVideoSinkDevice_Initialize(NativeOglVideoSinkDevice&, AtomBase&, const WorldState&);
    static bool OglVideoSinkDevice_PostInitialize(NativeOglVideoSinkDevice&, AtomBase&);
    static bool OglVideoSinkDevice_Start(NativeOglVideoSinkDevice&, AtomBase&);
    static void OglVideoSinkDevice_Stop(NativeOglVideoSinkDevice&, AtomBase&);
    static void OglVideoSinkDevice_Uninitialize(NativeOglVideoSinkDevice&, AtomBase&);
    static bool OglVideoSinkDevice_Send(NativeOglVideoSinkDevice&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void OglVideoSinkDevice_Visit(NativeOglVideoSinkDevice&, AtomBase&, Visitor& vis);
    static bool OglVideoSinkDevice_Recv(NativeOglVideoSinkDevice&, AtomBase&, int, const Packet&);
    static void OglVideoSinkDevice_Finalize(NativeOglVideoSinkDevice&, AtomBase&, RealtimeSourceConfig&);
    static void OglVideoSinkDevice_Update(NativeOglVideoSinkDevice&, AtomBase&, double dt);
    static bool OglVideoSinkDevice_IsReady(NativeOglVideoSinkDevice&, AtomBase&, PacketIO& io);
    static bool OglVideoSinkDevice_AttachContext(NativeOglVideoSinkDevice&, AtomBase& a, AtomBase& other);
    static void OglVideoSinkDevice_DetachContext(NativeOglVideoSinkDevice&, AtomBase& a, AtomBase& other);
    #endif
    #if (defined flagHAL && defined flagDX12)
    struct NativeD12VideoSinkDevice;
    static bool D12VideoSinkDevice_Create(NativeD12VideoSinkDevice*& dev);
    static void D12VideoSinkDevice_Destroy(NativeD12VideoSinkDevice*& dev);
    static bool D12VideoSinkDevice_Initialize(NativeD12VideoSinkDevice&, AtomBase&, const WorldState&);
    static bool D12VideoSinkDevice_PostInitialize(NativeD12VideoSinkDevice&, AtomBase&);
    static bool D12VideoSinkDevice_Start(NativeD12VideoSinkDevice&, AtomBase&);
    static void D12VideoSinkDevice_Stop(NativeD12VideoSinkDevice&, AtomBase&);
    static void D12VideoSinkDevice_Uninitialize(NativeD12VideoSinkDevice&, AtomBase&);
    static bool D12VideoSinkDevice_Send(NativeD12VideoSinkDevice&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void D12VideoSinkDevice_Visit(NativeD12VideoSinkDevice&, AtomBase&, Visitor& vis);
    static bool D12VideoSinkDevice_Recv(NativeD12VideoSinkDevice&, AtomBase&, int, const Packet&);
    static void D12VideoSinkDevice_Finalize(NativeD12VideoSinkDevice&, AtomBase&, RealtimeSourceConfig&);
    static void D12VideoSinkDevice_Update(NativeD12VideoSinkDevice&, AtomBase&, double dt);
    static bool D12VideoSinkDevice_IsReady(NativeD12VideoSinkDevice&, AtomBase&, PacketIO& io);
    static bool D12VideoSinkDevice_AttachContext(NativeD12VideoSinkDevice&, AtomBase& a, AtomBase& other);
    static void D12VideoSinkDevice_DetachContext(NativeD12VideoSinkDevice&, AtomBase& a, AtomBase& other);
    #endif
    #if defined flagHAL
    struct NativeContextBase;
    static bool ContextBase_Create(NativeContextBase*& dev);
    static void ContextBase_Destroy(NativeContextBase*& dev);
    static bool ContextBase_Initialize(NativeContextBase&, AtomBase&, const WorldState&);
    static bool ContextBase_PostInitialize(NativeContextBase&, AtomBase&);
    static bool ContextBase_Start(NativeContextBase&, AtomBase&);
    static void ContextBase_Stop(NativeContextBase&, AtomBase&);
    static void ContextBase_Uninitialize(NativeContextBase&, AtomBase&);
    static bool ContextBase_Send(NativeContextBase&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void ContextBase_Visit(NativeContextBase&, AtomBase&, Visitor& vis);
    static bool ContextBase_Recv(NativeContextBase&, AtomBase&, int, const Packet&);
    static void ContextBase_Finalize(NativeContextBase&, AtomBase&, RealtimeSourceConfig&);
    static void ContextBase_Update(NativeContextBase&, AtomBase&, double dt);
    static bool ContextBase_IsReady(NativeContextBase&, AtomBase&, PacketIO& io);
    static bool ContextBase_AttachContext(NativeContextBase&, AtomBase& a, AtomBase& other);
    static void ContextBase_DetachContext(NativeContextBase&, AtomBase& a, AtomBase& other);
    #endif
    #if defined flagHAL
    struct NativeEventsBase;
    static bool EventsBase_Create(NativeEventsBase*& dev);
    static void EventsBase_Destroy(NativeEventsBase*& dev);
    static bool EventsBase_Initialize(NativeEventsBase&, AtomBase&, const WorldState&);
    static bool EventsBase_PostInitialize(NativeEventsBase&, AtomBase&);
    static bool EventsBase_Start(NativeEventsBase&, AtomBase&);
    static void EventsBase_Stop(NativeEventsBase&, AtomBase&);
    static void EventsBase_Uninitialize(NativeEventsBase&, AtomBase&);
    static bool EventsBase_Send(NativeEventsBase&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void EventsBase_Visit(NativeEventsBase&, AtomBase&, Visitor& vis);
    static bool EventsBase_Recv(NativeEventsBase&, AtomBase&, int, const Packet&);
    static void EventsBase_Finalize(NativeEventsBase&, AtomBase&, RealtimeSourceConfig&);
    static void EventsBase_Update(NativeEventsBase&, AtomBase&, double dt);
    static bool EventsBase_IsReady(NativeEventsBase&, AtomBase&, PacketIO& io);
    static bool EventsBase_AttachContext(NativeEventsBase&, AtomBase& a, AtomBase& other);
    static void EventsBase_DetachContext(NativeEventsBase&, AtomBase& a, AtomBase& other);
    #endif
    #if defined flagHAL
    struct NativeGuiSinkBase;
    static bool GuiSinkBase_Create(NativeGuiSinkBase*& dev);
    static void GuiSinkBase_Destroy(NativeGuiSinkBase*& dev);
    static bool GuiSinkBase_Initialize(NativeGuiSinkBase&, AtomBase&, const WorldState&);
    static bool GuiSinkBase_PostInitialize(NativeGuiSinkBase&, AtomBase&);
    static bool GuiSinkBase_Start(NativeGuiSinkBase&, AtomBase&);
    static void GuiSinkBase_Stop(NativeGuiSinkBase&, AtomBase&);
    static void GuiSinkBase_Uninitialize(NativeGuiSinkBase&, AtomBase&);
    static bool GuiSinkBase_Send(NativeGuiSinkBase&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void GuiSinkBase_Visit(NativeGuiSinkBase&, AtomBase&, Visitor& vis);
    static bool GuiSinkBase_Recv(NativeGuiSinkBase&, AtomBase&, int, const Packet&);
    static void GuiSinkBase_Finalize(NativeGuiSinkBase&, AtomBase&, RealtimeSourceConfig&);
    static void GuiSinkBase_Update(NativeGuiSinkBase&, AtomBase&, double dt);
    static bool GuiSinkBase_IsReady(NativeGuiSinkBase&, AtomBase&, PacketIO& io);
    static bool GuiSinkBase_AttachContext(NativeGuiSinkBase&, AtomBase& a, AtomBase& other);
    static void GuiSinkBase_DetachContext(NativeGuiSinkBase&, AtomBase& a, AtomBase& other);
    #endif
    #if defined flagHAL
    struct NativeGuiFileSrc;
    static bool GuiFileSrc_Create(NativeGuiFileSrc*& dev);
    static void GuiFileSrc_Destroy(NativeGuiFileSrc*& dev);
    static bool GuiFileSrc_Initialize(NativeGuiFileSrc&, AtomBase&, const WorldState&);
    static bool GuiFileSrc_PostInitialize(NativeGuiFileSrc&, AtomBase&);
    static bool GuiFileSrc_Start(NativeGuiFileSrc&, AtomBase&);
    static void GuiFileSrc_Stop(NativeGuiFileSrc&, AtomBase&);
    static void GuiFileSrc_Uninitialize(NativeGuiFileSrc&, AtomBase&);
    static bool GuiFileSrc_Send(NativeGuiFileSrc&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static void GuiFileSrc_Visit(NativeGuiFileSrc&, AtomBase&, Visitor& vis);
    static bool GuiFileSrc_Recv(NativeGuiFileSrc&, AtomBase&, int, const Packet&);
    static void GuiFileSrc_Finalize(NativeGuiFileSrc&, AtomBase&, RealtimeSourceConfig&);
    static void GuiFileSrc_Update(NativeGuiFileSrc&, AtomBase&, double dt);
    static bool GuiFileSrc_IsReady(NativeGuiFileSrc&, AtomBase&, PacketIO& io);
    static bool GuiFileSrc_AttachContext(NativeGuiFileSrc&, AtomBase& a, AtomBase& other);
    static void GuiFileSrc_DetachContext(NativeGuiFileSrc&, AtomBase& a, AtomBase& other);
    #endif

    struct NativeUppOglDevice;
    struct NativeUppEventsBase;

    static bool NativeUppOglDevice_Create(NativeUppOglDevice*& dev);
    static void NativeUppOglDevice_Destroy(NativeUppOglDevice*& dev);
    static bool NativeUppOglDevice_Initialize(NativeUppOglDevice& dev, AtomBase& a, const WorldState& ws);
    static bool NativeUppOglDevice_PostInitialize(NativeUppOglDevice& dev, AtomBase& a);
    static bool NativeUppOglDevice_Start(NativeUppOglDevice&, AtomBase&);
    static void NativeUppOglDevice_Stop(NativeUppOglDevice&, AtomBase& a);
    static void NativeUppOglDevice_Uninitialize(NativeUppOglDevice& dev, AtomBase& a);
    static bool NativeUppOglDevice_Send(NativeUppOglDevice& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static bool NativeUppOglDevice_Recv(NativeUppOglDevice&, AtomBase&, int sink_ch, const Packet& in);
    static void NativeUppOglDevice_Finalize(NativeUppOglDevice& dev, AtomBase& a, RealtimeSourceConfig& cfg);
    static void NativeUppOglDevice_Update(NativeUppOglDevice&, AtomBase&, double dt);
    static bool NativeUppOglDevice_IsReady(NativeUppOglDevice&, AtomBase&, PacketIO& io);
    static void NativeUppOglDevice_Visit(NativeUppOglDevice&, AtomBase&, Vis& v);
    static bool NativeUppOglDevice_AttachContext(NativeUppOglDevice&, AtomBase&, AtomBase& a);
    static void NativeUppOglDevice_DetachContext(NativeUppOglDevice&, AtomBase&, AtomBase& a);

    static bool UppEventsBase_Create(NativeUppEventsBase*& dev);
    static void UppEventsBase_Destroy(NativeUppEventsBase*& dev);
    static bool UppEventsBase_Initialize(NativeUppEventsBase& dev, AtomBase& a, const WorldState&);
    static bool UppEventsBase_PostInitialize(NativeUppEventsBase& dev, AtomBase& a);
    static bool UppEventsBase_Start(NativeUppEventsBase&, AtomBase&);
    static void UppEventsBase_Stop(NativeUppEventsBase&, AtomBase& a);
    static void UppEventsBase_Uninitialize(NativeUppEventsBase& dev, AtomBase& a);
    static bool UppEventsBase_Send(NativeUppEventsBase& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
    static bool UppEventsBase_Recv(NativeUppEventsBase&, AtomBase&, int sink_ch, const Packet& in);
    static void UppEventsBase_Finalize(NativeUppEventsBase&, AtomBase&, RealtimeSourceConfig&);
    static void UppEventsBase_Update(NativeUppEventsBase&, AtomBase&, double dt);
    static bool UppEventsBase_IsReady(NativeUppEventsBase&, AtomBase&, PacketIO& io);
    static void UppEventsBase_Visit(NativeUppEventsBase&, AtomBase&, Vis& v);
    static bool UppEventsBase_AttachContext(NativeUppEventsBase&, AtomBase&, AtomBase& a);
    static void UppEventsBase_DetachContext(NativeUppEventsBase&, AtomBase&, AtomBase& a);

    struct Thread { };
    static Thread& Local() {thread_local static Thread t; return t;}
};

#if defined flagGUI
using UppAudioSinkDevice = HalAudioSinkDeviceT<HalUpp>;
using UppCenterVideoSinkDevice = HalCenterVideoSinkDeviceT<HalUpp>;
using UppCenterFboSinkDevice = HalCenterFboSinkDeviceT<HalUpp>;
using UppOglVideoSinkDevice = HalOglVideoSinkDeviceT<HalUpp>;
using UppD12VideoSinkDevice = HalD12VideoSinkDeviceT<HalUpp>;
using UppContextBase = HalContextBaseT<HalUpp>;
using UppEventsBase = HalEventsBaseT<HalUpp>;
using UppGuiSinkBase = HalGuiSinkBaseT<HalUpp>;
using UppGuiFileSrc = HalGuiFileSrcT<HalUpp>;

using AudioSinkDevice = UppAudioSinkDevice;
using CenterVideoSinkDevice = UppCenterVideoSinkDevice;
using CenterFboSinkDevice = UppCenterFboSinkDevice;
using OglVideoSinkDevice = UppOglVideoSinkDevice;
using D12VideoSinkDevice = UppD12VideoSinkDevice;
using ContextBase = UppContextBase;
using EventsBase = UppEventsBase;
using GuiSinkBase = UppGuiSinkBase;
using GuiFileSrc = UppGuiFileSrc;
#endif

#ifdef flagSDL2
using SdlAudioSinkDevice = HalAudioSinkDeviceT<HalSdl>;
using SdlCenterVideoSinkDevice = HalCenterVideoSinkDeviceT<HalSdl>;
using SdlCenterFboSinkDevice = HalCenterFboSinkDeviceT<HalSdl>;
using SdlOglVideoSinkDevice = HalOglVideoSinkDeviceT<HalSdl>;
using SdlD12VideoSinkDevice = HalD12VideoSinkDeviceT<HalSdl>;
using SdlContextBase = HalContextBaseT<HalSdl>;
using SdlEventsBase = HalEventsBaseT<HalSdl>;
using SdlGuiSinkBase = HalGuiSinkBaseT<HalSdl>;
using SdlGuiFileSrc = HalGuiFileSrcT<HalSdl>;

#ifndef flagGUI
using AudioSinkDevice = SdlAudioSinkDevice;
using CenterVideoSinkDevice = SdlCenterVideoSinkDevice;
using CenterFboSinkDevice = SdlCenterFboSinkDevice;
using OglVideoSinkDevice = SdlOglVideoSinkDevice;
using D12VideoSinkDevice = SdlD12VideoSinkDevice;
using ContextBase = SdlContextBase;
using EventsBase = SdlEventsBase;
using GuiSinkBase = SdlGuiSinkBase;
using GuiFileSrc = SdlGuiFileSrc;
#endif
#endif

END_UPP_NAMESPACE

#endif
