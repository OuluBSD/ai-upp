#include "Graphics.h"
#include <Core/TextParsing/TextParsing.h>


NAMESPACE_UPP

template <class Gfx>
bool ShaderBaseT<Gfx>::Initialize(const WorldState& ws) {
	
	if (!this->bf.Initialize(*this, ws))
		return false;
	
	int queue_size = 1;
	
	if (this->bf.IsAudio())
		queue_size = DEFAULT_AUDIO_QUEUE_SIZE;
	
	this->SetQueueSize(queue_size);
	
	return true;
}

template <class Gfx>
bool ShaderBaseT<Gfx>::PostInitialize() {
	return true;
}

template <class Gfx>
bool ShaderBaseT<Gfx>::Start() {
	if (!this->bf.ImageInitialize(false, Size(0,0)))
		return false;
	
	return this->bf.PostInitialize();
}

template <class Gfx>
void ShaderBaseT<Gfx>::Uninitialize() {
	
}

template <class Gfx>
bool ShaderBaseT<Gfx>::IsReady(PacketIO& io) {
	bool b = io.full_src_mask == 0;
	RTLOG("OglShaderBase::IsReady: " << (b ? "true" : "false"));
	return b;
}

template <class Gfx>
bool ShaderBaseT<Gfx>::Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	
	ValueFormat fmt = out.GetFormat();
	
	if (fmt.vd.val == ValCls::RECEIPT) {
		// pass
	}
	else if (fmt.vd.val == ValCls::FBO) {
		this->bf.GetBuffer().Process(*this->last_cfg);
		ASSERT(fmt.IsValid());
		
		InternalPacketData& data = out.GetData<InternalPacketData>();
		this->bf.GetBuffer().StoreOutputLink(data);
		RTLOG("ShaderBaseT::Send: 0, " << out.ToString());
		
	}
	else {
		TODO
	}
	
	return true;
}

template <class Gfx>
bool ShaderBaseT<Gfx>::Recv(int sink_ch, const Packet& in) {
	RTLOG("ShaderBaseT::Recv: " << sink_ch << ": " << in->ToString());
	bool succ = true;
	
	ValueFormat in_fmt = in->GetFormat();
	if (in_fmt.vd == VD(OGL,FBO)) {
		int channels = in_fmt.fbo.GetChannels();
		
		int base = this->GetSink()->GetSinkCount() > 1 ? 1 : 0;
		if (in->IsData<InternalPacketData>()) {
			succ = this->bf.GetBuffer().LoadInputLink(sink_ch - base, in->GetData<InternalPacketData>()) && succ;
		}
		else {
			RTLOG("OglShaderBase::ProcessPackets: cannot handle packet: " << in->ToString());
		}
	}
	
	return succ;
}

template <class Gfx>
void ShaderBaseT<Gfx>::Finalize(RealtimeSourceConfig& cfg) {
	this->last_cfg = &cfg;
}



template <class Gfx>
bool TextureBaseT<Gfx>::Initialize(const WorldState& ws) {
	
	String f = ws.Get(".filter");
	if (!f.IsEmpty()) {
		if (f == "nearest")
			filter = GVar::FILTER_NEAREST;
		else if (f == "linear")
			filter = GVar::FILTER_LINEAR;
		else if (f == "mipmap")
			filter = GVar::FILTER_MIPMAP;
		else {
			LOG("OglTextureBase::Initialize: error: invalid filter string '" << f << "'");
			return false;
		}
	}
	
	String w = ws.Get(".wrap");
	if (!w.IsEmpty()) {
		if (w == "clamp")
			wrap = GVar::WRAP_CLAMP;
		else if (w == "repeat")
			wrap = GVar::WRAP_REPEAT;
		else {
			LOG("OglTextureBase::Initialize: error: invalid wrap string '" << w << "'");
			return false;
		}
	}
	
	String arg_filepath = ws.Get(".filepath");
	if (!arg_filepath.IsEmpty())
		preload_path = RealizeFilepathArgument(arg_filepath);
	preload_vflip = ws.Get(".vflip") == "true";
	preload_swap_tb = ws.Get(".swap_top_bottom") == "true";
	preload_cubemap = ws.Get(".cubemap") == "true";
	
	return true;
}

template <class Gfx>
bool TextureBaseT<Gfx>::PostInitialize() {
	
	if (!preload_path.IsEmpty()) {
		if (!PreloadTextureFromFile())
			return false;
	}
	
	return true;
}

template <class Gfx>
void TextureBaseT<Gfx>::Uninitialize() {
	
}

template <class Gfx>
bool TextureBaseT<Gfx>::IsReady(PacketIO& io) {
	bool buffer_ready = this->bf.GetBuffer().IsSingleInitialized();
	bool has_video = (io.active_sink_mask & 0b10) || preload_pending || buffer_ready;
	bool b = io.full_src_mask == 0 && has_video;
	return b;
}

template <class Gfx>
bool TextureBaseT<Gfx>::Recv(int sink_ch, const Packet& p) {
	PacketValue& in = *p;
	const Vector<byte>& in_data = in.GetData();
	
	ValueFormat in_fmt = in.GetFormat();
	if (in_fmt.IsOrder())
		return true;
	
	ASSERT(in_fmt.IsVideo() || in_fmt.IsVolume());
	Size3 sz;
	int channels;
	if (in_fmt.IsVideo()) {
		sz			= in_fmt.vid.GetSize();
		channels	= in_fmt.vid.GetChannels();
		
		if (in_fmt.vid.IsCubemap()) {
			if (in.seq == 0) {
				loading_cubemap = true;
				cubemap.Clear();
			}
			
			if (loading_cubemap) {
				if (in.seq == cubemap.GetCount())
					cubemap.Add(p);
				
				if (cubemap.GetCount() < 6)
					return true;
				
				RTLOG("TextureBaseT<Gfx>::Recv: cubemap receiving succeeded");
			}
		}
	}
	else if (in_fmt.IsVolume()) {
		sz			= in_fmt.vol.GetSize();
		channels	= in_fmt.vol.GetChannels();
	}
	else
		TODO
	
	
	auto& buf = this->bf.GetBuffer();
	auto& stage = buf.InitSingle();
	if (!stage.IsInitialized()) {
		ASSERT(sz.cx > 0 && sz.cy > 0);
		auto& fb = stage.fb[0];
		fb.is_win_fbo = false;
		fb.size = sz.GetSize2();
		fb.depth = sz.cz;
		fb.channels = channels;
		fb.sample = GVar::SAMPLE_FLOAT;
		fb.filter = this->filter;
		fb.wrap = this->wrap;
		fb.fps = 0;
		
		if (loading_cubemap) {
			ASSERT(cubemap.GetCount() == 6);
			if (!stage.InitializeCubemap(
					fb.size,
					fb.channels,
					GVar::SAMPLE_U8,
					cubemap[0]->GetData(),
					cubemap[1]->GetData(),
					cubemap[2]->GetData(),
					cubemap[3]->GetData(),
					cubemap[4]->GetData(),
					cubemap[5]->GetData()
				))
				return false;
		}
		else if (sz.cz == 0) {
			if (!stage.InitializeTexture(
				fb.size,
				fb.channels,
				GVar::SAMPLE_U8,
				&*in_data.Begin(),
				in_data.GetCount()))
				return false;
		}
		else {
			if (!stage.InitializeVolume(
				Size3(fb.size.cx, fb.size.cy, fb.depth),
				fb.channels,
				GVar::SAMPLE_U8,
				in_data))
				return false;
		}
	}
	else {
		if (sz.cz == 0) {
			const Vector<byte>& data = in.GetData();
			stage.ReadTexture(
				sz.GetSize2(),
				channels,
				GVar::SAMPLE_U8,
				data.Begin(), data.GetCount());
		}
		else {
			stage.ReadTexture(
				sz,
				channels,
				GVar::SAMPLE_U8,
				in.GetData());
		}
	}
	
	return true;
}

template <class Gfx>
bool TextureBaseT<Gfx>::Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	if (preload_pending && !UploadPreloadedData())
		return false;
	if (src_ch == 0) {
		if (!this->bf.GetBuffer().IsSingleInitialized())
			return true;
		InternalPacketData& data = out.SetData<InternalPacketData>();
		this->GetBuffer().StoreOutputLink(data);
		return true;
	}
	
	if (src_ch >= 1) {
		// non-primary channel (src_ch>0) is allowed to not send packets
		if (!this->bf.GetBuffer().IsSingleInitialized())
			return false;
		
		ValueFormat fmt = out.GetFormat();
		
		if (fmt.vd == VD(OGL,FBO)) {
			InternalPacketData& data = out.GetData<InternalPacketData>();
			this->GetBuffer().StoreOutputLink(data);
			RTLOG("OglTextureBase::ProcessPackets: 0, " << src_ch << ": " << out.ToString());
		}
	}
	
	return true;
}

template <class Gfx>
void TextureBaseT<Gfx>::Visit(Vis& v) {VIS_THIS(BufferBase);}

template <class Gfx>
bool TextureBaseT<Gfx>::NegotiateSinkFormat(LinkBase& link, int sink_ch, const ValueFormat& new_fmt) {
	// accept all valid video formats for now
	if (new_fmt.IsValid() && (new_fmt.IsVideo() || new_fmt.IsVolume())) {
		ISinkPtr sink = this->GetSink();
		ValueBase& val = sink->GetValue(sink_ch);
		val.SetFormat(new_fmt);
		return true;
	}
	return false;
}

template <class Gfx>
bool TextureBaseT<Gfx>::PreloadTextureFromFile() {
	Vector<Image> imgs;
	if (!LoadImages(imgs))
		return false;
	if (imgs.IsEmpty())
		return false;
	if (!StorePreloadedImages(imgs))
		return false;
	preload_path.Clear();
	return true;
}

template <class Gfx>
bool TextureBaseT<Gfx>::LoadImages(Vector<Image>& out) const {
	if (preload_path.IsEmpty())
		return true;

	if (preload_cubemap) {
		String dir = GetFileDirectory(preload_path);
		String title = GetFileTitle(preload_path);
		String ext = GetFileExt(preload_path);
		LOG("TextureBaseT::LoadImages: loading cubemap from path=\"" << preload_path << "\" title=\"" << title << "\"");
		for (int i = 0; i < 6; i++) {
			String path = i == 0 ? preload_path : AppendFileName(dir, title + "_" + IntStr(i) + ext);
			LOG("TextureBaseT::LoadImages: loading cubemap face " << i << " from \"" << path << "\"");
			Image img;
			if (!LoadImageFile(path, img))
				return false;
			out.Add(img);
		}
		LOG("TextureBaseT::LoadImages: loaded " << out.GetCount() << " cubemap faces successfully");
	}
	else {
		Image img;
		if (!LoadImageFile(preload_path, img))
			return false;
		out.Add(img);
	}
	
	if (!ApplyPreloadTransforms(out))
		return false;
	
	return true;
}

template <class Gfx>
bool TextureBaseT<Gfx>::LoadImageFile(const String& path, Image& out) const {
	String ext = ToLower(GetFileExt(path));
	if (ext == ".jpg" || ext == ".jpeg")
		out = JPGRaster().LoadFile(path);
	else if (ext == ".png")
		out = PNGRaster().LoadFile(path);
	else
		out = StreamRaster::LoadFileAny(path);
	if (out.IsEmpty()) {
		LOG("TextureBaseT: error: empty image: " << path);
		return false;
	}
	return true;
}

template <class Gfx>
bool TextureBaseT<Gfx>::ApplyPreloadTransforms(Vector<Image>& imgs) const {
	if (preload_vflip) {
		for (Image& img : imgs)
			img = MirrorVertical(img);
	}
	if (preload_cubemap && preload_swap_tb && imgs.GetCount() >= 4)
		Swap(imgs[2], imgs[3]);
	return true;
}

template <class Gfx>
bool TextureBaseT<Gfx>::StorePreloadedImages(const Vector<Image>& imgs) {
	if (imgs.IsEmpty())
		return false;
	preload_size = imgs[0].GetSize();
	preload_bytes.SetCount(imgs.GetCount());
	for (int i = 0; i < imgs.GetCount(); i++)
		DataFromImage(imgs[i], preload_bytes[i]);
	preload_pending = true;
	return true;
}

template <class Gfx>
bool TextureBaseT<Gfx>::UploadPreloadedData() {
	if (!preload_pending)
		return true;

	LOG("TextureBaseT::UploadPreloadedData: uploading preloaded texture, cubemap=" << (preload_cubemap ? "true" : "false"));

	auto& buf = this->bf.GetBuffer();
	auto& stage = buf.InitSingle();
	auto& fb = stage.fb[0];
	fb.is_win_fbo = false;
	fb.depth = 0;
	fb.channels = 4;
	fb.sample = GVar::SAMPLE_FLOAT;
	fb.filter = this->filter;
	fb.wrap = this->wrap;
	fb.fps = 0;
	fb.size = preload_size;

	bool ok = true;
	if (preload_cubemap) {
		if (preload_bytes.GetCount() != 6) {
			LOG("TextureBaseT: error: expected 6 cubemap faces for '" << preload_path << "'");
			ok = false;
		}
		else {
			LOG("TextureBaseT::UploadPreloadedData: calling InitializeCubemap with 6 faces, size=" << fb.size);
			ok = stage.InitializeCubemap(
				fb.size,
				fb.channels,
				GVar::SAMPLE_U8,
				preload_bytes[0],
				preload_bytes[1],
				preload_bytes[2],
				preload_bytes[3],
				preload_bytes[4],
				preload_bytes[5]);
			LOG("TextureBaseT::UploadPreloadedData: InitializeCubemap " << (ok ? "succeeded" : "FAILED"));
		}
	}
	else {
		ok = stage.InitializeTexture(
			fb.size,
			fb.channels,
			GVar::SAMPLE_U8,
			preload_bytes[0].Begin(),
			preload_bytes[0].GetCount());
	}
	
	if (!ok) {
		LOG("TextureBaseT: error: failed to upload preloaded texture: " << preload_path);
		return false;
	}
	
	preload_pending = false;
	preload_bytes.Clear();
	return true;
}












template <class Gfx>
bool FboReaderBaseT<Gfx>::Initialize(const WorldState& ws) {
	ISourcePtr src = this->GetSource();
	ValueFormat out_fmt = src->GetSourceValue(src->GetSourceCount()-1).GetFormat();
	if (out_fmt.IsAudio()) {
		this->SetQueueSize(DEFAULT_AUDIO_QUEUE_SIZE);
	}
	
	{
		int sample_rate = ws.GetInt(".samplerate", 1024);
		
		ISourcePtr src = this->GetSource();
		int c = src->GetSourceCount();
		ValueBase& v = src->GetSourceValue(c-1);
		ValueFormat fmt = v.GetFormat();
		if (!fmt.IsAudio())
			return false;
		
		AudioFormat& afmt = fmt;
		afmt.SetType(BinarySample::U16_LE);
		afmt.SetSampleRate(sample_rate);
		
		v.SetFormat(fmt);
	}
	
	return true;
}

template <class Gfx>
bool FboReaderBaseT<Gfx>::PostInitialize() {
	return true;
}

template <class Gfx>
void FboReaderBaseT<Gfx>::Uninitialize() {
	
}

template <class Gfx>
bool FboReaderBaseT<Gfx>::IsReady(PacketIO& io) {
	dword iface_sink_mask = this->iface.GetSinkMask();
	bool b = io.active_sink_mask == iface_sink_mask && io.full_src_mask == 0;
	RTLOG("OglFboReaderBase::IsReady: " << (b ? "true" : "false") << " (" << io.nonempty_sinks << ", " << io.sinks.GetCount() << ", " << HexStr(iface_sink_mask) << ", " << HexStr(io.active_sink_mask) << ")");
	return b;
}

template <class Gfx>
bool FboReaderBaseT<Gfx>::Recv(int sink_ch, const Packet& in) {
	ValueFormat fmt = in->GetFormat();
		
	if (fmt.IsFbo()) {
		InternalPacketData& v = in->GetData<InternalPacketData>();
		if (v.IsText("gfxbuf")) {
			src_buf = CastPtr<BufferStage>(static_cast<GfxBufferStage*>(v.ptr));
			ASSERT(src_buf);
		}
		else {
			TODO
			return false;
		}
	}
	
	return true;
}

template <class Gfx>
bool FboReaderBaseT<Gfx>::Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	ValueFormat fmt = out.GetFormat();
	if (fmt.IsReceipt()) {
		
	}
	else if (fmt.IsAudio()) {
		if (!src_buf)
			return false;
		
		AudioFormat& afmt = fmt;
		
		auto& fb = src_buf->GetFramebuffer();
		int afmt_size = afmt.GetSize();
		ASSERT(fb.size.cx == afmt.sample_rate && fb.size.cy == 1 && fb.channels == afmt_size);
		int len = afmt.sample_rate * fb.channels * GVar::GetSampleSize(fb.sample);
		ASSERT(len > 0);
		Vector<byte>& out_data = out.Data();
		out_data.SetCount(len);
		
		NativeFrameBufferConstPtr frame_buf = fb.GetReadFramebuffer();
		ASSERT(frame_buf);
		Gfx::BindFramebufferRO(frame_buf);
		Gfx::ReadPixels(0, 0, afmt.sample_rate, 1, fb.sample, fb.channels, out_data.Begin());
		Gfx::UnbindFramebuffer();
		
		src_buf = 0;
	}
	else TODO
	
	return true;
}

template <class Gfx>
bool FboReaderBaseT<Gfx>::NegotiateSinkFormat(LinkBase& link, int sink_ch, const ValueFormat& new_fmt) {
	
	TODO
	
}

template <class Gfx>
void FboReaderBaseT<Gfx>::Visit(Vis& v) {
	VIS_THIS(BufferBase);
}















template <class Gfx>
bool KeyboardBaseT<Gfx>::Initialize(const WorldState& ws) {
	
	target = ws.Get(".target");
	if (target.IsEmpty()) {
		LOG("EventStateBase::Initialize: error: target state argument is required");
		return false;
	}
	String normalized = NormalizePathSeparators(target);
	auto* state = this->val.template FindOwnerWithPathAndCast<EnvState>(normalized);
	if (!state && normalized != target)
		state = this->val.template FindOwnerWithPathAndCast<EnvState>(target);
	if (!state) {
		LOG("EventStateBase::Initialize: error: state '" << target << "' not found in parent space: " << this->val.GetPath());
		return false;
	}
	
	FboKbd::KeyVec& data = state->template Set<FboKbd::KeyVec>(KEYBOARD_PRESSED);
	data.SetAll(false);
	
	return true;
}

template <class Gfx>
bool KeyboardBaseT<Gfx>::PostInitialize() {
	
	return true;
}

template <class Gfx>
void KeyboardBaseT<Gfx>::Uninitialize() {
	
}

template <class Gfx>
bool KeyboardBaseT<Gfx>::IsReady(PacketIO& io) {
	if (!state) return false;
	ASSERT(io.srcs.GetCount() >= 2);
	if (io.srcs.GetCount() < 2) return false;
	
	dword iface_sink_mask = this->iface.GetSinkMask();
	bool b = io.active_sink_mask == iface_sink_mask && io.full_src_mask == 0;
	RTLOG("KeyboardBaseT<Gfx>::IsReady: " << (b ? "true" : "false") << " (" << io.nonempty_sinks << ", " << io.sinks.GetCount() << ", " << HexStr(iface_sink_mask) << ", " << HexStr(io.active_sink_mask) << ")");
	return b;
}

template <class Gfx>
bool KeyboardBaseT<Gfx>::Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	RTLOG("KeyboardBaseT<Gfx>::Send");
	auto& buf = this->bf.GetBuffer();
	auto& stage = buf.InitSingle();
	
	ValueFormat fmt = out.GetFormat();
	if (fmt.IsFbo()) {
		Size sz(FboKbd::key_tex_w, FboKbd::key_tex_h);
		int channels = 1;
		FboKbd::KeyVec& data = state->Set<FboKbd::KeyVec>(KEYBOARD_PRESSED);
		
		//LOG("KeyboardBaseT<Gfx>::Send: " << HexStr(data.GetHashValue()));
		
		if (!stage.IsInitialized()) {
			ASSERT(sz.cx > 0 && sz.cy > 0);
			auto& fb = stage.fb[0];
			fb.is_win_fbo = false;
			fb.size = sz;
			fb.channels = channels;
			fb.sample = GVar::SAMPLE_FLOAT;
			fb.fps = 0;
			
			if (!stage.InitializeTexture(
				Size(sz.cx, sz.cy),
				channels,
				GVar::SAMPLE_U8,
				data.Get(),
				data.GetCount() * sizeof(byte)))
				return false;
		}
		else {
			stage.ReadTexture(
				sz,
				channels,
				GVar::SAMPLE_U8,
				data.Get(),
				data.GetCount() * sizeof(byte));
		}
		
		
		InternalPacketData& d = out.GetData<InternalPacketData>();
		this->GetBuffer().StoreOutputLink(d);
		RTLOG("KeyboardBaseT<Gfx>::Send: 0, " << src_ch << ": " << out.ToString());
		
	}
	
	return true;
}












template <class Gfx>
bool AudioBaseT<Gfx>::Initialize(const WorldState& ws) {
	
	return true;
}

template <class Gfx>
bool AudioBaseT<Gfx>::PostInitialize() {
	
	// e.g. opengl doesn't support 2-channel float input always, so request 16-bit uint
	ISinkPtr sink = this->GetSink();
	for(int i = 0; i < sink->GetSinkCount(); i++) {
		ValueBase& v = sink->GetValue(i);
		ValueFormat fmt = v.GetFormat();
		if (fmt.IsAudio()) {
			AudioFormat& afmt = fmt;
			afmt.SetType(SoundSample::U16_LE);
			if (!this->GetLink()->NegotiateSinkFormat(i, fmt))
				return false;
		}
	}
	
	return true;
}

template <class Gfx>
void AudioBaseT<Gfx>::Uninitialize() {
	
}

template <class Gfx>
bool AudioBaseT<Gfx>::IsReady(PacketIO& io) {
	dword iface_sink_mask = this->iface.GetSinkMask();
	bool b = io.active_sink_mask == iface_sink_mask && io.full_src_mask == 0;
	RTLOG("AudioBaseT<Gfx>::IsReady: " << (b ? "true" : "false") << " (" << io.nonempty_sinks << ", " << io.sinks.GetCount() << ", " << HexStr(iface_sink_mask) << ", " << HexStr(io.active_sink_mask) << ")");
	return b;
}

template <class Gfx>
bool AudioBaseT<Gfx>::Recv(int sink_ch, const Packet& p) {
	RTLOG("AudioBaseT<Gfx>::Recv");
	
	const PacketValue& in = *p;
	ValueFormat fmt = in.GetFormat();
	if (fmt.IsAudio()) {
		auto& buf = this->bf.GetBuffer();
		auto& stage = buf.InitSingle();
		AudioFormat& afmt = fmt;
		
		ValueFormat sink_fmt = this->GetSink()->GetValue(sink_ch).GetFormat();
		AudioFormat& sink_afmt = sink_fmt;
		ASSERT_(sink_afmt.type == afmt.type, "packet conversion did not happen");
		
		Size sz(afmt.sample_rate, 1);
		int channels = afmt.GetSize();
		const Vector<byte>& data = in.GetData();
		
		GVar::Sample sample = GetGVarType(afmt.type);
		int sample_size = GVar::GetSampleSize(sample);
		
		if (!stage.IsInitialized()) {
			ASSERT(sz.cx > 0 && sz.cy > 0);
			auto& fb = stage.fb[0];
			fb.is_win_fbo = false;
			fb.is_audio = true;
			fb.size = sz;
			fb.channels = channels;
			fb.sample = sample;
			fb.fps = 0;
			
			// opengl fails with 2 channel internal format, so force it to 3
			//if (fb.channels == 2)
			//	fb.channels = 4;
			
			if (!stage.InitializeTexture(
				Size(sz.cx, sz.cy),
				channels,
				sample,
				&*data.Begin(),
				data.GetCount()))
				return false;
		}
		else {
			stage.ReadTexture(
				sz,
				channels,
				sample,
				&*data.Begin(),
				data.GetCount());
		}
	}
	
	return true;
}

template <class Gfx>
bool AudioBaseT<Gfx>::Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	RTLOG("AudioBaseT<Gfx>::Send");
	ValueFormat fmt = out.GetFormat();
	if (fmt.IsFbo()) {
		InternalPacketData& data = out.GetData<InternalPacketData>();
		this->GetBuffer().StoreOutputLink(data);
		RTLOG("AudioBaseT<Gfx>::Send: 0, " << src_ch << ": " << out.ToString());
	}
	return true;
}

template <class Gfx>
bool AudioBaseT<Gfx>::NegotiateSinkFormat(LinkBase& link, int sink_ch, const ValueFormat& new_fmt) {
	// accept all valid video formats for now
	if (new_fmt.IsValid() && new_fmt.IsAudio()) {
		ISinkPtr sink = this->GetSink();
		ValueBase& val = sink->GetValue(sink_ch);
		val.SetFormat(new_fmt);
		return true;
	}
	return false;
}










GFX3D_EXCPLICIT_INITIALIZE_CLASS(TextureBaseT)
GFX3D_EXCPLICIT_INITIALIZE_CLASS(ShaderBaseT)
GFX3D_EXCPLICIT_INITIALIZE_CLASS(FboReaderBaseT)
GFX3D_EXCPLICIT_INITIALIZE_CLASS(KeyboardBaseT)
GFX3D_EXCPLICIT_INITIALIZE_CLASS(AudioBaseT)


END_UPP_NAMESPACE
