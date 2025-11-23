#include "Graphics.h"

NAMESPACE_UPP



template <class Gfx>
FboAtomT<Gfx>* FboAtomT<Gfx>::latest;

template <class Gfx>
FboAtomT<Gfx>& FboAtomT<Gfx>::Latest() {ASSERT(latest); return *latest;}


template <class Gfx>
FboAtomT<Gfx>::FboAtomT(VfsValue& n) : GfxAtomBase(n) {
	latest = this;
}

template <class Gfx>
bool FboAtomT<Gfx>::Initialize(const WorldState& ws) {
	ws_at_init = ws;
	
	ISourcePtr src = this->GetSource();
	int src_count = src->GetSourceCount();
	ValueBase& val = src->GetSourceValue(src_count-1);
	src_type = val.GetFormat().vd;
	
	draw_mem = ws.Get(".drawmem") == "true"; // Dumb "local render" (forward raw data)
	//gfxbuf = ws.Get(".gfxbuf") == "true"; // SoftRender rending locally (not just data-forwarding)
	program = ws.Get(".program");
	//gfxpack = ws.Get(".gfxpack") == "true";
	
	if (program.IsEmpty()) {
		LOG("FboAtomT<Gfx>::Initialize: error: no 'program' attribute was given");
		return false;
	}
	
	
	String bin  = program + "_program";
	auto& bin_map  = GfxProgramLibrary::GetBinders();
	int bin_i  = bin_map.Find(bin);
	
	if (bin_i < 0) {
		LOG("FboAtomT<Gfx>::Initialize: error: program '" << bin << "' not found");
		return false;
	}
	
	Index<String> shader_keys;
	ws.FindKeys(".shader.", shader_keys);
	for (String key : shader_keys) {
		String value = ws.GetString(key);
		int begin = 8;
		int i = key.Find(".", begin);
		if (i <= begin) {
			LOG("FboAtomT<Gfx>::Initialize: error: invalid key '" << key << "'");
			return false;
		}
		PipelineState& pipe = data.GetAddPipeline("default");
		
		String prog_name = key.Mid(begin, i - begin);
		ProgramState& prog = pipe.GetAddProgram(prog_name);
		String shader_key = key.Mid(i+1);
		if (shader_key == "frag.path") {
			if (!prog.LoadShaderFile(GVar::FRAGMENT_SHADER, value, "")) {
				LOG("FboAtomT<Gfx>::Initialize: error: loading shader failed from '" << value << "'");
				return false;
			}
		}
		else if (shader_key == "frag.name") {
			if (!prog.LoadBuiltinShader(GVar::FRAGMENT_SHADER, value + "_fragment")) {
				LOG("FboAtomT<Gfx>::Initialize: error: loading builtin shader failed from '" << value << "'");
				return false;
			}
		}
		else if (shader_key == "vtx.path") {
			if (!prog.LoadShaderFile(GVar::VERTEX_SHADER, value, "")) {
				LOG("FboAtomT<Gfx>::Initialize: error: loading shader failed from '" << value << "'");
				return false;
			}
		}
		else if (shader_key == "vtx.name") {
			if (!prog.LoadBuiltinShader(GVar::VERTEX_SHADER, value + "_vertex")) {
				LOG("FboAtomT<Gfx>::Initialize: error: loading builtin shader failed from '" << value << "'");
				return false;
			}
		}
		else {
			LOG("FboAtomT<Gfx>::Initialize: error: invalid key '" << shader_key << "'");
			return false;
		}
		
		prog.pending_compilation = true;
	}
	
	
	// Create BinderIfaceVideo
	own_binder = bin_map[bin_i]();
	if (!own_binder)
		return false;
	own_binder->Initialize(ws_at_init);
	binders.Add(&*own_binder);
	
	
	Index<String> keys;
	ws.FindKeys(".program.arg.", keys);
	for (String key : keys) {
		String arg_key = key.Mid(13);
		String value = ws.Get(key);
		for (BinderIfaceVideo* b : binders) {
			if (!b->Arg(arg_key, value)) {
				LOG("FboAtomT<Gfx>::Initialize: error: program '" << bin << "' did not accept argument: " << arg_key << " = " << value);
				return false;
			}
		}
	}
	
	String type = ws.GetString(".type");
	if (type == "stereo")
		data.is_stereo = true;
	
	data.eng = &GetEngine();
	accel_sd.SetTarget(data);
	
	
	return true;
}

template <class Gfx>
bool FboAtomT<Gfx>::PostInitialize() {
	// Remove alpha channel
	if (src_type == VD(CENTER, VIDEO)) {
		ISourcePtr src = this->GetSource();
		int src_count = src->GetSourceCount();
		LinkBase* link = this->GetLink();
		ASSERT(link);
		
		Index<int> video_src_chs;
		for (auto& ex : link->SideSinks()) {
			if (ex.local_ch_i >= 0)
				video_src_chs.FindAdd(ex.local_ch_i);
		}
		if (video_src_chs.IsEmpty() && src_count > 0)
			video_src_chs.Add(src_count - 1);
		
		for (int src_ch : video_src_chs) {
			if (src_ch < 0 || src_ch >= src_count)
				continue;
			ValueBase& val = src->GetSourceValue(src_ch);
			ValueFormat fmt = val.GetFormat();
			if (fmt.vd != VD(CENTER, VIDEO))
				continue;
			fmt.vid.SetType(ColorSampleFD::RGB_U8_LE);
			if (!link->NegotiateSourceFormat(src_ch, fmt))
				return false;
		}
	}
	return true;
}

template <class Gfx>
void FboAtomT<Gfx>::Uninitialize() {
	if (own_binder) {
		own_binder->Uninitialize();
		own_binder.Clear();
	}
	data.Clear();
}

template <class Gfx>
bool FboAtomT<Gfx>::IsReady(PacketIO& io) {
	dword iface_sink_mask = iface.GetSinkMask();
	bool b =
		(io.active_sink_mask & 0x1) &&
		io.full_src_mask == 0 &&
		binders.GetCount() > 0;
	
	for (BinderIfaceVideo* binder : binders)
		if (!binder->Render(accel_sd))
			b = false;
	
	RTLOG("FboAtomT::IsReady: " << (b ? "true" : "false") << " (" << io.nonempty_sinks << ", " << io.sinks.GetCount() << ", " << HexStr(iface_sink_mask) << ", " << HexStr(io.active_sink_mask) << ")");
	return b;
}

template <class Gfx>
bool FboAtomT<Gfx>::Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	RTLOG("FboAtomT::ProcessPackets:");
	ValueFormat fmt = out.GetFormat();
	 
	if (src_type == VD(CENTER,FBO) ||
		src_type == VD(OGL,FBO)) {
		InternalPacketData& data = out.SetData<InternalPacketData>();
		
		data.ptr = &static_cast<GfxDataState&>(this->data);
		data.SetText("gfxstate");
		ASSERT(data.ptr);
		
		if (packet_router && !router_source_ports.IsEmpty() && fmt.IsValid()) {
			int credits = RequestCredits(src_ch, 1);
			if (credits <= 0) {
				RTLOG("FboAtomT::Send: credit request denied for src_ch=" << src_ch);
				return false;
			}
			Packet route_pkt = CreatePacket(out.GetOffset());
			route_pkt.Pick(out);
			route_pkt->SetFormat(fmt);
			bool routed = EmitViaRouter(src_ch, route_pkt);
			AckCredits(src_ch, credits);
			out.Pick(*route_pkt);
			if (!routed)
				return false;
		}
	}
	else {
		ASSERT_(0, "TODO");
		return false;
	}
	
	#if 0
	if (src_ch > 0 && io.src[src_ch].p) {
		PacketIO::Sink& prim_sink = io.sink[0];
		PacketIO::Source& prim_src = io.src[0];
		prim_src.from_sink_ch = 0;
		prim_src.p = ReplyPacket(0, prim_sink.p);
	}
	#endif
	
	return true;
}

template <class Gfx>
void FboAtomT<Gfx>::AddBinder(BinderIfaceVideo* iface) {
	VectorGetAdd(binders, iface);
}

template <class Gfx>
void FboAtomT<Gfx>::RemoveBinder(BinderIfaceVideo* iface) {
	VectorRemoveKey(binders, iface);
}

template <class Gfx>
bool FboAtomT<Gfx>::Recv(int sink_ch, const Packet& in) {
	ValueFormat fmt = in->GetFormat();
	RTLOG("FboAtomT::Recv: sink=" << sink_ch << " vd=" << (int)fmt.vd.val);
	if (!in->IsData<InternalPacketData>())
		return true;
	
	const InternalPacketData& data = in->GetData<InternalPacketData>();
	if (!data.ptr) {
		RTLOG("FboAtomT::Recv: data.ptr is null, returning false");
		return false;
	}
	
	if (data.IsText("gfxbuf")) {
		RTLOG("FboAtomT::Recv: gfxbuf sink=" << sink_ch);
		int input_idx = max(sink_ch - 1, 0);
		bool linked = false;
		for (int i = 0; i < this->data.pipelines.GetCount(); i++) {
			PipelineState& pipe = this->data.pipelines[i];
			for (int j = 0; j < pipe.programs.GetCount(); j++) {
				ProgramState& prog = pipe.programs[j];
				if (prog.LoadInputLink(input_idx, data))
					linked = true;
			}
		}
		
		if (!linked) {
			LOG("FboAtomT<Gfx>::Recv: warning: could not bind gfxbuf input #" << input_idx);
			return false;
		}
		return true;
	}
	else {
		GfxDataState* gfx_state = (GfxDataState*)data.ptr;
		DataState* state = CastPtr<DataState>(gfx_state);
		ASSERT(state);
		if (state) {
			VectorGetAdd(this->data.linked, state);
		}
	}
	
	return true;
}

template <class Gfx>
void FboAtomT<Gfx>::Finalize(RealtimeSourceConfig& cfg) {
	last_cfg = &cfg;
}

template <class Gfx>
Callback1<FboAtomT<Gfx>*>	FboAtomT<Gfx>::WhenInitialize;


X11SW_EXCPLICIT_INITIALIZE_CLASS(FboAtomT)
X11OGL_EXCPLICIT_INITIALIZE_CLASS(FboAtomT)
SDLSW_EXCPLICIT_INITIALIZE_CLASS(FboAtomT)
SDLOGL_EXCPLICIT_INITIALIZE_CLASS(FboAtomT)
WINDX_EXCPLICIT_INITIALIZE_CLASS(FboAtomT)



END_UPP_NAMESPACE
