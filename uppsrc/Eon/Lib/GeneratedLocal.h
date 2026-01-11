#ifndef _EonLib_GeneratedLocal_h_
#define _EonLib_GeneratedLocal_h_

// This file is generated. Do not modify this file.

class CenterCustomer : public CustomerBase {

public:
	ATOM_CTOR_(CenterCustomer, CustomerBase)
	//ATOMTYPE(CenterCustomer)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};

class TestRealtimeSrc : public RollingValueBase {

public:
	ATOM_CTOR_(TestRealtimeSrc, RollingValueBase)
	//ATOMTYPE(TestRealtimeSrc)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};

class TestRealtimeSink : public VoidSinkBase {

public:
	ATOM_CTOR_(TestRealtimeSink, VoidSinkBase)
	//ATOMTYPE(TestRealtimeSink)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};

class TestPollerSink : public VoidPollerSinkBase {

public:
	ATOM_CTOR_(TestPollerSink, VoidPollerSinkBase)
	//ATOMTYPE(TestPollerSink)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};

#if (defined flagBUILTIN_PORTAUDIO && defined flagAUDIO) || (defined flagPORTAUDIO && defined flagAUDIO)
class PortaudioSink : public PortaudioSinkDevice {

public:
	ATOM_CTOR_(PortaudioSink, PortaudioSinkDevice)
	//ATOMTYPE(PortaudioSink)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if defined flagFFMPEG
class AudioDecoderSrc : public FfmpegSourceDevice {

public:
	ATOM_CTOR_(AudioDecoderSrc, FfmpegSourceDevice)
	//ATOMTYPE(AudioDecoderSrc)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if defined flagAUDIO
class AudioDbgSrc : public AudioGenBase {

public:
	ATOM_CTOR_(AudioDbgSrc, AudioGenBase)
	//ATOMTYPE(AudioDbgSrc)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

class AudioSplitter : public VoidBase {

public:
	ATOM_CTOR_(AudioSplitter, VoidBase)
	//ATOMTYPE(AudioSplitter)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};

class AudioSplitterUser : public VoidBase {

public:
	ATOM_CTOR_(AudioSplitterUser, VoidBase)
	//ATOMTYPE(AudioSplitterUser)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};

class AudioJoiner : public VoidBase {

public:
	ATOM_CTOR_(AudioJoiner, VoidBase)
	//ATOMTYPE(AudioJoiner)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};

class AudioJoinerUser : public VoidBase {

public:
	ATOM_CTOR_(AudioJoinerUser, VoidBase)
	//ATOMTYPE(AudioJoinerUser)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};

class AudioJoiner2User : public VoidBase {

public:
	ATOM_CTOR_(AudioJoiner2User, VoidBase)
	//ATOMTYPE(AudioJoiner2User)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};

#if defined flagAUDIO
class AudioMixer16 : public AudioMixerBase {

public:
	ATOM_CTOR_(AudioMixer16, AudioMixerBase)
	//ATOMTYPE(AudioMixer16)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if defined flagSCREEN
class VideoDbgSrc : public VideoGenBase {

public:
	ATOM_CTOR_(VideoDbgSrc, VideoGenBase)
	//ATOMTYPE(VideoDbgSrc)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagOPENCV && defined flagLINUX && defined flagCAMERA)
class WebcamPipe : public V4L2OpenCVCamera {

public:
	ATOM_CTOR_(WebcamPipe, V4L2OpenCVCamera)
	//ATOMTYPE(WebcamPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagOPENCV && defined flagLINUX && defined flagCAMERA)
class WebcamAtom : public V4L2OpenCVCamera {

public:
	ATOM_CTOR_(WebcamAtom, V4L2OpenCVCamera)
	//ATOMTYPE(WebcamAtom)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if defined flagFFMPEG
class AudioLoaderAtom : public FfmpegSourceDevice {

public:
	ATOM_CTOR_(AudioLoaderAtom, FfmpegSourceDevice)
	//ATOMTYPE(AudioLoaderAtom)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if defined flagFFMPEG
class VideoLoaderAtom : public FfmpegSourceDevice {

public:
	ATOM_CTOR_(VideoLoaderAtom, FfmpegSourceDevice)
	//ATOMTYPE(VideoLoaderAtom)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if defined flagSCREEN
class EventStatePipe : public EventStateBase {

public:
	ATOM_CTOR_(EventStatePipe, EventStateBase)
	//ATOMTYPE(EventStatePipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if defined flagSCREEN
class HandleProgEvents : public HandleEventsBase {

public:
	ATOM_CTOR_(HandleProgEvents, HandleEventsBase)
	//ATOMTYPE(HandleProgEvents)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if defined flagSCREEN
class CenterProgPipe : public HandleVideoBase {

public:
	ATOM_CTOR_(CenterProgPipe, HandleVideoBase)
	//ATOMTYPE(CenterProgPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if defined flagSCREEN
class OglProgPipe : public HandleVideoBase {

public:
	ATOM_CTOR_(OglProgPipe, HandleVideoBase)
	//ATOMTYPE(OglProgPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if defined flagSCREEN
class HandleProgVideo : public HandleVideoBase {

public:
	ATOM_CTOR_(HandleProgVideo, HandleVideoBase)
	//ATOMTYPE(HandleProgVideo)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagX11 && defined flagSCREEN)
class X11SwFboProg : public X11SwFboBase {

public:
	ATOM_CTOR_(X11SwFboProg, X11SwFboBase)
	//ATOMTYPE(X11SwFboProg)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagX11 && defined flagSCREEN && defined flagOGL)
class X11OglFboProg : public X11OglFboBase {

public:
	ATOM_CTOR_(X11OglFboProg, X11OglFboBase)
	//ATOMTYPE(X11OglFboProg)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSDL2 && defined flagHAL && defined flagFBO)
class SdlSwFboProg : public SdlCenterFboSinkDevice {

public:
	ATOM_CTOR_(SdlSwFboProg, SdlCenterFboSinkDevice)
	//ATOMTYPE(SdlSwFboProg)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
class SdlOglFboProg : public SdlOglFboBase {

public:
	ATOM_CTOR_(SdlOglFboProg, SdlOglFboBase)
	//ATOMTYPE(SdlOglFboProg)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagLOCALHMD && defined flagVR)
class LocalHMDPipe : public LocalHMDSinkDevice {

public:
	ATOM_CTOR_(LocalHMDPipe, LocalHMDSinkDevice)
	//ATOMTYPE(LocalHMDPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagFREEBSD && defined flagVR) || (defined flagLINUX && defined flagVR)
class RemoteVRServerPipe : public RemoteVRServerSinkDevice {

public:
	ATOM_CTOR_(RemoteVRServerPipe, RemoteVRServerSinkDevice)
	//ATOMTYPE(RemoteVRServerPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagFREEBSD && defined flagHACK && defined flagVR) || (defined flagLINUX && defined flagHACK && defined flagVR)
class BluetoothHoloPipe : public DevBluetoothSinkDevice {

public:
	ATOM_CTOR_(BluetoothHoloPipe, DevBluetoothSinkDevice)
	//ATOMTYPE(BluetoothHoloPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagX11 && defined flagSCREEN)
class X11SwShaderPipe : public X11SwShaderBase {

public:
	ATOM_CTOR_(X11SwShaderPipe, X11SwShaderBase)
	//ATOMTYPE(X11SwShaderPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagX11 && defined flagSCREEN && defined flagOGL)
class X11OglShaderPipe : public X11OglShaderBase {

public:
	ATOM_CTOR_(X11OglShaderPipe, X11OglShaderBase)
	//ATOMTYPE(X11OglShaderPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
class SdlOglShaderPipe : public SdlOglShaderBase {

public:
	ATOM_CTOR_(SdlOglShaderPipe, SdlOglShaderBase)
	//ATOMTYPE(SdlOglShaderPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagX11 && defined flagSCREEN)
class X11SwFboAtomPipe : public X11SwSinkDevice {

public:
	ATOM_CTOR_(X11SwFboAtomPipe, X11SwSinkDevice)
	//ATOMTYPE(X11SwFboAtomPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagX11 && defined flagOGL && defined flagSCREEN)
class X11OglFboAtomPipe : public X11OglSinkDevice {

public:
	ATOM_CTOR_(X11OglFboAtomPipe, X11OglSinkDevice)
	//ATOMTYPE(X11OglFboAtomPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSDL2 && defined flagHAL && defined flagFBO)
class SdlSwFboAtomPipe : public SdlCenterFboSinkDevice {

public:
	ATOM_CTOR_(SdlSwFboAtomPipe, SdlCenterFboSinkDevice)
	//ATOMTYPE(SdlSwFboAtomPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSDL2 && defined flagHAL && defined flagOGL)
class SdlOglFboAtomPipe : public SdlOglVideoSinkDevice {

public:
	ATOM_CTOR_(SdlOglFboAtomPipe, SdlOglVideoSinkDevice)
	//ATOMTYPE(SdlOglFboAtomPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSDL2 && defined flagHAL && defined flagOGL)
class SdlOglProgAtomPipe : public SdlOglVideoSinkDevice {

public:
	ATOM_CTOR_(SdlOglProgAtomPipe, SdlOglVideoSinkDevice)
	//ATOMTYPE(SdlOglProgAtomPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagX11 && defined flagSCREEN)
class X11ContextAtom : public X11Context {

public:
	ATOM_CTOR_(X11ContextAtom, X11Context)
	//ATOMTYPE(X11ContextAtom)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagX11 && defined flagSCREEN)
class X11SwContextAtom : public X11SwContext {

public:
	ATOM_CTOR_(X11SwContextAtom, X11SwContext)
	//ATOMTYPE(X11SwContextAtom)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagX11 && defined flagOGL && defined flagSCREEN)
class X11OglContextAtom : public X11OglContext {

public:
	ATOM_CTOR_(X11OglContextAtom, X11OglContext)
	//ATOMTYPE(X11OglContextAtom)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagX11 && defined flagSCREEN)
class X11EventAtomPipe : public X11EventsBase {

public:
	ATOM_CTOR_(X11EventAtomPipe, X11EventsBase)
	//ATOMTYPE(X11EventAtomPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagX11 && defined flagSCREEN)
class X11SwEventAtomPipe : public X11SwEventsBase {

public:
	ATOM_CTOR_(X11SwEventAtomPipe, X11SwEventsBase)
	//ATOMTYPE(X11SwEventAtomPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagX11 && defined flagOGL && defined flagSCREEN)
class X11OglEventAtomPipe : public X11OglEventsBase {

public:
	ATOM_CTOR_(X11OglEventAtomPipe, X11OglEventsBase)
	//ATOMTYPE(X11OglEventAtomPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagWIN32 && !defined flagUWP && defined flagSCREEN)
class WinContextAtom : public WinContext {

public:
	ATOM_CTOR_(WinContextAtom, WinContext)
	//ATOMTYPE(WinContextAtom)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagWIN32 && !defined flagUWP && defined flagSCREEN)
class WinVideoAtomPipe : public WinSinkDevice {

public:
	ATOM_CTOR_(WinVideoAtomPipe, WinSinkDevice)
	//ATOMTYPE(WinVideoAtomPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

class DxCustomer : public CustomerBase {

public:
	ATOM_CTOR_(DxCustomer, CustomerBase)
	//ATOMTYPE(DxCustomer)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};

#if (defined flagWIN32 && defined flagDX11 && defined flagSCREEN)
class WinD11ContextAtom : public WinD11Context {

public:
	ATOM_CTOR_(WinD11ContextAtom, WinD11Context)
	//ATOMTYPE(WinD11ContextAtom)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSCREEN && defined flagWIN32 && defined flagDX11)
class WinD11FboProg : public WinD11FboBase {

public:
	ATOM_CTOR_(WinD11FboProg, WinD11FboBase)
	//ATOMTYPE(WinD11FboProg)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagWIN32 && defined flagDX11 && defined flagSCREEN)
class WinD11FboAtomPipe : public WinD11SinkDevice {

public:
	ATOM_CTOR_(WinD11FboAtomPipe, WinD11SinkDevice)
	//ATOMTYPE(WinD11FboAtomPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagWIN32 && defined flagDX11 && defined flagSCREEN)
class WinD11FboAtomSA : public WinD11SinkDevice {

public:
	ATOM_CTOR_(WinD11FboAtomSA, WinD11SinkDevice)
	//ATOMTYPE(WinD11FboAtomSA)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

class OglCustomer : public CustomerBase {

public:
	ATOM_CTOR_(OglCustomer, CustomerBase)
	//ATOMTYPE(OglCustomer)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};

#if (defined flagSDL2 && defined flagHAL)
class SdlContextAtom : public SdlContextBase {

public:
	ATOM_CTOR_(SdlContextAtom, SdlContextBase)
	//ATOMTYPE(SdlContextAtom)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSDL2 && defined flagHAL)
class SdlEventAtomPipe : public SdlEventsBase {

public:
	ATOM_CTOR_(SdlEventAtomPipe, SdlEventsBase)
	//ATOMTYPE(SdlEventAtomPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

class TestEventSrcPipe : public TestEventSrcBase {

public:
	ATOM_CTOR_(TestEventSrcPipe, TestEventSrcBase)
	//ATOMTYPE(TestEventSrcPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};

#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
class SdlOglImageLoader : public SdlOglImageBase {

public:
	ATOM_CTOR_(SdlOglImageLoader, SdlOglImageBase)
	//ATOMTYPE(SdlOglImageLoader)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagUWP && defined flagDX12 && defined flagHAL)
class HoloContextAtom : public HoloContextBase {

public:
	ATOM_CTOR_(HoloContextAtom, HoloContextBase)
	//ATOMTYPE(HoloContextAtom)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagUWP && defined flagDX12 && defined flagHAL)
class HoloEventAtomPipe : public HoloEventsBase {

public:
	ATOM_CTOR_(HoloEventAtomPipe, HoloEventsBase)
	//ATOMTYPE(HoloEventAtomPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagUWP && defined flagDX12 && defined flagHAL && defined flagDX12)
class HoloD12FboAtomSA : public HoloD12VideoSinkDevice {

public:
	ATOM_CTOR_(HoloD12FboAtomSA, HoloD12VideoSinkDevice)
	//ATOMTYPE(HoloD12FboAtomSA)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if defined flagVOLUMETRIC
class VolumeLoaderAtom : public RawByteStaticSource {

public:
	ATOM_CTOR_(VolumeLoaderAtom, RawByteStaticSource)
	//ATOMTYPE(VolumeLoaderAtom)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagX11 && defined flagSCREEN)
class X11VideoAtomPipe : public X11SinkDevice {

public:
	ATOM_CTOR_(X11VideoAtomPipe, X11SinkDevice)
	//ATOMTYPE(X11VideoAtomPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagX11 && defined flagOGL && defined flagSCREEN)
class X11OglVideoAtomPipe : public X11OglSinkDevice {

public:
	ATOM_CTOR_(X11OglVideoAtomPipe, X11OglSinkDevice)
	//ATOMTYPE(X11OglVideoAtomPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagX11 && defined flagOGL && defined flagSCREEN)
class X11OglFboAtomSA : public X11OglSinkDevice {

public:
	ATOM_CTOR_(X11OglFboAtomSA, X11OglSinkDevice)
	//ATOMTYPE(X11OglFboAtomSA)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagX11 && defined flagSCREEN)
class X11SwVideoAtomPipe : public X11SwSinkDevice {

public:
	ATOM_CTOR_(X11SwVideoAtomPipe, X11SwSinkDevice)
	//ATOMTYPE(X11SwVideoAtomPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSDL2 && defined flagHAL && defined flagOGL)
class SdlOglFboAtomSA : public SdlOglVideoSinkDevice {

public:
	ATOM_CTOR_(SdlOglFboAtomSA, SdlOglVideoSinkDevice)
	//ATOMTYPE(SdlOglFboAtomSA)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSDL2 && defined flagHAL && defined flagOGL)
class SdlOglFboPipe : public SdlOglVideoSinkDevice {

public:
	ATOM_CTOR_(SdlOglFboPipe, SdlOglVideoSinkDevice)
	//ATOMTYPE(SdlOglFboPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSDL2 && defined flagHAL && defined flagOGL)
class SdlOglFboAtom : public SdlOglVideoSinkDevice {

public:
	ATOM_CTOR_(SdlOglFboAtom, SdlOglVideoSinkDevice)
	//ATOMTYPE(SdlOglFboAtom)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagGUI && defined flagHAL)
class UppEventAtomPipe : public UppEventsBase {

public:
	ATOM_CTOR_(UppEventAtomPipe, UppEventsBase)
	//ATOMTYPE(UppEventAtomPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if defined flagHAL
class CenterGuiFileSrc : public CenterGuiFileSrcBase {

public:
	ATOM_CTOR_(CenterGuiFileSrc, CenterGuiFileSrcBase)
	//ATOMTYPE(CenterGuiFileSrc)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;

};
#endif

#if (defined flagGUI && defined flagHAL)
class UppGuiSinkDevice : public UppGuiSinkBase {

public:
	ATOM_CTOR_(UppGuiSinkDevice, UppGuiSinkBase)
	//ATOMTYPE(UppGuiSinkDevice)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSDL2 && defined flagHAL && defined flagVIDEO)
class SdlVideoAtomPipe : public SdlCenterVideoSinkDevice {

public:
	ATOM_CTOR_(SdlVideoAtomPipe, SdlCenterVideoSinkDevice)
	//ATOMTYPE(SdlVideoAtomPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSDL2 && defined flagHAL && defined flagVIDEO)
class SdlProgAtomPipe : public SdlCenterVideoSinkDevice {

public:
	ATOM_CTOR_(SdlProgAtomPipe, SdlCenterVideoSinkDevice)
	//ATOMTYPE(SdlProgAtomPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagX11 && defined flagSCREEN)
class X11ProgAtomPipe : public X11SinkDevice {

public:
	ATOM_CTOR_(X11ProgAtomPipe, X11SinkDevice)
	//ATOMTYPE(X11ProgAtomPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagX11 && defined flagSCREEN)
class X11SwFboGuiProg : public X11SwFboProgBase {

public:
	ATOM_CTOR_(X11SwFboGuiProg, X11SwFboProgBase)
	//ATOMTYPE(X11SwFboGuiProg)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagX11 && defined flagSCREEN && defined flagOGL)
class X11OglFboGuiProg : public X11OglFboProgBase {

public:
	ATOM_CTOR_(X11OglFboGuiProg, X11OglFboProgBase)
	//ATOMTYPE(X11OglFboGuiProg)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
class SdlOglFboGuiProg : public SdlOglFboProgBase {

public:
	ATOM_CTOR_(SdlOglFboGuiProg, SdlOglFboProgBase)
	//ATOMTYPE(SdlOglFboGuiProg)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSDL2 && defined flagHAL && defined flagVIDEO)
class SdlVideoAtom : public SdlCenterVideoSinkDevice {

public:
	ATOM_CTOR_(SdlVideoAtom, SdlCenterVideoSinkDevice)
	//ATOMTYPE(SdlVideoAtom)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSDL2 && defined flagHAL && defined flagAUDIO)
class SdlAudioAtom : public SdlAudioSinkDevice {

public:
	ATOM_CTOR_(SdlAudioAtom, SdlAudioSinkDevice)
	//ATOMTYPE(SdlAudioAtom)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
class SdlOglShaderAtom : public SdlOglShaderBase {

public:
	ATOM_CTOR_(SdlOglShaderAtom, SdlOglShaderBase)
	//ATOMTYPE(SdlOglShaderAtom)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
class SdlOglShaderAtomSA : public SdlOglShaderBase {

public:
	ATOM_CTOR_(SdlOglShaderAtomSA, SdlOglShaderBase)
	//ATOMTYPE(SdlOglShaderAtomSA)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
class SdlOglTextureSource : public SdlOglTextureBase {

public:
	ATOM_CTOR_(SdlOglTextureSource, SdlOglTextureBase)
	//ATOMTYPE(SdlOglTextureSource)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
class SdlOglVolumeSource : public SdlOglTextureBase {

public:
	ATOM_CTOR_(SdlOglVolumeSource, SdlOglTextureBase)
	//ATOMTYPE(SdlOglVolumeSource)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
class SdlOglAudioSink : public SdlOglFboReaderBase {

public:
	ATOM_CTOR_(SdlOglAudioSink, SdlOglFboReaderBase)
	//ATOMTYPE(SdlOglAudioSink)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
class SdlOglKeyboardSource : public SdlOglKeyboardBase {

public:
	ATOM_CTOR_(SdlOglKeyboardSource, SdlOglKeyboardBase)
	//ATOMTYPE(SdlOglKeyboardSource)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
class SdlOglAudioSource : public SdlOglAudioBase {

public:
	ATOM_CTOR_(SdlOglAudioSource, SdlOglAudioBase)
	//ATOMTYPE(SdlOglAudioSource)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if defined flagMIDI
class MidiFileReaderPipe : public MidiFileReaderAtom {

public:
	ATOM_CTOR_(MidiFileReaderPipe, MidiFileReaderAtom)
	//ATOMTYPE(MidiFileReaderPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if defined flagMIDI
class MidiFileReader : public MidiFileReaderAtom {

public:
	ATOM_CTOR_(MidiFileReader, MidiFileReaderAtom)
	//ATOMTYPE(MidiFileReader)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if defined flagMIDI
class MidiFileReader16 : public MidiFileReaderAtom {

public:
	ATOM_CTOR_(MidiFileReader16, MidiFileReaderAtom)
	//ATOMTYPE(MidiFileReader16)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if defined flagMIDI
class MidiNullSink : public MidiNullAtom {

public:
	ATOM_CTOR_(MidiNullSink, MidiNullAtom)
	//ATOMTYPE(MidiNullSink)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagFLUIDLITE && defined flagAUDIO && defined flagMIDI) || (defined flagFLUIDSYNTH && defined flagAUDIO && defined flagMIDI)
class FluidsynthPipe : public FluidsynthInstrument {

public:
	ATOM_CTOR_(FluidsynthPipe, FluidsynthInstrument)
	//ATOMTYPE(FluidsynthPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagAUDIO && defined flagMIDI)
class SoftInstrumentPipe : public SoftInstrument {

public:
	ATOM_CTOR_(SoftInstrumentPipe, SoftInstrument)
	//ATOMTYPE(SoftInstrumentPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagAUDIO && defined flagMIDI)
class FmSynthPipe : public FmSynthInstrument {

public:
	ATOM_CTOR_(FmSynthPipe, FmSynthInstrument)
	//ATOMTYPE(FmSynthPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagLV2 && defined flagAUDIO && defined flagMIDI)
class LV2InstrumentPipe : public LV2Instrument {

public:
	ATOM_CTOR_(LV2InstrumentPipe, LV2Instrument)
	//ATOMTYPE(LV2InstrumentPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagAUDIO && defined flagMIDI)
class CoreSynthPipe : public CoreSynthInstrument {

public:
	ATOM_CTOR_(CoreSynthPipe, CoreSynthInstrument)
	//ATOMTYPE(CoreSynthPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagAUDIO && defined flagMIDI)
class CoreDrummerPipe : public CoreDrummerInstrument {

public:
	ATOM_CTOR_(CoreDrummerPipe, CoreDrummerInstrument)
	//ATOMTYPE(CoreDrummerPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if defined flagAUDIO
class CoreEffectPipe : public AudioCoreEffect {

public:
	ATOM_CTOR_(CoreEffectPipe, AudioCoreEffect)
	//ATOMTYPE(CoreEffectPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if defined flagAUDIO
class CoreEffectAtom : public AudioCoreEffect {

public:
	ATOM_CTOR_(CoreEffectAtom, AudioCoreEffect)
	//ATOMTYPE(CoreEffectAtom)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagLV2 && defined flagAUDIO)
class LV2EffectPipe : public LV2Effect {

public:
	ATOM_CTOR_(LV2EffectPipe, LV2Effect)
	//ATOMTYPE(LV2EffectPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagBUILTIN_PORTMIDI && defined flagMIDI) || (defined flagPORTMIDI && defined flagMIDI)
class PortmidiPipe : public PortmidiSource {

public:
	ATOM_CTOR_(PortmidiPipe, PortmidiSource)
	//ATOMTYPE(PortmidiPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagBUILTIN_PORTMIDI && defined flagMIDI) || (defined flagPORTMIDI && defined flagMIDI)
class PortmidiSend : public PortmidiSource {

public:
	ATOM_CTOR_(PortmidiSend, PortmidiSource)
	//ATOMTYPE(PortmidiSend)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if defined flagAUDIO
class CoreAudioFileOut : public CoreAudioSink {

public:
	ATOM_CTOR_(CoreAudioFileOut, CoreAudioSink)
	//ATOMTYPE(CoreAudioFileOut)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#endif
