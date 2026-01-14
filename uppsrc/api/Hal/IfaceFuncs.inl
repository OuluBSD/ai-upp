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

#if (defined flagHAL && defined flagSCREEN)
static bool CenterScreenSinkDevice_Create(NativeCenterScreenSinkDevice*& dev);
static void CenterScreenSinkDevice_Destroy(NativeCenterScreenSinkDevice*& dev);
static bool CenterScreenSinkDevice_Initialize(NativeCenterScreenSinkDevice&, AtomBase&, const WorldState&);
static bool CenterScreenSinkDevice_PostInitialize(NativeCenterScreenSinkDevice&, AtomBase&);
static bool CenterScreenSinkDevice_Start(NativeCenterScreenSinkDevice&, AtomBase&);
static void CenterScreenSinkDevice_Stop(NativeCenterScreenSinkDevice&, AtomBase&);
static void CenterScreenSinkDevice_Uninitialize(NativeCenterScreenSinkDevice&, AtomBase&);
static bool CenterScreenSinkDevice_Send(NativeCenterScreenSinkDevice&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
static void CenterScreenSinkDevice_Visit(NativeCenterScreenSinkDevice&, AtomBase&, Visitor& vis);
static bool CenterScreenSinkDevice_Recv(NativeCenterScreenSinkDevice&, AtomBase&, int, const Packet&);
static void CenterScreenSinkDevice_Finalize(NativeCenterScreenSinkDevice&, AtomBase&, RealtimeSourceConfig&);
static void CenterScreenSinkDevice_Update(NativeCenterScreenSinkDevice&, AtomBase&, double dt);
static bool CenterScreenSinkDevice_IsReady(NativeCenterScreenSinkDevice&, AtomBase&, PacketIO& io);
static bool CenterScreenSinkDevice_AttachContext(NativeCenterScreenSinkDevice&, AtomBase& a, AtomBase& other);
static void CenterScreenSinkDevice_DetachContext(NativeCenterScreenSinkDevice&, AtomBase& a, AtomBase& other);
#endif

#if (defined flagHAL && defined flagFBO && defined flagSCREEN)
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

#if (defined flagHAL && defined flagOGL && defined flagSCREEN)
static bool OglScreenSinkDevice_Create(NativeOglScreenSinkDevice*& dev);
static void OglScreenSinkDevice_Destroy(NativeOglScreenSinkDevice*& dev);
static bool OglScreenSinkDevice_Initialize(NativeOglScreenSinkDevice&, AtomBase&, const WorldState&);
static bool OglScreenSinkDevice_PostInitialize(NativeOglScreenSinkDevice&, AtomBase&);
static bool OglScreenSinkDevice_Start(NativeOglScreenSinkDevice&, AtomBase&);
static void OglScreenSinkDevice_Stop(NativeOglScreenSinkDevice&, AtomBase&);
static void OglScreenSinkDevice_Uninitialize(NativeOglScreenSinkDevice&, AtomBase&);
static bool OglScreenSinkDevice_Send(NativeOglScreenSinkDevice&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
static void OglScreenSinkDevice_Visit(NativeOglScreenSinkDevice&, AtomBase&, Visitor& vis);
static bool OglScreenSinkDevice_Recv(NativeOglScreenSinkDevice&, AtomBase&, int, const Packet&);
static void OglScreenSinkDevice_Finalize(NativeOglScreenSinkDevice&, AtomBase&, RealtimeSourceConfig&);
static void OglScreenSinkDevice_Update(NativeOglScreenSinkDevice&, AtomBase&, double dt);
static bool OglScreenSinkDevice_IsReady(NativeOglScreenSinkDevice&, AtomBase&, PacketIO& io);
static bool OglScreenSinkDevice_AttachContext(NativeOglScreenSinkDevice&, AtomBase& a, AtomBase& other);
static void OglScreenSinkDevice_DetachContext(NativeOglScreenSinkDevice&, AtomBase& a, AtomBase& other);
#endif

#if (defined flagHAL && defined flagDX12 && defined flagSCREEN)
static bool D12ScreenSinkDevice_Create(NativeD12ScreenSinkDevice*& dev);
static void D12ScreenSinkDevice_Destroy(NativeD12ScreenSinkDevice*& dev);
static bool D12ScreenSinkDevice_Initialize(NativeD12ScreenSinkDevice&, AtomBase&, const WorldState&);
static bool D12ScreenSinkDevice_PostInitialize(NativeD12ScreenSinkDevice&, AtomBase&);
static bool D12ScreenSinkDevice_Start(NativeD12ScreenSinkDevice&, AtomBase&);
static void D12ScreenSinkDevice_Stop(NativeD12ScreenSinkDevice&, AtomBase&);
static void D12ScreenSinkDevice_Uninitialize(NativeD12ScreenSinkDevice&, AtomBase&);
static bool D12ScreenSinkDevice_Send(NativeD12ScreenSinkDevice&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
static void D12ScreenSinkDevice_Visit(NativeD12ScreenSinkDevice&, AtomBase&, Visitor& vis);
static bool D12ScreenSinkDevice_Recv(NativeD12ScreenSinkDevice&, AtomBase&, int, const Packet&);
static void D12ScreenSinkDevice_Finalize(NativeD12ScreenSinkDevice&, AtomBase&, RealtimeSourceConfig&);
static void D12ScreenSinkDevice_Update(NativeD12ScreenSinkDevice&, AtomBase&, double dt);
static bool D12ScreenSinkDevice_IsReady(NativeD12ScreenSinkDevice&, AtomBase&, PacketIO& io);
static bool D12ScreenSinkDevice_AttachContext(NativeD12ScreenSinkDevice&, AtomBase& a, AtomBase& other);
static void D12ScreenSinkDevice_DetachContext(NativeD12ScreenSinkDevice&, AtomBase& a, AtomBase& other);
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

