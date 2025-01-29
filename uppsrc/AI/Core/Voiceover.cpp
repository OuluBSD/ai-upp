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
	ValueMap params = this->params;
	String scene = params("scene");
	String people = params("people");
	String transcription = params("transcription");
	
	if (transcription.IsEmpty()) {
		Fail("transcription required, but is empty");
		return;
	}
	
	if (phase == PHASE_FIND_NATURAL_PARTS) {
		
		if (IsPhaseInit()) {
			// Make coarse input parts
			input_coarse_parts.Clear();
			transcription.Replace("\r", "");
			lines = Split(transcription, "\n");
			const int per_part = 1000;
			Vector<String> cur_part;
			int cur_part_total = 0;
			for(int i = 0; i < lines.GetCount(); i++) {
				String& line = lines[i];
				line = TrimBoth(line);
				if (line.IsEmpty())
					continue;
				if (cur_part_total > 0 && cur_part_total + line.GetCount() >= per_part) {
					Swap(input_coarse_parts.Add(), cur_part); // add part
					cur_part_total = 0;
				}
				cur_part << lines[i] << "\n";
				cur_part_total += lines[i].GetCount() + 1;
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
		}
	}
	else if (phase == PHASE_CLASSIFY_NATURAL_PARTS) {
		SetNotRunning();
	}
	else if (phase == PHASE_MAKE_STORYLINE_SUGGESTIONS) {
		SetNotRunning();
	}
	else if (phase == PHASE_CLASSIFY_PARTS) {
		SetNotRunning();
	}
	else if (phase == PHASE_GENERATE) {
		SetNotRunning();
	}
	else {
		SetError("Invalid phase");
		SetNotRunning();
		return;
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
