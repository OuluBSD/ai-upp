#include "Lib.h"

// This file is generated. Do not modify this file.

NAMESPACE_UPP


String CenterCustomer::GetAction() {
	return "center.customer";
}

AtomTypeCls CenterCustomer::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //CENTERCUSTOMER;
	t.role = AtomRole::CUSTOMER;
	t.AddIn(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,ORDER),0);
	return t;
}

LinkTypeCls CenterCustomer::GetLinkType() {
	return LINKTYPE(CUSTOMER, CUSTOMER);
}

void CenterCustomer::Visit(Vis& v) {
	VIS_THIS(CustomerBase);
}

AtomTypeCls CenterCustomer::GetType() const {
	return GetAtomType();
}


String TestRealtimeSrc::GetAction() {
	return "center.audio.src.test";
}

AtomTypeCls TestRealtimeSrc::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //TESTREALTIMESRC;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls TestRealtimeSrc::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void TestRealtimeSrc::Visit(Vis& v) {
	VIS_THIS(RollingValueBase);
}

AtomTypeCls TestRealtimeSrc::GetType() const {
	return GetAtomType();
}


String TestRealtimeSink::GetAction() {
	return "center.audio.sink.test.realtime";
}

AtomTypeCls TestRealtimeSink::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //TESTREALTIMESINK;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,AUDIO),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls TestRealtimeSink::GetLinkType() {
	return LINKTYPE(INTERVAL_PIPE, PROCESS);
}

void TestRealtimeSink::Visit(Vis& v) {
	VIS_THIS(VoidSinkBase);
}

AtomTypeCls TestRealtimeSink::GetType() const {
	return GetAtomType();
}


String TestPollerSink::GetAction() {
	return "center.audio.sink.test.poller";
}

AtomTypeCls TestPollerSink::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //TESTPOLLERSINK;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,AUDIO),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls TestPollerSink::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void TestPollerSink::Visit(Vis& v) {
	VIS_THIS(VoidPollerSinkBase);
}

AtomTypeCls TestPollerSink::GetType() const {
	return GetAtomType();
}


#if (defined flagBUILTIN_PORTAUDIO && defined flagAUDIO) || (defined flagPORTAUDIO && defined flagAUDIO)
String PortaudioSink::GetAction() {
	return "center.audio.sink.hw";
}

AtomTypeCls PortaudioSink::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //PORTAUDIOSINK;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,AUDIO),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls PortaudioSink::GetLinkType() {
	return LINKTYPE(EXTERNAL_PIPE, PROCESS);
}

void PortaudioSink::Visit(Vis& v) {
	VIS_THIS(PortaudioSinkDevice);
}

AtomTypeCls PortaudioSink::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagFFMPEG
String AudioDecoderSrc::GetAction() {
	return "perma.audio.source.decoder";
}

AtomTypeCls AudioDecoderSrc::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //AUDIODECODERSRC;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls AudioDecoderSrc::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void AudioDecoderSrc::Visit(Vis& v) {
	VIS_THIS(FfmpegSourceDevice);
}

AtomTypeCls AudioDecoderSrc::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagAUDIO
String AudioDbgSrc::GetAction() {
	return "center.audio.src.dbg_generator";
}

AtomTypeCls AudioDbgSrc::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //AUDIODBGSRC;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls AudioDbgSrc::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void AudioDbgSrc::Visit(Vis& v) {
	VIS_THIS(AudioGenBase);
}

AtomTypeCls AudioDbgSrc::GetType() const {
	return GetAtomType();
}
#endif


String AudioSplitter::GetAction() {
	return "center.audio.side.src.center";
}

AtomTypeCls AudioSplitter::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //AUDIOSPLITTER;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,AUDIO),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls AudioSplitter::GetLinkType() {
	return LINKTYPE(SPLITTER, PROCESS);
}

void AudioSplitter::Visit(Vis& v) {
	VIS_THIS(VoidBase);
}

AtomTypeCls AudioSplitter::GetType() const {
	return GetAtomType();
}


String AudioSplitterUser::GetAction() {
	return "center.audio.side.src.center.user";
}

AtomTypeCls AudioSplitterUser::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //AUDIOSPLITTERUSER;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,AUDIO),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),1);
	return t;
}

LinkTypeCls AudioSplitterUser::GetLinkType() {
	return LINKTYPE(SPLITTER, PROCESS);
}

void AudioSplitterUser::Visit(Vis& v) {
	VIS_THIS(VoidBase);
}

AtomTypeCls AudioSplitterUser::GetType() const {
	return GetAtomType();
}


String AudioJoiner::GetAction() {
	return "center.audio.side.sink.center";
}

AtomTypeCls AudioJoiner::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //AUDIOJOINER;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,AUDIO),0);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls AudioJoiner::GetLinkType() {
	return LINKTYPE(JOINER, PROCESS);
}

void AudioJoiner::Visit(Vis& v) {
	VIS_THIS(VoidBase);
}

AtomTypeCls AudioJoiner::GetType() const {
	return GetAtomType();
}


String AudioJoinerUser::GetAction() {
	return "center.audio.side.sink.center.user";
}

AtomTypeCls AudioJoinerUser::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //AUDIOJOINERUSER;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls AudioJoinerUser::GetLinkType() {
	return LINKTYPE(JOINER, PROCESS);
}

void AudioJoinerUser::Visit(Vis& v) {
	VIS_THIS(VoidBase);
}

AtomTypeCls AudioJoinerUser::GetType() const {
	return GetAtomType();
}


String AudioJoiner2User::GetAction() {
	return "center.audio.side.sink2.center.user";
}

AtomTypeCls AudioJoiner2User::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //AUDIOJOINER2USER;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls AudioJoiner2User::GetLinkType() {
	return LINKTYPE(JOINER, PROCESS);
}

void AudioJoiner2User::Visit(Vis& v) {
	VIS_THIS(VoidBase);
}

AtomTypeCls AudioJoiner2User::GetType() const {
	return GetAtomType();
}


#if defined flagAUDIO
String AudioMixer16::GetAction() {
	return "center.audio.mixer16";
}

AtomTypeCls AudioMixer16::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //AUDIOMIXER16;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls AudioMixer16::GetLinkType() {
	return LINKTYPE(MERGER, PROCESS);
}

void AudioMixer16::Visit(Vis& v) {
	VIS_THIS(AudioMixerBase);
}

AtomTypeCls AudioMixer16::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagSCREEN
String VideoDbgSrc::GetAction() {
	return "center.video.src.dbg_generator";
}

AtomTypeCls VideoDbgSrc::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //VIDEODBGSRC;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,VIDEO),0);
	return t;
}

LinkTypeCls VideoDbgSrc::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void VideoDbgSrc::Visit(Vis& v) {
	VIS_THIS(VideoGenBase);
}

AtomTypeCls VideoDbgSrc::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagOPENCV && defined flagLINUX && defined flagCAMERA)
String WebcamPipe::GetAction() {
	return "center.video.webcam.pipe";
}

AtomTypeCls WebcamPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //WEBCAMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,VIDEO),0);
	return t;
}

LinkTypeCls WebcamPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void WebcamPipe::Visit(Vis& v) {
	VIS_THIS(V4L2OpenCVCamera);
}

AtomTypeCls WebcamPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagOPENCV && defined flagLINUX && defined flagCAMERA)
String WebcamAtom::GetAction() {
	return "center.video.webcam";
}

AtomTypeCls WebcamAtom::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //WEBCAMATOM;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,VIDEO),1);
	return t;
}

LinkTypeCls WebcamAtom::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void WebcamAtom::Visit(Vis& v) {
	VIS_THIS(V4L2OpenCVCamera);
}

AtomTypeCls WebcamAtom::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagFFMPEG
String AudioLoaderAtom::GetAction() {
	return "center.audio.loader";
}

AtomTypeCls AudioLoaderAtom::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //AUDIOLOADERATOM;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,AUDIO),1);
	return t;
}

LinkTypeCls AudioLoaderAtom::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void AudioLoaderAtom::Visit(Vis& v) {
	VIS_THIS(FfmpegSourceDevice);
}

AtomTypeCls AudioLoaderAtom::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagFFMPEG
String VideoLoaderAtom::GetAction() {
	return "center.video.loader";
}

AtomTypeCls VideoLoaderAtom::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //VIDEOLOADERATOM;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,VIDEO),1);
	t.AddOut(VD(CENTER,AUDIO),1);
	return t;
}

LinkTypeCls VideoLoaderAtom::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void VideoLoaderAtom::Visit(Vis& v) {
	VIS_THIS(FfmpegSourceDevice);
}

AtomTypeCls VideoLoaderAtom::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagSCREEN
String EventStatePipe::GetAction() {
	return "state.event.pipe";
}

AtomTypeCls EventStatePipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //EVENTSTATEPIPE;
	t.role = AtomRole::DRIVER_PIPE;
	t.AddIn(VD(CENTER,EVENT),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls EventStatePipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void EventStatePipe::Visit(Vis& v) {
	VIS_THIS(EventStateBase);
}

AtomTypeCls EventStatePipe::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagSCREEN
String HandleProgEvents::GetAction() {
	return "center.events.prog.ecs";
}

AtomTypeCls HandleProgEvents::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //HANDLEPROGEVENTS;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls HandleProgEvents::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void HandleProgEvents::Visit(Vis& v) {
	VIS_THIS(HandleEventsBase);
}

AtomTypeCls HandleProgEvents::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagSCREEN
String CenterProgPipe::GetAction() {
	return "center.video.prog.pipe";
}

AtomTypeCls CenterProgPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //CENTERPROGPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,PROG),0);
	return t;
}

LinkTypeCls CenterProgPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void CenterProgPipe::Visit(Vis& v) {
	VIS_THIS(HandleVideoBase);
}

AtomTypeCls CenterProgPipe::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagSCREEN
String OglProgPipe::GetAction() {
	return "ogl.prog.pipe";
}

AtomTypeCls OglProgPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //OGLPROGPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,ORDER),0);
	t.AddOut(VD(OGL,PROG),0);
	return t;
}

LinkTypeCls OglProgPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void OglProgPipe::Visit(Vis& v) {
	VIS_THIS(HandleVideoBase);
}

AtomTypeCls OglProgPipe::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagSCREEN
String HandleProgVideo::GetAction() {
	return "center.video.prog.ecs";
}

AtomTypeCls HandleProgVideo::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //HANDLEPROGVIDEO;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls HandleProgVideo::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void HandleProgVideo::Visit(Vis& v) {
	VIS_THIS(HandleVideoBase);
}

AtomTypeCls HandleProgVideo::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagX11 && defined flagSCREEN)
String X11SwFboProg::GetAction() {
	return "x11.sw.fbo.program";
}

AtomTypeCls X11SwFboProg::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11SWFBOPROG;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,FBO),1);
	t.AddOut(VD(CENTER,FBO),0);
	return t;
}

LinkTypeCls X11SwFboProg::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void X11SwFboProg::Visit(Vis& v) {
	VIS_THIS(X11SwFboBase);
}

AtomTypeCls X11SwFboProg::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagX11 && defined flagSCREEN && defined flagOGL)
String X11OglFboProg::GetAction() {
	return "x11.ogl.fbo.program";
}

AtomTypeCls X11OglFboProg::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11OGLFBOPROG;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,ORDER),0);
	t.AddIn(VD(OGL,FBO),1);
	t.AddOut(VD(OGL,FBO),0);
	return t;
}

LinkTypeCls X11OglFboProg::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void X11OglFboProg::Visit(Vis& v) {
	VIS_THIS(X11OglFboBase);
}

AtomTypeCls X11OglFboProg::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSDL2 && defined flagHAL && defined flagFBO && defined flagSCREEN)
String SdlSwFboProg::GetAction() {
	return "sdl.sw.fbo.program";
}

AtomTypeCls SdlSwFboProg::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLSWFBOPROG;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,FBO),1);
	t.AddOut(VD(CENTER,FBO),0);
	return t;
}

LinkTypeCls SdlSwFboProg::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void SdlSwFboProg::Visit(Vis& v) {
	VIS_THIS(SdlCenterFboSinkDevice);
}

AtomTypeCls SdlSwFboProg::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
String SdlOglFboProg::GetAction() {
	return "sdl.ogl.fbo.program";
}

AtomTypeCls SdlOglFboProg::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLOGLFBOPROG;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,ORDER),0);
	t.AddIn(VD(OGL,FBO),1);
	t.AddOut(VD(OGL,FBO),0);
	return t;
}

LinkTypeCls SdlOglFboProg::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void SdlOglFboProg::Visit(Vis& v) {
	VIS_THIS(SdlOglFboBase);
}

AtomTypeCls SdlOglFboProg::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSOFTHMD && defined flagVR)
String SoftHMDPipe::GetAction() {
	return "x11.ogl.holo.events";
}

AtomTypeCls SoftHMDPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SOFTHMDPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,EVENT),0);
	return t;
}

LinkTypeCls SoftHMDPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void SoftHMDPipe::Visit(Vis& v) {
	VIS_THIS(SoftHMDSinkDevice);
}

AtomTypeCls SoftHMDPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagFREEBSD && defined flagVR) || (defined flagLINUX && defined flagVR)
String RemoteVRServerPipe::GetAction() {
	return "tcp.ogl.holo.events";
}

AtomTypeCls RemoteVRServerPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //REMOTEVRSERVERPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,EVENT),0);
	return t;
}

LinkTypeCls RemoteVRServerPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void RemoteVRServerPipe::Visit(Vis& v) {
	VIS_THIS(RemoteVRServerSinkDevice);
}

AtomTypeCls RemoteVRServerPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagFREEBSD && defined flagHACK && defined flagVR) || (defined flagLINUX && defined flagHACK && defined flagVR)
String BluetoothHoloPipe::GetAction() {
	return "bluetooth.holo.events";
}

AtomTypeCls BluetoothHoloPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //BLUETOOTHHOLOPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,EVENT),0);
	return t;
}

LinkTypeCls BluetoothHoloPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void BluetoothHoloPipe::Visit(Vis& v) {
	VIS_THIS(DevBluetoothSinkDevice);
}

AtomTypeCls BluetoothHoloPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagX11 && defined flagSCREEN)
String X11SwShaderPipe::GetAction() {
	return "x11.sw.fbo.pipe";
}

AtomTypeCls X11SwShaderPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11SWSHADERPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,FBO),0);
	return t;
}

LinkTypeCls X11SwShaderPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void X11SwShaderPipe::Visit(Vis& v) {
	VIS_THIS(X11SwShaderBase);
}

AtomTypeCls X11SwShaderPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagX11 && defined flagSCREEN && defined flagOGL)
String X11OglShaderPipe::GetAction() {
	return "x11.ogl.fbo.pipe";
}

AtomTypeCls X11OglShaderPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11OGLSHADERPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,ORDER),0);
	t.AddOut(VD(OGL,FBO),0);
	return t;
}

LinkTypeCls X11OglShaderPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void X11OglShaderPipe::Visit(Vis& v) {
	VIS_THIS(X11OglShaderBase);
}

AtomTypeCls X11OglShaderPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
String SdlOglShaderPipe::GetAction() {
	return "sdl.ogl.fbo.pipe";
}

AtomTypeCls SdlOglShaderPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLOGLSHADERPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,ORDER),0);
	t.AddOut(VD(OGL,FBO),0);
	return t;
}

LinkTypeCls SdlOglShaderPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void SdlOglShaderPipe::Visit(Vis& v) {
	VIS_THIS(SdlOglShaderBase);
}

AtomTypeCls SdlOglShaderPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagX11 && defined flagSCREEN)
String X11SwFboAtomPipe::GetAction() {
	return "x11.sw.fbo.sink";
}

AtomTypeCls X11SwFboAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11SWFBOATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,FBO),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls X11SwFboAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void X11SwFboAtomPipe::Visit(Vis& v) {
	VIS_THIS(X11SwSinkDevice);
}

AtomTypeCls X11SwFboAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagX11 && defined flagOGL && defined flagSCREEN)
String X11OglFboAtomPipe::GetAction() {
	return "x11.ogl.fbo.sink";
}

AtomTypeCls X11OglFboAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11OGLFBOATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,FBO),0);
	t.AddOut(VD(OGL,RECEIPT),0);
	return t;
}

LinkTypeCls X11OglFboAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void X11OglFboAtomPipe::Visit(Vis& v) {
	VIS_THIS(X11OglSinkDevice);
}

AtomTypeCls X11OglFboAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSDL2 && defined flagHAL && defined flagFBO && defined flagSCREEN)
String SdlSwFboAtomPipe::GetAction() {
	return "sdl.sw.fbo.sink";
}

AtomTypeCls SdlSwFboAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLSWFBOATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,FBO),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls SdlSwFboAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void SdlSwFboAtomPipe::Visit(Vis& v) {
	VIS_THIS(SdlCenterFboSinkDevice);
}

AtomTypeCls SdlSwFboAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSDL2 && defined flagHAL && defined flagOGL && defined flagSCREEN)
String SdlOglFboAtomPipe::GetAction() {
	return "sdl.ogl.fbo.sink";
}

AtomTypeCls SdlOglFboAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLOGLFBOATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,FBO),0);
	t.AddOut(VD(OGL,RECEIPT),0);
	return t;
}

LinkTypeCls SdlOglFboAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void SdlOglFboAtomPipe::Visit(Vis& v) {
	VIS_THIS(SdlOglScreenSinkDevice);
}

AtomTypeCls SdlOglFboAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSDL2 && defined flagHAL && defined flagOGL && defined flagSCREEN)
String SdlOglProgAtomPipe::GetAction() {
	return "sdl.ogl.prog.pipe";
}

AtomTypeCls SdlOglProgAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLOGLPROGATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,PROG),0);
	t.AddOut(VD(OGL,RECEIPT),0);
	return t;
}

LinkTypeCls SdlOglProgAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void SdlOglProgAtomPipe::Visit(Vis& v) {
	VIS_THIS(SdlOglScreenSinkDevice);
}

AtomTypeCls SdlOglProgAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagX11 && defined flagSCREEN)
String X11ContextAtom::GetAction() {
	return "x11.context";
}

AtomTypeCls X11ContextAtom::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11CONTEXTATOM;
	t.role = AtomRole::DRIVER;
	t.AddIn(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls X11ContextAtom::GetLinkType() {
	return LINKTYPE(DRIVER, DRIVER);
}

void X11ContextAtom::Visit(Vis& v) {
	VIS_THIS(X11Context);
}

AtomTypeCls X11ContextAtom::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagX11 && defined flagSCREEN)
String X11SwContextAtom::GetAction() {
	return "x11.sw.context";
}

AtomTypeCls X11SwContextAtom::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11SWCONTEXTATOM;
	t.role = AtomRole::DRIVER;
	t.AddIn(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls X11SwContextAtom::GetLinkType() {
	return LINKTYPE(DRIVER, DRIVER);
}

void X11SwContextAtom::Visit(Vis& v) {
	VIS_THIS(X11SwContext);
}

AtomTypeCls X11SwContextAtom::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagX11 && defined flagOGL && defined flagSCREEN)
String X11OglContextAtom::GetAction() {
	return "x11.ogl.context";
}

AtomTypeCls X11OglContextAtom::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11OGLCONTEXTATOM;
	t.role = AtomRole::DRIVER;
	t.AddIn(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls X11OglContextAtom::GetLinkType() {
	return LINKTYPE(DRIVER, DRIVER);
}

void X11OglContextAtom::Visit(Vis& v) {
	VIS_THIS(X11OglContext);
}

AtomTypeCls X11OglContextAtom::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagX11 && defined flagSCREEN)
String X11EventAtomPipe::GetAction() {
	return "x11.event.pipe";
}

AtomTypeCls X11EventAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11EVENTATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,EVENT),0);
	return t;
}

LinkTypeCls X11EventAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void X11EventAtomPipe::Visit(Vis& v) {
	VIS_THIS(X11EventsBase);
}

AtomTypeCls X11EventAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagX11 && defined flagSCREEN)
String X11SwEventAtomPipe::GetAction() {
	return "x11.sw.event.pipe";
}

AtomTypeCls X11SwEventAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11SWEVENTATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,EVENT),0);
	return t;
}

LinkTypeCls X11SwEventAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void X11SwEventAtomPipe::Visit(Vis& v) {
	VIS_THIS(X11SwEventsBase);
}

AtomTypeCls X11SwEventAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagX11 && defined flagOGL && defined flagSCREEN)
String X11OglEventAtomPipe::GetAction() {
	return "x11.ogl.event.pipe";
}

AtomTypeCls X11OglEventAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11OGLEVENTATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,EVENT),0);
	return t;
}

LinkTypeCls X11OglEventAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void X11OglEventAtomPipe::Visit(Vis& v) {
	VIS_THIS(X11OglEventsBase);
}

AtomTypeCls X11OglEventAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagWIN32 && !defined flagUWP && defined flagSCREEN)
String WinContextAtom::GetAction() {
	return "win.context";
}

AtomTypeCls WinContextAtom::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //WINCONTEXTATOM;
	t.role = AtomRole::DRIVER;
	t.AddIn(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls WinContextAtom::GetLinkType() {
	return LINKTYPE(DRIVER, DRIVER);
}

void WinContextAtom::Visit(Vis& v) {
	VIS_THIS(WinContext);
}

AtomTypeCls WinContextAtom::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagWIN32 && !defined flagUWP && defined flagSCREEN)
String WinVideoAtomPipe::GetAction() {
	return "win.video.pipe";
}

AtomTypeCls WinVideoAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //WINVIDEOATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,VIDEO),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls WinVideoAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void WinVideoAtomPipe::Visit(Vis& v) {
	VIS_THIS(WinSinkDevice);
}

AtomTypeCls WinVideoAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


String DxCustomer::GetAction() {
	return "dx.customer";
}

AtomTypeCls DxCustomer::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //DXCUSTOMER;
	t.role = AtomRole::CUSTOMER;
	t.AddIn(VD(DX,RECEIPT),0);
	t.AddOut(VD(DX,ORDER),0);
	return t;
}

LinkTypeCls DxCustomer::GetLinkType() {
	return LINKTYPE(CUSTOMER, CUSTOMER);
}

void DxCustomer::Visit(Vis& v) {
	VIS_THIS(CustomerBase);
}

AtomTypeCls DxCustomer::GetType() const {
	return GetAtomType();
}


#if (defined flagWIN32 && defined flagDX11 && defined flagSCREEN)
String WinD11ContextAtom::GetAction() {
	return "win.dx.context";
}

AtomTypeCls WinD11ContextAtom::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //WIND11CONTEXTATOM;
	t.role = AtomRole::DRIVER;
	t.AddIn(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls WinD11ContextAtom::GetLinkType() {
	return LINKTYPE(DRIVER, DRIVER);
}

void WinD11ContextAtom::Visit(Vis& v) {
	VIS_THIS(WinD11Context);
}

AtomTypeCls WinD11ContextAtom::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSCREEN && defined flagWIN32 && defined flagDX11)
String WinD11FboProg::GetAction() {
	return "win.dx.fbo.program";
}

AtomTypeCls WinD11FboProg::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //WIND11FBOPROG;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(DX,ORDER),0);
	t.AddIn(VD(DX,FBO),1);
	t.AddOut(VD(DX,FBO),0);
	return t;
}

LinkTypeCls WinD11FboProg::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void WinD11FboProg::Visit(Vis& v) {
	VIS_THIS(WinD11FboBase);
}

AtomTypeCls WinD11FboProg::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagWIN32 && defined flagDX11 && defined flagSCREEN)
String WinD11FboAtomPipe::GetAction() {
	return "win.dx.fbo.sink";
}

AtomTypeCls WinD11FboAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //WIND11FBOATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(DX,FBO),0);
	t.AddOut(VD(DX,RECEIPT),0);
	return t;
}

LinkTypeCls WinD11FboAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void WinD11FboAtomPipe::Visit(Vis& v) {
	VIS_THIS(WinD11SinkDevice);
}

AtomTypeCls WinD11FboAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagWIN32 && defined flagDX11 && defined flagSCREEN)
String WinD11FboAtomSA::GetAction() {
	return "win.dx.fbo.standalone";
}

AtomTypeCls WinD11FboAtomSA::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //WIND11FBOATOMSA;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(DX,ORDER),0);
	t.AddOut(VD(DX,RECEIPT),0);
	return t;
}

LinkTypeCls WinD11FboAtomSA::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void WinD11FboAtomSA::Visit(Vis& v) {
	VIS_THIS(WinD11SinkDevice);
}

AtomTypeCls WinD11FboAtomSA::GetType() const {
	return GetAtomType();
}
#endif


String OglCustomer::GetAction() {
	return "ogl.customer";
}

AtomTypeCls OglCustomer::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //OGLCUSTOMER;
	t.role = AtomRole::CUSTOMER;
	t.AddIn(VD(OGL,RECEIPT),0);
	t.AddOut(VD(OGL,ORDER),0);
	return t;
}

LinkTypeCls OglCustomer::GetLinkType() {
	return LINKTYPE(CUSTOMER, CUSTOMER);
}

void OglCustomer::Visit(Vis& v) {
	VIS_THIS(CustomerBase);
}

AtomTypeCls OglCustomer::GetType() const {
	return GetAtomType();
}


#if (defined flagSDL2 && defined flagHAL)
String SdlContextAtom::GetAction() {
	return "sdl.context";
}

AtomTypeCls SdlContextAtom::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLCONTEXTATOM;
	t.role = AtomRole::DRIVER;
	t.AddIn(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls SdlContextAtom::GetLinkType() {
	return LINKTYPE(DRIVER, DRIVER);
}

void SdlContextAtom::Visit(Vis& v) {
	VIS_THIS(SdlContextBase);
}

AtomTypeCls SdlContextAtom::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSDL2 && defined flagHAL)
String SdlEventAtomPipe::GetAction() {
	return "sdl.event.pipe";
}

AtomTypeCls SdlEventAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLEVENTATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,EVENT),0);
	return t;
}

LinkTypeCls SdlEventAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void SdlEventAtomPipe::Visit(Vis& v) {
	VIS_THIS(SdlEventsBase);
}

AtomTypeCls SdlEventAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


String TestEventSrcPipe::GetAction() {
	return "event.src.test.pipe";
}

AtomTypeCls TestEventSrcPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //TESTEVENTSRCPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,EVENT),0);
	return t;
}

LinkTypeCls TestEventSrcPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void TestEventSrcPipe::Visit(Vis& v) {
	VIS_THIS(TestEventSrcBase);
}

AtomTypeCls TestEventSrcPipe::GetType() const {
	return GetAtomType();
}


#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
String SdlOglImageLoader::GetAction() {
	return "center.image.loader";
}

AtomTypeCls SdlOglImageLoader::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLOGLIMAGELOADER;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,VIDEO),1);
	return t;
}

LinkTypeCls SdlOglImageLoader::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void SdlOglImageLoader::Visit(Vis& v) {
	VIS_THIS(SdlOglImageBase);
}

AtomTypeCls SdlOglImageLoader::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagUWP && defined flagDX12 && defined flagHAL)
String HoloContextAtom::GetAction() {
	return "holo.context";
}

AtomTypeCls HoloContextAtom::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //HOLOCONTEXTATOM;
	t.role = AtomRole::DRIVER;
	t.AddIn(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls HoloContextAtom::GetLinkType() {
	return LINKTYPE(DRIVER, DRIVER);
}

void HoloContextAtom::Visit(Vis& v) {
	VIS_THIS(HoloContextBase);
}

AtomTypeCls HoloContextAtom::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagUWP && defined flagDX12 && defined flagHAL)
String HoloEventAtomPipe::GetAction() {
	return "holo.event.pipe";
}

AtomTypeCls HoloEventAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //HOLOEVENTATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,EVENT),0);
	return t;
}

LinkTypeCls HoloEventAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void HoloEventAtomPipe::Visit(Vis& v) {
	VIS_THIS(HoloEventsBase);
}

AtomTypeCls HoloEventAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagUWP && defined flagDX12 && defined flagHAL && defined flagDX12 && defined flagSCREEN)
String HoloD12FboAtomSA::GetAction() {
	return "holo.fbo.standalone";
}

AtomTypeCls HoloD12FboAtomSA::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //HOLOD12FBOATOMSA;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(DX,ORDER),0);
	t.AddOut(VD(DX,RECEIPT),0);
	return t;
}

LinkTypeCls HoloD12FboAtomSA::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void HoloD12FboAtomSA::Visit(Vis& v) {
	VIS_THIS(HoloD12ScreenSinkDevice);
}

AtomTypeCls HoloD12FboAtomSA::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagVOLUMETRIC
String VolumeLoaderAtom::GetAction() {
	return "center.volume.loader";
}

AtomTypeCls VolumeLoaderAtom::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //VOLUMELOADERATOM;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,VOLUME),1);
	return t;
}

LinkTypeCls VolumeLoaderAtom::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void VolumeLoaderAtom::Visit(Vis& v) {
	VIS_THIS(RawByteStaticSource);
}

AtomTypeCls VolumeLoaderAtom::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagX11 && defined flagSCREEN)
String X11VideoAtomPipe::GetAction() {
	return "x11.video.pipe";
}

AtomTypeCls X11VideoAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11VIDEOATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,VIDEO),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls X11VideoAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void X11VideoAtomPipe::Visit(Vis& v) {
	VIS_THIS(X11SinkDevice);
}

AtomTypeCls X11VideoAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagX11 && defined flagOGL && defined flagSCREEN)
String X11OglVideoAtomPipe::GetAction() {
	return "glx.video.pipe";
}

AtomTypeCls X11OglVideoAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11OGLVIDEOATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,VIDEO),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls X11OglVideoAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void X11OglVideoAtomPipe::Visit(Vis& v) {
	VIS_THIS(X11OglSinkDevice);
}

AtomTypeCls X11OglVideoAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagX11 && defined flagOGL && defined flagSCREEN)
String X11OglFboAtomSA::GetAction() {
	return "x11.ogl.fbo.standalone";
}

AtomTypeCls X11OglFboAtomSA::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11OGLFBOATOMSA;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,ORDER),0);
	t.AddOut(VD(OGL,RECEIPT),0);
	return t;
}

LinkTypeCls X11OglFboAtomSA::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void X11OglFboAtomSA::Visit(Vis& v) {
	VIS_THIS(X11OglSinkDevice);
}

AtomTypeCls X11OglFboAtomSA::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagX11 && defined flagSCREEN)
String X11SwVideoAtomPipe::GetAction() {
	return "x11.sw.video.pipe";
}

AtomTypeCls X11SwVideoAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11SWVIDEOATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,FBO),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls X11SwVideoAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void X11SwVideoAtomPipe::Visit(Vis& v) {
	VIS_THIS(X11SwSinkDevice);
}

AtomTypeCls X11SwVideoAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSDL2 && defined flagHAL && defined flagOGL && defined flagSCREEN)
String SdlOglFboAtomSA::GetAction() {
	return "sdl.fbo.standalone";
}

AtomTypeCls SdlOglFboAtomSA::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLOGLFBOATOMSA;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,ORDER),0);
	t.AddOut(VD(OGL,RECEIPT),0);
	return t;
}

LinkTypeCls SdlOglFboAtomSA::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void SdlOglFboAtomSA::Visit(Vis& v) {
	VIS_THIS(SdlOglScreenSinkDevice);
}

AtomTypeCls SdlOglFboAtomSA::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSDL2 && defined flagHAL && defined flagOGL && defined flagSCREEN)
String SdlOglFboPipe::GetAction() {
	return "sdl.fbo.sink";
}

AtomTypeCls SdlOglFboPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLOGLFBOPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,FBO),0);
	t.AddOut(VD(OGL,RECEIPT),0);
	return t;
}

LinkTypeCls SdlOglFboPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void SdlOglFboPipe::Visit(Vis& v) {
	VIS_THIS(SdlOglScreenSinkDevice);
}

AtomTypeCls SdlOglFboPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSDL2 && defined flagHAL && defined flagOGL && defined flagSCREEN)
String SdlOglFboAtom::GetAction() {
	return "sdl.fbo";
}

AtomTypeCls SdlOglFboAtom::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLOGLFBOATOM;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,ORDER),0);
	t.AddIn(VD(OGL,FBO),1);
	t.AddIn(VD(OGL,FBO),1);
	t.AddIn(VD(OGL,FBO),1);
	t.AddIn(VD(OGL,FBO),1);
	t.AddOut(VD(OGL,RECEIPT),0);
	t.AddOut(VD(OGL,FBO),1);
	t.AddOut(VD(OGL,FBO),1);
	t.AddOut(VD(OGL,FBO),1);
	t.AddOut(VD(OGL,FBO),1);
	return t;
}

LinkTypeCls SdlOglFboAtom::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void SdlOglFboAtom::Visit(Vis& v) {
	VIS_THIS(SdlOglScreenSinkDevice);
}

AtomTypeCls SdlOglFboAtom::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagGUI && defined flagHAL)
String UppEventAtomPipe::GetAction() {
	return "upp.event.pipe";
}

AtomTypeCls UppEventAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //UPPEVENTATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,EVENT),0);
	return t;
}

LinkTypeCls UppEventAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void UppEventAtomPipe::Visit(Vis& v) {
	VIS_THIS(UppEventsBase);
}

AtomTypeCls UppEventAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagGUI && defined flagHAL)
String CenterGuiFileSrc::GetAction() {
	return "center.gui.filesrc";
}

AtomTypeCls CenterGuiFileSrc::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //CENTERGUIFILESRC;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,GUI),0);
	return t;
}

LinkTypeCls CenterGuiFileSrc::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void CenterGuiFileSrc::Visit(Vis& v) {
	VIS_THIS(UppGuiFileSrc);
}

AtomTypeCls CenterGuiFileSrc::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagGUI && defined flagHAL)
String UppGuiSinkDevice::GetAction() {
	return "upp.gui.sink";
}

AtomTypeCls UppGuiSinkDevice::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //UPPGUISINKDEVICE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,GUI),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls UppGuiSinkDevice::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void UppGuiSinkDevice::Visit(Vis& v) {
	VIS_THIS(UppGuiSinkBase);
}

AtomTypeCls UppGuiSinkDevice::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSDL2 && defined flagHAL && defined flagSCREEN)
String SdlVideoAtomPipe::GetAction() {
	return "sdl.video.pipe";
}

AtomTypeCls SdlVideoAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLVIDEOATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,VIDEO),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls SdlVideoAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void SdlVideoAtomPipe::Visit(Vis& v) {
	VIS_THIS(SdlCenterScreenSinkDevice);
}

AtomTypeCls SdlVideoAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSDL2 && defined flagHAL && defined flagSCREEN)
String SdlProgAtomPipe::GetAction() {
	return "sdl.prog.pipe";
}

AtomTypeCls SdlProgAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLPROGATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,PROG),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls SdlProgAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void SdlProgAtomPipe::Visit(Vis& v) {
	VIS_THIS(SdlCenterScreenSinkDevice);
}

AtomTypeCls SdlProgAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagX11 && defined flagSCREEN)
String X11ProgAtomPipe::GetAction() {
	return "x11.prog.pipe";
}

AtomTypeCls X11ProgAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11PROGATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,PROG),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls X11ProgAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void X11ProgAtomPipe::Visit(Vis& v) {
	VIS_THIS(X11SinkDevice);
}

AtomTypeCls X11ProgAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagX11 && defined flagSCREEN)
String X11SwFboGuiProg::GetAction() {
	return "x11.sw.prog";
}

AtomTypeCls X11SwFboGuiProg::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11SWFBOGUIPROG;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,PROG),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,FBO),1);
	return t;
}

LinkTypeCls X11SwFboGuiProg::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void X11SwFboGuiProg::Visit(Vis& v) {
	VIS_THIS(X11SwFboProgBase);
}

AtomTypeCls X11SwFboGuiProg::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagX11 && defined flagSCREEN && defined flagOGL)
String X11OglFboGuiProg::GetAction() {
	return "x11.ogl.prog";
}

AtomTypeCls X11OglFboGuiProg::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11OGLFBOGUIPROG;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,PROG),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(OGL,FBO),1);
	return t;
}

LinkTypeCls X11OglFboGuiProg::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void X11OglFboGuiProg::Visit(Vis& v) {
	VIS_THIS(X11OglFboProgBase);
}

AtomTypeCls X11OglFboGuiProg::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
String SdlOglFboGuiProg::GetAction() {
	return "sdl.ogl.prog";
}

AtomTypeCls SdlOglFboGuiProg::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLOGLFBOGUIPROG;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,PROG),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(OGL,FBO),1);
	return t;
}

LinkTypeCls SdlOglFboGuiProg::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void SdlOglFboGuiProg::Visit(Vis& v) {
	VIS_THIS(SdlOglFboProgBase);
}

AtomTypeCls SdlOglFboGuiProg::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSDL2 && defined flagHAL && defined flagSCREEN)
String SdlVideoAtom::GetAction() {
	return "sdl.video";
}

AtomTypeCls SdlVideoAtom::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLVIDEOATOM;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,VIDEO),1);
	t.AddIn(VD(CENTER,VIDEO),1);
	t.AddIn(VD(CENTER,VIDEO),1);
	t.AddIn(VD(CENTER,VIDEO),1);
	t.AddOut(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,VIDEO),1);
	t.AddOut(VD(CENTER,VIDEO),1);
	t.AddOut(VD(CENTER,VIDEO),1);
	t.AddOut(VD(CENTER,VIDEO),1);
	return t;
}

LinkTypeCls SdlVideoAtom::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void SdlVideoAtom::Visit(Vis& v) {
	VIS_THIS(SdlCenterScreenSinkDevice);
}

AtomTypeCls SdlVideoAtom::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSDL2 && defined flagHAL && defined flagAUDIO)
String SdlAudioAtom::GetAction() {
	return "sdl.audio";
}

AtomTypeCls SdlAudioAtom::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLAUDIOATOM;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,AUDIO),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls SdlAudioAtom::GetLinkType() {
	return LINKTYPE(EXTERNAL_PIPE, PROCESS);
}

void SdlAudioAtom::Visit(Vis& v) {
	VIS_THIS(SdlAudioSinkDevice);
}

AtomTypeCls SdlAudioAtom::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
String SdlOglShaderAtom::GetAction() {
	return "sdl.ogl.fbo.side";
}

AtomTypeCls SdlOglShaderAtom::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLOGLSHADERATOM;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,ORDER),0);
	t.AddIn(VD(OGL,FBO),1);
	t.AddIn(VD(OGL,FBO),1);
	t.AddIn(VD(OGL,FBO),1);
	t.AddIn(VD(OGL,FBO),1);
	t.AddOut(VD(OGL,RECEIPT),0);
	t.AddOut(VD(OGL,FBO),1);
	t.AddOut(VD(OGL,FBO),1);
	t.AddOut(VD(OGL,FBO),1);
	t.AddOut(VD(OGL,FBO),1);
	return t;
}

LinkTypeCls SdlOglShaderAtom::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void SdlOglShaderAtom::Visit(Vis& v) {
	VIS_THIS(SdlOglShaderBase);
}

AtomTypeCls SdlOglShaderAtom::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
String SdlOglShaderAtomSA::GetAction() {
	return "ogl.fbo.source.standalone";
}

AtomTypeCls SdlOglShaderAtomSA::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLOGLSHADERATOMSA;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,ORDER),0);
	t.AddOut(VD(OGL,RECEIPT),0);
	return t;
}

LinkTypeCls SdlOglShaderAtomSA::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void SdlOglShaderAtomSA::Visit(Vis& v) {
	VIS_THIS(SdlOglShaderBase);
}

AtomTypeCls SdlOglShaderAtomSA::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
String SdlOglTextureSource::GetAction() {
	return "sdl.ogl.fbo.image";
}

AtomTypeCls SdlOglTextureSource::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLOGLTEXTURESOURCE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,ORDER),0);
	t.AddIn(VD(CENTER,VIDEO),1);
	t.AddOut(VD(OGL,RECEIPT),0);
	t.AddOut(VD(OGL,FBO),1);
	return t;
}

LinkTypeCls SdlOglTextureSource::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void SdlOglTextureSource::Visit(Vis& v) {
	VIS_THIS(SdlOglTextureBase);
}

AtomTypeCls SdlOglTextureSource::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
String SdlOglVolumeSource::GetAction() {
	return "sdl.ogl.fbo.volume";
}

AtomTypeCls SdlOglVolumeSource::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLOGLVOLUMESOURCE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,ORDER),0);
	t.AddIn(VD(CENTER,VOLUME),1);
	t.AddOut(VD(OGL,RECEIPT),0);
	t.AddOut(VD(OGL,FBO),1);
	return t;
}

LinkTypeCls SdlOglVolumeSource::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void SdlOglVolumeSource::Visit(Vis& v) {
	VIS_THIS(SdlOglTextureBase);
}

AtomTypeCls SdlOglVolumeSource::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
String SdlOglAudioSink::GetAction() {
	return "sdl.ogl.fbo.center.audio";
}

AtomTypeCls SdlOglAudioSink::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLOGLAUDIOSINK;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,ORDER),0);
	t.AddIn(VD(OGL,FBO),1);
	t.AddOut(VD(OGL,RECEIPT),0);
	t.AddOut(VD(CENTER,AUDIO),1);
	return t;
}

LinkTypeCls SdlOglAudioSink::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void SdlOglAudioSink::Visit(Vis& v) {
	VIS_THIS(SdlOglFboReaderBase);
}

AtomTypeCls SdlOglAudioSink::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
String SdlOglKeyboardSource::GetAction() {
	return "sdl.ogl.fbo.keyboard";
}

AtomTypeCls SdlOglKeyboardSource::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLOGLKEYBOARDSOURCE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,ORDER),0);
	t.AddIn(VD(OGL,FBO),1);
	t.AddIn(VD(OGL,FBO),1);
	t.AddIn(VD(OGL,FBO),1);
	t.AddIn(VD(OGL,FBO),1);
	t.AddOut(VD(OGL,RECEIPT),0);
	t.AddOut(VD(OGL,FBO),1);
	t.AddOut(VD(OGL,FBO),1);
	t.AddOut(VD(OGL,FBO),1);
	t.AddOut(VD(OGL,FBO),1);
	return t;
}

LinkTypeCls SdlOglKeyboardSource::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void SdlOglKeyboardSource::Visit(Vis& v) {
	VIS_THIS(SdlOglKeyboardBase);
}

AtomTypeCls SdlOglKeyboardSource::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSCREEN && defined flagSDL2 && defined flagOGL)
String SdlOglAudioSource::GetAction() {
	return "sdl.ogl.center.fbo.audio";
}

AtomTypeCls SdlOglAudioSource::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SDLOGLAUDIOSOURCE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,ORDER),0);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddOut(VD(OGL,RECEIPT),0);
	t.AddOut(VD(OGL,FBO),1);
	return t;
}

LinkTypeCls SdlOglAudioSource::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void SdlOglAudioSource::Visit(Vis& v) {
	VIS_THIS(SdlOglAudioBase);
}

AtomTypeCls SdlOglAudioSource::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagMIDI
String MidiFileReaderPipe::GetAction() {
	return "midi.file.reader.pipe";
}

AtomTypeCls MidiFileReaderPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //MIDIFILEREADERPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,MIDI),0);
	return t;
}

LinkTypeCls MidiFileReaderPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void MidiFileReaderPipe::Visit(Vis& v) {
	VIS_THIS(MidiFileReaderAtom);
}

AtomTypeCls MidiFileReaderPipe::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagMIDI
String MidiFileReader::GetAction() {
	return "midi.file.reader";
}

AtomTypeCls MidiFileReader::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //MIDIFILEREADER;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,MIDI),1);
	return t;
}

LinkTypeCls MidiFileReader::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void MidiFileReader::Visit(Vis& v) {
	VIS_THIS(MidiFileReaderAtom);
}

AtomTypeCls MidiFileReader::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagMIDI
String MidiFileReader16::GetAction() {
	return "midi.file.reader16";
}

AtomTypeCls MidiFileReader16::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //MIDIFILEREADER16;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	return t;
}

LinkTypeCls MidiFileReader16::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void MidiFileReader16::Visit(Vis& v) {
	VIS_THIS(MidiFileReaderAtom);
}

AtomTypeCls MidiFileReader16::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagMIDI
String MidiNullSink::GetAction() {
	return "midi.null.sink";
}

AtomTypeCls MidiNullSink::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //MIDINULLSINK;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,MIDI),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls MidiNullSink::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void MidiNullSink::Visit(Vis& v) {
	VIS_THIS(MidiNullAtom);
}

AtomTypeCls MidiNullSink::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagFLUIDLITE && defined flagAUDIO && defined flagMIDI) || (defined flagFLUIDSYNTH && defined flagAUDIO && defined flagMIDI)
String FluidsynthPipe::GetAction() {
	return "fluidsynth.pipe";
}

AtomTypeCls FluidsynthPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //FLUIDSYNTHPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls FluidsynthPipe::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void FluidsynthPipe::Visit(Vis& v) {
	VIS_THIS(FluidsynthInstrument);
}

AtomTypeCls FluidsynthPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSOFTINSTRU && defined flagAUDIO && defined flagMIDI)
String SoftInstrumentPipe::GetAction() {
	return "softinstru.pipe";
}

AtomTypeCls SoftInstrumentPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SOFTINSTRUMENTPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls SoftInstrumentPipe::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void SoftInstrumentPipe::Visit(Vis& v) {
	VIS_THIS(SoftInstrument);
}

AtomTypeCls SoftInstrumentPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSOFTSYNTH && defined flagAUDIO && defined flagMIDI)
String FmSynthPipe::GetAction() {
	return "fmsynth.pipe";
}

AtomTypeCls FmSynthPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //FMSYNTHPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls FmSynthPipe::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void FmSynthPipe::Visit(Vis& v) {
	VIS_THIS(FmSynthInstrument);
}

AtomTypeCls FmSynthPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagLV2 && defined flagAUDIO && defined flagMIDI)
String LV2InstrumentPipe::GetAction() {
	return "lv2.instrument.pipe";
}

AtomTypeCls LV2InstrumentPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //LV2INSTRUMENTPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls LV2InstrumentPipe::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void LV2InstrumentPipe::Visit(Vis& v) {
	VIS_THIS(LV2Instrument);
}

AtomTypeCls LV2InstrumentPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSOFTAUDIO && defined flagAUDIO && defined flagMIDI)
String CoreSynthPipe::GetAction() {
	return "coresynth.pipe";
}

AtomTypeCls CoreSynthPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //CORESYNTHPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls CoreSynthPipe::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void CoreSynthPipe::Visit(Vis& v) {
	VIS_THIS(CoreSynthInstrument);
}

AtomTypeCls CoreSynthPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSOFTAUDIO && defined flagAUDIO && defined flagMIDI)
String CoreDrummerPipe::GetAction() {
	return "coredrummer.pipe";
}

AtomTypeCls CoreDrummerPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //COREDRUMMERPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),1);
	return t;
}

LinkTypeCls CoreDrummerPipe::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void CoreDrummerPipe::Visit(Vis& v) {
	VIS_THIS(CoreDrummerInstrument);
}

AtomTypeCls CoreDrummerPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSOFTAUDIO && defined flagAUDIO)
String CoreEffectPipe::GetAction() {
	return "corefx.pipe";
}

AtomTypeCls CoreEffectPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //COREEFFECTPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,AUDIO),0);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls CoreEffectPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void CoreEffectPipe::Visit(Vis& v) {
	VIS_THIS(AudioCoreEffect);
}

AtomTypeCls CoreEffectPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagSOFTAUDIO && defined flagAUDIO)
String CoreEffectAtom::GetAction() {
	return "corefx.atom";
}

AtomTypeCls CoreEffectAtom::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //COREEFFECTATOM;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls CoreEffectAtom::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void CoreEffectAtom::Visit(Vis& v) {
	VIS_THIS(AudioCoreEffect);
}

AtomTypeCls CoreEffectAtom::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagLV2 && defined flagAUDIO)
String LV2EffectPipe::GetAction() {
	return "lv2.effect.pipe";
}

AtomTypeCls LV2EffectPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //LV2EFFECTPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,AUDIO),0);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls LV2EffectPipe::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void LV2EffectPipe::Visit(Vis& v) {
	VIS_THIS(LV2Effect);
}

AtomTypeCls LV2EffectPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagBUILTIN_PORTMIDI && defined flagMIDI) || (defined flagPORTMIDI && defined flagMIDI)
String PortmidiPipe::GetAction() {
	return "midi.src.portmidi";
}

AtomTypeCls PortmidiPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //PORTMIDIPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,MIDI),0);
	return t;
}

LinkTypeCls PortmidiPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void PortmidiPipe::Visit(Vis& v) {
	VIS_THIS(PortmidiSource);
}

AtomTypeCls PortmidiPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagBUILTIN_PORTMIDI && defined flagMIDI) || (defined flagPORTMIDI && defined flagMIDI)
String PortmidiSend::GetAction() {
	return "midi.src.side.portmidi";
}

AtomTypeCls PortmidiSend::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //PORTMIDISEND;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	return t;
}

LinkTypeCls PortmidiSend::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void PortmidiSend::Visit(Vis& v) {
	VIS_THIS(PortmidiSource);
}

AtomTypeCls PortmidiSend::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagAUDIO
String CoreAudioFileOut::GetAction() {
	return "audio.file.writer";
}

AtomTypeCls CoreAudioFileOut::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //COREAUDIOFILEOUT;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,AUDIO),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls CoreAudioFileOut::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void CoreAudioFileOut::Visit(Vis& v) {
	VIS_THIS(CoreAudioSink);
}

AtomTypeCls CoreAudioFileOut::GetType() const {
	return GetAtomType();
}
#endif



END_UPP_NAMESPACE
