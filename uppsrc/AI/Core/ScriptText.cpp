#include "Core.h"

NAMESPACE_UPP


ScriptTextProcess::ScriptTextProcess() {
	
}
	
int ScriptTextProcess::GetPhaseCount() const {
	return PHASE_COUNT;
}

int ScriptTextProcess::GetBatchCount(int phase) const {
	return 1;
}

int ScriptTextProcess::GetSubBatchCount(int phase, int batch) const {
	return 1;
}
	
void ScriptTextProcess::DoPhase() {
	SrcTextData& data = *this->data;
	
	if (IsPhase(PHASE_INPUT)) {
		String genre = params("genre");
		String author = params("author");
		String title = params("title");
		Value input_text = params("input_text");
		if (genre.IsEmpty()) genre = "User input";
		if (author.IsEmpty()) author = "User";
		if (title.IsEmpty()) title = "<No title>";
		AuthorDataset& ed = data.GetAddAuthor(author);
		VectorFindAdd(ed.genres, genre);
		ScriptDataset& script = ed.GetAddScript(title);
		script.text = AsJSON(input_text, true);
		
		NextPhase();
	}
	else if (IsPhase(PHASE_TOKENIZE)) {
		Tokenize();
	}
	else if (IsPhase(PHASE_ANALYZE_ARTISTS)) {
		AnalyzeArtists();
	}
	else if (IsPhase(PHASE_ANALYZE_ELEMENTS)) {
		AnalyzeElements();
	}
	else {
		SetNotRunning();
	}
}

void ScriptTextProcess::Tokenize() {
	if (tk.IsEmpty()) tk.Create();
	NaturalTokenizer& tk = *this->tk;
	
	ASSERT(p.srctxt);
	auto& src = *p.srctxt;
	Vector<int> token_is;
	Vector<AuthorDataset>& entities = src.authors;
	
	int well_filter_loss = 0, parse_loss = 0, foreign_loss = 0;
	
	bool filter_foreign = true;
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
	
	if (batch >= src.authors.GetCount()) {
		NextPhase();
		return;
	}
	
	auto& author = src.authors[batch];
	if (sub_batch >= author.scripts.GetCount()) {
		NextBatch();
		return;
	}
	
	int worker_total = total++;
	
	auto& script = author.scripts[sub_batch];
	
	String str = script.text;
	
	if (str.Left(1) == "[") {
		Value v = ParseJSON(str);
		str.Clear();
		
		String script_title = author.name + " - " + script.title;
		hash_t ss_hash = script_title.GetHashValue();
		
		int ss_i = src.scripts.Find(ss_hash);
		if (skip_ready && ss_i >= 0) {
			NextSubBatch();
			return;
		}
		
		data_lock.Enter();
		ScriptStruct& ss = MapGetAdd(src.scripts, ss_hash, ss_i);
		ss.parts.Clear();
		
		int prev_msect = -1, prev_sect = -1, prev_ssect = -1;
		ScriptStruct::Part* part = 0;
		ScriptStruct::SubPart* subpart = 0;
		ScriptStruct::SubSubPart* ssubpart = 0;
		int ssub_line_i = 0;
		for(int i = 0; i < v.GetCount(); i++) {
			String line_txt = v[i].ToString();
			int line = i;
			int sect = 0;
			int msect = 0;
			
			if (prev_msect != msect) {
				part = &ss.parts.Add();
				subpart = &part->sub.Add();
				ssubpart = &subpart->sub.Add();
				part->type = -1;
				part->num = -1;
				subpart->repeat = 0;
				ssub_line_i = 0;
			}
			else if (prev_sect != sect) {
				subpart = &part->sub.Add();
				subpart->repeat = 0;
				ssubpart = &subpart->sub.Add();
				ssub_line_i = 0;
			}
			//else if (ssub_line_i > 0 && ssub_line_i % 4 == 0)
			//	ssubpart = &subpart->sub.Add();
			
			if (line_txt.Left(1) == "[")
				continue;
			
			if (!tk.Parse(line_txt))
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
			
			prev_msect = msect;
			prev_sect = sect;
			ssub_line_i++;
		}
		data_lock.Leave();
		
		if (0) {
			LOG(src.GetScriptDump(ss_i));
		}
		
		data_lock.Leave();
	}
	else if (str.Left(1) == "{") {
		Panic("TODO");
	}
	else {
		Vector<String> lines = Split(str, "\n");
		for(int i = 0; i < lines.GetCount(); i++) {
			String& s = lines[i];
			s = TrimBoth(s);
			if (s.Left(1) == "[")
				lines.Remove(i--);
		}
		str = Join(lines, "\n");
		
		// Ignore files with hard ambiguities
		if (0 && str.Find(" well ") >= 0) {
			// well or we'll... too expensive to figure out
			well_filter_loss++;
			NextSubBatch();
			return;
		}
		
		static thread_local TryNo5tStructureSolver solver;
		
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
		
		String script_title = author.name + " - " + script.title;
		hash_t ss_hash = script_title.GetHashValue();
		
		int ss_i = src.scripts.Find(ss_hash);
		if (skip_ready && ss_i >= 0) {
			/*if (0) {
				data_lock.Enter();
				ScriptStruct& ss = src.scripts[ss_i];
				LOG(src.GetScriptDump(ss_i));
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

void ScriptTextProcess::AnalyzeArtists() {
	ASSERT(p.srctxt);
	auto& src = *p.srctxt;
	
	bool user_genres = params("genres").Is<ValueArray>();
	
	if (batch >= src.authors.GetCount() || user_genres) {
		NextPhase();
		return;
	}
	
	AuthorDataset& ent = src.authors[batch];
	if (ent.genres.GetCount()) {
		NextBatch();
		return;
	}
	Panic("TODO");
	#if 0
	args.fn = 1;
	args.artist = ent.name;
	
	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.GetSourceDataAnalysis(args, [this](String result) {
		ASSERT(p.srctxt);
		auto& src = *p.srctxt;
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
		AuthorDataset& ent = src.authors[batch];
		ent.genres <<= genres;
		
		NextBatch();
		SetWaiting(false);
	});
	#endif
}

void ScriptTextProcess::AnalyzeElements() {
	ASSERT(p.srctxt);
	auto& src = *p.srctxt;
	Vector<AuthorDataset>& entities = src.authors;
	
	if (batch >= src.scripts.GetCount()) {
		NextPhase();
		return;
	}
	ScriptStruct& ss = src.scripts[batch];
	if (ss.parts.GetCount() && ss.parts[0].cls >= 0) {
		NextBatch();
		return;
	}
	
	ValueArray text_arg = src.GetScriptValue(batch);
	
	args.fn = FN_ANALYZE_ELEMENTS;
	args.params = ValueMap();
	args.params("text") = text_arg;
	
	//Vector<String> all_sections = Split(args.text, "[");
	if (text_arg.GetCount() == 0 /*|| all_sections.GetCount() >= 50*/) {
		NextBatch();
		return;
	}
	
	// Another hotfix
	/*if (args.text.Find("http://") >= 0 || args.text.Find("https://") >= 0) {
		NextBatch();
		return;
	}*/
	
	bool keep_going = true;
	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	if (m.keep_going_counter >= 50) {
		SetNotRunning();
		return;
	}
	
	m.Get(args, [this](String result) {
		ASSERT(p.srctxt);
		auto& src = *p.srctxt;
		TaskArgs& args = this->args;
		ScriptStruct& ss = src.scripts[batch];
		
		LOG(result);
		Value v = ParseJSON(result);
		Value elements = v("response-short")("elements");
		LOG(UPP::AsJSON(v, true));
		LOG(UPP::AsJSON(elements, true));
		Value input_lines = args.params("text");
		LOG(UPP::AsJSON(input_lines, true));
		
		ValueArray arr0 = elements;
		for(int i = 0; i < ss.parts.GetCount(); i++) {
			auto& p = ss.parts[i];
			ValueArray arr1 = arr0[i];
			int pos = 0;
			for(int j = 0; j < p.sub.GetCount(); j++) {
				auto& s = p.sub[j];
				for(int k = 0; k < s.sub.GetCount(); k++) {
					auto& ss = s.sub[k];
					String val = arr1[pos++].ToString();
					if (!val.IsEmpty()) {
						int el_i = src.element_keys.FindAdd(val);
						ss.cls = el_i;
					}
				}
			}
		}
		
		NextBatch();
		SetWaiting(false);
	});
	
	
}

ScriptTextProcess& ScriptTextProcess::Get(DatasetPtrs p, VfsPath path, Value params, SrcTextData& data, Event<> WhenStopped) {
	static ArrayMap<hash_t, ScriptTextProcess> arr;
	String key = (String)path + ";" + StoreAsJson(params);
	hash_t hash = key.GetHashValue();
	auto& o = arr.GetAdd(hash);
	o.p = p;
	o.params = params;
	o.data = &data;
	o.WhenStopped = WhenStopped;
	ASSERT(params.Is<ValueMap>());
	ASSERT(p.srctxt);
	ASSERT_(p.src, "A complete database must be present");
	return o;
}

INITIALIZER_COMPONENT(ScriptText);

END_UPP_NAMESPACE
