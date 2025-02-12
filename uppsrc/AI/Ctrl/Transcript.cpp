#include "Ctrl.h"
#include <ide/ide.h>


NAMESPACE_UPP


TranscriptProofreadCtrl::TranscriptProofreadCtrl() {
	CtrlLayout(*this);
	
	start.WhenAction = THISBACK(Start);
	
	ai.WhenAction = [this] {
		int ai_idx = this->ai.GetIndex();
		auto& comp = GetExt<COMPNAME>();
		comp.value("ai-idx") = ai_idx;
	};
	
	misspelled.WhenAction = [this] {
		auto& comp = GetExt<COMPNAME>();
		comp.value("misspelled") = this->misspelled.GetData();
	};
	
	lines.AddColumn("Begin");
	lines.AddColumn("End");
	lines.AddColumn("Text");
	lines.AddIndex("IDX");
	lines.ColumnWidths("1 1 3");
}

void TranscriptProofreadCtrl::Data() {
	auto& comp = GetExt<TranscriptProofread>();
	SetAiProviders(this->ai, comp.value("ai-idx"));
	
	finder.UpdateSources(*this, transcripts, THISBACK(DataFile));
	DataFile();
}

void TranscriptProofreadCtrl::DataFile() {
	int idx = transcripts.GetIndex();
	if (idx < 0 && idx >= this->transcripts.GetCount())
		return;
	COMPNAME& comp = GetExt<COMPNAME>();
	auto& vidfile = *finder.file_ptrs[idx];
	comp.value("text") = vidfile.value("text");
	this->misspelled.SetData(comp.value("misspelled"));
	
	String text = comp.value("proofread");
	if (text.IsEmpty()) {
		this->lines.Clear();
	}
	else {
		TranscriptResponse r;
		LoadFromJson(r,text);
		for(int i = 0; i < r.segments.GetCount(); i++) {
			auto& segment = r.segments[i];
			lines.Set(i, 0, GetDurationString(segment.start));
			lines.Set(i, 1, GetDurationString(segment.end));
			lines.Set(i, 2, segment.text);
			lines.Set(i, "IDX", i);
		}
	}
}

void TranscriptProofreadCtrl::Start() {
	PostCallback([this]{this->start.Disable();});
	Event<> fn = [this]{
		ts.Reset();
		COMPNAME& comp = GetExt<COMPNAME>();
		TaskMgr& m = AiTaskManager();
		TaskArgs args;
		args.fn = FN_TRANSCRIPT_PROOFREAD_1;
		int idx = ai.GetIndex();
		args.params("ai_provider_idx") = this->ai.GetKey(idx);
		args.params("misspelled") = comp.value("misspelled");
		args.params("text") = comp.value("text");
		PostCallback([this,&comp]{this->status.SetLabel("Making proofread of transcript of: " + (String)comp.value("path"));});
		m.Get(args, [this](String s) {
			COMPNAME& comp = GetExt<COMPNAME>();
			s = "- #" + s;
			LOG(s);
			
			String text = comp.value("text");
			TranscriptResponse r;
			LoadFromJson(r, text);
			
			Index<int> modified_lines;
			Vector<String> lines = Split(s, "\n");
			for (String& l : lines) {
				if (l.Left(3) != "- #") continue;
				l = TrimLeft(l.Mid(3));
				int line = ScanInt(l);
				int i = l.Find(" ");
				if (i < 0) continue;
				l = l.Mid(i+1);
				if (line >= 0 && line < r.segments.GetCount()) {
					auto& src_segment = r.segments[line];
					src_segment.text = TrimBoth(l);
					modified_lines.FindAdd(line);
				}
			}
			Vector<int> rm_list;
			// Combine time-range of removed segments
			for(int i = 0; i < r.segments.GetCount(); i++) {
				if (modified_lines.Find(i) < 0)
					rm_list << i;
				else if (modified_lines.Find(i) >= 0) {
					auto& to_segment = r.segments[i];
					to_segment.tokens.Clear();
					if (i+1 < i < r.segments.GetCount()) {
						i++;
						for(; i < r.segments.GetCount(); i++) {
							if (modified_lines.Find(i) >= 0) {
								i--;
								break;
							}
							auto& from_segment = r.segments[i];
							rm_list << i;
							to_segment.end = from_segment.end;
						}
					}
				}
			}
			if (!rm_list.IsEmpty())
				r.segments.Remove(rm_list);
			
			r.text = "";
			for(int i = 0; i < r.segments.GetCount(); i++) {
				if (i) text << " ";
				r.text << r.segments[i].text;
			}
			
			comp.value("proofread") = StoreAsJson(r);
			
			PostCallback([this,s]{
				this->status.SetLabel("Proofread was completed in " + ts.ToString());
				this->start.Enable();
				DataFile();
			});
		});
	};
	fn(); // I left this event-call-pattern for code duplication, where wrapper with callback-param is needed
}

void TranscriptProofreadCtrl::ToolMenu(Bar& bar) {
	
}

INITIALIZER_COMPONENT_CTRL(TranscriptProofread, TranscriptProofreadCtrl)




ProofreadStorylineCtrl::ProofreadStorylineCtrl() {
	
}

void ProofreadStorylineCtrl::Data() {
	
}

void ProofreadStorylineCtrl::ToolMenu(Bar& bar) {
	
}

INITIALIZER_COMPONENT_CTRL(ProofreadStoryline, ProofreadStorylineCtrl)






StorylineConversionCtrl::StorylineConversionCtrl() {
	
}

void StorylineConversionCtrl::Data() {
	
}

void StorylineConversionCtrl::ToolMenu(Bar& bar) {
	
}

INITIALIZER_COMPONENT_CTRL(StorylineConversion, StorylineConversionCtrl)






StorylineScriptCtrl::StorylineScriptCtrl() {
	
}

void StorylineScriptCtrl::Data() {
	
}

void StorylineScriptCtrl::ToolMenu(Bar& bar) {
	
}

INITIALIZER_COMPONENT_CTRL(StorylineScript, StorylineScriptCtrl)



ScriptConversionCtrl::ScriptConversionCtrl() {
	
}

void ScriptConversionCtrl::Data() {
	
}

void ScriptConversionCtrl::ToolMenu(Bar& bar) {
	
}

INITIALIZER_COMPONENT_CTRL(ScriptConversion, ScriptConversionCtrl)



END_UPP_NAMESPACE
