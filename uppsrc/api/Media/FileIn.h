#ifndef _IMedia_FileIn_h_
#define _IMedia_FileIn_h_

NAMESPACE_UPP



template <class Backend> class FileInputT;



template <class Backend>
class AudioFrameQueueT :
	public AudioInputFrameT<Backend>
{
	off32_gen	gen;
	
public:
	using FileInput = typename Backend::FileInput;
	using Base = AudioInputFrameT<Backend>;
	using AVFrame = typename Backend::AVFrame;
	using AVSampleFormat = typename Backend::AVSampleFormat;
	//RTTI_DECL1(AudioFrameQueueT, Base)
	
	AudioFrameQueueT() {}
	
	void				FillAudioBuffer(double time_pos, AVFrame* frame);
	void				Visit(Vis& v) {}
	
	void				Close() override;
	void				FillBuffer() override;
	
	int64				GetCurrentOffset() const {return gen.GetCurrent().value;}
	
};




template <class Backend>
class VideoFrameQueueT :
	public VideoInputFrameT<Backend>
{
	using AVFrame = typename Backend::AVFrame;
	using AVCodecContextPtr = typename Backend::AVCodecContextRef;
	using ImgConvContextPtr = typename Backend::ImgConvContextRef;
	using Frame = typename Backend::Frame;
	using Recycler = Upp::Recycler<Frame,true>;
	using Pool = RecyclerPool<Frame,true>;
	
	struct SwsContext*		img_convert_ctx = 0;
	LinkedList<Recycler>	frames;
	Pool					pool;
	int						min_buf_samples = MIN_AUDIO_BUFFER_SAMPLES;
	off32_gen				gen;
	
	
public:
	using FileInput = typename Backend::FileInput;
	using Base = VideoInputFrameT<Backend>;
	//RTTI_DECL1(VideoFrameQueueT, Base)
	~VideoFrameQueueT() {Clear();}
	
	void				Visit(Vis& v) {}
	void				Init(AVCodecContextPtr& ctx);
	
	void				Close() override;
	void				Clear() override;
	void				FillBuffer() override;
	
	void				Process(double time_pos, AVFrame* frame, bool vflip=true);
	void				DropFrames(int i);
	
};



template <class Backend>
class FileChannelT
{
	
protected:
	#ifdef flagMSC
	using FileInput = typename FileInputT<Backend>;
	#else
	using FileInput = class FileInputT<Backend>;
	#endif
	using AVFrame = typename Backend::AVFrame;
	using AVCodecContextPtr = typename Backend::AVCodecContextRef;
	using AVFormatContext = typename Backend::AVFormatContext;
	using AVCodecParserContextPtr = typename Backend::AVCodecParserContextRef;
	using AVCodec = typename Backend::AVCodec;
	using AVPacket = typename Backend::AVPacket;
	using AVStream = typename Backend::AVStream;
	using AVCodecParameters = typename Backend::AVCodecParameters;
	using AVDictionary = typename Backend::AVDictionary;
	friend FileInput;
	
	AVFormatContext* file_fmt_ctx = NULL;
	AVCodecContextPtr codec_ctx = NULL;
	AVCodecParserContextPtr parser = NULL;
	AVFrame* frame = NULL;
	AVCodec codec = NULL;
	double frame_pos_time = 0;
	int stream_i = -1;
	bool is_open = false;
	String errstr;
	
	
	int DecodePacket(AVPacket& pkt, int *got_frame);
	
public:
	~FileChannelT() {Clear();}
	bool IsOpen() const {return is_open;}
	
	void Clear();
	void ClearDevice();
	
	bool OpenVideo(AVFormatContext* file_fmt_ctx, ValueFormat& fmt);
	bool OpenAudio(AVFormatContext* file_fmt_ctx, ValueFormat& fmt);
	bool OpenDevice();
	
	bool ReadFrame(AVPacket& pkt);
	double GetSeconds() const {return frame_pos_time;}
	
};



template <class Backend>
class FileInputT :
	public PacketBufferParent
{
	using AudioFrameQueue = typename Backend::AudioFrameQueue;
	using VideoFrameQueue = typename Backend::VideoFrameQueue;
	using FileChannel = typename Backend::FileChannel;
	using AVFormatContext = typename Backend::AVFormatContext;
	using AVPacket = typename Backend::AVPacket;
	using AVCodecParameters = typename Backend::AVCodecParameters;
	
	AudioFrameQueue aframe;
	VideoFrameQueue vframe;
	
	bool has_audio;
	bool has_video;
	bool is_dev_open;
	String path;
	String errstr;
	FileChannel v;
	FileChannel a;
	AVFormatContext* file_fmt_ctx = NULL;
	AVPacket* pkt = 0;
	bool is_eof = false;
	bool pkt_ref = false;
	
	bool HasMediaOpen() const {return has_video || has_audio;}
	void ClearDevice();
	void ClearPacketData();
	void ClearPacket();
	void InitPacket();
	bool IsFrameLoaded() const {return pkt_ref;}
	bool ReadFrame();
	bool ProcessVideoFrame();
	bool ProcessAudioFrame();
	
	
public:
	//RTTI_DECL_R0(FileInputT)
	FileInputT();
	~FileInputT() {Clear();}
	
	bool						IsEof() const;
	void						Visit(Vis& v) {vis % aframe % vframe;}
	void						Clear();
	double						GetSeconds() const;
	bool						IsAudioOpen() const;
	
	void						FillVideoBuffer();
	void						FillAudioBuffer();
	AudioFrameQueue&			GetAudio() {return aframe;}
	VideoFrameQueue&			GetVideo() {return vframe;}
	
	bool						IsOpenAudio() const {return has_audio;}
	bool						IsOpenVideo() const {return has_video;}
	bool						IsOpen() const;
	bool						Open();
	void						Close();
	bool						OpenFile(String path);
	
	String	GetLastError() const;
	
	void	DropVideoFrames(int frames);
	String						GetPath() const;
	
	
	Callback WhenStopped;
	
};



END_UPP_NAMESPACE

#endif
