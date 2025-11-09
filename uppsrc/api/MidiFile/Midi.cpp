#include "MidiFile.h"

NAMESPACE_UPP



MidiFileReaderAtom::MidiFileReaderAtom(VfsValue& n) : Atom(n) {
	
}

bool MidiFileReaderAtom::Initialize(const WorldState& ws) {
	close_machine = ws.GetBool(".close_machine", false);
	drum_side_ch = ws.GetInt(".drum.ch", -1);
	use_global_time = ws.GetBool(".use.global.time", false);
	require_success = ws.GetBool(".require.success", false);
	
	String path = ws.GetString(".filepath");
	if (path.IsEmpty()) {
		LOG("MidiFileReaderAtom::Initialize: error: 'filepath' argument is required, but not given");
		return false;
	}
	
	path = RealizeShareFile(path);
	if (!OpenFilePath(path)) {
		LOG("MidiFileReaderAtom::Initialize: error: " << last_error << ": " << path);
		return false;
	}
	
	AddAtomToUpdateList();
	
	return true;
}

bool MidiFileReaderAtom::PostInitialize() {
	split_channels = link->SideSinks().GetCount() > 1;
	
	return true;
}

void MidiFileReaderAtom::Uninitialize() {
	RemoveAtomFromUpdateList();
	
	Clear();
}

void MidiFileReaderAtom::Clear() {
	last_error.Clear();
	pending_final_status = false;
	final_status_sent = false;
	total_events_sent = 0;
	song_dt = -1;
	track_i.SetCount(0);
	tmp.Reset();
}

bool MidiFileReaderAtom::OpenFilePath(String path) {
	Clear();
	
	if (!FileExists(path)) {
		last_error = "file does not exist: " + path;
		return false;
	}
	
	if (!file.Open(path)) {
		last_error = file.GetLastError();
		return false;
	}
	
	file.DoTimeAnalysis();
	file.LinkNotePairs();
	
	track_i.SetCount(file.GetTrackCount(), 0);
	
	return true;
}

void MidiFileReaderAtom::DumpMidiFile() {
	int track_count = file.GetTrackCount();
	LOG("TPQ: " << file.GetTicksPerQuarterNote());
	if (track_count > 1) {
		LOG("TRACKS: " << track_count);
	}
	for (int track = 0; track < track_count; track++) {
		if (track_count > 1) {
			LOG("\nTrack " << track);
		}
		LOG("Tick\tSeconds\tDur\tMessage");
		const auto& t = file[track];
		for (int event = 0; event < t.GetCount(); event++) {
			const auto& e = t[event];
			LOG(IntStr(e.tick));
			LOG("\t" << DblStr(e.seconds));
			String s = "\t";
			if (e.IsNoteOn()) {
				s << DblStr(e.GetDurationInSeconds());
			}
			for (int i = 0; i < e.GetCount(); i++)
				s << "\t" << IntStr(e[i]);
			LOG(s);
		}
	}
}

void MidiFileReaderAtom::Update(double dt) {
	if (close_machine && IsEnd()) {
		if (!require_success || final_status_sent)
			GetEngine().SetNotRunning();
	}
	
	// The first update is often laggy, so wait until the second one
	if (song_dt < 0) {
		song_dt = 0;
		return;
	}
	
	song_dt += dt;
	//LOG("midi song dt: " << song_dt);
	
	tmp.midi.Reserve(1000);
	
	for(int i = 0; i < file.GetTrackCount(); i++) {
		CollectTrackEvents(i);
	}
	
	if (require_success && !final_status_sent && IsEnd()) {
		pending_final_status = true;
	}
}

void MidiFileReaderAtom::CollectTrackEvents(int i) {
	if (i >= track_i.GetCount())
		track_i.SetCount(i+1,0);
	
	int& iter = track_i[i];
	const auto& t = file[i];
	
	double dt_limit;
	if (require_success)
		dt_limit = 1e9;
	else if (use_global_time)
		dt_limit = GlobalAudioTime::Local().Get();
	else
		dt_limit = song_dt;
	
	while (iter < t.GetCount()) {
		const auto& e = t[iter];
		if (e.seconds <= dt_limit) {
			iter++;
			tmp.midi.Add(&e);
		}
		else break;
	}
}

bool MidiFileReaderAtom::IsEnd() const {
	for(int i = 0; i < track_i.GetCount(); i++) {
		const int& iter = track_i[i];
		const auto& t = file[i];
		if (iter < t.GetCount())
			return false;
	}
	return true;
}

bool MidiFileReaderAtom::IsReady(PacketIO& io) {
	bool has_events = tmp.midi.GetCount() > 0;
	if (!has_events && require_success && pending_final_status && !final_status_sent)
		has_events = true;
	bool sink_active = (io.active_sink_mask & 0x1) != 0;
	if (!sink_active && require_success && pending_final_status && !final_status_sent)
		sink_active = true;
	bool src_ready = io.full_src_mask == 0;
	if (!src_ready && require_success && pending_final_status && !final_status_sent)
		src_ready = true;
	return src_ready && sink_active && has_events;
}

bool MidiFileReaderAtom::Recv(int sink_ch, const Packet& in) {
	return true;
}

void PacketValue_ClearMidiEventData(PacketValue& p) {
	Vector<byte>& data = p.Data();
	if (data.IsEmpty()) return;
	ASSERT((data.GetCount() % sizeof(MidiIO::Event)) == 0);
	MidiIO::Event* dst = (MidiIO::Event*)(byte*)data.Begin();
	int items = data.GetCount() / sizeof(MidiIO::Event);
	MidiIO::Event* end = dst + items;
	while (dst != end) {
		dst->~Event();
		dst++;
	}
	data.Clear();
}

bool MidiFileReaderAtom::Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	out.ClearDataType();
	ValueFormat fmt = out.GetFormat();
	Vector<byte>& data = out.Data();
	data.SetCount(0);
	
	if (require_success && pending_final_status && !final_status_sent && src_ch == 0 && tmp.midi.GetCount() == 0) {
		if (!fmt.IsMidi())
			return false;
		MidiPipelineStatus& status = out.SetData<MidiPipelineStatus>();
		status.event_count = total_events_sent;
		status.eof = true;
		status.success = true;
		pending_final_status = false;
		final_status_sent = true;
		return true;
	}
	
	if (!fmt.IsMidi())
		return false;
	
	out.SetDataClearFunction(&PacketValue_ClearMidiEventData);
	
	int total_batch_events = tmp.midi.GetCount();
	if (total_batch_events == 0)
		return true;
	
	bool reset_tmp = false;
	
	if (!split_channels) {
		int sz = total_batch_events * sizeof(MidiIO::Event);
		data.SetCount(sz);
		
		MidiIO::Event* dst = (MidiIO::Event*)(byte*)data.Begin();
		for(const MidiIO::Event* ev : tmp.midi) {
			//LOG("track " << ev->track << ": " << ev->ToString());
			new(dst++) MidiIO::Event(*ev);
		}
		
		reset_tmp = true;
	}
	else {
		int sz = total_batch_events * sizeof(MidiIO::Event);
		data.SetCount(sz);
		
		int count = 0;
		MidiIO::Event* dst = (MidiIO::Event*)(byte*)data.Begin();
		bool is_drum_ch = drum_side_ch >= 0 && src_ch == drum_side_ch;
		for(const MidiIO::Event* ev : tmp.midi) {
			//LOG("track " << ev->track << ": " << ev->ToString());
			
			#if 0
			if (is_drum_ch && ev->IsNoteOn()) {
				LOG("track " << ev->track << ": " << ev->GetChannel() << ": " << ev->ToString());
			}
			#endif
			
			if (ev->IsNote() ||
				ev->IsNoteOn() ||
				ev->IsNoteOff() ||
				ev->IsPitchbend() ||
				ev->IsAftertouch() ||
				ev->IsPressure() ||
				ev->IsPatchChange()) {
				if (is_drum_ch) {
					// Midi channel 10 is drum channel (here 10-1==9)
					if (ev->GetChannel() == 9) {
						new(dst++) MidiIO::Event(*ev);
						count++;
					}
				}
				else {
					// midi tracks starts from 1 practically, like side-channels
					if (ev->track == src_ch) {
						new(dst++) MidiIO::Event(*ev);
						count++;
					}
				}
			}
			else {
				new(dst++) MidiIO::Event(*ev);
				count++;
			}
		}
		
		data.SetCount(count * sizeof(MidiIO::Event));
		if (src_ch == 0)
			reset_tmp = true;
	}
	
	if (reset_tmp) {
		tmp.Reset();
		total_events_sent += total_batch_events;
	}
	
	// channel 0 is sent last, so use that information to finalize temp buffer usage
	#if 0
	if (src_ch == 0) {
		for(const MidiIO::Event* ev : tmp.midi) {
			LOG("track " << ev->track << ": " << ev->GetChannel() << ": " << ev->ToString());
		}
	}
	#endif
	
	return true;
}
















MidiNullAtom::MidiNullAtom(VfsValue& n) : Atom(n) {
	
}

bool MidiNullAtom::Initialize(const WorldState& ws) {
	verbose = ws.GetBool(".verbose", false);
	require_success = ws.GetBool(".require.success", false);
	final_status_seen = false;
	received_event_count = 0;
	
	return true;
}

void MidiNullAtom::Uninitialize() {
	
}

bool MidiNullAtom::IsReady(PacketIO& io) {
	return true;
}

bool MidiNullAtom::Recv(int sink_ch, const Packet& in) {
	if (in->IsCustomData() && in->IsData<MidiPipelineStatus>()) {
		const MidiPipelineStatus& status = in->GetData<MidiPipelineStatus>();
		bool ok = status.eof && status.success && status.event_count == received_event_count;
		final_status_seen = true;
		
		if (require_success) {
			if (ok) {
				Cout() << "success!\n";
			}
			else {
				String msg = Format("failure: expected %lld events, received %lld", status.event_count, received_event_count);
				Cout() << msg << '\n';
				GetEngine().SetFailed(msg);
			}
			GetEngine().SetNotRunning();
		}
		
		return ok || !require_success;
	}
	
	const Vector<byte>& data = in->Data();
	int count = data.GetCount() / sizeof(MidiIO::Event);
	received_event_count += count;
	
	if (verbose) {
		LOG("MidiNullAtom::Recv: " << count << " midi events");
		
		const MidiIO::Event* ev  = (const MidiIO::Event*)(const byte*)data.Begin();
		const MidiIO::Event* end = ev + count;
		while (ev != end) {
			LOG("track " << ev->track << ": " << ev->ToString());
			ev++;
		}
	}
	
	return true;
}

bool MidiNullAtom::Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	return true;
}



END_UPP_NAMESPACE
