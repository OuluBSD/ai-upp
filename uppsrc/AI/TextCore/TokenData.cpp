#include "TextCore.h"


NAMESPACE_UPP


TokenDataProcess::TokenDataProcess() {
	
}

int TokenDataProcess::GetPhaseCount() const {
	return PHASE_COUNT;
}

int TokenDataProcess::GetBatchCount(int phase) const {
	ASSERT(p.src);
	auto& src = *p.src;
	switch (phase) {
		case PHASE_GET: return (src.tokens.GetCount() + per_action_task - 1) / per_action_task;
		default: return 0;
	}
}

int TokenDataProcess::GetSubBatchCount(int phase, int batch) const {
	return 1;
}

void TokenDataProcess::DoPhase() {
	switch (phase) {
		case PHASE_GET: Get(); return;
		default: break;
	}
}

TokenDataProcess& TokenDataProcess::Get(DatasetPtrs p) {
	static ArrayMap<String, TokenDataProcess> arr;
	
	String key = p.src->filepath;
	TokenDataProcess& ts = arr.GetAdd(key);
	ts.p = pick(p);
	return ts;
}

void TokenDataProcess::Get() {
	ASSERT(p.src);
	auto& src = *p.src;
	auto& wrds = *p.wrd;
	auto& da = *p.da;
	
	TokenArgs& args = token_args;
	args.fn = 0;
	args.words.Clear();
	
	if (batch == 0) total = 0;
	
	int begin = batch * per_action_task;
	int end = begin + per_action_task;
	end = min(end, src.tokens.GetCount());
	int count = end - begin;
	if (count <= 0) {
		NextPhase();
		return;
	}
	
	for(int i = begin; i < end; i++) {
		const String& tk = src.tokens.GetKey(i);
		args.words << tk;
	}
	
	total += count;
	
	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.GetTokenData(args, [this](String result) {
		TokenArgs& args = token_args;
		auto& src = *p.src;
		auto& wrds = *p.wrd;
		auto& da = *p.da;
		
		// 9. suppote: verb | noun
		
		result.Replace("\r", "");
		Vector<String> lines = Split(result, "\n");
		
		int offset = 3+1;
		
		for (String& line : lines) {
			line = TrimBoth(line);
			
			if (line.IsEmpty() ||!IsDigit(line[0]))
				continue;
			
			/*int line_i = ScanInt(line);
			line_i -= offset;
			if (line_i < 0 || line_i >= args.words.GetCount())
				continue;
			
			const String& orig_word = args.words[line_i];*/
			
			int a = line.Find(".");
			if (a < 0) continue;
			line = TrimBoth(line.Mid(a+1));
			
			a = line.Find(":");
			if (a == 0) {
				// Rare case of ":" being asked
				line = ":" + line;
				a = 1;
			}
			else if (a < 0)
				continue;
			
			//int orig_word_i = ;
			
			String result_word = TrimBoth(line.Left(a));
			
			/*ExportWord& wrd =
				orig_word_i >= 0 ?
					da.words[orig_word_i] :
					da.words.GetAdd(result_word, orig_word_i);*/
			int orig_word_i = -1;
			ExportWord& wrd = MapGetAdd(wrds.words, result_word, orig_word_i);
			
			//TODO // token to word
			
			line = TrimBoth(line.Mid(a+1));
			
			a = line.Find("(");
			if (a >= 0)
				line = line.Left(a);
			
			Vector<String> parts = Split(line, "|");
			for (String& p : parts) {
				p = TrimBoth(p);
				int wc_i = wrds.word_classes.FindAdd(p);
				if (wrd.class_count < wrd.MAX_CLASS_COUNT)
					FixedIndexFindAdd(wrd.classes, wrd.MAX_CLASS_COUNT, wrd.class_count, wc_i);
			}
			
			actual++;
		}
		
		
		da.diagnostics.GetAdd("tokens: total") = IntStr(total);
		da.diagnostics.GetAdd("tokens: actual") =  IntStr(actual);
		da.diagnostics.GetAdd("tokens: percentage") =  DblStr((double)actual / (double) total * 100);
		
		NextBatch();
		SetWaiting(false);
	});
}


END_UPP_NAMESPACE
