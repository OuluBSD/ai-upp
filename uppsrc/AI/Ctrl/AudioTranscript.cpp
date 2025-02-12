#include "Ctrl.h"
#include <ide/ide.h>

NAMESPACE_UPP

AudioTranscriptCtrl::AudioTranscriptCtrl() {
	CtrlLayout(*this);
	make_audio.WhenAction = THISBACK1(MakeAudio, Event<>());
	start.WhenAction = THISBACK(Start);
	ai.WhenAction = [this] {
		int ai_idx = this->ai.GetIndex();
		AudioTranscript& comp = GetExt<AudioTranscript>();
		comp.value("ai-idx") = ai_idx;
	};
	language.WhenAction = [this] {
		AudioTranscript& comp = GetExt<AudioTranscript>();
		comp.value("language") = this->language.GetData();
	};
	lines.AddColumn("Begin");
	lines.AddColumn("End");
	lines.AddColumn("Text");
	lines.AddIndex("IDX");
	lines.ColumnWidths("1 1 3");
}

void AudioTranscriptCtrl::MakeAudio(Event<> cb_ready) {
	if (vidpath.IsEmpty())
		return;
	
	PostCallback([this]{this->make_audio.Disable();});
	
	Thread::Start([this,cb_ready]{
		CombineHash ch;
		ch << range_begin << range_end;
		hash_t h = ch;
		String dir = ConfigFile("cache");
		RealizeDirectory(dir);
		mp3path = AppendFileName(dir, GetFileTitle(vidpath) + "-" + IntStr(h & 0xFFFFFF) + ".mp3");
		double range_len = range_end - range_begin;
		String cmd =
			"ffmpeg -y -i \"" + vidpath + "\" "
			"-ss " + DblStr(range_begin) + " -t " + DblStr(range_len) + " "
			"-vn -acodec mp3 -ar 44100 -ac 1 -ab 128k "
			"-filter:a \"speechnorm=e=12.5:r=0.0001:l=1\" "
			"\"" + mp3path + "\"";
		//DLOG(cmd);
		String out;
		if (!Sys(cmd, out))
			PostCallback([this]{this->status.SetLabel("Wrote audio file: " + mp3path);});
		else
			PostCallback([this]{this->status.SetLabel("Failed to extract audio");});
		PostCallback([this]{this->make_audio.Enable();});
		PostCallback(cb_ready);
	});
}

void AudioTranscriptCtrl::Start() {
	PostCallback([this]{this->start.Disable();});
	Event<> fn = [this]{
		ts.Reset();
		String mp3 = LoadFile(mp3path);
		if (mp3.IsEmpty())
			return;
		AudioTranscript& comp = GetExt<AudioTranscript>();
		TaskMgr& m = AiTaskManager();
		TranscriptionArgs args;
		int idx = ai.GetIndex();
		args.file = mp3path;
		args.ai_provider_idx = this->ai.GetKey(idx);
		args.language = comp.value("language");
		PostCallback([this]{this->status.SetLabel("Making transcript of audio file: " + this->mp3path);});
		m.GetTranscription(args, [this](String s) {
			AudioTranscript& comp = GetExt<AudioTranscript>();
			comp.value("text") = s;
			//DLOG(s);
			PostCallback([this,s]{
				this->status.SetLabel("Transcription was completed in " + ts.ToString());
				this->start.Enable();
				DataFile();
			});
		});
	};
	if (mp3path.IsEmpty())
		MakeAudio(fn);
	else
		fn();
}

bool AudioTranscriptCtrl::UpdateSources() {
	file_ptrs.Clear();
	file_paths.Clear();
	AudioTranscript& comp = GetExt<AudioTranscript>();
	if (!comp.node.owner)
		return false;
	String sel_path = comp.value("path");
	files.Clear();
	files.WhenAction.Clear();
	file_ptrs = comp.node.owner->FindAll<VideoSourceFileRange>();
	int cursor = 0;
	for(int i = 0; i < file_ptrs.GetCount(); i++) {
		auto& file = *file_ptrs[i];
		ValueMap map = file.value;
		String path = map("path");
		double range_begin = file.value("range_begin");
		double range_end = file.value("range_end");
		double total = range_end - range_begin;
		String total_str = GetDurationString(total);
		file_paths.Add(path);
		String title = Format("%s (%.2f - %.2f, %s)", GetFileName(path), range_begin, range_end, total_str);
		files.Add(title);
		if (sel_path == path)
			cursor = i;
	}
	bool change_file = false;
	if (cursor >= 0 && cursor < files.GetCount()) {
		files.SetIndex(cursor);
	}
	files.WhenAction = THISBACK(DataFile);
	return change_file;
}

void AudioTranscriptCtrl::Data() {
	this->ai.Clear();
	auto& ai_mgr = TheIde()->ai_manager;
	for(int i = 0; i < ai_mgr.GetCount(); i++) {
		auto& prov = ai_mgr[i];
		if (prov.IsFeatureAudioToText())
			ai.Add(i, ai_mgr[i].name + " (" + ai_mgr[i].GetTypeString() + ")");
	}
	if (ai.GetCount()) {
		AudioTranscript& comp = GetExt<AudioTranscript>();
		int ai_idx = comp.value("ai-idx");
		if (ai_idx >= 0 && ai_idx < ai.GetCount())
			ai.SetIndex(ai_idx);
		else
			ai.SetIndex(0);
	}
	
	UpdateSources();
	DataFile();
}

void AudioTranscriptCtrl::DataFile() {
	int idx = files.GetIndex();
	if (idx < 0 && idx >= this->files.GetCount())
		return;
	auto& vidfile = *file_ptrs[idx];
	
	this->duration    = vidfile.value("duration");
	this->frame_rate  = FractionDbl((String)vidfile.value("frame_rate"));
	this->vidpath     = vidfile.value("path");
	this->range_begin = vidfile.value("range_begin");
	this->range_end   = vidfile.value("range_end");
	
	AudioTranscript& comp = GetExt<AudioTranscript>();
	this->language.SetData(comp.value("language"));
	
	String text = comp.value("text");
	if (text.IsEmpty()) {
		this->lines.Clear();
	}
	else {
		
		LoadFromJson(r,text);
		this->line_editors.SetCount(r.segments.GetCount());
		for(int i = 0; i < r.segments.GetCount(); i++) {
			auto& segment = r.segments[i];
			lines.Set(i, 0, GetDurationString(segment.start));
			lines.Set(i, 1, GetDurationString(segment.end));
			lines.Set(i, 2, segment.text);
			lines.Set(i, "IDX", i);
			EditString& es = line_editors[i];
			es.SetData(segment.text);
			lines.SetCtrl(i, 2, es);
			es.WhenAction = [this,&es,&segment]{
				segment.text = es.GetData();
				tc.Set(500, THISBACK(SaveTextChanges));
			};
		}
	}
}

void AudioTranscriptCtrl::SaveTextChanges() {
	AudioTranscript& comp = GetExt<AudioTranscript>();
	String text = StoreAsJson(r, false);
	//LOG(text);
	comp.value("text") = text;
}

void AudioTranscriptCtrl::ToolMenu(Bar& bar) {
	
}

INITIALIZER_COMPONENT_CTRL(AudioTranscript, AudioTranscriptCtrl)





ScriptSpeechCtrl::ScriptSpeechCtrl() {
	
}

void ScriptSpeechCtrl::Data() {
	
}

void ScriptSpeechCtrl::ToolMenu(Bar& bar) {
	
}

INITIALIZER_COMPONENT_CTRL(ScriptSpeech, ScriptSpeechCtrl)



END_UPP_NAMESPACE
