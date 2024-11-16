#include "TextCore.h"

NAMESPACE_UPP


int EditorPtrs::GetActiveEntityIndex() const {return VectorFindPtr(entity, TextDatabase::Single().entities);}
int EditorPtrs::GetActiveComponentIndex() const {if (!entity || !component) return -1; return VectorFindPtr(component, entity->comps);}
int EditorPtrs::GetActiveScriptIndex() const {if (!entity || !script) return -1; return VectorFindPtr(static_cast<Component*>(script), entity->comps);}


DatasetAnalysis::DatasetAnalysis() {
	
}

void DatasetAnalysis::Load() {
	TextDatabase& db = TextDatabase::Single();
	SourceDataAnalysis& sda = db.a;
	
	TODO
	/*
	String dir =
		AppendFileName(MetaDatabase::Single().dir,
		MetaDatabase::Single().share + DIR_SEPS +
		GetAppModeDir() + DIR_SEPS +
		"data");
	
	RealizeDirectory(dir);
	
	String ds_dir = dir;
	RealizeDirectory(ds_dir);
	
	
	for(int i = 1; i < LNG_COUNT; i++) {
		String trans_ds_key = String(GetLanguageKey(i)).Left(2);
		translations[i].Load(ds_dir, trans_ds_key);
		phrase_translations[i].Load(ds_dir, "phrase_trans_" + trans_ds_key);
	}
	
	scripts.Load(ds_dir, "scripts");
	tokens.Load(ds_dir, "tokens");
	token_texts.Load(ds_dir, "tokenized texts");
	word_classes.Load(ds_dir, "word classes");
	words.Load(ds_dir, "words");
	ambiguous_word_pairs.Load(ds_dir, "ambiguous word pairs");
	virtual_phrases.Load(ds_dir, "virtual phrases");
	virtual_phrase_parts.Load(ds_dir, "virtual phrase parts");
	virtual_phrase_structs.Load(ds_dir, "virtual phrase structure types");
	struct_part_types.Load(ds_dir, "structure part types");
	struct_types.Load(ds_dir, "structure types");
	phrase_parts.Load(ds_dir, "phrase parts");
	attrs.Load(ds_dir, "attributes");
	actions.Load(ds_dir, "action");
	parallel.Load(ds_dir, "action parallel");
	trans.Load(ds_dir, "action transition");
	action_phrases.Load(ds_dir, "action phrases");
	wordnets.Load(ds_dir, "wordnets");
	diagnostics.Load(ds_dir, "diagnostics");
	simple_attrs.Load(ds_dir, "simple_attrs");
	element_keys.Load(ds_dir, "element_keys");
	
	
	String comp_dir = AppendFileName(ds_dir, GetAppModeKeyN(AM_COMPONENT));
	RealizeDirectory(comp_dir);
	
	components.Clear();
	FindFile ff(AppendFileName(comp_dir, "*"));
	do {
		if (!ff.IsDirectory()) continue;
		String title = ff.GetName();
		if (title.Find(".") == 0) continue;
		
		ComponentAnalysis& sa = components.Add(title);
		sa.da = this;
		sa.Load(ff.GetPath());
	}
	while (ff.Next());*/
}

// TODO remove?
#if 0
ComponentAnalysis& DatasetAnalysis::GetComponentAnalysis(const String& name) {
	TextDatabase& db = TextDatabase::Single();
	SourceDataAnalysis& sda = db.a;
	
	// TODO dir
	String dir; TODO/* =
		AppendFileName(TextDatabase::Single().dir,
		TextDatabase::Single().share + DIR_SEPS +
		GetAppModeDir() + DIR_SEPS +
		"data");*/
	
	RealizeDirectory(dir);
	
	String comp_dir = AppendFileName(dir, "components" DIR_SEPS + name);
	RealizeDirectory(comp_dir);
	
	ComponentAnalysis& sa = components.GetAdd(name);
	
	if (sa.da == 0) {
		sa.da = this;
		sa.Load(comp_dir);
	}
	
	return sa;
}
#endif

String SrcTextData::GetTokenTextString(const TokenText& txt) const {
	String o;
	for(int tk_i : txt.tokens) {
		//const Token& tk = tokens[tk_i];
		const String& key = tokens.GetKey(tk_i);
		
		if (key.GetCount() == 1 && NaturalTokenizer::IsToken(key[0])) {
			o << key;
		}
		else {
			if (!o.IsEmpty())
				o << " ";
			o << key;
		}
	}
	return o;
}

String DatasetPtrs::GetTokenTypeString(const TokenText& txt) const {
	ASSERT(src);
	const SrcTextData& src = *this->src;
	auto& da = *this->da;
	String o;
	for(int tk_i : txt.tokens) {
		const Token& tk = src.tokens[tk_i];
		int w_i = tk.word_;
		if (w_i < 0) {
			String key = ToLower(src.tokens.GetKey(tk_i));
			w_i = da.words.Find(key);
			tk.word_ = w_i;
		}
		if (w_i < 0) {
			o << "{error}";
		}
		else {
			const ExportWord& ew = da.words[w_i];
			o << "{";
			for(int i = 0; i < ew.class_count; i++) {
				if (i) o << "|";
				int class_i = ew.classes[i];
				const String& wc = da.word_classes[class_i];
				o << wc;
			}
			o << "}";
			/*if (key.GetCount() == 1 && NaturalTokenizer::IsToken(key[0])) {
				o << key;
			}
			else {
				if (!o.IsEmpty())
					o << " ";
				o << key;
			}*/
		}
	}
	return o;
}

String DatasetPtrs::GetWordString(const Vector<int>& words) const {
	ASSERT(src);
	const SrcTextData& src = *this->src;
	auto& da = *this->da;
	String o;
	for(int w_i : words) {
		if (w_i < 0) continue;
		const String& key = da.words.GetKey(w_i);
		
		if (key.GetCount() == 1 && NaturalTokenizer::IsToken(key[0])) {
			o << key;
		}
		else {
			if (!o.IsEmpty())
				o << " ";
			o << key;
		}
	}
	return o;
}

WString DatasetPtrs::GetWordPronounciation(const Vector<int>& words) const {
	ASSERT(src);
	const SrcTextData& src = *this->src;
	auto& da = *this->da;
	WString o;
	for(int w_i : words) {
		if (w_i < 0) continue;
		const ExportWord& ew = da.words[w_i];
		const WString& key = ew.phonetic;
		
		if (key.GetCount() == 1 && NaturalTokenizer::IsToken(key[0])) {
			o << key;
		}
		else {
			if (!o.IsEmpty())
				o << " ";
			o << key;
		}
	}
	return o;
}

String DatasetPtrs::GetTypeString(const Vector<int>& word_classes) const {
	ASSERT(src);
	const SrcTextData& src = *this->src;
	auto& da = *this->da;
	String o;
	for(int wc_i : word_classes) {
		if (wc_i < 0)
			o << "{error}";
		else {
			const String& wc = da.word_classes[wc_i];
			o << "{" << wc << "}";
		}
	}
	return o;
}

String DatasetPtrs::GetActionString(const Vector<int>& actions) const {
	ASSERT(src);
	const SrcTextData& src = *this->src;
	auto& da = *this->da;
	String o;
	for(int act_i : actions) {
		if (!o.IsEmpty()) o << ", ";
		if (act_i < 0)
			o << "error";
		else {
			const ActionHeader& ah = da.actions.GetKey(act_i);
			o << ah.action;
			if (!ah.arg.IsEmpty())
				o << "(" << ah.arg << ")";
		}
	}
	return o;
}

String DatasetPtrs::GetScriptDump(int i) const {
	ASSERT(src);
	const SrcTextData& src = *this->src;
	String s;
	const ScriptStruct& ss = src.scripts[i];
	for(int i = 0; i < ss.parts.GetCount(); i++) {
		const auto& part = ss.parts[i];
		//if (s.GetCount()) s << "\n";
		s << Format("[%d: %s]\n", i, GetTextTypeString(part.type) + " " + IntStr(part.num+1));
		
		for(int j = 0; j < part.sub.GetCount(); j++) {
			const auto& sub = part.sub[j];
			//if (s.GetCount()) s << "\n";
			s << Format("\t[%d.%d: repeat %.2!m]\n", i,j, sub.repeat);
			
			bool show_subsub = sub.sub.GetCount() > 1;
			for(int k = 0; k < sub.sub.GetCount(); k++) {
				const auto& ssub = sub.sub[k];
				if (show_subsub)
					s << Format("\t\t[%d.%d.%d]\n", i,j,k);
				for(int l = 0; l < ssub.token_texts.GetCount(); l++) {
					int tt_i = ssub.token_texts[l];
					if (tt_i < 0) continue;
					const TokenText& tt = src.token_texts[tt_i];
					if (show_subsub) s.Cat('\t');
					s << "\t\t" << src.GetTokenTextString(tt) << "\n";
				}
			}
		}
	}
	return s;
}

VectorMap<int,int> DatasetPtrs::GetSortedElements() {
	ASSERT(src);
	const SrcTextData& src = *this->src;
	auto& da = *this->da;
	VectorMap<int,int> vmap;
	for(const ScriptStruct& ss : src.scripts.GetValues()) {
		for(const auto& part : ss.parts) {
			if (part.cls >= 0)
				vmap.GetAdd(part.cls,0)++;
			for(const auto& sub : part.sub) {
				if (sub.cls >= 0)
					vmap.GetAdd(sub.cls,0)++;
				for(const auto& ssub : sub.sub) {
					if (sub.cls >= 0)
						vmap.GetAdd(sub.cls,0)++;
				}
			}
		}
	}
	SortByValue(vmap, StdGreater<int>());
	return vmap;
}

VectorMap<int,int> DatasetPtrs::GetSortedElementsOfPhraseParts() {
	ASSERT(src);
	const SrcTextData& src = *this->src;
	auto& da = *this->da;
	VectorMap<int,int> vmap;
	for (const auto& pp : da.phrase_parts.GetValues()) {
		if (pp.el_i >= 0)
			vmap.GetAdd(pp.el_i,0)++;
	}
	SortByValue(vmap, StdGreater<int>());
	return vmap;
}

ArrayMap<String, DatasetField>& DatasetIndex() {
	static ArrayMap<String, DatasetField> map;
	return map;
}

END_UPP_NAMESPACE
