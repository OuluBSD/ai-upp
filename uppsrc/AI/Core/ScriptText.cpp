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
	else if (IsPhase(PHASE_WORD_CLASSES)) {
		WordClasses();
	}
	else if (IsPhase(PHASE_AMBIGUOUS_WORD_PAIRS)) {
		AmbiguousWordPairs();
	}
	else if (IsPhase(PHASE_IMPORT_TOKEN_TEXTS)) {
		ImportTokenTexts();
	}
	else if (IsPhase(PHASE_CLASSIFY_SENTENCES)) {
		ClassifySentences();
	}
	else if (IsPhase(PHASE_VIRTUAL_PHRASE_PARTS)) {
		VirtualPhraseParts();
	}
	else if (IsPhase(PHASE_VIRTUAL_PHRASE_PART_STRUCTS)) {
		VirtualPhraseStructs();
	}
	else if (IsPhaseRange(PHASE_ELEMENT,PHASE_CONTENT)) {
		PhrasePartAnalysis();
	}
	else if (IsPhase(PHASE_COLORS)) {
		Colors();
	}
	else if (IsPhase(PHASE_ATTRS)) {
		Attrs();
	}
	else if (IsPhase(PHASE_MAIN_GROUPS)) {
		MainGroups();
	}
	else if (IsPhase(PHASE_SIMPLIFY_ATTRS)) {
		SimplifyAttrs();
	}
	else if (IsPhase(PHASE_JOIN_ORPHANED)) {
		JoinOrphaned();
	}
	else if (IsPhase(PHASE_FIX_DATA)) {
		FixData();
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
		
		Value v = ParseJSON(result);
		Value elements = v("response-short")("elements");
		Value input_lines = args.params("text");
		
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

void ScriptTextProcess::WordClasses() {
	ASSERT(p.srctxt);
	auto& src = *p.srctxt;
	
	args.fn = FN_WORD_CLASSES;
	args.params = ValueMap();
	
	if (batch == 0) total = 0;
	
	int begin = batch * words_per_action_task;
	int end = begin + words_per_action_task;
	end = min(end, src.tokens.GetCount());
	int count = end - begin;
	if (count <= 0) {
		NextPhase();
		return;
	}
	
	ValueArray words;
	for(int i = begin; i < end; i++) {
		const String& tk = src.tokens.GetKey(i);
		words << tk;
	}
	args.params("words") = words;
	
	total += count;
	
	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.Get(args, [this](String result) {
		TaskArgs& args = this->args;
		auto& src = p.src->Data();
		
		Value v = ParseJSON(result);
		Value word_classes = v("response-short")("word classes");
		Value input_words = args.params("words");
		LOG(AsJSON(v, true));
		
		#if 0
		
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
		
		#endif
		
		NextBatch();
		SetWaiting(false);
	});
}

void ScriptTextProcess::AmbiguousWordPairs() {
	ASSERT(p.src);
	auto& src = p.src->Data();
	
	args.fn = FN_WORD_PAIR_CLASSES;
	
	Panic("TODO");
	#if 0
	args.words.Clear();
	
	int begin = batch * per_action_task;
	int end = begin + per_action_task;
	end = min(end, src.ambiguous_word_pairs.GetCount());
	int iter = 0;
	
	tmp_ptrs.Clear();
	
	for (const WordPairType& wp : src.ambiguous_word_pairs.GetValues()) {
		if (wp.from < 0 || wp.to < 0)
			continue;
		if (wp.from_type >= 0 && wp.to_type >= 0)
			continue;
		
		if (iter >= begin && iter < end) {
			const String& from = src.words.GetKey(wp.from);
			const String& to = src.words.GetKey(wp.to);
			args.words << (from + " " + to);
			tmp_ptrs << (void*)&wp;
		}
		else if (iter >= end)
			break;
		iter++;
	}
	if (args.words.IsEmpty()) {
		NextPhase();
		return;
	}
	
	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.GetTokenData(args, [this](String res) {
		TokenArgs& args = token_args;
		ASSERT(p.src);
		auto& src = p.src->Data();
		
		// 9. is something : verb, noun
		
		RemoveEmptyLines(res);
		Vector<String> lines = Split(res, "\n");
		
		bool line_match = tmp_ptrs.GetCount() == lines.GetCount();
		int offset = 1+1;
		
		int line_i = -1;
		for (String& line : lines) {
			line_i++;
			line = TrimBoth(line);
			
			if (line.IsEmpty() ||!IsDigit(line[0]))
				continue;
			
			int a = line.Find(".");
			if (a < 0) continue;
			line = TrimBoth(line.Mid(a+1));
			
			a = line.ReverseFind(":");
			if (a < 0)
				continue;
			
			Vector<String> result_words = Split(TrimBoth(line.Left(a)), " ");
			if (result_words.GetCount() != 2)
				continue;
			
			WordPairType* wpp;
			if (line_match)
				wpp = (WordPairType*)tmp_ptrs[line_i];
			else {
				int w_i0 = src.words.Find(result_words[0]);
				int w_i1 = src.words.Find(result_words[1]);
				CombineHash ch;
				ch.Do(w_i0).Put(1).Do(w_i1);
				hash_t h = ch;
				
				//ExportWord& wrd0 = src.words[w_i0];
				//ExportWord& wrd1 = src.words[w_i1];
				wpp = &src.ambiguous_word_pairs.GetAdd(h);
			}
			WordPairType& wp = *wpp;
			
			line = TrimBoth(line.Mid(a+1));
			
			Vector<String> parts = Split(line, ",");
			if (parts.GetCount() != 2)
				continue;
			int wc_i_list[2];
			for(int i = 0; i < parts.GetCount(); i++) {
				String& p = parts[i];
				p = TrimBoth(p);
				wc_i_list[i] = src.word_classes.FindAdd(p);
			}
			
			wp.from_type = wc_i_list[0];
			wp.to_type = wc_i_list[1];
		}
		
		int a = 0;
		for (const WordPairType& wp : src.ambiguous_word_pairs.GetValues()) {
			if (wp.from < 0 || wp.to < 0)
				continue;
			if (wp.from_type >= 0 && wp.to_type >= 0)
				a++;
		}
		src.diagnostics.GetAdd("ambiguous word pairs: total") = IntStr(src.ambiguous_word_pairs.GetCount());
		src.diagnostics.GetAdd("ambiguous word pairs: actual") = IntStr(a);
		src.diagnostics.GetAdd("ambiguous word pairs: percentage") =  DblStr((double)a / (double)src.ambiguous_word_pairs.GetCount() * 100);
		
		NextBatch();
		SetWaiting(false);
	});
	#endif
}

void ScriptTextProcess::ImportTokenTexts() {
	ASSERT(p.src);
	auto& src = p.src->Data();
	
	Vector<int> word_is, word_classes;
	actual = 0;
	total = 0;
	for(int i = 0; i < src.token_texts.GetCount(); i++) {
		TokenText& txt = src.token_texts[i];
		total++;
		
		bool succ = true;
		word_is.SetCount(0);
		word_classes.SetCount(0);
		for(int tk_i : txt.tokens) {
			const Token& tk = src.tokens[tk_i];
			int w_i = tk.word_;
			if (w_i < 0) {
				String key = src.tokens.GetKey(tk_i);
				w_i = src.words.Find(key);
				if (w_i < 0) {
					key = ToLower(src.tokens.GetKey(tk_i));
					w_i = src.words.Find(key);
				}
				tk.word_ = w_i;
			}
			word_is << w_i;
		}
		
		int prev_w_i = -1;
		for(int j = 0; j < word_is.GetCount(); j++) {
			int w_i = word_is[j];
			int next_w_i = j+1 < word_is.GetCount() ? word_is[j] : -1;
			succ = succ && GetTypePhrase(word_classes, src, next_w_i, w_i, prev_w_i);
			prev_w_i = w_i;
		}
		
		if (word_classes.IsEmpty())
			succ = false;
		
		if (succ) {
			CombineHash ch;
			for (int wc_i : word_classes)
				ch.Do(wc_i);
			hash_t h = ch;
			
			int vp_i = -1;
			VirtualPhrase& vp = MapGetAdd(src.virtual_phrases, h, vp_i);
			Swap(word_classes, vp.word_classes);
			
			txt.virtual_phrase = vp_i;
			actual++;
		}
	}
	
	src.diagnostics.GetAdd("token texts to virtual phrases: total") = IntStr(total);
	src.diagnostics.GetAdd("token texts to virtual phrases: actual") =  IntStr(actual);
	src.diagnostics.GetAdd("token texts to virtual phrases: percentage") =  DblStr((double)actual / (double) total * 100);
	
	//int punctuation_mark_i = src.word_classes.FindAdd("punctuation mark");
	//int punctuation_i = src.word_classes.FindAdd("punctuation");
	for(int i = 0; i < src.virtual_phrase_parts.GetCount(); i++)
		src.virtual_phrase_parts[i].count = 0;
	
	for(int i = 0; i < src.virtual_phrases.GetCount(); i++) {
		VirtualPhrase& vp = src.virtual_phrases[i];
		Vector<Vector<int>> tmps;
		Vector<int> tmp;
		
		// NOTE: see duplicate in fn 3
		
		for (int wc_i : vp.word_classes) {
			String wc = src.word_classes[wc_i];
			int a = wc.Find("punctuation");
			int b = wc.Find("conjunction");
			//if (type == punctuation_mark_i || type == punctuation_i) {
			if (a >= 0 || b >= 0) {
				if (tmp.GetCount()) {
					Swap(tmps.Add(), tmp);
					tmp.SetCount(0);
				}
				if (b >= 0)
					tmp << wc_i;
			}
			else
				tmp << wc_i;
		}
		if (tmp.GetCount()) {
			Swap(tmps.Add(), tmp);
			tmp.SetCount(0);
		}
		CombineHash struct_ch;
		Vector<int> vpp_is;
		for (const Vector<int>& tmp : tmps) {
			CombineHash ch;
			for (int type : tmp)
				ch.Do(type).Put(1);
			hash_t h = ch;
			
			int vpp_i = -1;
			VirtualPhrasePart& vpp = MapGetAdd(src.virtual_phrase_parts, h, vpp_i);
			if (vpp.word_classes.IsEmpty())
				vpp.word_classes <<= tmp;
			vpp.count++;
			vpp_is << vpp_i;
			struct_ch.Do(vpp_i).Put(1);
		}
		hash_t vps_h = struct_ch;
		int vps_i = -1;
		VirtualPhraseStruct& vps = MapGetAdd(src.virtual_phrase_structs, vps_h, vps_i);
		//if (vps.parts.IsEmpty())
			vps.virtual_phrase_parts <<= vpp_is;
		vp.virtual_phrase_struct = vps_i;
	}
	LOG(src.virtual_phrase_parts.GetCount());
	LOG(src.virtual_phrase_parts.GetCount() * 100.0 / src.virtual_phrases.GetCount());
	NextPhase();
}

void ScriptTextProcess::ClassifySentences() {
	ASSERT(p.src);
	auto& src = p.src->Data();
	
	args.fn = FN_CLASSIFY_SENTENCE;
	Panic("TODO");
	#if 0
	args.words.Clear();
	
	int iter = 0;
	int begin = batch * per_action_task;
	int end = begin + per_action_task;
	end = min(end, src.virtual_phrase_parts.GetCount());
	tmp_ptrs.Clear();
	
	for (const VirtualPhrasePart& vpp : src.virtual_phrase_parts.GetValues()) {
		
		if (vpp.struct_part_type >= 0)
			continue;
		
		if (iter >= begin && iter < end) {
			String s;
			int punct_count = 0;
			bool fail = false;
			for(int j = 0; j < vpp.word_classes.GetCount(); j++) {
				if (j) s << ",";
				int wc_i = vpp.word_classes[j];
				if (wc_i >= src.word_classes.GetCount()) {fail = true; break;}
				String wc = src.word_classes[wc_i];
				
				int a = wc.Find("(");
				if (a >= 0) wc = wc.Left(a);
				a = wc.Find(",");
				if (a >= 0) wc = wc.Left(a);
				
				if (wc.Find("punctuation") >= 0)
					punct_count++;
				
				s << wc;
			}
			
			if (punct_count > 8 || fail)
				continue;
			
			args.words << s;
			tmp_ptrs << (void*)&vpp;
		}
		else if (iter >= end)
			break;
		
		iter++;
	}
	
	if (args.words.IsEmpty()) {
		NextPhase();
		return;
	}
	
	
	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.GetTokenData(args, [this](String res) {
		TokenArgs& args = token_args;
		ASSERT(p.src);
		auto& src = p.src->Data();
		
		actual = 0;
		total = 0;
		
		RemoveEmptyLines(res);
		Vector<String> lines = Split(res, "\n");
		bool line_match = tmp_ptrs.GetCount() == lines.GetCount();
		
		Vector<int> word_classes;
		int line_i = -1;
		for (String& line : lines) {
			line_i++;
			line = TrimBoth(line);
			
			if (line.IsEmpty() ||!IsDigit(line[0]))
				continue;
			
			int a = line.Find(".");
			if (a < 0) continue;
			line = TrimBoth(line.Mid(a+1));
			
			a = line.ReverseFind(":");
			if (a < 0)
				continue;
			
			Vector<String> classes = Split(TrimBoth(line.Left(a)), ",");
			word_classes.SetCount(0);
			
			VirtualPhrasePart* vpp_p;
			if (line_match)
				vpp_p = (VirtualPhrasePart*)tmp_ptrs[line_i];
			else {
				bool fail = false;
				CombineHash ch;
				for (String& c : classes) {
					c = TrimBoth(c);
					int wc_i = src.word_classes.FindAdd(c);
					if (wc_i < 0) {
						fail = true;
						break;
					}
					word_classes << wc_i;
					ch.Do(wc_i).Put(1);
				}
				if (fail) continue;
				hash_t h = ch;
				vpp_p = &src.virtual_phrase_parts.GetAdd(h);
			}
			VirtualPhrasePart& vpp = *vpp_p;
			
			if (vpp.word_classes.IsEmpty())
				vpp.word_classes <<= word_classes;
			
			line = TrimBoth(line.Mid(a+1));
			
			vpp.struct_part_type = src.struct_part_types.FindAdd(line);
		}
		
		
		int a = 0;
		for (const VirtualPhrasePart& vpp : src.virtual_phrase_parts.GetValues())
			if (vpp.struct_part_type >= 0)
				a++;
		src.diagnostics.GetAdd("virtual phrases: total") = IntStr(src.virtual_phrase_parts.GetCount());
		src.diagnostics.GetAdd("virtual phrases: actual") =  IntStr(a);
		src.diagnostics.GetAdd("virtual phrases: percentage") =  DblStr((double)a / (double) src.virtual_phrase_parts.GetCount() * 100);
		
		
		NextBatch();
		SetWaiting(false);
	});
	#endif
}

void ScriptTextProcess::VirtualPhraseParts() {
	Panic("TODO");
	#if 0
	TokenArgs& args = token_args;
	ASSERT(p.src);
	auto& src = p.src->Data();
	
	args.fn = FN_CLASSIFY_SENTENCE_STRUCTURES;
	args.words.Clear();
	
	int begin = batch * per_action_task;
	int end = begin + per_action_task;
	end = min(end, src.virtual_phrase_structs.GetCount());
	
	int iter = 0;
	tmp_ptrs.Clear();
	for (const VirtualPhraseStruct& vps : src.virtual_phrase_structs.GetValues()) {
		if (vps.struct_type >= 0)
			continue;
		
		if (iter >= begin && iter < end) {
			String s;
			bool fail = false;
			for(int j = 0; j < vps.virtual_phrase_parts.GetCount(); j++) {
				if (j) s << " + ";
				int vpp_i = vps.virtual_phrase_parts[j];
				
				const VirtualPhrasePart& vpp = src.virtual_phrase_parts[vpp_i];
				if (vpp.struct_part_type < 0) {
					fail = true;
					break;
				}
				
				String type_str = src.struct_part_types[vpp.struct_part_type];
				if (type_str.IsEmpty()) {
					fail = true;
					break;
				}
				s << type_str;
			}
			if (fail)
				continue;
			if (s.IsEmpty())
				continue;
			
			args.words << s;
			tmp_ptrs << (void*)&vps;
		}
		else if (iter >= end)
			break;
		
		iter++;
	}
	
	if (args.words.IsEmpty()) {
		NextPhase();
		return;
	}
	
	
	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.GetTokenData(args, [this](String res) {
		TokenArgs& args = token_args;
		ASSERT(p.src);
		auto& src = p.src->Data();
		
		// 61. compound-complex sentence + complex sentence: compound-complex sentence
		
		int offset = 3+1;
		RemoveEmptyLines(res);
		Vector<String> lines = Split(res, "\n");
		bool line_match = tmp_ptrs.GetCount() == lines.GetCount();
		
		for (String& line : lines) {
			line = TrimBoth(line);
			
			if (line.IsEmpty() ||!IsDigit(line[0]))
				continue;
			
			int a = line.Find(".");
			if (a < 0) continue;
			
			int line_i = ScanInt(line.Left(a));
			line_i -= offset;
			if (line_i < 0 || line_i >= tmp_ptrs.GetCount())
				continue;
			
			VirtualPhraseStruct& vps = *(VirtualPhraseStruct*)tmp_ptrs[line_i];
			
			line = TrimBoth(line.Mid(a+1));
			
			a = line.ReverseFind(":");
			if (a < 0)
				continue;
			
			Vector<String> classes = Split(TrimBoth(line.Left(a)), "+", false);
			//sp_is.SetCount(0);
			bool fail = false;
			CombineHash ch;
			for (String& c : classes) {
				c = TrimBoth(c);
				if (c.IsEmpty()) {
					fail = true;
					break;
				}
				int sp_i = src.struct_part_types.Find(c);
				if (sp_i < 0) {
					fail = true;
					break;
				}
				//sp_is << sp_i;
				ch.Do(sp_i).Put(1);
			}
			if (fail)
				continue;
			
			String struct_type = TrimBoth(line.Mid(a+1));
			
			vps.struct_type = src.struct_types.FindAdd(struct_type);
		}
		
		int a = 0;
		for (const VirtualPhraseStruct& vps : src.virtual_phrase_structs.GetValues())
			if (vps.struct_type >= 0)
				a++;
		src.diagnostics.GetAdd("virtual phrase structs: total") = IntStr(src.virtual_phrase_structs.GetCount());
		src.diagnostics.GetAdd("virtual phrase structs: actual") =  IntStr(a);
		src.diagnostics.GetAdd("virtual phrase structs: percentage") =  DblStr((double)a / (double) src.virtual_phrase_structs.GetCount() * 100);
		
		NextBatch();
		SetWaiting(false);
	});
	#endif
}

void ScriptTextProcess::VirtualPhraseStructs() {
	ASSERT(p.src);
	auto& src = p.src->Data();
	
	
	if (batch >= src.token_texts.GetCount()) {
		src.diagnostics.GetAdd("token text to phrase: total") = IntStr(src.token_texts.GetCount());
		src.diagnostics.GetAdd("token text to phrase: actual") = IntStr(actual);
		src.diagnostics.GetAdd("token text to phrase: percentage") =  DblStr((double)actual / (double)src.token_texts.GetCount() * 100);
		NextPhase();
		return;
	}
	
	
	// NOTE: see duplicate in fn 0
	// TODO reduce duplicate code
	Vector<int> word_is, word_classes;
	int i = batch;
	{
		const TokenText& txt = src.token_texts[i];
		if (txt.virtual_phrase < 0) {
			NextBatch();
			return;
		}
		
		// NOTE: see duplicate in fn 0
		bool succ = true;
		word_is.SetCount(0);
		word_classes.SetCount(0);
		for(int tk_i : txt.tokens) {
			const Token& tk = src.tokens[tk_i];
			int w_i = tk.word_;
			if (w_i < 0) {
				String key = src.tokens.GetKey(tk_i);
				w_i = src.words.Find(key);
				if (w_i < 0) {
					key = ToLower(src.tokens.GetKey(tk_i));
					w_i = src.words.Find(key);
				}
				tk.word_ = w_i;
			}
			word_is << w_i;
		}
		
		const VirtualPhrase& vp = src.virtual_phrases[txt.virtual_phrase];
		if (word_is.GetCount() != vp.word_classes.GetCount()) {
			NextBatch();
			return;
		}
		
		actual++;
		
		Vector<Vector<int>> w_isv, wc_isv;
		Vector<int> w_is, wc_is;
		
		// NOTE: see duplicate in fn 0
		int c = word_is.GetCount();
		for(int j = 0; j < vp.word_classes.GetCount(); j++) {
			int w_i = word_is[j];
			int wc_i = vp.word_classes[j];
			
			String wc = src.word_classes[wc_i];
			int a = wc.Find("punctuation");
			int b = wc.Find("conjunction");
			//if (type == punctuation_mark_i || type == punctuation_i) {
			if (a >= 0 || b >= 0) {
				if (w_is.GetCount()) {
					Swap(w_isv.Add(), w_is);
					Swap(wc_isv.Add(), wc_is);
					w_is.SetCount(0);
					wc_is.SetCount(0);
				}
				if (b >= 0) {
					wc_is << wc_i;
					w_is << w_i; // NOTE: this is NOT duplicate
				}
			}
			else {
				wc_is << wc_i;
				w_is << w_i;
			}
		}
		if (w_is.GetCount()) {
			Swap(wc_isv.Add(), wc_is);
			Swap(w_isv.Add(), w_is);
			wc_is.SetCount(0);
			w_is.SetCount(0);
		}
		
		
		for(int j = 0; j < w_isv.GetCount(); j++) {
			const Vector<int>& wc_is = wc_isv[j];
			const Vector<int>& w_is = w_isv[j];
			
			hash_t wc_h, w_h;
			{
				CombineHash ch;
				for (int wc_i : wc_is)
					ch.Do(wc_i).Put(1);
				wc_h = ch;
			}
			{
				CombineHash ch;
				for (int w_i : w_is)
					ch.Do(w_i).Put(1);
				w_h = ch;
			}
			
			int pp_i = -1;
			PhrasePart& pp = MapGetAdd(src.phrase_parts, w_h, pp_i);
			pp.words <<= w_is;
			pp.tt_i = i;
			pp.virtual_phrase_part = src.virtual_phrase_parts.Find(wc_h);
			
		}
	}
	
	NextBatch();
}

void ScriptTextProcess::PhrasePartAnalysis() {
	ASSERT(p.src);
	auto& src = p.src->Data();
	
	Panic("TODO");
	#if 0
	PhraseArgs& args = phrase_args;
	switch (phase) {
		case PHASE_ELEMENT:		args.fn = FN_CLASSIFY_PHRASE_ELEMENT; break;
		case PHASE_COLOR:		args.fn = FN_CLASSIFY_PHRASE_COLOR; break;
		case PHASE_ATTR:		args.fn = FN_CLASSIFY_PHRASE_ATTR; break;
		case PHASE_ACTIONS:		args.fn = FN_CLASSIFY_PHRASE_ACTIONS; break;
		case PHASE_SCORES:		args.fn = FN_CLASSIFY_PHRASE_SCORES; break;
		case PHASE_TYPECLASS:	args.fn = FN_CLASSIFY_PHRASE_TYPECLASS; break;
		case PHASE_CONTENT:		args.fn = FN_CLASSIFY_PHRASE_CONTENT; break;
		default: Panic("TODO");
	}
	args.phrases.Clear();
	args.elements.Clear();
	args.typeclasses.Clear();
	args.contents.Clear();
	
	ASSERT(src.typeclasses.GetCount());
	ASSERT(src.contents.GetCount());
	args.typeclasses <<= src.typeclasses.GetKeys();
	for(int i = 0; i < src.contents.GetCount(); i++) {
		const auto& it = src.contents[i];
		String s;
		s << "A: " << it.parts[0] << ", B: " << it.parts[1] << ", C: " << it.parts[2];
		args.contents << s;
	}
	
	int per_action_task = BatchCount(fn);
	
	int begin = batch * per_action_task;
	int end = begin + per_action_task;
	
	Color no_clr(0,0,0);
	tmp_ptrs.SetCount(0);
	tmp.SetCount(0);
	
	if (batch == 0) {
		tmp_iters.SetCount(0);
		int trimmed_by[PHASE_COUNT];
		memset(trimmed_by, 0, sizeof(trimmed_by));
		
		if (fn == PHASE_ELEMENT && vmap.IsEmpty())
			vmap = src.GetSortedElements();
		
		int iter = 0;
		int idx = -1;
		for (const PhrasePart& pp : src.phrase_parts.GetValues()) {
			idx++;
			
			if ((fn == PHASE_COLOR && pp.clr != no_clr) || (fn > PHASE_COLOR && pp.clr == no_clr)) {
				trimmed_by[fn]++;
				continue;
			}
			
			if ((fn == PHASE_ATTR && pp.attr >= 0) || (fn > PHASE_ATTR && pp.attr < 0)){
				trimmed_by[fn]++;
				continue;
			}
			
			if ((fn == PHASE_ACTIONS && !pp.actions.IsEmpty()) || (fn > PHASE_ACTIONS && pp.actions.IsEmpty())){
				trimmed_by[fn]++;
				continue;
			}
			
			if ((fn == PHASE_SCORES && pp.HasScores()) || (fn > PHASE_SCORES && !pp.HasScores())){
				trimmed_by[fn]++;
				continue;
			}
			
			if ((fn == PHASE_TYPECLASS && !pp.typecasts.IsEmpty()) || (fn > PHASE_TYPECLASS && pp.typecasts.IsEmpty())){
				trimmed_by[fn]++;
				continue;
			}
			
			if ((fn == PHASE_CONTENT && !pp.contrasts.IsEmpty()) || (fn > PHASE_CONTENT && pp.contrasts.IsEmpty())){
				trimmed_by[fn]++;
				continue;
			}
			
			if ((fn == PHASE_ELEMENT && pp.el_i >= 0) || (fn > PHASE_ELEMENT && pp.el_i < 0)) {
				trimmed_by[fn]++;
				continue;
			}
			
			tmp_iters << idx;
			iter++;
		}
	}
	
	for(int i = begin; i < end && i < tmp_iters.GetCount(); i++) {
		int idx = tmp_iters[i];
		const PhrasePart& pp = src.phrase_parts[idx];
		String phrase = src.GetWordString(pp.words);
		args.phrases << phrase;
		tmp_ptrs << (void*)&pp;
		tmp << idx;
	}
	
	if (args.phrases.IsEmpty()) {
		NextPhase();
		return;
	}
	
	if (fn == PHASE_ELEMENT) {
		ASSERT(vmap.GetCount());
		int max_elements = 30;
		for(int i = 0; i < vmap.GetCount(); i++) {
			int el_i = vmap.GetKey(i);
			String element = src.element_keys[el_i];
			if (element == "n/a" ||
				element == "none" ||
				element.Left(1) == "(")
				continue;
			args.elements << element;
			if (args.elements.GetCount() >= max_elements)
				break;
		}
	}
	
	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	if (args.fn == PHASE_COLOR)
		m.GetPhraseData(args, THISBACK(OnPhraseColors));
	else if (args.fn == PHASE_ATTR)
		m.GetPhraseData(args, THISBACK(OnPhraseAttrs));
	else if (args.fn == PHASE_ACTIONS)
		m.GetPhraseData(args, THISBACK(OnPhraseActions));
	else if (args.fn == PHASE_SCORES)
		m.GetPhraseData(args, THISBACK(OnPhraseScores));
	else if (args.fn == PHASE_TYPECLASS)
		m.GetPhraseData(args, THISBACK(OnPhraseTypeclasses));
	else if (args.fn == PHASE_CONTENT)
		m.GetPhraseData(args, THISBACK(OnPhraseContrast));
	else if (args.fn == PHASE_ELEMENT)
		m.GetPhraseData(args, THISBACK(OnPhraseElement));
	else
		TODO;
	#endif
}







void ScriptTextProcess::Prepare(int fn) {
	//DatasetPtrs p = GetDataset();
	ASSERT(p.src);
	auto& src = p.src->Data();
	
	Panic("TODO");
	#if 0
	args.fn = fn;
	
	int per_action_task = BatchCount(fn);
	int begin = batch * per_action_task;
	int end = begin + per_action_task;
	
	Color black(0,0,0);
	int iter = 0;
	for(int i = 0; i < uniq_acts.GetCount(); i++) {
		ActionHeader ah;
		ah.action = uniq_acts.GetKey(i);
		
		const VectorMap<String,int>& idx = uniq_acts[i];
		for(int j = 0; j < idx.GetCount(); j++) {
			ah.arg = idx.GetKey(j);
			
			if ((ah.action.GetCount() && ah.action[0] == '\"') || (ah.arg.GetCount() && ah.arg[0] == '\"'))
				continue;
			
			if (iter >= begin && iter < end) {
				ExportAction& aa = src.actions.GetAdd(ah);
				
				if (fn == 0 && aa.clr != black)
					continue;
				
				if (fn == 1 && aa.attr >= 0)
					continue;
				
				String s = uniq_acts.GetKey(i) + "(" + idx.GetKey(j) + ")";
				args.actions << s;
			}
			iter++;
		}
	}
	if (args.actions.IsEmpty()) {
		NextPhase();
		return; // ready
	}
	#endif
}

void ScriptTextProcess::Colors() {
	Prepare(FN_CLASSIFY_PHRASE_METAPHORICAL_COLOR);
	
	Panic("TODO");
	#if 0
	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.GetActionAnalysis(args, [this](String result) {
		ASSERT(p.src);
		auto& src = p.src->Data();
		
		// "attention-humor(not taking life too seriously)" RGB(255, 255, 0)
		
		Color black(0,0,0);
		Color non_black(1,1,1);
		
		result.Replace("\r", "");
		Vector<String> lines = Split(result, "\n");
		for (String& line : lines) {
			line = TrimBoth(line);
			if (line.IsEmpty()) continue;
			if (line[0] != '\"') continue;
			int a = 1;
			int b = line.ReverseFind("\"");
			if (b < 0) continue;
			String full_action = line.Mid(a, b-a);
			
			a = line.Find("RGB(", b);
			if (a < 0) continue;
			a += 4;
			b = line.Find(")", a);
			if (b < 0) continue;
			String clr_str = line.Mid(a,b-a);
			Vector<String> clr_parts = Split(clr_str, ",");
			if (clr_parts.GetCount() != 3) continue;
			int R = ScanInt(TrimLeft(clr_parts[0]));
			int G = ScanInt(TrimLeft(clr_parts[1]));
			int B = ScanInt(TrimLeft(clr_parts[2]));
			Color clr(R,G,B);
			a = full_action.Find("(");
			if (a < 0) continue;
			
			ActionHeader ah;
			ah.action = full_action.Left(a);
			a++;
			b = full_action.ReverseFind(")");
			ah.arg = full_action.Mid(a,b-a);
			
			if (clr == black)
				clr = non_black;
			
			if ((ah.action.GetCount() && ah.action[0] == '\"') || (ah.arg.GetCount() && ah.arg[0] == '\"'))
				continue;
			ExportAction& aa = src.actions.GetAdd(ah);
			aa.clr = clr;
		}
		
		int a = 0;
		for (const ExportAction& ea : src.actions.GetValues())
			if (ea.clr != black)
				a++;
		src.diagnostics.GetAdd("actionlist colors: total") = IntStr(src.virtual_phrase_parts.GetCount());
		src.diagnostics.GetAdd("actionlist colors: actual") =  IntStr(a);
		src.diagnostics.GetAdd("actionlist colors: percentage") =  DblStr((double)a / (double) src.virtual_phrase_parts.GetCount() * 100);
		
		NextBatch();
		SetWaiting(false);
	});
	#endif
}

void ScriptTextProcess::Attrs() {
	Prepare(FN_CLASSIFY_PHRASE_ACTION_ATTR);
	
	Panic("TODO");
	#if 0
	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.GetActionAnalysis(args, [this](String result) {
		ASSERT(p.src);
		auto& src = p.src->Data();
		
		// "attention-procedures(planning)" problem solving strategy / shortcut taking
		
		result.Replace("\r", "");
		Vector<String> lines = Split(result, "\n");
		for (String& line : lines) {
			line = TrimBoth(line);
			if (line.IsEmpty()) continue;
			if (line[0] != '\"') continue;
			int a = 1;
			int b = line.ReverseFind("\"");
			if (b < 0) continue;
			String full_action = line.Mid(a, b-a);
			
			a = b+1;
			b = line.Find("/", b);
			if (a < 0) continue;
			AttrHeader ath;
			ath.group = TrimBoth(line.Mid(a,b-a));
			a = b+1;
			ath.value = TrimBoth(line.Mid(a));
			
			a = full_action.Find("(");
			if (a < 0) continue;
			ActionHeader ah;
			ah.action = full_action.Left(a);
			a++;
			b = full_action.ReverseFind(")");
			ah.arg = full_action.Mid(a,b-a);
			
			ASSERT(ah.action.Find("\"") != 0 && ah.arg.Find("\"") != 0);
			ExportAction& ea = src.actions.GetAdd(ah);
			MapGetAdd(src.attrs, ath, ea.attr);
		}
		
		int a = 0;
		for (const ExportAction& ea : src.actions.GetValues())
			if (ea.attr >= 0)
				a++;
		src.diagnostics.GetAdd("actionlist attrs: total") = IntStr(src.virtual_phrase_parts.GetCount());
		src.diagnostics.GetAdd("actionlist attrs: actual") =  IntStr(a);
		src.diagnostics.GetAdd("actionlist attrs: percentage") =  DblStr((double)a / (double) src.virtual_phrase_parts.GetCount() * 100);
		
		NextBatch();
		SetWaiting(false);
	});
	#endif
}










void ScriptTextProcess::MainGroups() {
	ASSERT(p.src);
	auto& src = p.src->Data();
	RealizeBatch_AttrExtremesBatch();
	
	Panic("TODO");
	#if 0
	Vector<AttrExtremesBatch>& batches = attr_extremes_batches;
	
	if (batch >= batches.GetCount()) {
		NextPhase();
		return;
	}
	AttrExtremesBatch& batch = batches[this->batch];
	
	const Index<String>& values = uniq_attrs.Get(batch.group);
	if (values.GetCount() < 2) {
		NextPhase();
		return;
	}
	
	
	args.fn = FN_SORT_ATTRS;
	args.group = batch.group;
	args.values <<= values.GetKeys();
	
	if (args.group.IsEmpty()) {
		NextBatch();
		return;
	}
	
	tmp_str = args.group;
	
	
	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.GetAttributes(args, [this](String res) {
		ASSERT(p.src);
		auto& src = p.src->Data();
		
		RemoveEmptyLines2(res);
		Vector<String> parts = Split(res, "\n");
		
		if (parts.GetCount() != 2) {
			if (parts.GetCount() == 1) {
				if (parts[0].Find(" vs. ") >= 0)	parts = Split(parts[0], " vs. ");
				if (parts[0].Find("/") >= 0)		parts = Split(parts[0], "/");
				if (parts[0].Find(" vs ") >= 0)		parts = Split(parts[0], " vs ");
				if (parts[0].Find(" - ") >= 0)		parts = Split(parts[0], " - ");
				if (parts[0].Find(" and ") >= 0)	parts = Split(parts[0], " and ");
			}
		}
		if (parts.GetCount() == 2) {
			if (parts[0].Find(" vs. ") >= 0)
				parts = Split(parts[0], " vs. ");
			String& f = parts[0];
			String& l = parts[1];
			RemoveLineChar(f);
			RemoveLineChar(l);
			int a = f.Find("1.");
			if (a >= 0) {
				f = f.Mid(a+2);
			}
			f = TrimBoth(f);
			l = TrimBoth(l);
			
			Vector<String> keys;
			keys.SetCount(2);
			for(int i = 0; i < parts.GetCount(); i++) {
				String& part = parts[i];
				int a0 = part.Find(":");
				int a1 = part.Find("-");
				int a2 = part.Find("/");
				int a3 = part.Find("\n");
				int a4 = part.Find(",");
				int a5 = part.Find("(");
				int a = INT_MAX;
				if (a0 >= 0 && a0 < a) a = a0;
				if (a1 >= 0 && a1 < a) a = a1;
				if (a2 >= 0 && a2 < a) a = a2;
				if (a3 >= 0 && a3 < a) a = a3;
				if (a4 >= 0 && a4 < a) a = a4;
				if (a5 >= 0 && a5 < a) a = a5;
				if (a == INT_MAX) a = part.GetCount();
				/*if (a == INT_MAX && part.GetCount() < 100) a = part.GetCount();
				if (a == INT_MAX) {
					WString ws = part.ToWString();
					LOG(ws);
					DUMPC(ws);
				}*/
				if (a == INT_MAX)
					continue;
				String& key = keys[i];
				key = TrimBoth(part.Left(a));
			}
			
			if (keys[0].GetCount() && keys[1].GetCount()) {
				String group = ToLower(tmp_str);
				int attr_i[2] = {-1,-1};
				for(int i = 0; i < keys.GetCount(); i++) {
					AttrHeader ah;
					ah.group = group;
					ah.value = ToLower(keys[i]);
					
					MapGetAdd(src.attrs, ah, attr_i[i]);
				}
				
				
				ExportSimpleAttr& sat = src.simple_attrs.GetAdd(tmp_str);
				sat.attr_i0 = attr_i[0];
				sat.attr_i1 = attr_i[1];
				
				//const Index<String>& v = uniq_attrs.Get(group);
			}
		}
		
		
		NextBatch();
		SetWaiting(false);
	});
	#endif
}

void ScriptTextProcess::RealizeBatch_AttrExtremesBatch() {
	Panic("TODO");
	#if 0
	ASSERT(p.src);
	auto& src = p.src->Data();
	
	/*if (uniq_attrs.IsEmpty())*/ {
		uniq_attrs.Clear();
		for(int i = 0; i < src.attrs.GetCount(); i++) {
			const AttrHeader& ah = src.attrs.GetKey(i);
			uniq_attrs.GetAdd(ah.group).FindAdd(ah.value);
		}
		
		struct Sorter {
			bool operator()(const Index<String>& a, const Index<String>& b) const {
				if (a.GetCount() != b.GetCount())
					return a.GetCount() > b.GetCount();
				if (a.GetCount() && b.GetCount())
					return StdLess<String>()(a[0], b[0]);
				return false;
			}
		};
		SortByValue(uniq_attrs, Sorter());
	}
	
	Vector<AttrExtremesBatch>& batches = attr_extremes_batches;
	
	if (batches.IsEmpty()) {
		for(int i = 0; i < uniq_attrs.GetCount(); i++) {
			String group = uniq_attrs.GetKey(i);
			int j = src.simple_attrs.Find(group);
			if (j >= 0) {
				const ExportSimpleAttr& esa = src.simple_attrs[j];
				if (esa.attr_i0 >= 0 && esa.attr_i1 >= 0)
					continue;
			}
			AttrExtremesBatch& b = batches.Add();
			b.group = group;
		}
	}
	#endif
}

void ScriptTextProcess::SimplifyAttrs() {
	ASSERT(p.src);
	auto& src = p.src->Data();
	int per_batch = 50;
	
	Panic("TODO");
	#if 0
	Vector<AttrPolarBatch>& batches = attr_polar_batches;
	
	if (batches.IsEmpty()) {
		for(int i = 0; i < uniq_attrs.GetCount(); i++) {
			String group = uniq_attrs.GetKey(i);
			int j = src.simple_attrs.Find(group);
			if (j < 0) continue;
			const auto& gsa = src.simple_attrs[j];
			
			const Index<String>& v = uniq_attrs[i];
			AttrPolarBatch& b = batches.Add();
			b.attr0 = src.attrs.GetKey(gsa.attr_i0).value;
			b.attr1 = src.attrs.GetKey(gsa.attr_i1).value;
			ASSERT(src.attrs.GetKey(gsa.attr_i0).group == group);
			b.group = group;
			for(int j = 0; j < v.GetCount(); j++) {
				if (batches.Top().attrs.GetCount() >= per_batch) {
					AttrPolarBatch& b0 = batches.Add();
					AttrPolarBatch& b = batches[batches.GetCount()-2];
					b0.group = b.group;
					b0.attr0 = b.attr0;
					b0.attr1 = b.attr1;
				}
				
				AttrHeader ah;
				ah.group = group;
				ah.value = v[j];
				int k = src.attrs.Find(ah);
				if (k >= 0) {
					const ExportAttr& ea = src.attrs[k];
					if (ea.positive >= 0)
						continue;
				}
				
				batches.Top().attrs << v[j];
			}
			if (batches.Top().attrs.IsEmpty())
				batches.Remove(batches.GetCount()-1);
		}
	}
	
	
	if (batch >= batches.GetCount()) {
		NextPhase();
		return;
	}
	
	AttrPolarBatch& batch = batches[this->batch];
	
	args.fn = FN_ATTR_POLAR_OPPOSITES;
	args.group = batch.group;
	args.values <<= batch.attrs;
	args.attr0 = batch.attr0;
	args.attr1 = batch.attr1;
	
	tmp_words <<= batch.attrs;
	tmp_str = args.group;
	
	
	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.GetAttributes(args, [this](String res) {
		ASSERT(p.src);
		auto& src = p.src->Data();
		
		RemoveEmptyLines2(res);
		Vector<String> lines = Split(res, "\n");
		
		
		if (lines.GetCount() == tmp_words.GetCount()) {
			String group = tmp_str;
			int i = src.simple_attrs.Find(group);
			String pos_value, neg_value;
			if (i >= 0) {
				const ExportSimpleAttr& esa = src.simple_attrs[i];
				pos_value = src.attrs.GetKey(esa.attr_i0).value;
				neg_value = src.attrs.GetKey(esa.attr_i1).value;
			}
			for(int i = 0; i < lines.GetCount(); i++) {
				String key = tmp_words[i];
				String value = TrimBoth(ToLower(lines[i]));
				bool is_negative = value.Find("negative") >= 0;
				
				// Force the value, if the key is the extreme (and AI got it wrong somehow)
				if (key == pos_value)
					is_negative = false;
				else if (key == neg_value)
					is_negative = true;
				
				AttrHeader ah;
				ah.group = group;
				ah.value = key;
				int j = src.attrs.Find(ah);
				if (j < 0)
					continue;
				
				ExportAttr& ea = src.attrs[j];
				ea.positive = !is_negative;
				ea.simple_attr = i;
			}
			
		}
		
		NextBatch();
		SetWaiting(false);
	});
	#endif
}

void ScriptTextProcess::JoinOrphaned() {
	ASSERT(p.src);
	auto& src = p.src->Data();
	
	int per_batch = 35;
	Panic("TODO");
	#if 0
	Vector<AttrJoinBatch>& batches = attr_join_batches;
	
	if (batches.IsEmpty()) {
		for(int i = 0; i < uniq_attrs.GetCount(); i++) {
			String group = uniq_attrs.GetKey(i);
			const Index<String>& v = uniq_attrs[i];
			if (v.GetCount() > 1) continue;
			if (v.IsEmpty()) break;
			if (batches.IsEmpty() || batches.Top().values.GetCount() >= per_batch) {
				batches.Add();
			}
			AttrHeader ah;
			ah.group = group;
			ah.value = v[0];
			
			int j = src.attrs.Find(ah);
			if (j < 0)
				continue;
			
			const ExportAttr& ea = src.attrs[j];
			if (ea.link >= 0)
				continue; // already linked
			
			AttrJoinBatch& b = batches.Top();
			b.values << (ah.group + ": " + ah.value);
		}
	}
	
	if (batch >= batches.GetCount()) {
		NextPhase();
		return;
	}
	
	AttrJoinBatch& batch = batches[this->batch];
	
	args.fn = FN_MATCHING_ATTR;
	//args.groups <<= batch.groups;
	args.values <<= batch.values;
	int count = min(20, uniq_attrs.GetCount());
	tmp_words2.Clear();
	for(int i = 0; i < count; i++) {
		String group = uniq_attrs.GetKey(i);
		if (!group.IsEmpty()) {
			const ExportSimpleAttr& ea = src.simple_attrs.GetAdd(group);
			String a0 = src.attrs.GetKey(ea.attr_i0).value;
			String a1 = src.attrs.GetKey(ea.attr_i1).value;
			args.groups << (group + ": +(" + a0 + "), -(" + a1 + ")");
			tmp_words2 << group;
		}
	}
	
	tmp_words <<= batch.values;
	tmp_str = args.group;
	
	
	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.GetAttributes(args, [this](String res) {
		ASSERT(p.src);
		auto& src = p.src->Data();
		
		RemoveEmptyLines2(res);
		Vector<String> lines = Split(res, "\n");
		
		
		if (lines.GetCount() == tmp_words.GetCount()) {
			
			for(int i = 0; i < lines.GetCount(); i++) {
				String& l = lines[i];
				Vector<String> ah_parts = Split(tmp_words[i], ": ");
				if (ah_parts.GetCount() != 2)
					continue;
				AttrHeader ah;
				ah.group = ah_parts[0];
				ah.value = ah_parts[1];
				
				int attr_i = src.attrs.Find(ah);
				if (attr_i < 0)
					continue;
				ExportAttr& ea = src.attrs[attr_i];
				
				String digit, sign;
				for(int i = 0; i < l.GetCount(); i++) {
					int chr = l[i];
					if (IsDigit(chr))
						digit.Cat(chr);
					else if (chr == '+' || chr == '-') {
						sign.Cat(chr);
						break;
					}
					else if (chr == ',')
						break;
				}
				if (digit.IsEmpty() || sign.IsEmpty())
					continue;
				
				int group_i = ScanInt(digit);
				bool is_positive = sign == "+";
				
				if (group_i < 0 || group_i >= tmp_words2.GetCount())
					continue;
				String group = tmp_words2[group_i];
				
				AttrHeader link_ah;
				link_ah.group = group;
				link_ah.value = ah.value;
				int link_i = -1;
				MapGetAdd(src.attrs, link_ah, link_i);
				
				ea.link = link_i;
			}
			
		}
		
		NextBatch();
		SetWaiting(false);
	});
	#endif
}

void ScriptTextProcess::FixData() {
	ASSERT(p.src);
	auto& src = p.src->Data();
	
	for(int i = 0; i < src.attrs.GetCount(); i++) {
		src.attrs[i].simple_attr = -1;
	}
	Panic("TODO");
	#if 0
	// Fix: add simple_attr index value to ExportAttr
	for(int i = 0; i < uniq_attrs.GetCount(); i++) {
		AttrHeader ah;
		ah.group = uniq_attrs.GetKey(i);
		const auto& values = uniq_attrs[i];
		int sa_i = src.simple_attrs.Find(ah.group);
		if (sa_i < 0)
			continue;
		for(int j = 0; j < values.GetCount(); j++) {
			ah.value = values[j];
			int k = src.attrs.Find(ah);
			ASSERT(k >= 0);
			ExportAttr& ea = src.attrs[k];
			ea.simple_attr = sa_i;
		}
	}
	#endif
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
