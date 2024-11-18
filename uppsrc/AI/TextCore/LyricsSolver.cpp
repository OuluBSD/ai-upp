#include "TextCore.h"


NAMESPACE_UPP


void NavigatorState::RemoveDuplicate(const NavigatorState& s) {
	if (sorter == s.sorter) sorter = 0;
	if (element == s.element) element.Clear();
	if (attr.group == s.attr.group) attr.group.Clear();
	if (attr.value == s.attr.value) attr.value.Clear();
	if (clr_i == s.clr_i) clr_i = -1;
	if (act.action == s.act.action) act.action.Clear();
	if (act.arg == s.act.arg) act.arg.Clear();
	if (typeclass_i == s.typeclass_i) typeclass_i = -1;
	if (con_i == s.con_i) con_i = -1;
}

ArrayMap<String, ScriptSolver>& __ScriptSolvers() {
	static ArrayMap<String, ScriptSolver> map;
	return map;
}


ScriptSolver::ScriptSolver() {
	
}

ScriptSolver& ScriptSolver::Get(const DatasetPtrs& p, const String& ecs_path) {
	ASSERT(p.script);
	ArrayMap<String, ScriptSolver>& map = __ScriptSolvers();
	int i = map.Find(ecs_path);
	if (i >= 0)
		return map[i];
	
	ScriptSolver& ls = map.Add(ecs_path);
	ls.p = p;
	ls.artist = p.entity;
	ls.script = p.script;
	return ls;
}

int ScriptSolver::GetPhaseCount() const {
	return LS_COUNT;
}

void ScriptSolver::DoPhase() {
	TODO
	/*TextDatabase& db = GetDatabase();
	SourceData& sd = db.src_data;
	SourceDataAnalysis& sda = db.src_data.a;
	DatasetAnalysis& da = sda.dataset;
	Script& song = *this->script;
	sa = &da.GetComponentAnalysis(appmode, artist->file_title + " - " + song.file_title);*/
	
	if (phase == LS_FILL_LINES) {
		ProcessFillLines();
	}
	else if (phase == LS_COMPARISON) {
		ProcessComparison();
	}
	else if (phase == LS_TITLE) {
		ProcessTitle();
	}
}

void ScriptSolver::ClearScript() {
	for(int i = 0; i < script->parts.GetCount(); i++) {
		DynPart& sp = script->parts[i];
		for(int j = 0; j < sp.sub.GetCount(); j++) {
			DynSub& ds = sp.sub[j];
			for(int k = 0; k < ds.lines.GetCount(); k++) {
				DynLine& dl = ds.lines[k];
				dl.pp_i = -1;
				dl.end_pp_i = -1;
			}
		}
		sp.phrase_parts.Clear();
	}
}

void ScriptSolver::ProcessFillLines() {
	Script& song = *p->script;
	bool collect_token_texts = true; //song.lng_i == LNG_NATIVE;
	
	ComponentAnalysis& sa = da.GetComponentAnalysis(appmode, artist->file_title + " - " + song.file_title);
	
	if (/*!skip_ready &&*/ batch == 0 && sub_batch == 0) {
		added_phrases.Clear();
		sa.script_suggs.Clear();
	}
	
	if (sa.script_suggs.GetCount() >= sugg_limit) {
		NextPhase();
		return;
	}
	
	// Realize suggestion and minimum data
	ScriptSuggestion& sugg = sa.script_suggs.GetAdd(batch);
	{
		sugg.parts.SetCount(song.parts.GetCount());
		for(int i = 0; i < song.parts.GetCount(); i++) {
			auto& line = sugg.parts[i];
			DynPart& sp = song.parts[i];
			line.name = sp.GetName(appmode);
			if (!sp.person.IsEmpty())
				line.name += " by the singer '" + sp.person + "'";
		}
	}
	
	
	ScriptSolverArgs args; // 10
	args.fn = 10;
	args.lng_i = song.lng_i;
	args.is_story = song.is_story;
	args.is_unsafe = song.is_unsafe;
	args.is_self_centered = song.is_self_centered;
	
	MakeBelief(song, args, 1);
	
	// Add existing scripts
	active_part = -1;
	for(int i = 0; i < sugg.parts.GetCount(); i++) {
		auto& l = sugg.parts[i];
		const auto& lines = l.lines;
		DynPart& sp = song.parts[i];
		args.elements.Add(sp.el.element);
		int len = sp.GetExpectedLineCount();
		if (active_part == -1 && lines.GetCount() < len) {
			active_part = i; // Set active part to get new lines for
		}
		if (lines.IsEmpty()) continue;
		
		String s = sugg.parts[i].name + " (" + sp.el.element + ")";
		args.song.Add(s) = Join(lines, "\n");
	}
	
	if (active_part == -1) {
		NextBatch();
		return;
	}
	
	
	args.part = sugg.parts[active_part].name;
	DynPart& part = song.parts[active_part];
	
	// Get elements of generated rhyme
	{
		auto& spart = sugg.parts[active_part];
		int cur_len = spart.lines.GetCount();
		int expected_len = part.GetExpectedLineCount();
		args.rhyme_element = part.GetLineElementString(cur_len);
	}
	
	int per_part = 20;
	int min_per_part = 15;
	bool fail = false;
	int begin = batch * per_part;
	int end = begin + per_part;
	this->phrases.Clear();
	int con_type = part.GetContrastIndex();
	
	// Prefer con_type but use all if no phrases for some reason
	for(int i = -1; i < PART_COUNT; i++)
	{
		if (i == con_type) continue;
		int idx = i < 0 ? con_type : i;
		
		const auto& map = song.phrase_parts[idx];
		
		// Save offsets for reading
		args.offsets << args.phrases.GetCount();
		
		// Add phrases
		int end0 = min(map.GetCount(), end);
		int count = end0 - begin;
		/*if (count < min_per_part) {
			fail = true;
			continue;
		}*/
		for(int j = begin; j < end0; j++) {
			int pp_i = map[j];
			if (collect_token_texts) {
				const PhrasePart& pp = sa.phrase_parts[idx][pp_i];
				String s = da.GetWordString(pp.words);
				args.phrases << s;
				this->phrases << s;
			}
			else {
				const TranslatedPhrasePart& tpp = sa.trans_phrase_combs[song.lng_i][idx][pp_i];
				args.phrases << tpp.phrase;
				this->phrases << tpp.phrase;
			}
		}
		
		if (args.phrases.GetCount() >= min_per_part)
			break;
	}
	if (args.phrases.IsEmpty())
		fail = true;
	
	if (fail) {
		NextPhase();
		return;
	}
	
	
	args.vision = song.content_vision;
	args.ret_fail = true;
	
	 
	SetWaiting(1);
	
	TaskMgr& m = AiTaskManager();
	m.GetScriptSolver(appmode, args, THISBACK(OnProcessFillLines));
}

void ScriptSolver::OnProcessFillLines(String res) {
	TextDatabase& db = GetDatabase();
	SourceData& sd = db.src_data;
	SourceDataAnalysis& sda = db.src_data.a;
	DatasetAnalysis& da = sda.dataset;
	Script& song = *this->script;
	
	ComponentAnalysis& sa = da.GetComponentAnalysis(appmode, artist->file_title + " - " + song.file_title);
	
	CombineHash ch;
	bool fail = false;
	
	RemoveEmptyLines3(res);
	Vector<String> lines = Split(res, "\n");
	
	Vector<int> part_is;
	
	res = TrimBoth(res);
	
	/*if (res.Find("Normal 1") >= 0) {
		LOG(res);
	}*/
	
	int result_line = 0;
	if (res.GetCount() && IsDigit(res[0])) {
		int result_line = ScanInt(res);
		
		ScriptSuggestion& sugg = sa.script_suggs.GetAdd(batch);
		
		DynPart& sp = song.parts[active_part];
		if (result_line >= 0 && result_line < this->phrases.GetCount()) {
			String phrase = this->phrases[result_line];
			sugg.parts[active_part].lines.Add(phrase);
		}
	}
	else if (res.GetCount()) {
		int a = res.Find("):");
		if (a >= 0) {
			res = TrimBoth(res.Mid(a+2));
		}
		
		a = res.Find("\"");
		if (a > 0)
			res = res.Left(a);
		else
			RemoveQuotes(res);
		
		ScriptSuggestion& sugg = sa.script_suggs.GetAdd(batch);
		
		DynPart& sp = song.parts[active_part];
		Vector<String> lines = Split(res, "/");
		if (lines.GetCount() == 1)
			lines = Split(res, "\n");
		if (lines.GetCount() > 2)
			lines.SetCount(2);
		
		for (auto& l : lines) {
			if (l.Left(6) == "line #") {
				int a = l.Find(":");
				if (a >= 0)
					l = TrimBoth(l.Mid(a+1));
			}
		}
		auto& dst = sugg.parts[active_part].lines;
		for (String& line : lines)
			dst.Add(TrimBoth(line));
	}
	
	
	SetWaiting(0);
	NextSubBatch();
}


void ScriptSolver::ProcessComparison() {
	TextDatabase& db = GetDatabase();
	SourceData& sd = db.src_data;
	SourceDataAnalysis& sda = db.src_data.a;
	DatasetAnalysis& da = sda.dataset;
	Script& song = *this->script;
	
	
	ComponentAnalysis& sa = da.GetComponentAnalysis(appmode, artist->file_title + " - " + song.file_title);
	
	ScriptSolverArgs args; // 7
	args.fn = 7;
	
	MakeBelief(song, args, 0);
	
	if (batch == 0 && sub_batch == 0) {
		// Clear 'visited' vector, which stores visited suggestion comparisons
		visited.Clear();
		
		// Clear output already in case of errors or break of processing
		song.__text.Clear();
		
		// Find average length of suggestions
		Vector<int> lengths;
		for(int i = 0; i < sa.script_suggs.GetCount(); i++)
			lengths.Add(sa.script_suggs[i].GetText().GetCount());
		Sort(lengths, StdLess<int>());
		int mode = lengths[lengths.GetCount()/2];
		
		
		// Add all proper suggestions to remaining-vector
		matches.Clear();
		remaining.Clear();
		for(int i = 0; i < sa.script_suggs.GetCount(); i++) {
			// If the suggestion is very short, then skip it
			String sugg = sa.script_suggs[i].GetText();
			double diff = fabs((sugg.GetCount() / (double)mode) - 1.0);
			if (diff >= 0.66667)
				continue;
			
			
			remaining.Add(i);
		}
	}
	
	if (remaining.GetCount() <= 1) {
		NextPhase();
		return;
	}
	
	for(int i = 0; i < 2; i++) {
		int sugg_i = remaining[i];
		const ScriptSuggestion& ls = sa.script_suggs[sugg_i];
		String content = ls.GetText();
		
		if (TrimBoth(content).IsEmpty()) {
			OnProcessComparisonFail(i);
			return;
		}
		
		WString ws = content.ToWString();
		int chr_limit = 3000;
		if (ws.GetCount() > chr_limit)
			ws = ws.Left(chr_limit);
		content = ws.ToString();
		
		args.phrases << content;
	}
	
	args.part = song.content_vision;
	
	SetWaiting(1);
	
	TaskMgr& m = AiTaskManager();
	m.GetScriptSolver(appmode, args, THISBACK(OnProcessComparison));
}

void ScriptSolver::OnProcessComparison(String res) {
	TextDatabase& db = GetDatabase();
	SourceData& sd = db.src_data;
	SourceDataAnalysis& sda = db.src_data.a;
	DatasetAnalysis& da = sda.dataset;
	Script& song = *this->script;
	
	
	ComponentAnalysis& sa = da.GetComponentAnalysis(appmode, artist->file_title + " - " + song.file_title);
	
	int loser = 0;
	
	
	res = "entry #1: S0:" + res;
	
	RemoveEmptyLines3(res);
	Vector<String> lines = Split(res, "\n");
	Vector<double> total_scores;
	for (String& line : lines) {
		int a = line.Find(":");
		if (a < 0) continue;
		line = TrimBoth(line.Mid(a+1));
		Vector<String> parts = Split(line, ",");
		int score_sum = 0, score_count = 0;
		for (String& p : parts) {
			p = TrimBoth(p);
			a = p.Find(":");
			if (a < 0) continue;
			p = p.Mid(a+1);
			int score = ScanInt(TrimLeft(p));
			score_sum += score;
			score_count++;
		}
		double score = (double)score_sum / score_count;
		total_scores << score;
	}
	if (total_scores.GetCount() >= 2) {
		double s0 = total_scores[0];
		double s1 = total_scores[1];
		
		// Give second chance, if score is same
		if (s0 == s1) {
			int sugg0 = remaining[0];
			int sugg1 = remaining[1];
			hash_t hash = sugg0 * 1000 + sugg1; // easy
			if (visited.FindAdd(hash) < 0) {
				// Move first suggestion to last
				remaining.Remove(0);
				remaining.Add(sugg0);
				NextSubBatch(); // just to avoid clearing in the "batch==0&&sub_batch==0" case
				return;
			}
		}
		
		loser = s0 > s1 ? 1:0;
	}
	else
		loser = 0; // error
	
	
	OnProcessComparisonFail(loser);
}

void ScriptSolver::OnProcessComparisonFail(int loser) {
	TextDatabase& db = GetDatabase();
	SourceData& sd = db.src_data;
	SourceDataAnalysis& sda = db.src_data.a;
	DatasetAnalysis& da = sda.dataset;
	Script& song = *this->script;
	
	
	ComponentAnalysis& sa = da.GetComponentAnalysis(appmode, artist->file_title + " - " + song.file_title);
	
	
	int loser_sugg_i = remaining[loser];
	ScriptSuggestion& ls = sa.script_suggs[loser_sugg_i];
	ls.rank = remaining.GetCount()-1; // if remaining one, then rank is 0
	String& output = song.__suggestions.GetAdd(ls.rank);
	output = ls.GetText();
	FixOffensiveWords(output);
	remaining.Remove(loser);
	
	if (remaining.GetCount() == 1) {
		int sugg_i = remaining[0];
		ScriptSuggestion& ls = sa.script_suggs[sugg_i];
		ls.rank = 0;
		
		String content = ls.GetText();
		FixOffensiveWords(content);
		song.__suggestions.GetAdd(ls.rank) = content;
		SortByKey(song.__suggestions, StdLess<int>());
		
		
		LOG("Winner scripts:");
		LOG(content);
		
		song.__text = content;
		
		for(int i = 0; i < ls.parts.GetCount(); i++) {
			DynPart& sp = song.parts[i];
			
			/*
			sp->generated.Get().Clear();
			const auto& from = ls.lines[i];
			for(int j = 0; j < from.GetCount(); j++)
				sp->generated.Get().Add().text = from[j];
			*/
		}
	}
	
	SetWaiting(0);
	NextBatch();
}

void ScriptSolver::ProcessTitle() {
	Script& song = *this->script;
	ScriptSolverArgs args; // 8
	args.fn = 8;
	
	args.part = song.__text;
	args.lng_i = song.lng_i;
	
	SetWaiting(1);
	
	TaskMgr& m = AiTaskManager();
	m.GetScriptSolver(appmode, args, [this](String res) {
		res = TrimBoth(res);
		RemoveQuotes(res);
		
		TaskMgr& m = AiTaskManager();
		//script->english_title = res;
		//script->native_title.Clear();
		
		int lng = MetaDatabase::Single().GetLanguageIndex();
		/*if (lng != LNG_ENGLISH) {
			String code = GetLanguageCode(lng);
			m.Translate("EN-US", script->english_title, code, [this](String res) {
				res = TrimBoth(res);
				script->native_title = res;
				SetWaiting(0);
				NextPhase();
			});
		}
		else*/ {
			//script->native_title = script->english_title;
			script->native_title = res;
			SetWaiting(0);
			NextPhase();
		}
	});
}

void ScriptSolver::GetSuggestions(const DynPart& part, const DynSub& sub, const Vector<const DynLine*>& lines, Event<> WhenPartiallyReady) {
	tmp_part = const_cast<DynPart*>(&part);
	tmp_sub = const_cast<DynSub*>(&sub);
	tmp_lines <<= lines;
	this->WhenPartiallyReady = WhenPartiallyReady;
	
	Script& song = *this->script;
	ScriptSolverArgs args; // 18
	args.fn = 18;
	
	args.lng_i = song.lng_i;
	
	for(int i = 0; i < lines.GetCount(); i++) {
		const DynLine& l = *lines[i];
		if (l.text.IsEmpty())
			break;
		args.phrases << l.text;
		if (!l.edit_text.IsEmpty())
			args.phrases2 << l.edit_text;
		else if (!l.alt_text.IsEmpty())
			args.phrases2 << l.alt_text;
		else
			args.phrases2 << l.user_text;
	}
	
	Index<String> elements;
	if (part.el.element.GetCount()) elements.FindAdd(part.el.element);
	if (sub.el.element.GetCount()) elements.FindAdd(sub.el.element);
	args.elements <<= elements.GetKeys();
	
	TaskMgr& m = AiTaskManager();
	m.GetScriptSolver(appmode, args, [this](String res) {
		Vector<String> lines = Split(res, "\n");
		
		for(int i = 0; i < lines.GetCount(); i++) {
			String& l = lines[i];
			l = TrimBoth(l);
			if (l.IsEmpty()) {
				lines.Remove(i--);
				continue;
			}
			if (!IsDigit(l[0]) && i > 0) {
				lines[i-1] << " / " << l;
				lines.Remove(i--);
				continue;
			}
		}
		
		for (String& s : lines) {
			if (IsDigit(s[0])) {
				int a = s.Find(".");
				if (a >= 0)
					s = TrimLeft(s.Mid(a+1));
			}
		}
		DUMPC(lines);
		
		for (const DynLine* l : tmp_lines) {
			DynLine& dl = const_cast<DynLine&>(*l);
			dl.suggs.Clear();
		}
		
		for (String& s : lines) {
			Vector<String> parts = Split(s, "/");
			for(int i = 0; i < parts.GetCount() && i < tmp_lines.GetCount(); i++) {
				DynLine& dl = const_cast<DynLine&>(*tmp_lines[i]);
				dl.suggs << TrimBoth(parts[i]);
			}
			for(int i = parts.GetCount(); i < tmp_lines.GetCount(); i++) {
				DynLine& dl = const_cast<DynLine&>(*tmp_lines[i]);
				dl.suggs.Clear();
			}
		}
		
		this->WhenPartiallyReady();
	});
}

void ScriptSolver::GetExpanded(int part_i, int sub_i, int line_i, Event<> WhenPartiallyReady) {
	DynPart& part = script->parts[part_i];
	DynSub& sub = part.sub[sub_i];
	DynLine& line = sub.lines[line_i];
	tmp_part = &part;
	tmp_sub = &sub;
	tmp_line = &line;
	this->WhenPartiallyReady = WhenPartiallyReady;
	
	if (line.text.IsEmpty())
		return;
	
	Script& song = *this->script;
	ScriptSolverArgs args; // 21
	args.fn = 21;
	args.lng_i = song.lng_i;
	
	if (!part.story.IsEmpty()) args.phrases2 << part.story;
	if (!sub.story.IsEmpty())  args.phrases2 << sub.story;
	
	for(int i = 0; i < part.sub.GetCount(); i++) {
		const auto& s = part.sub[i];
		for(int j = 0; j < s.lines.GetCount(); j++) {
			const auto& dl = s.lines[j];
			args.phrases << dl.text;
			
			auto& state = args.line_states.Add();
			state.content = dl.text;
			
			const auto& ents = GetTypeclassEntities(appmode, dl.safety, artist->is_female);
			state.style_type = ents.GetKey(dl.style_type);
			const auto& vec = dl.style_type < ents.GetCount() ? ents[dl.style_type] : ents.Top();
			state.style_entity = dl.style_entity < vec.GetCount() ? vec [dl.style_entity] : vec.Top();
			state.safety = dl.safety;
			state.connector = dl.connector;
		}
	}
	args.ref = line.text;
	
	TaskMgr& m = AiTaskManager();
	m.GetScriptSolver(appmode, args, [this](String res) {
		res = TrimBoth(res);
		res.Replace("\r", "");
		res.Replace("\n\n", "\n");
		RemoveQuotes(res);
		if (res.Left(2) == "- ") res = TrimBoth(res.Mid(2));
		
		tmp_line->expanded = res;
		
		this->WhenPartiallyReady();
	});
}

void ScriptSolver::GetSuggestions2(int part_i, int sub_i, const Vector<const DynLine*>& lines, Event<> WhenPartiallyReady) {
	DynPart& part = script->parts[part_i];
	DynSub& sub = part.sub[sub_i];
	tmp_part = &part;
	tmp_sub = &sub;
	tmp_lines <<= lines;
	this->WhenPartiallyReady = WhenPartiallyReady;
	
	Script& song = *this->script;
	ScriptSolverArgs args; // 22
	args.fn = 22;
	args.lng_i = song.lng_i;
	
	
	NavigatorState line_state;
	for(int i = 0; i < lines.GetCount(); i++) {
		const DynLine& dl = *lines[i];
		if (dl.text.IsEmpty())
			break;
		args.phrases << dl.text;
		args.phrases2 << dl.expanded;
		
		auto& state = args.line_states.Add();
		ReadNavigatorState(song, part_i, sub_i, i, line_state,  2);
		CopyState(state, line_state);
		
		const auto& ents = GetTypeclassEntities(appmode, dl.safety, artist->is_female);
		state.style_type = ents.GetKey(dl.style_type);
		state.style_entity = ents[dl.style_type][dl.style_entity];
		state.safety = dl.safety;
		state.line_len = dl.line_len;
		state.connector = dl.connector;
		state.line_begin = dl.line_begin;
	}
	
	TaskMgr& m = AiTaskManager();
	m.GetScriptSolver(appmode, args, [this](String res) {
		Vector<String> lines = Split(res, "\n");
		
		for(int i = 0; i < lines.GetCount(); i++) {
			String& l = lines[i];
			l = TrimBoth(l);
			if (l.IsEmpty()) {
				lines.Remove(i--);
				continue;
			}
			if (!IsDigit(l[0]) && i > 0) {
				lines[i-1] << " / " << l;
				lines.Remove(i--);
				continue;
			}
		}
		
		for (String& s : lines) {
			if (IsDigit(s[0])) {
				int a = s.Find(".");
				if (a >= 0)
					s = TrimLeft(s.Mid(a+1));
			}
		}
		DUMPC(lines);
		
		for (const DynLine* l : tmp_lines) {
			DynLine& dl = const_cast<DynLine&>(*l);
			dl.suggs.Clear();
		}
		
		for (String& s : lines) {
			RemoveQuotes(s);
			Vector<String> parts = Split(s, "/");
			for(int i = 0; i < parts.GetCount() && i < tmp_lines.GetCount(); i++) {
				DynLine& dl = const_cast<DynLine&>(*tmp_lines[i]);
				dl.suggs << TrimBoth(parts[i]);
			}
			for(int i = parts.GetCount(); i < tmp_lines.GetCount(); i++) {
				DynLine& dl = const_cast<DynLine&>(*tmp_lines[i]);
				dl.suggs.Clear();
			}
		}
		
		this->WhenPartiallyReady();
	});
}

void ScriptSolver::GetStyleSuggestion(int part_i, int sub_i, const Vector<const DynLine*>& lines, Event<> WhenPartiallyReady) {
	DynPart& part = script->parts[part_i];
	DynSub& sub = part.sub[sub_i];
	tmp_part = &part;
	tmp_sub = &sub;
	tmp_lines <<= lines;
	this->WhenPartiallyReady = WhenPartiallyReady;
	
	Script& song = *this->script;
	ScriptSolverArgs args; // 23
	args.fn = 23;
	args.lng_i = song.lng_i;
	
	
	NavigatorState line_state;
	for(int i = 0; i < lines.GetCount(); i++) {
		const DynLine& dl = *lines[i];
		if (dl.text.IsEmpty())
			break;
		args.phrases << dl.text;
		args.phrases2 << dl.expanded;
		
		auto& state = args.line_states.Add();
		ReadNavigatorState(song, part_i, sub_i, i, line_state,  2);
		CopyState(state, line_state);
		
		const auto& ents = GetTypeclassEntities(appmode, dl.safety, artist->is_female);
		state.style_type = ents.GetKey(dl.style_type);
		state.style_entity = ents[dl.style_type][dl.style_entity];
		state.safety = dl.safety;
		state.line_len = dl.line_len;
		state.connector = dl.connector;
		state.line_begin = dl.line_begin;
		
		if (i == 0) {
			for(int i = 0; i < ents.GetCount(); i++) {
				args.styles.Add(ents.GetKey(i));
			}
		}
	}
	
	TaskMgr& m = AiTaskManager();
	m.GetScriptSolver(appmode, args, [this](String res) {
		Vector<String> lines = Split(res, "\n");
		
		res = TrimBoth(res);
		if (res.IsEmpty() || !IsDigit(res[0]))
			return;
		
		int type = ScanInt(res);
		
		for (const DynLine* l : tmp_lines) {
			DynLine& dl = const_cast<DynLine&>(*l);
			dl.style_type = type;
		}
		
		this->WhenPartiallyReady();
	});
}

void ScriptSolver::GetSubStory(int part_i, int sub_i, Event<> WhenPartiallyReady) {
	DynPart& part = script->parts[part_i];
	DynSub& sub = part.sub[sub_i];
	tmp_part = const_cast<DynPart*>(&part);
	tmp_sub = const_cast<DynSub*>(&sub);
	this->WhenPartiallyReady = WhenPartiallyReady;
	
	Script& song = *this->script;
	ScriptSolverArgs args; // 19
	args.fn = 19;
	args.lng_i = song.lng_i;
	
	
	// Get line phrases and properties
	NavigatorState sub_state, line_state;
	ReadNavigatorState(song, part_i, sub_i, -1, sub_state,  1);
	CopyState(args.state, sub_state);
	
	for(int i = 0; i < sub.lines.GetCount(); i++) {
		const DynLine& l = sub.lines[i];
		if (l.text.IsEmpty())
			break;
		
		ReadNavigatorState(song, part_i, sub_i, i, line_state,  2);
		line_state.RemoveDuplicate(sub_state);
		CopyState(args.line_states.Add(), line_state);
		
		args.phrases << l.text;
	}
	
	// Know the previous story
	for(int i = 0; i <= part_i; i++) {
		DynPart& p0 = script->parts[i];
		if (p0.story.GetCount()) {
			args.previously << p0.story << "\n";
			continue;
		}
		for(int j = 0; j < p0.sub.GetCount(); j++) {
			DynSub& s0 = p0.sub[j];
			if (&s0 == &sub)
				break;
			args.previously << s0.story << "\n";
			continue;
		}
	}
	
	// Peek upcoming lyrics
	bool write = false;
	for(int i = part_i; i < script->parts.GetCount(); i++) {
		DynPart& p0 = script->parts[i];
		for(int j = 0; j < p0.sub.GetCount(); j++) {
			DynSub& s0 = p0.sub[j];
			if (&s0 == &sub) {
				write = true;
				continue;
			}
			if (write) {
				for(int k = 0; k < s0.lines.GetCount(); k++) {
					DynLine& l0 = s0.lines[k];
					if (l0.text.GetCount())
						args.peek << l0.text << "\n";
				}
			}
		}
	}
	
	TaskMgr& m = AiTaskManager();
	m.GetScriptSolver(appmode, args, [this](String res) {
		res = TrimBoth(res);
		res.Replace("\r", "");
		res.Replace("\n\n", "\n");
		tmp_sub->story = res;
		this->WhenPartiallyReady();
	});
}

void ScriptSolver::GetPartStory(int part_i, Event<> WhenPartiallyReady) {
	DynPart& part = script->parts[part_i];
	tmp_part = const_cast<DynPart*>(&part);
	tmp_sub = 0;
	this->WhenPartiallyReady = WhenPartiallyReady;
	
	Script& song = *this->script;
	ScriptSolverArgs args; // 20
	args.fn = 20;
	args.lng_i = song.lng_i;
	
	
	// Get line phrases and properties
	NavigatorState part_state, sub_state;
	ReadNavigatorState(song, part_i, -1, -1, part_state,  0);
	CopyState(args.state, part_state);
	
	for(int i = 0; i < part.sub.GetCount(); i++) {
		const DynSub& ds = part.sub[i];
		if (ds.story.IsEmpty())
			continue;
		
		
		ReadNavigatorState(song, part_i, i, -1, sub_state,  1);
		sub_state.RemoveDuplicate(part_state);
		CopyState(args.line_states.Add(), sub_state);
		
		args.phrases << ds.story;
	}
	
	// Know the previous story
	for(int i = 0; i < part_i; i++) {
		DynPart& p0 = script->parts[i];
		if (p0.story.GetCount()) {
			args.previously << p0.story << "\n";
			continue;
		}
		for(int j = 0; j < p0.sub.GetCount(); j++) {
			DynSub& s0 = p0.sub[j];
			args.previously << s0.story << "\n";
			continue;
		}
	}
	
	// Peek upcoming lyrics
	for(int i = part_i+1; i < script->parts.GetCount(); i++) {
		DynPart& p0 = script->parts[i];
		for(int j = 0; j < p0.sub.GetCount(); j++) {
			DynSub& s0 = p0.sub[j];
			for(int k = 0; k < s0.lines.GetCount(); k++) {
				DynLine& l0 = s0.lines[k];
				if (l0.text.GetCount())
					args.peek << l0.text << "\n";
			}
		}
	}
	
	TaskMgr& m = AiTaskManager();
	m.GetScriptSolver(appmode, args, [this](String res) {
		res = TrimBoth(res);
		res.Replace("\r", "");
		res.Replace("\n\n", "\n");
		tmp_part->story = res;
		this->WhenPartiallyReady();
	});
}





void ReadNavigatorState(Script& s, int part_i, int sub_i, int line_i, NavigatorState& state, int depth_limit) {
	state.Clear();
	if (part_i < 0 || part_i >= s.parts.GetCount())
		return;
	
	DynPart& dp = s.parts[part_i];
	
	#define COPY(v)   if (state.v.IsEmpty()) state.v = el . v;
	#define COPY_I(v) if (state.v < 0) state.v = el . v;
	#define COPY_S(v) if (state.v == 0) state.v = el . v;
	LineElement* elp = 0;
	if (sub_i >= 0 && line_i >= 0 && depth_limit >= 2) {
		DynSub& ds = dp.sub[sub_i];
		DynLine& dl = ds.lines[line_i];
		auto& el = dl.el;
		COPY(element)
		COPY(attr.group)
		COPY(attr.value)
		COPY_I(clr_i)
		COPY(act.action)
		COPY(act.arg)
		COPY_I(typeclass_i)
		COPY_I(con_i)
		COPY_S(sorter)
		if (state.el == 0) {state.depth = 2; state.el = &el;}
	}
	if (sub_i >= 0 && depth_limit >= 1) {
		DynSub& ds = dp.sub[sub_i];
		auto& el = ds.el;
		COPY(element)
		COPY(attr.group)
		COPY(attr.value)
		COPY_I(clr_i)
		COPY(act.action)
		COPY(act.arg)
		COPY_I(typeclass_i)
		COPY_I(con_i)
		COPY_S(sorter)
		if (state.el == 0) {state.depth = 1; state.el = &el;}
	}
	if (depth_limit >= 0) {
		auto& el = dp.el;
		COPY(element)
		COPY(attr.group)
		COPY(attr.value)
		COPY_I(clr_i)
		COPY(act.action)
		COPY(act.arg)
		COPY_I(typeclass_i)
		COPY_I(con_i)
		COPY_S(sorter)
		if (state.el == 0) {state.depth = 0; state.el = &el;}
	}
	#undef COPY
	#undef COPY_I
	#undef COPY_S
}

void ScriptSolver::CopyState(ScriptSolverArgs::State& to, const NavigatorState& from) {
	#define COPY(x) to.x = from.x;
	COPY(element)
	COPY(attr)
	COPY(clr_i)
	COPY(act)
	to.typeclass = from.typeclass_i >= 0 ? GetTypeclasses(appmode)[from.typeclass_i] : String();
	const auto& cons = GetContents(appmode);
	int c0 = from.con_i / 3;
	int c1 = from.con_i % 3;
	to.content.Clear();
	to.content_mod.Clear();
	if (from.con_i >= 0 && c0 < cons.GetCount()) {
		to.content = cons[c0].key;
		to.content_mod = cons[c0].parts[c1];
	}
	#undef COPY
}


END_UPP_NAMESPACE
