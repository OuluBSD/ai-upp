#include "Core.h"

NAMESPACE_UPP

#define PROCESS_ASSERT(x)  if (!(x)) {SetError("assert failed: " #x); SetNotRunning(); LOG("ScriptTextProcess: " << __FILE__ << ":" << __LINE__ << ": error: assert failed: " #x); return;}

ScriptTextProcess::ScriptTextProcess() {}

int ScriptTextProcess::GetPhaseCount() const { return PHASE_COUNT; }

int ScriptTextProcess::GetBatchCount(int phase) const { return 1; }

int ScriptTextProcess::GetSubBatchCount(int phase, int batch) const { return 1; }

void ScriptTextProcess::DoPhase()
{
	SrcTextData& data = *this->data;

	if(IsPhase(PHASE_INPUT)) {
		String genre = params("genre");
		String author = params("author");
		String title = params("title");
		Value input_text = params("input_text");
		if(genre.IsEmpty())
			genre = "User input";
		if(author.IsEmpty())
			author = "User";
		if(title.IsEmpty())
			title = "<No title>";
		AuthorDataset& ed = data.GetAddAuthor(author);
		VectorFindAdd(ed.genres, genre);
		ScriptDataset& script = ed.GetAddScript(title);
		script.text = AsJSON(input_text, true);

		NextPhase();
	}
	else if(IsPhase(PHASE_TOKENIZE)) {
		Tokenize();
	}
	else if(IsPhase(PHASE_ANALYZE_PUBLIC_FIGURE)) {
		AnalyzePublicFigure();
	}
	else if(IsPhase(PHASE_ANALYZE_ELEMENTS)) {
		AnalyzeElements();
	}
	else if(IsPhase(PHASE_WORD_CLASSES)) {
		WordClasses();
	}
	else if(IsPhase(PHASE_AMBIGUOUS_WORD_PAIRS)) {
		AmbiguousWordPairs();
	}
	else if(IsPhase(PHASE_IMPORT_TOKEN_TEXTS)) {
		ImportTokenTexts();
	}
	else if(IsPhase(PHASE_CLASSIFY_SENTENCES)) {
		ClassifySentences();
	}
	else if(IsPhase(PHASE_VIRTUAL_PHRASE_PARTS)) {
		VirtualPhraseParts();
	}
	else if(IsPhase(PHASE_VIRTUAL_PHRASE_PART_STRUCTS)) {
		VirtualPhraseStructs();
	}
	else if(IsPhaseRange(PHASE_ELEMENT, PHASE_CONTENT)) {
		PhrasePartAnalysis();
	}
	else if(IsPhase(PHASE_COLORS)) {
		Colors();
	}
	else if(IsPhase(PHASE_ATTRS)) {
		Attrs();
	}
	else if(IsPhase(PHASE_MAIN_GROUPS)) {
		MainGroups();
	}
	else if(IsPhase(PHASE_SIMPLIFY_ATTRS)) {
		SimplifyAttrs();
	}
	else if(IsPhase(PHASE_JOIN_ORPHANED)) {
		JoinOrphaned();
	}
	else if(IsPhase(PHASE_FIX_DATA)) {
		FixData();
	}
	else {
		SetNotRunning();
	}
}

void ScriptTextProcess::Tokenize()
{
	if(tk.IsEmpty())
		tk.Create();
	NaturalTokenizer& tk = *this->tk;

	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;
	Vector<int> token_is;
	Vector<AuthorDataset>& entities = src.authors;

	int well_filter_loss = 0, parse_loss = 0, foreign_loss = 0;

	bool filter_foreign = true;
	{
		int lng_i = src.GetLanguage();
		if(lng_i != LNG_ENGLISH)
			filter_foreign = false;
	}

	int phase = this->phase, batch = this->batch, sub_batch = this->sub_batch;
	if(parallel) {
		WorkerData& w = Worker();
		phase = w.phase;
		batch = w.batch;
		sub_batch = w.sub_batch;
	}

	if(!batch && !sub_batch) {
		total = 0;
		actual = 0;
		ts.Reset();
	}

	if(batch >= src.authors.GetCount()) {
		NextPhase();
		return;
	}

	auto& author = src.authors[batch];
	if(sub_batch >= author.scripts.GetCount()) {
		NextBatch();
		return;
	}

	int worker_total = total++;

	auto& script = author.scripts[sub_batch];

	String str = script.text;

	// TODO merge duplicate code in these if-elseif-else conditionals to one
	if(str.Left(1) == "[") {
		Value v = ParseJSON(str);
		str.Clear();

		String script_title = author.name + " - " + script.title;
		hash_t ss_hash = script_title.GetHashValue();

		int ss_i = src.scripts.Find(ss_hash);
		if(skip_ready && ss_i >= 0) {
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

			if(prev_msect != msect) {
				part = &ss.parts.Add();
				subpart = &part->sub.Add();
				ssubpart = &subpart->sub.Add();
				part->type = -1;
				part->num = -1;
				subpart->repeat = 0;
				ssub_line_i = 0;
			}
			else if(prev_sect != sect) {
				subpart = &part->sub.Add();
				subpart->repeat = 0;
				ssubpart = &subpart->sub.Add();
				ssub_line_i = 0;
			}
			// else if (ssub_line_i > 0 && ssub_line_i % 4 == 0)
			//	ssubpart = &subpart->sub.Add();

			if(line_txt.Left(1) == "[")
				continue;

			if(!tk.Parse(line_txt))
				continue;

			if(tk.GetLines().GetCount() == 1) {
				// data_lock.Enter();
				const auto& line = tk.GetLines()[0];

				token_is.SetCount(0);
				CombineHash ch;
				for(const WString& line : line) {
					String s = line.ToString();
					int tk_i = -1;
					Token& tk = MapGetAdd(src.tokens, s, tk_i);
					ch.Do(tk_i);
					token_is << tk_i;
				}
				hash_t h = ch;

				int tt_i = -1;
				TokenText& tt = MapGetAdd(src.token_texts, h, tt_i);
				if(tt.tokens.IsEmpty()) {
					Swap(tt.tokens, token_is);
				}

				if(tt_i >= 0)
					ssubpart->token_texts << tt_i;
				// data_lock.Leave();
			}

			prev_msect = msect;
			prev_sect = sect;
			ssub_line_i++;
		}
		data_lock.Leave();

		if(0) {
			LOG(src.GetScriptDump(ss_i));
		}

		data_lock.Leave();
	}
	else if(str.Left(1) == "{") {
		Panic("TODO");
	}
	else {
		Vector<String> lines = Split(str, "\n");
		for(int i = 0; i < lines.GetCount(); i++) {
			String& s = lines[i];
			s = TrimBoth(s);
			if(s.Left(1) == "[")
				lines.Remove(i--);
		}
		str = Join(lines, "\n");

		// Ignore files with hard ambiguities
		if(0 && str.Find(" well ") >= 0) {
			// well or we'll... too expensive to figure out
			well_filter_loss++;
			NextSubBatch();
			return;
		}

		static thread_local TryNo5tStructureSolver solver;

		tk.Clear();
		HotfixReplaceWord(str);
		if(!tk.Parse(str)) {
			parse_loss++;
			NextSubBatch();
			return;
		}
		if(filter_foreign && tk.HasForeign()) {
			foreign_loss++;
			NextSubBatch();
			return;
		}

		String script_title = author.name + " - " + script.title;
		hash_t ss_hash = script_title.GetHashValue();

		int ss_i = src.scripts.Find(ss_hash);
		if(skip_ready && ss_i >= 0) {
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

			if(prev_msect != sect.meta_section) {
				part = &ss.parts.Add();
				subpart = &part->sub.Add();
				ssubpart = &subpart->sub.Add();
				part->type = msect.type;
				part->num = msect.num;
				subpart->repeat = sect.repeat;
				ssub_line_i = 0;
			}
			else if(prev_sect != line.section) {
				subpart = &part->sub.Add();
				subpart->repeat = sect.repeat;
				ssubpart = &subpart->sub.Add();
				ssub_line_i = 0;
			}
			else if(ssub_line_i > 0 && ssub_line_i % 4 == 0)
				ssubpart = &subpart->sub.Add();

			if(line.txt.Left(1) == "[")
				continue;

			if(!tk.Parse(line.txt))
				continue;

			if(tk.GetLines().GetCount() == 1) {
				// data_lock.Enter();
				const auto& line = tk.GetLines()[0];

				token_is.SetCount(0);
				CombineHash ch;
				for(const WString& line : line) {
					String s = line.ToString();
					int tk_i = -1;
					Token& tk = MapGetAdd(src.tokens, s, tk_i);
					ch.Do(tk_i);
					token_is << tk_i;
				}
				hash_t h = ch;

				int tt_i = -1;
				TokenText& tt = MapGetAdd(src.token_texts, h, tt_i);
				if(tt.tokens.IsEmpty()) {
					Swap(tt.tokens, token_is);
				}

				if(tt_i >= 0)
					ssubpart->token_texts << tt_i;
				// data_lock.Leave();
			}

			prev_msect = sect.meta_section;
			prev_sect = line.section;
			ssub_line_i++;
		}
		data_lock.Leave();

		if(0) {
			LOG(src.GetScriptDump(ss_i));
		}
	}

	actual++;
	NextSubBatch();

	if(worker_total % 500 == 0) {
		src.diagnostics.GetAdd("SourceDataImporter: total") = IntStr(total);
		src.diagnostics.GetAdd("SourceDataImporter: actual") = IntStr(actual);
		src.diagnostics.GetAdd("SourceDataImporter: percentage") =
			DblStr((double)actual / (double)total * 100);
		src.diagnostics.GetAdd("SourceDataImporter: filter 'well' loss") =
			DblStr((double)well_filter_loss / (double)total * 100);
		src.diagnostics.GetAdd("SourceDataImporter: filter 'parse success' loss") =
			DblStr((double)parse_loss / (double)total * 100);
		src.diagnostics.GetAdd("SourceDataImporter: filter 'foreign' loss") =
			DblStr((double)foreign_loss / (double)total * 100);
		src.diagnostics.GetAdd("SourceDataImporter: duration of song process") = ts.ToString();
	}
}

void ScriptTextProcess::AnalyzePublicFigure()
{
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;

	bool user_genres = params("genres").Is<ValueArray>();

	if(batch >= src.authors.GetCount() || user_genres) {
		NextPhase();
		return;
	}

	AuthorDataset& ent = src.authors[batch];
	if(ent.genres.GetCount()) {
		NextBatch();
		return;
	}

	args.fn = FN_ANALYZE_PUBLIC_FIGURE;
	args.params = ValueMap();

	args.params("type") = params("type");
	if(args.params("type").IsVoid())
		args.params("type") = "public figure"; // TODO artist, producer, etc.

	args.params("name") = params("name");
	if(args.params("name").IsVoid())
		args.params("name") = ent.name;

	args.params("description") = params("description");
	if(args.params("description").IsVoid())
		args.params("description") = "";

	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.Get(args, [this](String result) {
		PROCESS_ASSERT(p.srctxt);
		auto& src = *p.srctxt;
		TaskArgs& args = this->args;

		Value v = ParseJSON(result);
		ValueArray genres = v("response")("genres");

		AuthorDataset& ent = src.authors[batch];
		ent.genres.Clear();
		for(int i = 0; i < genres.GetCount(); i++)
			ent.genres.Add(genres[i].ToString());

		NextBatch();
		SetWaiting(false);
	});
}

void ScriptTextProcess::AnalyzeElements()
{
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;
	Vector<AuthorDataset>& entities = src.authors;

	if(batch >= src.scripts.GetCount()) {
		NextPhase();
		return;
	}
	ScriptStruct& ss = src.scripts[batch];
	if(ss.parts.GetCount() && ss.parts[0].cls >= 0) {
		NextBatch();
		return;
	}

	ValueArray text_arg = src.GetScriptValue(batch);

	args.fn = FN_ANALYZE_ELEMENTS;
	args.params = ValueMap();
	args.params("text") = text_arg;

	// Vector<String> all_sections = Split(args.text, "[");
	if(text_arg.GetCount() == 0 /*|| all_sections.GetCount() >= 50*/) {
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
	if(m.keep_going_counter >= 50) {
		SetNotRunning();
		return;
	}

	m.Get(args, [this](String result) {
		PROCESS_ASSERT(p.srctxt);
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
					if(!val.IsEmpty()) {
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

void ScriptTextProcess::WordClasses()
{
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;

	args.fn = FN_WORD_CLASSES;
	args.params = ValueMap();

	if(batch == 0)
		total = 0;

	int begin = batch * words_per_action_task;
	int end = begin + words_per_action_task;
	end = min(end, src.tokens.GetCount());
	int count = end - begin;
	if(count <= 0) {
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
		PROCESS_ASSERT(p.srctxt);
		auto& src = *p.srctxt;
		TaskArgs& args = this->args;
		ScriptStruct& ss = src.scripts[batch];
		
		Value v = ParseJSON(result);
		ValueArray word_classes = v("response-short")("word classes");
		ValueArray input_words = args.params("words");
		// LOG(AsJSON(v, true));
		int count = min(word_classes.GetCount(), input_words.GetCount());

		for(int i = 0; i < count; i++) {
			String result_word = input_words[i].ToString();

			int orig_word_i = -1;
			ExportWord& wrd = MapGetAdd(src.words, result_word, orig_word_i);

			ValueArray classes = word_classes[i];
			for(int j = 0; j < classes.GetCount(); j++) {
				String cls = classes[j].ToString();
				int wc_i = src.word_classes.FindAdd(cls);
				if(wrd.class_count < wrd.MAX_CLASS_COUNT)
					FixedIndexFindAdd(wrd.classes, wrd.MAX_CLASS_COUNT, wrd.class_count, wc_i);
			}
			actual++;
		}
		src.diagnostics.GetAdd("tokens: total") = IntStr(total);
		src.diagnostics.GetAdd("tokens: actual") = IntStr(actual);
		src.diagnostics.GetAdd("tokens: percentage") =
			DblStr((double)actual / (double)total * 100);

		NextBatch();
		SetWaiting(false);
	});
}

void ScriptTextProcess::AmbiguousWordPairs()
{
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;

	args.fn = FN_WORD_PAIR_CLASSES;
	args.params = ValueMap();

	int begin = batch * words_per_action_task;
	int end = begin + words_per_action_task;
	end = min(end, src.ambiguous_word_pairs.GetCount());
	int iter = 0;

	ValueArray word_pairs;
	tmp_wp_ptrs.Clear();
	for(WordPairType& wp : src.ambiguous_word_pairs.GetValues()) {
		if(wp.from < 0 || wp.to < 0)
			continue;
		if(wp.from_type >= 0 && wp.to_type >= 0)
			continue;

		if(iter >= begin && iter < end) {
			const String& from = src.words.GetKey(wp.from);
			const String& to = src.words.GetKey(wp.to);
			ValueArray arr;
			arr.Add(from);
			arr.Add(to);
			word_pairs.Add(arr);
			tmp_wp_ptrs.Add(&wp);
		}
		else if(iter >= end)
			break;
		iter++;
	}
	if(word_pairs.IsEmpty()) {
		NextPhase();
		return;
	}

	args.params("word pairs") = word_pairs;

	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.Get(args, [this](String res) {
		TaskArgs& args = this->args;
		auto& src = *p.srctxt;

		Value v = ParseJSON(res);
		LOG(AsJSON(v, true));
		ValueArray word_pair_classes = v("response-short")("word pairs classes");
		ValueArray input_words = args.params("word pairs");
		int c = min(word_pair_classes.GetCount(), input_words.GetCount());

		bool line_match = word_pair_classes.GetCount() == input_words.GetCount();
		PROCESS_ASSERT(line_match);
		int offset = 1 + 1;

		for(int i = 0; i < word_pair_classes.GetCount(); i++) {
			ValueArray input_word_pair = input_words[i];
			ValueArray result_words = word_pair_classes[i];
			if(result_words.GetCount() != 2)
				continue;

			WordPairType& wp = *tmp_wp_ptrs[i];
			int wc_i_list[2];
			for(int i = 0; i < result_words.GetCount(); i++) {
				String p = result_words[i].ToString();
				wc_i_list[i] = src.word_classes.FindAdd(p);
			}
			wp.from_type = wc_i_list[0];
			wp.to_type = wc_i_list[1];
		}

		int a = 0;
		for(const WordPairType& wp : src.ambiguous_word_pairs.GetValues()) {
			if(wp.from < 0 || wp.to < 0)
				continue;
			if(wp.from_type >= 0 && wp.to_type >= 0)
				a++;
		}
		src.diagnostics.GetAdd("ambiguous word pairs: total") =
			IntStr(src.ambiguous_word_pairs.GetCount());
		src.diagnostics.GetAdd("ambiguous word pairs: actual") = IntStr(a);
		src.diagnostics.GetAdd("ambiguous word pairs: percentage") =
			DblStr((double)a / (double)src.ambiguous_word_pairs.GetCount() * 100);

		NextBatch();
		SetWaiting(false);
	});
}

void ScriptTextProcess::ImportTokenTexts()
{
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;

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
			if(w_i < 0) {
				String key = src.tokens.GetKey(tk_i);
				w_i = src.words.Find(key);
				if(w_i < 0) {
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
			int next_w_i = j + 1 < word_is.GetCount() ? word_is[j] : -1;
			succ = succ && GetTypePhrase(word_classes, src, next_w_i, w_i, prev_w_i);
			prev_w_i = w_i;
		}

		if(word_classes.IsEmpty())
			succ = false;

		if(succ) {
			CombineHash ch;
			for(int wc_i : word_classes)
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
	src.diagnostics.GetAdd("token texts to virtual phrases: actual") = IntStr(actual);
	src.diagnostics.GetAdd("token texts to virtual phrases: percentage") =
		DblStr((double)actual / (double)total * 100);

	// int punctuation_mark_i = src.word_classes.FindAdd("punctuation mark");
	// int punctuation_i = src.word_classes.FindAdd("punctuation");
	for(int i = 0; i < src.virtual_phrase_parts.GetCount(); i++)
		src.virtual_phrase_parts[i].count = 0;

	for(int i = 0; i < src.virtual_phrases.GetCount(); i++) {
		VirtualPhrase& vp = src.virtual_phrases[i];
		Vector<Vector<int>> tmps;
		Vector<int> tmp;

		// NOTE: see duplicate in fn 3

		for(int wc_i : vp.word_classes) {
			String wc = src.word_classes[wc_i];
			int a = wc.Find("punctuation");
			int b = wc.Find("conjunction");
			// if (type == punctuation_mark_i || type == punctuation_i) {
			if(a >= 0 || b >= 0) {
				if(tmp.GetCount()) {
					Swap(tmps.Add(), tmp);
					tmp.SetCount(0);
				}
				if(b >= 0)
					tmp << wc_i;
			}
			else
				tmp << wc_i;
		}
		if(tmp.GetCount()) {
			Swap(tmps.Add(), tmp);
			tmp.SetCount(0);
		}
		CombineHash struct_ch;
		Vector<int> vpp_is;
		for(const Vector<int>& tmp : tmps) {
			CombineHash ch;
			for(int type : tmp)
				ch.Do(type).Put(1);
			hash_t h = ch;

			int vpp_i = -1;
			VirtualPhrasePart& vpp = MapGetAdd(src.virtual_phrase_parts, h, vpp_i);
			if(vpp.word_classes.IsEmpty())
				vpp.word_classes <<= tmp;
			vpp.count++;
			vpp_is << vpp_i;
			struct_ch.Do(vpp_i).Put(1);
		}
		hash_t vps_h = struct_ch;
		int vps_i = -1;
		VirtualPhraseStruct& vps = MapGetAdd(src.virtual_phrase_structs, vps_h, vps_i);
		// if (vps.parts.IsEmpty())
		vps.virtual_phrase_parts <<= vpp_is;
		vp.virtual_phrase_struct = vps_i;
	}
	LOG(src.virtual_phrase_parts.GetCount());
	LOG(src.virtual_phrase_parts.GetCount() * 100.0 / src.virtual_phrases.GetCount());
	NextPhase();
}

void ScriptTextProcess::ClassifySentences()
{
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;

	args.fn = FN_CLASSIFY_SENTENCE;
	args.params = ValueMap();

	int iter = 0;
	int begin = batch * words_per_action_task;
	int end = begin + words_per_action_task;
	end = min(end, src.virtual_phrase_parts.GetCount());
	tmp_vpp_ptrs.Clear();

	ValueArray classified_sentences;
	for(VirtualPhrasePart& vpp : src.virtual_phrase_parts.GetValues()) {

		if(vpp.struct_part_type >= 0)
			continue;

		if(iter >= begin && iter < end) {
			String s;
			int punct_count = 0;
			bool fail = false;
			for(int j = 0; j < vpp.word_classes.GetCount(); j++) {
				if(j)
					s << ",";
				int wc_i = vpp.word_classes[j];
				if(wc_i >= src.word_classes.GetCount()) {
					fail = true;
					break;
				}
				String wc = src.word_classes[wc_i];

				int a = wc.Find("(");
				if(a >= 0)
					wc = wc.Left(a);
				a = wc.Find(",");
				if(a >= 0)
					wc = wc.Left(a);

				if(wc.Find("punctuation") >= 0)
					punct_count++;

				s << wc;
			}

			if(punct_count > 8 || fail)
				continue;

			classified_sentences << s;
			tmp_vpp_ptrs << &vpp;
		}
		else if(iter >= end)
			break;

		iter++;
	}
	args.params("classified_sentences") = classified_sentences;

	if(classified_sentences.IsEmpty()) {
		NextPhase();
		return;
	}

	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.Get(args, [this](String res) {
		TaskArgs& args = this->args;
		auto& src = *p.srctxt;

		Value v = ParseJSON(res);
		LOG(AsJSON(v, true));
		ValueArray titles = v("response-short")("titles");
		ValueArray input_words = args.params("classified_sentences");
		PROCESS_ASSERT(!titles.IsEmpty());
		
		actual = 0;
		total = 0;
		int a = tmp_vpp_ptrs.GetCount();
		int b = titles.GetCount();
		int c = min(titles.GetCount(), input_words.GetCount());
		bool line_match = a == b;
		PROCESS_ASSERT(line_match); // soft assert -> stop process & show error message
		
		for(int i = 0; i < c; i++) {
			ValueArray classes = input_words[i];
			String title = titles[i].ToString();
			Vector<int> word_classes;
			VirtualPhrasePart& vpp = *tmp_vpp_ptrs[i];
			{
				bool fail = false;
				for (Value v : classes) {
					String c = v.ToString();
					int wc_i = src.word_classes.FindAdd(c);
					if (wc_i < 0) {
						fail = true;
						break;
					}
					word_classes << wc_i;
				}
				if (fail) continue;
			}
			
			if (vpp.word_classes.IsEmpty())
				vpp.word_classes <<= word_classes;
			
			vpp.struct_part_type = src.struct_part_types.FindAdd(title);
		}

		a = 0;
		for(const VirtualPhrasePart& vpp : src.virtual_phrase_parts.GetValues())
			if(vpp.struct_part_type >= 0)
				a++;
		src.diagnostics.GetAdd("virtual phrases: total") =
			IntStr(src.virtual_phrase_parts.GetCount());
		src.diagnostics.GetAdd("virtual phrases: actual") = IntStr(a);
		src.diagnostics.GetAdd("virtual phrases: percentage") =
			DblStr((double)a / (double)src.virtual_phrase_parts.GetCount() * 100);

		NextBatch();
		SetWaiting(false);
	});
}

void ScriptTextProcess::VirtualPhraseParts()
{
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;

	args.fn = FN_CLASSIFY_SENTENCE_STRUCTURES;
	args.params = ValueMap();

	int begin = batch * vpp_per_action_task;
	int end = begin + vpp_per_action_task;
	end = min(end, src.virtual_phrase_structs.GetCount());

	int iter = 0;
	tmp_vps_ptrs.Clear();
	ValueArray sents;
	for(VirtualPhraseStruct& vps : src.virtual_phrase_structs.GetValues()) {
		if(vps.struct_type >= 0)
			continue;

		if(iter >= begin && iter < end) {
			ValueArray words;
			bool fail = false;
			for(int j = 0; j < vps.virtual_phrase_parts.GetCount(); j++) {
				int vpp_i = vps.virtual_phrase_parts[j];

				const VirtualPhrasePart& vpp = src.virtual_phrase_parts[vpp_i];
				if(vpp.struct_part_type < 0) {
					fail = true;
					break;
				}

				String type_str = src.struct_part_types[vpp.struct_part_type];
				if(type_str.IsEmpty()) {
					fail = true;
					break;
				}
				words.Add(type_str);
			}
			if(fail)
				continue;
			if(words.IsEmpty())
				continue;

			sents.Add(words);
			tmp_vps_ptrs << &vps;
		}
		else if(iter >= end)
			break;

		iter++;
	}
	if(sents.IsEmpty()) {
		NextPhase();
		return;
	}
	params("classified_sentences") = sents;

	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.Get(args, [this](String res) {
		TaskArgs& args = this->args;
		auto& src = *p.srctxt;

		Value v = ParseJSON(res);
		// LOG(AsJSON(v, true));
		ValueArray categories = v("response-short")("categories");
		ValueArray input_words = args.params("classified_sentences");
		int c = min(categories.GetCount(), input_words.GetCount());

		bool line_match = tmp_vps_ptrs.GetCount() == input_words.GetCount();
		PROCESS_ASSERT(line_match);

		for(int i = 0; i < c; i++) {
			VirtualPhraseStruct& vps = *tmp_vps_ptrs[i];
			String struct_type = categories[i].ToString();
			vps.struct_type = src.struct_types.FindAdd(struct_type);
		}

		int a = 0;
		for(const VirtualPhraseStruct& vps : src.virtual_phrase_structs.GetValues())
			if(vps.struct_type >= 0)
				a++;
		src.diagnostics.GetAdd("virtual phrase structs: total") =
			IntStr(src.virtual_phrase_structs.GetCount());
		src.diagnostics.GetAdd("virtual phrase structs: actual") = IntStr(a);
		src.diagnostics.GetAdd("virtual phrase structs: percentage") =
			DblStr((double)a / (double)src.virtual_phrase_structs.GetCount() * 100);

		NextBatch();
		SetWaiting(false);
	});
}

void ScriptTextProcess::VirtualPhraseStructs()
{
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;

	if(batch >= src.token_texts.GetCount()) {
		src.diagnostics.GetAdd("token text to phrase: total") =
			IntStr(src.token_texts.GetCount());
		src.diagnostics.GetAdd("token text to phrase: actual") = IntStr(actual);
		src.diagnostics.GetAdd("token text to phrase: percentage") =
			DblStr((double)actual / (double)src.token_texts.GetCount() * 100);
		NextPhase();
		return;
	}

	// NOTE: see duplicate in fn 0
	// TODO reduce duplicate code
	Vector<int> word_is, word_classes;
	int i = batch;
	{
		const TokenText& txt = src.token_texts[i];
		if(txt.virtual_phrase < 0) {
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
			if(w_i < 0) {
				String key = src.tokens.GetKey(tk_i);
				w_i = src.words.Find(key);
				if(w_i < 0) {
					key = ToLower(src.tokens.GetKey(tk_i));
					w_i = src.words.Find(key);
				}
				tk.word_ = w_i;
			}
			word_is << w_i;
		}

		const VirtualPhrase& vp = src.virtual_phrases[txt.virtual_phrase];
		if(word_is.GetCount() != vp.word_classes.GetCount()) {
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
			// if (type == punctuation_mark_i || type == punctuation_i) {
			if(a >= 0 || b >= 0) {
				if(w_is.GetCount()) {
					Swap(w_isv.Add(), w_is);
					Swap(wc_isv.Add(), wc_is);
					w_is.SetCount(0);
					wc_is.SetCount(0);
				}
				if(b >= 0) {
					wc_is << wc_i;
					w_is << w_i; // NOTE: this is NOT duplicate
				}
			}
			else {
				wc_is << wc_i;
				w_is << w_i;
			}
		}
		if(w_is.GetCount()) {
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
				for(int wc_i : wc_is)
					ch.Do(wc_i).Put(1);
				wc_h = ch;
			}
			{
				CombineHash ch;
				for(int w_i : w_is)
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

void ScriptTextProcess::PhrasePartAnalysis()
{
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;

	switch(phase) {
	case PHASE_ELEMENT:
		args.fn = FN_CLASSIFY_PHRASE_ELEMENTS;
		break;
	case PHASE_COLOR:
		args.fn = FN_CLASSIFY_PHRASE_COLOR;
		break;
	case PHASE_ATTR:
		args.fn = FN_CLASSIFY_PHRASE_ATTR;
		break;
	case PHASE_ACTIONS:
		args.fn = FN_CLASSIFY_PHRASE_ACTIONS;
		break;
	case PHASE_SCORES:
		args.fn = FN_CLASSIFY_PHRASE_SCORES;
		break;
	case PHASE_TYPECLASS:
		args.fn = FN_CLASSIFY_PHRASE_TYPECLASS;
		break;
	case PHASE_CONTENT:
		args.fn = FN_CLASSIFY_PHRASE_CONTENT;
		break;
	default:
		Panic("TODO");
	}
	ValueArray phrases;
	ValueArray elements;
	ValueArray typeclasses;
	ValueArray contents;

	#define TYPECAST(idx, str, c) typeclasses.Add(str);
	TYPECAST_LIST
	#undef TYPECAST
	PROCESS_ASSERT(src.ctx.content.labels.GetCount());
	
	for(int i = 0; i < src.ctx.content.labels.GetCount(); i++) {
		const auto& it = src.ctx.content.labels[i];
		ValueArray arr;
		for(int j = 0; j < 3; j++)
			arr.Add(it.parts[j]);
		contents.Add(arr);
	}
	
	int per_action_task = 50;
	if (phase == PHASE_TYPECLASS)
		per_action_task = 20;
	else if (phase >= PHASE_ATTR)
		per_action_task = 35;
	
	int begin = batch * per_action_task;
	int end = begin + per_action_task;

	Color no_clr(0, 0, 0);
	tmp_pp_ptrs.SetCount(0);
	tmp.SetCount(0);

	if(batch == 0) {
		tmp_iters.SetCount(0);
		int trimmed_by[PHASE_COUNT];
		memset(trimmed_by, 0, sizeof(trimmed_by));
		
		if(phase == PHASE_ELEMENT && vmap.IsEmpty())
			vmap = src.GetSortedElements();

		int iter = 0;
		int idx = -1;
		for(const PhrasePart& pp : src.phrase_parts.GetValues()) {
			idx++;

			if((phase == PHASE_COLOR && pp.clr != no_clr) ||
			   (phase > PHASE_COLOR && pp.clr == no_clr)) {
				trimmed_by[phase]++;
				continue;
			}

			if((phase == PHASE_ATTR && pp.attr >= 0) || (phase > PHASE_ATTR && pp.attr < 0)) {
				trimmed_by[phase]++;
				continue;
			}

			if((phase == PHASE_ACTIONS && !pp.actions.IsEmpty()) ||
			   (phase > PHASE_ACTIONS && pp.actions.IsEmpty())) {
				trimmed_by[phase]++;
				continue;
			}

			if((phase == PHASE_SCORES && pp.HasScores()) ||
			   (phase > PHASE_SCORES && !pp.HasScores())) {
				trimmed_by[phase]++;
				continue;
			}

			if((phase == PHASE_TYPECLASS && !pp.typecasts.IsEmpty()) ||
			   (phase > PHASE_TYPECLASS && pp.typecasts.IsEmpty())) {
				trimmed_by[phase]++;
				continue;
			}

			if((phase == PHASE_CONTENT && !pp.contrasts.IsEmpty()) ||
			   (phase > PHASE_CONTENT && pp.contrasts.IsEmpty())) {
				trimmed_by[phase]++;
				continue;
			}

			if((phase == PHASE_ELEMENT && pp.el_i >= 0) || (phase > PHASE_ELEMENT && pp.el_i < 0)) {
				trimmed_by[phase]++;
				continue;
			}
			
			tmp_iters << idx;
			iter++;
		}
	}

	for(int i = begin; i < end && i < tmp_iters.GetCount(); i++) {
		int idx = tmp_iters[i];
		PhrasePart& pp = src.phrase_parts[idx];
		String phrase = src.GetWordString(pp.words);
		phrases << phrase;
		tmp_pp_ptrs << &pp;
		tmp << idx;
	}

	if(phrases.IsEmpty()) {
		NextPhase();
		return;
	}

	if(phase == PHASE_ELEMENT) {
		PROCESS_ASSERT(vmap.GetCount());
		int max_elements = 30;
		for(int i = 0; i < vmap.GetCount(); i++) {
			int el_i = vmap.GetKey(i);
			String element = src.element_keys[el_i];
			if(element == "n/a" || element == "none" || element.Left(1) == "(")
				continue;
			elements << element;
			if(elements.GetCount() >= max_elements)
				break;
		}
	}

	args.params("phrases") = phrases;
	args.params("elements") = elements;
	args.params("typeclasses") = typeclasses;
	args.params("contents") = contents;

	Panic("TODO add params to prompt");
	
	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	Event<String> cb;
	if(args.fn == PHASE_ELEMENT)
		cb = THISBACK(OnPhraseElement);
	else if(args.fn == PHASE_COLOR)
		cb =THISBACK(OnPhraseColors);
	else if(args.fn == PHASE_ATTR)
		cb =THISBACK(OnPhraseAttrs);
	else if(args.fn == PHASE_ACTIONS)
		cb =THISBACK(OnPhraseActions);
	else if(args.fn == PHASE_SCORES)
		cb =THISBACK(OnPhraseScores);
	else if(args.fn == PHASE_TYPECLASS)
		cb =THISBACK(OnPhraseTypeclasses);
	else if(args.fn == PHASE_CONTENT)
		cb =THISBACK(OnPhraseContrast);
	else
		TODO;
	m.Get(args, cb);
}

void ScriptTextProcess::OnPhraseColors(String res) {
	TaskArgs& args = this->args;
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;
	
	Value v = ParseJSON(res);
	// LOG(AsJSON(v, true));
	ValueArray output = v("response-short")("metaphorical RGB colors");
	ValueArray input = args.params("phrases");
	int c = min(output.GetCount(), input.GetCount());

	bool line_match = tmp_pp_ptrs.GetCount() == output.GetCount();
	PROCESS_ASSERT(line_match);

	Color black(0,0,0);
	Color non_black(1,1,1);
	
	for(int i = 0; i < c; i++) {
		PhrasePart& pp = *tmp_pp_ptrs[i];
		ValueArray clr_arr = output[i];
		int R = clr_arr[0];
		int G = clr_arr[1];
		int B = clr_arr[2];
		Color clr(R,G,B);
		
		if (clr == black)
			clr = non_black;
		
		pp.clr = clr;
	}
	
	
	int a = 0;
	for (const PhrasePart& pp : src.phrase_parts.GetValues())
		if (pp.clr != black)
			a++;
	src.diagnostics.GetAdd("phrase part color: total") = IntStr(src.phrase_parts.GetCount());
	src.diagnostics.GetAdd("phrase part color: actual") = IntStr(a);
	src.diagnostics.GetAdd("phrase part color: percentage") =  DblStr((double)a / (double)src.phrase_parts.GetCount() * 100);
	
	
	NextBatch();
	SetWaiting(false);
}

void ScriptTextProcess::OnPhraseAttrs(String res) {
	TaskArgs& args = this->args;
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;
	
	Value v = ParseJSON(res);
	// LOG(AsJSON(v, true));
	ValueArray output = v("response-short")("group_and_value_matches");
	ValueArray input = args.params("phrases");
	int c = min(output.GetCount(), input.GetCount());

	bool line_match = tmp_pp_ptrs.GetCount() == output.GetCount();
	PROCESS_ASSERT(line_match);

	for(int i = 0; i < c; i++) {
		PhrasePart& pp = *tmp_pp_ptrs[i];
		
		// This shouldn't happen
		if (pp.attr >= 0)
			continue;
		ValueArray attr = output[i];
		PROCESS_ASSERT(attr.GetCount() == 2);
		if (attr.GetCount() < 2)
			continue;
		
		AttrHeader ah;
		ah.group = ToLower(attr[0].ToString());
		ah.value = ToLower(attr[1].ToString());
		MapGetAdd(src.attrs, ah, pp.attr);
	}
	
	int a = 0;
	for (const PhrasePart& pp : src.phrase_parts.GetValues())
		if (pp.attr >= 0)
			a++;
	src.diagnostics.GetAdd("phrase part attrs: total") = IntStr(src.phrase_parts.GetCount());
	src.diagnostics.GetAdd("phrase part attrs: actual") = IntStr(a);
	src.diagnostics.GetAdd("phrase part attrs: percentage") =  DblStr((double)a / (double)src.phrase_parts.GetCount() * 100);
	
	NextBatch();
	SetWaiting(false);
}

void ScriptTextProcess::OnPhraseActions(String res) {
	TaskArgs& args = this->args;
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;
	
	Value v = ParseJSON(res);
	// LOG(AsJSON(v, true));
	ValueArray output = v("response-short")("action states");
	ValueArray input = args.params("phrases");
	int c = min(output.GetCount(), input.GetCount());

	bool line_match = tmp_pp_ptrs.GetCount() == output.GetCount();
	PROCESS_ASSERT(line_match);
	Vector<int> actions;

	for(int i = 0; i < c; i++) {
		PhrasePart& pp = *tmp_pp_ptrs[i];
		ValueArray phrase_actions = output[i];
		PROCESS_ASSERT(phrase_actions.GetCount());
		
		actions.SetCount(0);
		for(int i = 0; i < phrase_actions.GetCount(); i++) {
			ValueArray act_arr = phrase_actions[i];
			PROCESS_ASSERT(act_arr.GetCount() == 2);
			
			ActionHeader aa;
			aa.action = act_arr[0].ToString();
			aa.arg = act_arr[1].ToString();
			
			MapGetAdd(src.actions, aa, actions.Add());
		}
		Sort(actions, StdLess<int>());
		pp.actions <<= actions;
	}
	
	int a = 0;
	for (const PhrasePart& pp : src.phrase_parts.GetValues())
		if (!pp.actions.IsEmpty())
			a++;
	src.diagnostics.GetAdd("phrase part actions: total") = IntStr(src.phrase_parts.GetCount());
	src.diagnostics.GetAdd("phrase part actions: actual") = IntStr(a);
	src.diagnostics.GetAdd("phrase part actions: percentage") =  DblStr((double)a / (double)src.phrase_parts.GetCount() * 100);
	
	NextBatch();
	SetWaiting(false);
}

void ScriptTextProcess::OnPhraseScores(String res) {
	TaskArgs& args = this->args;
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;
	
	Value v = ParseJSON(res);
	// LOG(AsJSON(v, true));
	ValueArray output = v("response-short")("score factors");
	ValueArray input = args.params("phrases");
	int c = min(output.GetCount(), input.GetCount());

	bool line_match = tmp_pp_ptrs.GetCount() == output.GetCount();
	PROCESS_ASSERT(line_match);

	for(int i = 0; i < c; i++) {
		PhrasePart& pp = *tmp_pp_ptrs[i];
		ValueArray scores = output[i];
		PROCESS_ASSERT(scores.GetCount() == SCORE_COUNT);
		
		// Expect x values
		if (scores.GetCount() != SCORE_COUNT)
			continue;
		
		for(int j = 0; j < scores.GetCount(); j++)
			pp.scores[j] = scores[j];
	}
	
	int a = 0;
	for (const PhrasePart& pp : src.phrase_parts.GetValues())
		if (pp.HasScores())
			a++;
	src.diagnostics.GetAdd("phrase part scores: total") = IntStr(src.phrase_parts.GetCount());
	src.diagnostics.GetAdd("phrase part scores: actual") = IntStr(a);
	src.diagnostics.GetAdd("phrase part scores: percentage") =  DblStr((double)a / (double)src.phrase_parts.GetCount() * 100);
	
	
	NextBatch();
	SetWaiting(false);
}

void ScriptTextProcess::OnPhraseTypeclasses(String res) {
	TaskArgs& args = this->args;
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;
	
	Value v = ParseJSON(res);
	// LOG(AsJSON(v, true));
	ValueArray output = v("response-short")("score factors");
	ValueArray input = args.params("phrases");
	int c = min(output.GetCount(), input.GetCount());

	bool line_match = tmp_pp_ptrs.GetCount() == output.GetCount();
	PROCESS_ASSERT(line_match);

	for(int i = 0; i < c; i++) {
		PhrasePart& pp = *tmp_pp_ptrs[i];
		ValueArray values = output[i];
		PROCESS_ASSERT(values.GetCount() > 0);
		if (values.IsEmpty())
			continue;
		
		pp.typecasts.Clear();
		for(int j = 0; j < values.GetCount(); j++) {
			int opt = values[j];
			if (opt <= 0 || opt > TYPECAST_COUNT) {
				continue;
			}
			pp.typecasts.Add(opt);
		}
	}
	
	int a = 0;
	for (const PhrasePart& pp : src.phrase_parts.GetValues())
		if (pp.typecasts.GetCount())
			a++;
		
	src.diagnostics.GetAdd("Phrase Part Analysis: typeclasses: total") = IntStr(src.phrase_parts.GetCount());
	src.diagnostics.GetAdd("Phrase Part Analysis: typeclasses: actual") = IntStr(a);
	src.diagnostics.GetAdd("Phrase Part Analysis: typeclasses: percentage") =  DblStr((double)a / (double)src.phrase_parts.GetCount() * 100);
	
	NextBatch();
	SetWaiting(false);
}

void ScriptTextProcess::OnPhraseContrast(String res) {
	TaskArgs& args = this->args;
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;
	
	Value v = ParseJSON(res);
	// LOG(AsJSON(v, true));
	ValueArray output = v("response-short")("storylines");
	ValueArray input = args.params("phrases");
	int c = min(output.GetCount(), input.GetCount());

	bool line_match = tmp_pp_ptrs.GetCount() == output.GetCount();
	PROCESS_ASSERT(line_match);

	for(int i = 0; i < c; i++) {
		PhrasePart& pp = *tmp_pp_ptrs[i];
		ValueArray out = output[i];
		PROCESS_ASSERT(out.GetCount());
		
		if (out.IsEmpty())
			continue;
		
		pp.contrasts.Clear();
		for(int j = 0; j < out.GetCount(); j++) {
			ValueArray val = out[j];
			int opt = val[0];
			if (opt <= 0 || opt > CONTENT_COUNT) {
				//pp.contrasts.Clear();
				//break;
				continue;
			}
			String part = val[1].ToString();
			PROCESS_ASSERT(part.GetCount());
			int mod = -1;
			if      (part.Find("A") >= 0 || part.Find("a") >= 0) mod = 0;
			else if (part.Find("B") >= 0 || part.Find("b") >= 0) mod = 1;
			else if (part.Find("C") >= 0 || part.Find("c") >= 0) mod = 2;
			else continue;
			opt--; // convert to 0-based index
			int code = opt * PART_COUNT + mod;
			pp.contrasts.Add(code);
		}
	}
	
	
	int a = 0;
	for (const PhrasePart& pp : src.phrase_parts.GetValues())
		if (pp.contrasts.GetCount())
			a++;
	src.diagnostics.GetAdd("Phrase Part Analysis: content: total") = IntStr(src.phrase_parts.GetCount());
	src.diagnostics.GetAdd("Phrase Part Analysis: content: actual") = IntStr(a);
	src.diagnostics.GetAdd("Phrase Part Analysis: content: percentage") =  DblStr((double)a / (double)src.phrase_parts.GetCount() * 100);
	
	NextBatch();
	SetWaiting(false);
}

void ScriptTextProcess::OnPhraseElement(String res) {
	TaskArgs& args = this->args;
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;
	
	Value v = ParseJSON(res);
	// LOG(AsJSON(v, true));
	ValueArray output = v("response-short")("elements");
	ValueArray input = args.params("phrases");
	int c = min(output.GetCount(), input.GetCount());

	bool line_match = tmp_pp_ptrs.GetCount() == output.GetCount();
	PROCESS_ASSERT(line_match);

	for(int i = 0; i < c; i++) {
		PhrasePart& pp = *tmp_pp_ptrs[i];
		String element = output[i].ToString();
		PROCESS_ASSERT(element.Is());
		if (element.IsEmpty()) continue;
		pp.el_i = src.element_keys.FindAdd(element);
	}
	
	NextBatch();
	SetWaiting(false);
}

void ScriptTextProcess::Prepare(TaskFn fn)
{
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;

	args.fn = fn;
	
	int per_action_task = 0;
	int per_action_clrs = 60;
	int per_action_attrs = 40;
	if (fn == PHASE_COLORS)	per_action_task = per_action_clrs;
	if (fn == PHASE_ATTRS)	per_action_task = per_action_attrs;

	int begin = batch * per_action_task;
	int end = begin + per_action_task;
	
	Color black(0,0,0);
	int iter = 0;
	ValueArray actions;
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
				actions << s;
			}
			iter++;
		}
	}
	if (actions.IsEmpty()) {
		NextPhase();
		return; // ready
	}
	params("actions") = actions;
}

void ScriptTextProcess::Colors()
{
	Prepare(FN_CLASSIFY_ACTION_COLOR);

	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.Get(args, [this](String result) {
		TaskArgs& args = this->args;
		auto& src = *p.srctxt;

		Value v = ParseJSON(result);
		// LOG(AsJSON(v, true));
		ValueArray colors = v("response-short")("colors");
		ValueArray input_words = args.params("actions");
		PROCESS_ASSERT(input_words.GetCount());
		int c = min(colors.GetCount(), input_words.GetCount());

		bool line_match = tmp_vps_ptrs.GetCount() == input_words.GetCount();
		PROCESS_ASSERT(line_match);

		Color black(0,0,0);
		Color non_black(1,1,1);
		
		for(int i = 0; i < c; i++) {
			ValueArray act = input_words[i];
			ValueArray clr_val = colors[i];
			PROCESS_ASSERT(clr_val.GetCount() == 3);
			int R = clr_val[0];
			int G = clr_val[1];
			int B = clr_val[2];
			Color clr(R,G,B);
			
			ActionHeader ah;
			ah.action = act[0].ToString();
			ah.arg = act[1].ToString();
			
			if (clr == black)
				clr = non_black;
			
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
}

void ScriptTextProcess::Attrs()
{
	Prepare(FN_CLASSIFY_PHRASE_ACTION_ATTR);

	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.Get(args, [this](String result) {
		TaskArgs& args = this->args;
		auto& src = *p.srctxt;

		Value v = ParseJSON(result);
		// LOG(AsJSON(v, true));
		ValueArray output = v("response-short")("attributes");
		ValueArray input = args.params("actions");
		PROCESS_ASSERT(input.GetCount());
		int c = min(output.GetCount(), input.GetCount());

		bool line_match = tmp_vps_ptrs.GetCount() == input.GetCount();
		PROCESS_ASSERT(line_match);

		for(int i = 0; i < c; i++) {
			ValueArray attrs = output[i];
			ValueArray acts = input[i];
			PROCESS_ASSERT(attrs.GetCount() == 2);
			PROCESS_ASSERT(acts.GetCount() == 2);
			if (attrs.GetCount() != 2) continue;
			ActionHeader ah;
			ah.action = acts[0];
			ah.arg = acts[1];
			AttrHeader ath;
			ath.group = attrs[0];
			ath.value = attrs[1];
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
}

void ScriptTextProcess::MainGroups()
{
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;
	RealizeBatch_AttrExtremesBatch();
	
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
	args.params = ValueMap();
	args.params("category") = batch.group;
	
	ValueArray attributes;
	for (const auto& s : values.GetKeys())
		attributes.Add(s);
	args.params("attributes") = attributes;
	
	if (attributes.IsEmpty()) {
		NextBatch();
		return;
	}
	
	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.Get(args, [this](String result) {
		Vector<AttrExtremesBatch>& batches = attr_extremes_batches;
		AttrExtremesBatch& batch = batches[this->batch];
		TaskArgs& args = this->args;
		auto& src = *p.srctxt;

		Value v = ParseJSON(result);
		// LOG(AsJSON(v, true));
		ValueArray input = args.params("attributes");
		ValueArray output = v("response-short")("attribute_summarization");
		PROCESS_ASSERT(input.GetCount());
		PROCESS_ASSERT(output.GetCount() == 2);
		
		if (output.GetCount() == 2) {
			String group = ToLower(batch.group);
			int attr_i[2] = {-1,-1};
			for(int i = 0; i < 2; i++) {
				AttrHeader ah;
				ah.group = group;
				ah.value = ToLower(output[i].ToString());
				MapGetAdd(src.attrs, ah, attr_i[i]);
			}
			
			ExportSimpleAttr& sat = src.simple_attrs.GetAdd(batch.group);
			sat.attr_i0 = attr_i[0];
			sat.attr_i1 = attr_i[1];
		}
		
		
		NextBatch();
		SetWaiting(false);
	});
}

void ScriptTextProcess::RealizeBatch_AttrExtremesBatch()
{
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;
	
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
}

void ScriptTextProcess::SimplifyAttrs()
{
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;
	int per_batch = 50;

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
			PROCESS_ASSERT(src.attrs.GetKey(gsa.attr_i0).group == group);
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
	args.params = ValueMap();
	args.params("group") = batch.group;
	args.params("attr0") = batch.attr0;
	args.params("attr1") = batch.attr1;
	
	ValueArray values;
	for (const auto& s : batch.attrs)
		values.Add(s);
	args.params("values") = values;
	
	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.Get(args, [this](String result) {
		Vector<AttrPolarBatch>& batches = attr_polar_batches;
		AttrPolarBatch& batch = batches[this->batch];
		PROCESS_ASSERT(p.srctxt);
		auto& src = *p.srctxt;
		
		Value v = ParseJSON(result);
		// LOG(AsJSON(v, true));
		ValueArray input = args.params("values");
		ValueArray output = v("response-short")("attribute_summarization");
		PROCESS_ASSERT(input.GetCount());
		PROCESS_ASSERT(output.GetCount() == input.GetCount());
		int c = min(output.GetCount(), input.GetCount());
		
		String group = batch.group;
		int i = src.simple_attrs.Find(group);
		String pos_value, neg_value;
		if (i >= 0) {
			const ExportSimpleAttr& esa = src.simple_attrs[i];
			pos_value = src.attrs.GetKey(esa.attr_i0).value;
			neg_value = src.attrs.GetKey(esa.attr_i1).value;
		}
		for(int i = 0; i < c; i++) {
			String key = batch.attrs[i]; // == input[i]
			int value_idx = output[i];
			bool is_negative = value_idx > 0;
			
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
		
		NextBatch();
		SetWaiting(false);
	});
}

void ScriptTextProcess::JoinOrphaned()
{
	PROCESS_ASSERT(uniq_attrs.GetCount());
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;
	int per_batch = 35;
	
	Panic("TODO useless? check");
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
			b.values << ah;
		}
	}
	
	if (batch >= batches.GetCount()) {
		NextPhase();
		return;
	}
	
	AttrJoinBatch& batch = batches[this->batch];
	
	args.fn = FN_MATCHING_ATTR;
	args.params = ValueMap();
	
	ValueArray values;
	for (const auto& v : batch.values)
		values.Add(v);
	args.params("values") = values;
	
	ValueArray groups;
	int count = min(20, uniq_attrs.GetCount());
	for(int i = 0; i < count; i++) {
		String group = uniq_attrs.GetKey(i);
		if (!group.IsEmpty()) {
			const ExportSimpleAttr& ea = src.simple_attrs.GetAdd(group);
			String a0 = src.attrs.GetKey(ea.attr_i0).value;
			String a1 = src.attrs.GetKey(ea.attr_i1).value;
			ValueArray g;
			g.Add(group);
			g.Add(a0);
			g.Add(a1);
			groups.Add(g);
		}
	}
	args.params("groups") = groups;
	
	SetWaiting(true);
	TaskMgr& m = AiTaskManager();
	m.Get(args, [this](String result) {
		AttrJoinBatch& batch = attr_join_batches[this->batch];
		PROCESS_ASSERT(p.srctxt);
		auto& src = *p.srctxt;
		
		Value v = ParseJSON(result);
		// LOG(AsJSON(v, true));
		ValueArray input = args.params("values");
		ValueArray output = v("response-short")("attribute_summarization");
		PROCESS_ASSERT(input.GetCount());
		int c = min(output.GetCount(), input.GetCount());
		
		bool line_match = input.GetCount() == batch.values.GetCount();
		PROCESS_ASSERT(line_match);
		if (line_match) {
			
			for(int i = 0; i < c; i++) {
				AttrHeader& ah = batch.values[i];
				ValueArray out = output[i];
				PROCESS_ASSERT(out.GetCount());
				
				int attr_i = src.attrs.Find(ah);
				if (attr_i < 0)
					continue;
				ExportAttr& ea = src.attrs[attr_i];
				
				String digit, sign;
				for(int i = 0; i < out.GetCount(); i++) {
					int chr = out[i];
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
				
				if (group_i < 0 || group_i >= batch.values.GetCount())
					continue;
				String group = batch.values[group_i];
				
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

void ScriptTextProcess::FixData()
{
	PROCESS_ASSERT(p.srctxt);
	auto& src = *p.srctxt;

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
			PROCESS_ASSERT(k >= 0);
			ExportAttr& ea = src.attrs[k];
			ea.simple_attr = sa_i;
		}
	}
#endif
}

ScriptTextProcess& ScriptTextProcess::Get(DatasetPtrs p, VfsPath path, Value params,
                                          SrcTextData& data, Event<> WhenStopped)
{
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
