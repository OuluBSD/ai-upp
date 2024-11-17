#include "TextCore.h"

NAMESPACE_UPP


int EditorPtrs::GetActiveEntityIndex() const {return VectorFindPtr(entity, TextDatabase::Single().entities);}
int EditorPtrs::GetActiveComponentIndex() const {if (!entity || !component) return -1; return VectorFindPtr(component, entity->comps);}
int EditorPtrs::GetActiveScriptIndex() const {if (!entity || !script) return -1; return VectorFindPtr(static_cast<Component*>(script), entity->comps);}



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

String SrcTextData::GetTokenTypeString(const TokenText& txt) const {
	String o;
	for(int tk_i : txt.tokens) {
		const Token& tk = this->tokens[tk_i];
		int w_i = tk.word_;
		if (w_i < 0) {
			String key = ToLower(this->tokens.GetKey(tk_i));
			w_i = this->words.Find(key);
			tk.word_ = w_i;
		}
		if (w_i < 0) {
			o << "{error}";
		}
		else {
			const ExportWord& ew = this->words[w_i];
			o << "{";
			for(int i = 0; i < ew.class_count; i++) {
				if (i) o << "|";
				int class_i = ew.classes[i];
				const String& wc = this->word_classes[class_i];
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

String SrcTextData::GetWordString(const Vector<int>& words) const {
	String o;
	for(int w_i : words) {
		if (w_i < 0) continue;
		const String& key = this->words.GetKey(w_i);
		
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

WString SrcTextData::GetWordPronounciation(const Vector<int>& words) const {
	WString o;
	for(int w_i : words) {
		if (w_i < 0) continue;
		const ExportWord& ew = this->words[w_i];
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

String SrcTextData::GetTypeString(const Vector<int>& word_classes) const {
	String o;
	for(int wc_i : word_classes) {
		if (wc_i < 0)
			o << "{error}";
		else {
			const String& wc = this->word_classes[wc_i];
			o << "{" << wc << "}";
		}
	}
	return o;
}

String SrcTextData::GetActionString(const Vector<int>& actions) const {
	String o;
	for(int act_i : actions) {
		if (!o.IsEmpty()) o << ", ";
		if (act_i < 0)
			o << "error";
		else {
			const ActionHeader& ah = this->actions.GetKey(act_i);
			o << ah.action;
			if (!ah.arg.IsEmpty())
				o << "(" << ah.arg << ")";
		}
	}
	return o;
}

String SrcTextData::GetScriptDump(int i) const {
	String s;
	const ScriptStruct& ss = this->scripts[i];
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
					const TokenText& tt = this->token_texts[tt_i];
					if (show_subsub) s.Cat('\t');
					s << "\t\t" << this->GetTokenTextString(tt) << "\n";
				}
			}
		}
	}
	return s;
}

VectorMap<int,int> SrcTextData::GetSortedElements() {
	VectorMap<int,int> vmap;
	for(const ScriptStruct& ss : this->scripts.GetValues()) {
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

VectorMap<int,int> SrcTextData::GetSortedElementsOfPhraseParts() {
	VectorMap<int,int> vmap;
	for (const auto& pp : this->phrase_parts.GetValues()) {
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
