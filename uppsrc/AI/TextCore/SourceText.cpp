#include "TextCore.h"

NAMESPACE_UPP


SourceDataImporter::SourceDataImporter() {
	skip_ready = false;
	SetParallel();
}

int SourceDataImporter::GetPhaseCount() const {
	return PHASE_COUNT;
}

int SourceDataImporter::GetBatchCount(int phase) const {
	ASSERT(p.src);
	return p.src->entities.GetCount();
}

int SourceDataImporter::GetSubBatchCount(int phase, int batch) const {
	ASSERT(p.src);
	if (batch >= p.src->entities.GetCount())
		return 1;
	auto& entity = p.src->entities[batch];
	return entity.scripts.GetCount();
}

void SourceDataImporter::DoPhase() {
	switch (phase) {
		case PHASE_TOKENIZE:		Tokenize();		return;
		default: TODO;
	}
}

void SourceDataImporter::Tokenize() {
	TextDatabase& db = GetDatabase();
	Vector<int> token_is;
	ASSERT(p.src);
	SrcTextData& src = *p.src;
	Vector<EntityDataset>& entities = src.entities;
	
	int well_filter_loss = 0, parse_loss = 0, foreign_loss = 0;
	
	{
		int lng_i = src.GetLanguage();
		if (lng_i != LNG_ENGLISH)
			filter_foreign = false;
	}
	
	int phase = this->phase, batch = this->batch, sub_batch = this->sub_batch;
	if (parallel) {
		WorkerData& w = Worker();
		phase = w.phase;
		batch = w.batch;
		sub_batch = w.sub_batch;
	}
	
	if (!batch && !sub_batch) {
		total = 0;
		actual = 0;
		ts.Reset();
	}
	
	if (batch >= src.entities.GetCount()) {
		NextPhase();
		return;
	}
	
	auto& entity = src.entities[batch];
	if (sub_batch >= entity.scripts.GetCount()) {
		NextBatch();
		return;
	}
	
	int worker_total = total++;
	
	auto& script = entity.scripts[sub_batch];
	
	String str = script.text;
	
	{
		Vector<String> lines = Split(str, "\n");
		for(int i = 0; i < lines.GetCount(); i++) {
			String& s = lines[i];
			s = TrimBoth(s);
			if (s.Left(1) == "[")
				lines.Remove(i--);
		}
		str = Join(lines, "\n");
	}
	
	// Ignore files with hard ambiguities
	if (str.Find(" well ") >= 0) {
		// well or we'll... too expensive to figure out
		well_filter_loss++;
		NextSubBatch();
		return;
	}
	
	static thread_local TryNo5tStructureSolver solver;
	static thread_local NaturalTokenizer tk;
	
	tk.Clear();
	HotfixReplaceWord(str);
	if (!tk.Parse(str)) {
		parse_loss++;
		NextSubBatch();
		return;
	}
	if (filter_foreign && tk.HasForeign()) {
		foreign_loss++;
		NextSubBatch();
		return;
	}
	
	String script_title = entity.name + " - " + script.name;
	hash_t ss_hash = script_title.GetHashValue();
	
	int ss_i = src.scripts.Find(ss_hash);
	if (skip_ready && ss_i >= 0) {
		/*if (0) {
			data_lock.Enter();
			ScriptStruct& ss = da.scripts[ss_i];
			LOG(da.GetScriptDump(ss_i));
			data_lock.Leave();
		}*/
		NextSubBatch();
		return;
	}
	
	// Slow solver process
	solver.Process(script.text);
	
	data_lock.Enter();
	ScriptStruct& ss = MapGetAdd(src.scripts, ss_hash, ss_i);
	ss.parts.Clear();
	
	int prev_msect = -1, prev_sect = -1, prev_ssect = -1;
	ScriptStruct::Part* part = 0;
	ScriptStruct::SubPart* subpart = 0;
	ScriptStruct::SubSubPart* ssubpart = 0;
	int ssub_line_i = 0;
	for(int i = 0; i < solver.lines.GetCount(); i++) {
		auto& line = solver.lines[i];
		auto& sect = solver.sections[line.section];
		auto& msect = solver.meta_sections[sect.meta_section];
		
		if (prev_msect != sect.meta_section) {
			part = &ss.parts.Add();
			subpart = &part->sub.Add();
			ssubpart = &subpart->sub.Add();
			part->type = msect.type;
			part->num = msect.num;
			subpart->repeat = sect.repeat;
			ssub_line_i = 0;
		}
		else if (prev_sect != line.section) {
			subpart = &part->sub.Add();
			subpart->repeat = sect.repeat;
			ssubpart = &subpart->sub.Add();
			ssub_line_i = 0;
		}
		else if (ssub_line_i > 0 && ssub_line_i % 4 == 0)
			ssubpart = &subpart->sub.Add();
		
		if (line.txt.Left(1) == "[")
			continue;
		
		if (!tk.Parse(line.txt))
			continue;
		
		if (tk.GetLines().GetCount() == 1) {
			//data_lock.Enter();
			const auto& line = tk.GetLines()[0];
			
			token_is.SetCount(0);
			CombineHash ch;
			for (const WString& line : line) {
				String s = line.ToString();
				int tk_i = -1;
				Token& tk = MapGetAdd(src.tokens, s, tk_i);
				ch.Do(tk_i);
				token_is << tk_i;
			}
			hash_t h = ch;
			
			int tt_i = -1;
			TokenText& tt = MapGetAdd(src.token_texts, h, tt_i);
			if (tt.tokens.IsEmpty()) {
				Swap(tt.tokens, token_is);
			}
			
			if (tt_i >= 0)
				ssubpart->token_texts << tt_i;
			//data_lock.Leave();
		}
		
		prev_msect = sect.meta_section;
		prev_sect = line.section;
		ssub_line_i++;
	}
	data_lock.Leave();
	
	if (0) {
		LOG(src.GetScriptDump(ss_i));
	}
	
	actual++;
	NextSubBatch();
	
	if (worker_total % 500 == 0) {
		src.diagnostics.GetAdd("SourceDataImporter: total") = IntStr(total);
		src.diagnostics.GetAdd("SourceDataImporter: actual") =  IntStr(actual);
		src.diagnostics.GetAdd("SourceDataImporter: percentage") =  DblStr((double)actual / (double) total * 100);
		src.diagnostics.GetAdd("SourceDataImporter: filter 'well' loss") =  DblStr((double)well_filter_loss / (double) total * 100);
		src.diagnostics.GetAdd("SourceDataImporter: filter 'parse success' loss") =  DblStr((double)parse_loss / (double) total * 100);
		src.diagnostics.GetAdd("SourceDataImporter: filter 'foreign' loss") =  DblStr((double)foreign_loss / (double) total * 100);
		src.diagnostics.GetAdd("SourceDataImporter: duration of song process") =  ts.ToString();
	}
}

SourceDataImporter& SourceDataImporter::Get(DatasetPtrs p) {
	static ArrayMap<String, SourceDataImporter> arr;
	
	String key = p.src->filepath;
	auto& ts = arr.GetAdd(key);
	ts.p = pick(p);
	return ts;
}






















SourceAnalysisProcess::SourceAnalysisProcess() {
	
}

int SourceAnalysisProcess::GetPhaseCount() const {
	return PHASE_COUNT;
}

int SourceAnalysisProcess::GetBatchCount(int phase) const {
	
	switch (phase) {
		case PHASE_ANALYZE_ARTISTS:			return p.src->entities.GetCount();
		case PHASE_ANALYZE_ELEMENTS:		return p.src->scripts.GetCount();
		case PHASE_SUMMARIZE_CONTENT:		TODO;
		default: return 1;
	}
}

int SourceAnalysisProcess::GetSubBatchCount(int phase, int batch) const {
	return 1;
}

void SourceAnalysisProcess::DoPhase() {
	switch (phase) {
		case PHASE_ANALYZE_ARTISTS:			AnalyzeArtists(); return;
		case PHASE_ANALYZE_ELEMENTS:		AnalyzeElements(); return;
		case PHASE_SUMMARIZE_CONTENT:		SummarizeContent(); return;
		default: break;
	}
}

void SourceAnalysisProcess::AnalyzeArtists() {
	ASSERT(p.src);
	auto& src = *p.src;
	
	if (batch >= src.entities.GetCount()) {
		NextPhase();
		return;
	}
	
	EntityDataset& ent = src.entities[batch];
	if (ent.genres.GetCount()) {
		NextBatch();
		return;
	}
	args.fn = 1;
	args.artist = ent.name;
	
	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.GetSourceDataAnalysis(args, [this](String result) {
		auto& src = *p.src;
		SourceDataAnalysisArgs& args = this->args;
		
		RemoveEmptyLines3(result);
		RemoveEmptyLines2(result);
		//LOG(result);
		
		Vector<String> genres = Split(result, "\n");
		for (String& genre : genres) {
			genre = ToLower(TrimBoth(genre));
			int i = genre.Find(":");
			if (i >= 0)
				genre = TrimBoth(genre.Mid(i+1));
		}
		EntityDataset& ent = src.entities[batch];
		ent.genres <<= genres;
		
		NextBatch();
		SetWaiting(false);
	});
}

void SourceAnalysisProcess::AnalyzeElements() {
	ASSERT(p.src);
	auto& src = *p.src;
	Vector<EntityDataset>& entities = src.entities;
	
	if (batch >= src.scripts.GetCount()) {
		NextPhase();
		return;
	}
	ScriptStruct& ss = src.scripts[batch];
	if (ss.parts.GetCount() && ss.parts[0].cls >= 0) {
		NextBatch();
		return;
	}
	
	args.fn = 0;
	args.text = TrimBoth(src.GetScriptDump(batch));
	Vector<String> all_sections = Split(args.text, "[");
	if (args.text.IsEmpty() || all_sections.GetCount() >= 50) {
		NextBatch();
		return;
	}
	
	// Another hotfix
	if (args.text.Find("http://") >= 0 || args.text.Find("https://") >= 0) {
		NextBatch();
		return;
	}
	
	bool keep_going = true;
	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	if (m.keep_going_counter >= 50) {
		SetNotRunning();
		return;
	}
	
	m.GetSourceDataAnalysis(args, [this](String result) {
		ASSERT(p.src);
		auto& src = *p.src;
		SourceDataAnalysisArgs& args = this->args;
		ScriptStruct& ss = src.scripts[batch];
		
		RemoveEmptyLines3(result);
		//LOG(result);
		
		Vector<String> lines = Split(result, "\n");
		VectorMap<String,String> section_values;
		for (String& l : lines) {
			int a = l.Find("[");
			if (a < 0) continue;
			a++;
			int b = l.Find("]", a);
			if (b < 0) continue;
			String key = l.Mid(a,b-a);
			a = l.Find(":", b);
			if (a < 0) continue;
			a++;
			String value = ToLower(TrimBoth(l.Mid(a)));
			RemoveQuotes(value);
			for(int i = 0; i < key.GetCount(); i++) {
				int chr = key[i];
				if (chr == '.' || IsDigit(chr))
					continue;
				key = key.Left(i);
				break;
			}
			if (key.IsEmpty() || value.IsEmpty())
				continue;
			section_values.GetAdd(key, value);
		}
		for(int i = 0; i < ss.parts.GetCount(); i++) {
			auto& p = ss.parts[i];
			String key;
			key << i;
			int l = section_values.Find(key);
			if (l >= 0) {
				String& val = section_values[l];
				int el_i = src.element_keys.FindAdd(val);
				p.cls = el_i;
			}
			
			for(int j = 0; j < p.sub.GetCount(); j++) {
				auto& s = p.sub[j];
				String key;
				key << i << "." << j;
				int l = section_values.Find(key);
				if (l >= 0) {
					String& val = section_values[l];
					int el_i = src.element_keys.FindAdd(val);
					s.cls = el_i;
				}
				
				for(int k = 0; k < s.sub.GetCount(); k++) {
					auto& ss = s.sub[k];
					String key;
					key << i << "." << j << "." << k;
					int l = section_values.Find(key);
					if (l >= 0) {
						String& val = section_values[l];
						int el_i = src.element_keys.FindAdd(val);
						ss.cls = el_i;
					}
				}
			}
		}
		
		NextBatch();
		SetWaiting(false);
	}, keep_going);
	
	
}

void SourceAnalysisProcess::SummarizeContent() {
	
}

SourceAnalysisProcess& SourceAnalysisProcess::Get(DatasetPtrs p) {
	static ArrayMap<String, SourceAnalysisProcess> arr;
	
	String key = p.src->filepath;
	auto& ts = arr.GetAdd(key);
	ts.p = pick(p);
	return ts;
}


















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
	auto& ts = arr.GetAdd(key);
	ts.p = pick(p);
	return ts;
}

void TokenDataProcess::Get() {
	ASSERT(p.src);
	auto& src = *p.src;
	
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
		
		// 9. suppose: verb | noun
		
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
			ExportWord& wrd = MapGetAdd(src.words, result_word, orig_word_i);
			
			//TODO // token to word
			
			line = TrimBoth(line.Mid(a+1));
			
			a = line.Find("(");
			if (a >= 0)
				line = line.Left(a);
			
			Vector<String> parts = Split(line, "|");
			for (String& p : parts) {
				p = TrimBoth(p);
				int wc_i = src.word_classes.FindAdd(p);
				if (wrd.class_count < wrd.MAX_CLASS_COUNT)
					FixedIndexFindAdd(wrd.classes, wrd.MAX_CLASS_COUNT, wrd.class_count, wc_i);
			}
			
			actual++;
		}
		
		
		src.diagnostics.GetAdd("tokens: total") = IntStr(total);
		src.diagnostics.GetAdd("tokens: actual") =  IntStr(actual);
		src.diagnostics.GetAdd("tokens: percentage") =  DblStr((double)actual / (double) total * 100);
		
		NextBatch();
		SetWaiting(false);
	});
}

END_UPP_NAMESPACE
