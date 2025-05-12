#ifndef _IMedia_MediaAtomBase_h_
#define _IMedia_MediaAtomBase_h_


NAMESPACE_UPP


template <class Backend>
class MediaAtomBaseT :
	public Atom
{
	using FileInput = typename Backend::FileInput;
	using MediaStreamThread = typename Backend::MediaStreamThread;
	using VideoInputFramePtr = typename Backend::VideoInputFrameRef;
	using AudioInputFramePtr = typename Backend::AudioInputFrameRef;
	
	typedef enum {
		INVALID_MODE,
		AUDIO_ONLY,
		VIDEO_ONLY,
		AUDIOVIDEO,
	} Mode;
	
	FileInput			file_in;
	Mode				mode = INVALID_MODE;
	
	String last_error;
	MediaStreamThread vi;
	Size def_cap_sz;
	double time = 0;
	int audio_ch = -1, video_ch = -1;
	int def_cap_fps = 25;
	bool stops_machine = false;
	bool vflip = false;
	bool video_packet_ready = false, audio_packet_ready = false;
	String filepath;
	
	
	bool RealizeAudioFormat();
	bool RealizeVideoFormat();
	
public:
	typedef MediaAtomBaseT CLASSNAME;
	MediaAtomBaseT(MetaNode& n);
	
	bool Initialize(const Eon::WorldState& ws) override;
	void Uninitialize() override;
	//void Forward(FwdScope& fwd) override;
	bool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override;
	bool IsReady(PacketIO& io) override;
	bool PostInitialize() override;
	void Update(double dt) override;
	
	void Visit(Vis& v) override {v % file_in; VIS_THIS(Atom);}
	bool LoadFileAny(String path);
	void OnError();
	void OnStop();
	
	
	void SetError(String s);
	String GetLastError() const {return last_error;}
	
	MediaStreamThread& GetInput() {return vi;}
	
	
	Callback WhenStopped;
	
	
};



#ifdef flagFFMPEG
using FfmpegSourceDevice = MediaAtomBaseT<FfmpegMedia>;
using FfmpegSinkDevice = MediaAtomBaseT<FfmpegMedia>;
#endif

END_UPP_NAMESPACE

#endif
