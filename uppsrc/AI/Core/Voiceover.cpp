#include "Core.h"

NAMESPACE_UPP


VoiceoverProcess::VoiceoverProcess() {
	
}
	
int VoiceoverProcess::GetPhaseCount() const {
	return PHASE_COUNT;
}

int VoiceoverProcess::GetBatchCount(int phase) const {
	if (phase == PHASE_FIND_NATURAL_PARTS)
		return input_coarse_parts.GetCount();
	return 1;
}

int VoiceoverProcess::GetSubBatchCount(int phase, int batch) const {
	return 1;
}
	
void VoiceoverProcess::DoPhase() {
	String transcription = this->params("transcription");
	
	if (transcription.IsEmpty()) {
		Fail("transcription required, but is empty");
		return;
	}
	
	if (phase == PHASE_FIND_NATURAL_PARTS) {
		
		if (IsPhaseInit()) {
			// Make coarse input parts
			input_splits.Clear();
			input_coarse_parts.Clear();
			transcription.Replace("\r", "");
			transcription.Replace("  ", " ");
			lines = Split(transcription, "\n");
			{
				Vector<int> rmlist;
				for(int i = 0; i < lines.GetCount(); i++)
					if (lines[i] == "No speech")
						rmlist << i;
				if (rmlist.GetCount() > 1)
					lines.Remove(rmlist);
			}
			Vector<String> cur_part;
			int cur_part_total = 0;
			for(int i = 0; i < lines.GetCount(); i++) {
				String& line = lines[i];
				line = TrimBoth(line);
				if (line.IsEmpty())
					continue;
				if (cur_part_total > 0 && cur_part_total + line.GetCount() >= chars_per_coarse_part) {
					Swap(input_coarse_parts.Add(), cur_part); // add part
					cur_part_total = 0;
				}
				cur_part.Add(line);
				cur_part_total += line.GetCount() + 1;
			}
			if (cur_part_total > 0)
				Swap(input_coarse_parts.Add(), cur_part); // add part
			
			if (input_coarse_parts.IsEmpty()) {
				Fail("could not make coarse parts from transcript");
				return;
			}
		}
		
		if (batch >= input_coarse_parts.GetCount()) {
			NextPhase();
			return;
		}
		
		// Make input params for prompt maker
		const Vector<String>& srctxt_lines = input_coarse_parts[batch];
		ValueArray arr;
		for (const String& l : srctxt_lines) arr.Add(l);
		
		ValueMap params = this->params;
		params.Add("lines", arr);
		
		TaskArgs args;
		args.fn = TaskFn::FN_VOICEOVER_1_FIND_NATURAL_PARTS;
		args.params = params;
		
		SetWaiting(1);
		AiTaskManager().Get(args, [this](String res) {
			res.Replace("\r", "");
			RemoveEmptyLines3(res);
			Vector<String> lines = Split(res, "\n");
			Vector<int>& splits = input_splits.Add();
			for(int i = 0; i < lines.GetCount(); i++) {
				String& line = lines[i];
				line = TrimBoth(line);
				if (line.Left(6) == "line #") {
					line = line.Mid(6);
					int line_n = ScanInt(line);
					if (line_n > 0)
						splits << line_n;
				}
			}
			//DUMPC(splits);
			
			NextBatch();
			SetWaiting(0);
		}, "Voiceover 1/2: find natural parts " + IntStr(batch+1) + "/" + IntStr(input_coarse_parts.GetCount()));
	}
	else if (phase == PHASE_TRANSCRIPT) {
		
		if (IsPhaseInit()) {
			natural_parts.Clear();
			if (input_coarse_parts.GetCount() != input_splits.GetCount()) {
				Fail("part count mismatch");
				return;
			}
			
			Vector<String> all_lines;
			Vector<int> all_splits;
			for(int i = 0; i < input_splits.GetCount(); i++) {
				const Vector<int>& splits = input_splits[i];
				const Vector<String>& lines = input_coarse_parts[i];
				int line_offset = all_lines.GetCount();
				all_lines.Append(lines);
				for (int s : splits)
					all_splits << line_offset + s;
			}
			
			int j = 0;
			Vector<String>* part = &natural_parts.Add();
			for(int i = 0; i < all_lines.GetCount(); i++) {
				if (j < all_splits.GetCount() && all_splits[j] == i && part->GetCount() > 1)
					part = &natural_parts.Add();
				while (j < all_splits.GetCount() && i >= all_splits[j])
					j++;
				const String& l = all_lines[i];
				part->Add(l);
			}
			
			summarizations.Clear();
			summarizations.SetCount(natural_parts.GetCount());
			total_summarizations.Clear();
			total_summarizations.SetCount(natural_parts.GetCount());
		}
		
		if (batch >= natural_parts.GetCount()) {
			NextPhase();
			return;
		}
		
		if (sub_batch >= 2) {
			NextBatch();
			return;
		}
		
		
		ValueMap params = this->params;
		params.Add("transcription_part", Join(natural_parts[batch], "\n"));
		if (batch > 0)
			params.Add("prev_summarization", summarizations[batch-1]);
		if (batch > 1)
			params.Add("total_summarization", total_summarizations[batch-1]);
		if (sub_batch == 1) {
			if (batch == 0) {
				// Skip total summarization fo the first batch
				NextBatch();
				return;
			}
			// Use previous summarization for the second batch
			if (batch == 1)
				params.Add("total_summarization", summarizations[0]);
			params.Add("summarization", summarizations[batch]);
		}
		
		TaskArgs args;
		args.params = params;
		if (sub_batch == 0) {
			args.fn = TaskFn::FN_VOICEOVER_2A_SUMMARIZE;
			SetWaiting(1);
			AiTaskManager().Get(args, [this](String res) {
				res.Replace("\r", "");
				res = TrimBoth(res);
				summarizations[batch] = res;
				NextSubBatch();
				SetWaiting(0);
			}, "Voiceover 2/2a: summarize " + IntStr(batch+1) + "/" + IntStr(natural_parts.GetCount()));
		}
		else if (sub_batch == 1) {
			args.fn = TaskFn::FN_VOICEOVER_2B_SUMMARIZE_TOTAL;
			SetWaiting(1);
			AiTaskManager().Get(args, [this](String res) {
				res.Replace("\r", "");
				res = TrimBoth(res);
				total_summarizations[batch] = res;
				NextSubBatch();
				SetWaiting(0);
			}, "Voiceover 2/2b: make total summarization so far " + IntStr(batch+1) + "/" + IntStr(natural_parts.GetCount()));
		}
	}
	/*else if (phase == PHASE_MAKE_STORYLINE_SUGGESTIONS) {
		SetNotRunning();
	}
	else if (phase == PHASE_CLASSIFY_PARTS) {
		SetNotRunning();
	}
	else if (phase == PHASE_GENERATE) {
		SetNotRunning();
	}*/
	else {
		SetNotRunning();
	}
}

VoiceoverProcess& VoiceoverProcess::Get(VfsPath path, Value params) {
	static ArrayMap<hash_t, VoiceoverProcess> arr;
	String key = (String)path + ";" + StoreAsJson(params);
	hash_t hash = key.GetHashValue();
	auto& o = arr.GetAdd(hash);
	o.params = params;
	ASSERT(params.Is<ValueMap>());
	return o;
}

INITIALIZER_COMPONENT(VoiceoverText);

END_UPP_NAMESPACE
