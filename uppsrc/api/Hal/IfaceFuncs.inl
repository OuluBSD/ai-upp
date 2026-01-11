// This file have been generated automatically.
// DO NOT MODIFY THIS FILE!

#if (defined flagHAL && defined flagAUDIO)
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

